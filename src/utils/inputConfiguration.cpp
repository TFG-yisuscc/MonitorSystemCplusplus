//
// Created by yisus on 23/3/26.
//

#include "utils/inputConfiguration.h"

#include <stdexcept>

#include "clients/LlamaTest.h"
#include "clients/OllamaTest.h"
#include "clients/haoiloOllamaTest.h"

void InputConfiguration::validate() const {
    if (temperature_ < 0.0f)
        throw std::invalid_argument("temperature no puede ser negativa (valor: " + std::to_string(temperature_) + ")");
    if (batch_size_ < 0)
        throw std::invalid_argument("batch_size no puede ser negativo (valor: " + std::to_string(batch_size_) + ")");
    if (context_size_ < 0)
        throw std::invalid_argument("context_size no puede ser negativo (valor: " + std::to_string(context_size_) + ")");
    if (hardwarePeriod <= 0.0f)
        throw std::invalid_argument("hardware_period debe ser mayor que cero (valor: " + std::to_string(hardwarePeriod) + ")");
    if (num_prompts_ < 1 || num_prompts_ > 541)
        throw std::invalid_argument("num_prompts debe estar entre 1 y 541 (valor: " + std::to_string(num_prompts_) + ")");
    if (inferenceEngine_ == InferenceEngines::OTHER)
        throw std::invalid_argument("El motor de inferencia OTHER no esta soportado");
    if (inferenceEngine_ == InferenceEngines::HAILO_OLLAMA && hailo_server_port_ <= 0)
        throw std::invalid_argument("hailo_server_port debe ser mayor que cero");
}

InputConfiguration::InputConfiguration(nlohmann::json json_config) {
    try {
        inferenceEngine_ = json_config.at("inference_engine").get<InferenceEngines>();
        testType_ = json_config.at("test_type").get<TestType>();
        batch_size_ = json_config.at("batch_size").get<int>();
        context_size_ = json_config.at("context_size").get<int>();
        seed_ = json_config.at("seed").get<int>();
        num_prompts_ = json_config.at("num_prompts").get<int>();
        temperature_ = json_config.at("temperature").get<float>();
        model_path_or_name_ = json_config.at("model_path_or_name").get<std::string>();
        if (json_config.contains("annotations")) {
            const auto& a = json_config["annotations"];
            annotations = a.is_string() ? a.get<std::string>() : a.dump();
        } else if (json_config.contains("anotations")) {
            const auto& a = json_config["anotations"];
            annotations = a.is_string() ? a.get<std::string>() : a.dump();
        } else {
            annotations = "EMPTY";
        }
        ollama_url_ = json_config.value("ollama_url", "http://localhost:11434");
        hailo_server_host_ = json_config.value("hailo_server_host", std::string("localhost"));
        hailo_server_port_ = json_config.value("hailo_server_port", 8000);
        og_config_json = json_config.dump();
        hardwarePeriod = json_config.at("hardware_period").get<float>();
    } catch (const nlohmann::json::exception& e) {
        throw std::invalid_argument(std::string("JSON mal estructurado") + e.what());
    }
    validate();
}

void InputConfiguration::run() {
    // comienzo de timestamp es otrientativa
    this->timestamp_run_start = std::chrono::system_clock::now().time_since_epoch().count();
    // configuración de la carpeta donde se guardaran los test (results)
    // 1 nos vamos a la carpeta result y si no existe la creamos
    std::filesystem::path results_dir = "results";
    std::filesystem::create_directories(results_dir);
    //2 lo mismo creamos una carpeta con una marca especifica  donde se gardan todos los resultados
    std::filesystem::path run_dir = results_dir / std::to_string(timestamp_run_start);
    std::filesystem::create_directories(run_dir);
    //y los filepaths
    this->run_path_= run_dir.string();

    switch (inferenceEngine_) {
        case InferenceEngines::OLLAMA:
            runOllama();
            break;
        case InferenceEngines::LLAMA:
            runLlama();
            break;
        case InferenceEngines::HAILO_OLLAMA:
            runHailoOllama();
            break;
        default:
            throw std::invalid_argument("Invalid inference engine selected.");
    }
    //fin cronometro
    this-> timestamp_run_end = std::chrono::system_clock::now().time_since_epoch().count();
    // creamos el resumen y lo metemos en la carpeta con el resto de resumenes
    createResumen();
}
static nlohmann::json extractOllamaModelInfo(const nlohmann::json& show_response) {
    nlohmann::json info = nlohmann::json::object();
    std::string arch = "unknown";

    // Ollama <0.3 uses "modelinfo", >=0.3 uses "model_info"
    nlohmann::json mi = nlohmann::json::object();
    if (show_response.contains("modelinfo"))
        mi = show_response["modelinfo"];
    else if (show_response.contains("model_info"))
        mi = show_response["model_info"];

    if (!mi.empty()) {
        if (mi.contains("general.architecture") && mi["general.architecture"].is_string())
            arch = mi["general.architecture"].get<std::string>();
        info["architecture"] = arch;

        if (mi.contains("general.parameter_count"))
            info["n_params"] = mi["general.parameter_count"];

        auto try_key = [&](const std::string& key, const std::string& field) {
            if (mi.contains(key)) info[field] = mi[key];
        };
        try_key(arch + ".embedding_length",        "embedding_length");
        try_key(arch + ".block_count",             "n_layers");
        try_key(arch + ".context_length",          "max_context");
        try_key(arch + ".attention.head_count",    "n_heads");
        try_key(arch + ".attention.head_count_kv", "n_kv_heads");
    } else {
        info["architecture"] = arch;
    }

    if (show_response.contains("details")) {
        const auto& det = show_response["details"];
        if (det.contains("quantization_level"))
            info["quantization"] = det["quantization_level"];
        else if (det.contains("quantization"))
            info["quantization"] = det["quantization"];
        else
            info["quantization"] = "unknown";
    } else {
        info["quantization"] = "unknown";
    }

    // bits_per_weight: file_size_bytes * 8 / n_params
    if (info.contains("n_params") && info["n_params"].is_number()) {
        auto n_params = info["n_params"].get<int64_t>();
        int64_t file_size = 0;
        if (show_response.contains("size") && show_response["size"].is_number())
            file_size = show_response["size"].get<int64_t>();
        else if (mi.contains("general.file_size") && mi["general.file_size"].is_number())
            file_size = mi["general.file_size"].get<int64_t>();

        info["bits_per_weight"] = (n_params > 0 && file_size > 0)
            ? nlohmann::json((file_size * 8.0) / n_params)
            : nlohmann::json(nullptr);
    } else {
        info["bits_per_weight"] = nullptr;
    }

    return info;
}

static nlohmann::json fetchLlamaModelInfo(const std::string& model_path) {
    llama_backend_init();
    llama_model_params params = llama_model_default_params();
    params.n_gpu_layers = 0;
    llama_model* model = llama_model_load_from_file(model_path.c_str(), params);
    if (!model) {
        std::cerr << "Warning: could not load model for metadata extraction: " << model_path << "\n";
        return nullptr;
    }

    nlohmann::json info = nlohmann::json::object();
    try {
        char buf[512];

        info["n_params"] = (int64_t)llama_model_n_params(model);

        std::string arch = "unknown";
        if (llama_model_meta_val_str(model, "general.architecture", buf, sizeof(buf)) >= 0)
            arch = std::string(buf);
        info["architecture"] = arch;

        auto try_int_meta = [&](const std::string& key, const std::string& field) {
            if (llama_model_meta_val_str(model, key.c_str(), buf, sizeof(buf)) > 0) {
                try { info[field] = (int64_t)std::stoll(buf); } catch (...) {}
            }
        };
        try_int_meta(arch + ".embedding_length",          "embedding_length");
        try_int_meta(arch + ".block_count",               "n_layers");
        try_int_meta(arch + ".context_length",            "max_context");
        try_int_meta(arch + ".attention.head_count",      "n_heads");
        try_int_meta(arch + ".attention.head_count_kv",   "n_kv_heads");

        // bits_per_weight: file_size_bytes * 8 / n_params
        {
            auto n_params = info.value("n_params", (int64_t)0);
            int64_t file_size = 0;
            try {
                std::ifstream fs(model_path, std::ios::binary | std::ios::ate);
                if (fs.is_open()) file_size = (int64_t)fs.tellg();
            } catch (...) {}
            info["bits_per_weight"] = (n_params > 0 && file_size > 0)
                ? nlohmann::json((file_size * 8.0) / n_params)
                : nlohmann::json(nullptr);
        }

        // Quantization from filename (convention: stem ends in .Q4_0 etc.)
        auto stem = std::filesystem::path(model_path).stem().string();
        auto dot  = stem.rfind('.');
        if (dot != std::string::npos) {
            info["quantization"] = stem.substr(dot + 1);
        } else {
            auto dash = stem.rfind('-');
            if (dash != std::string::npos) {
                auto suf = stem.substr(dash + 1);
                if (!suf.empty() && (suf[0] == 'Q' || suf[0] == 'F' || suf[0] == 'I'))
                    info["quantization"] = suf;
                else
                    info["quantization"] = "unknown";
            } else {
                info["quantization"] = "unknown";
            }
        }
    } catch (...) {}

    llama_model_free(model);
    return info;
}

void InputConfiguration::runHailoOllama() {
    model_info_ = nullptr; // hailo no expone metadata estructurada via /api/show
    HaoiloOllamaTest hailoTest(model_path_or_name_, run_path_, temperature_,
                               seed_, num_prompts_, /*num_predict=*/512,
                               hardwarePeriod, hailo_server_host_, hailo_server_port_);
    switch (testType_) {
        case TestType::TYPE_0:
            hailoTest.runTestType0();
            break;
        case TestType::TYPE_1:
            hailoTest.runTestType1();
            break;
        case TestType::TYPE_2:
            hailoTest.runTestType1_5seg();
            break;
        default:
            throw std::invalid_argument("Invalid test type selected for HAILO_OLLAMA.");
    }
}

void InputConfiguration::runOllama() {
    try {
        ollama::setServerURL(ollama_url_);
        // verbose=false: evita que Ollama incluya template/licencia con chars mal escapados
        auto show_resp = ollama::show_model_info(model_path_or_name_, false);
        model_info_ = extractOllamaModelInfo(show_resp);
    } catch (const std::exception& e) {
        std::cerr << "Warning: could not fetch Ollama model info: " << e.what() << "\n";
        model_info_ = nullptr;
    } catch (...) {
        std::cerr << "Warning: could not fetch Ollama model info\n";
        model_info_ = nullptr;
    }
    OllamaTest ollamaTest(model_path_or_name_, run_path_, temperature_, batch_size_, context_size_, seed_, num_prompts_, hardwarePeriod);
    switch (testType_) {
        case TestType::TYPE_0:
            // Implementación del test para OLLAMA tipo 0
            ollamaTest.runTestType0();
            break;
        case TestType::TYPE_1:
            // Implementación del test para OLLAMA tipo 1
            ollamaTest.runTestType1();
            break;
        case TestType::TYPE_2:
            // Implementación del test para OLLAMA tipo 2
            ollamaTest.runTestType1_5seg();
            break;
        default:
            throw std::invalid_argument("Invalid test type selected for OLLAMA.");
    }
}
void InputConfiguration::runLlama() {
    try {
        model_info_ = fetchLlamaModelInfo(model_path_or_name_);
    } catch (const std::exception& e) {
        std::cerr << "Warning: could not fetch LLAMA model info: " << e.what() << "\n";
        model_info_ = nullptr;
    } catch (...) {
        std::cerr << "Warning: could not fetch LLAMA model info\n";
        model_info_ = nullptr;
    }
    LlamaTest llamaTest(run_path_, model_path_or_name_, temperature_, batch_size_, context_size_, seed_, num_prompts_, hardwarePeriod);
    switch (testType_) {
        case TestType::TYPE_0:
           llamaTest.runTestType0();
            break;
        case TestType::TYPE_1:
            // Implementación del test para LLAMA tipo 1
            llamaTest.runTestType1();
            break;
        case TestType::TYPE_2:
            // Implementación del test para LLAMA tipo 2
            llamaTest.runTestType1_5seg();
            break;
        default:
            throw std::invalid_argument("Invalid test type selected for LLAMA.");
    }
}

void InputConfiguration::createResumen() {
    nlohmann::json resumen;
    resumen["inference_engine"] = inferenceEngine_;
    resumen["test_type"] = testType_;
    resumen["batch_size"] = batch_size_;
    resumen["context_size"] = context_size_;
    resumen["seed"] = seed_;
    resumen["num_prompts"] = num_prompts_;
    resumen["temperature"] = temperature_;
    resumen["model_path_or_name"] = model_path_or_name_;
    resumen["hardware_period"] = hardwarePeriod;
    if (inferenceEngine_ == InferenceEngines::HAILO_OLLAMA) {
        resumen["hailo_server_host"] = hailo_server_host_;
        resumen["hailo_server_port"] = hailo_server_port_;
    }
    resumen["timestamp_run_start"] = timestamp_run_start;
    resumen["timestamp_run_end"] = timestamp_run_end;

    // Parse annotations as native JSON object and inject model_info
    nlohmann::json ann_json;
    try {
        ann_json = nlohmann::json::parse(annotations);
        if (!ann_json.is_object()) ann_json = nlohmann::json::object();
    } catch (...) {
        ann_json = nlohmann::json::object();
        if (annotations != "EMPTY" && !annotations.empty())
            ann_json["text"] = annotations;
    }
    ann_json["model_info"] = model_info_;
    resumen["annotations"] = ann_json;

    resumen["og_config_json"] = og_config_json;

    std::string resumen_filepath = run_path_ + "/resumen.json";
    std::ofstream file(resumen_filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file to write the summary.");
    }
    file.exceptions(std::ios::failbit | std::ios::badbit);
    file << resumen.dump(4);
    file.flush();
    file.close();
}

