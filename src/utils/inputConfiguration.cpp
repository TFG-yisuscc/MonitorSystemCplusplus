//
// Created by yisus on 23/3/26.
//

#include "utils/inputConfiguration.h"

#include <stdexcept>

#include "clients/LlamaTest.h"
#include "clients/OllamaTest.h"

InputConfiguration::InputConfiguration(nlohmann::json json_config) {
    try { //TODO revisar los parametros
        inferenceEngine_ = json_config.at("inference_engine").get<InferenceEngines>();
        testType_ = json_config.at("test_type").get<TestType>();
        batch_size_ = json_config.at("batch_size").get<int>();
        context_size_ = json_config.at("context_size").get<int>();
        seed_ = json_config.at("seed").get<int>();
        num_prompts_ = json_config.at("num_prompts").get<int>();
        temperature_ = json_config.at("temperature").get<float>();
        model_path_or_name_ = json_config.at("model_path_or_name").get<std::string>();
        anotations = json_config.value("anotations", "EMPTY");
        ollama_url_ = json_config.value("ollama_url", "http://localhost:11434");
        og_config_json = json_config.dump();
        hardwarePeriod = json_config.at("hardware_period").get<float>();
    } catch (const nlohmann::json::exception& e) {
        throw std::invalid_argument(std::string("JSON mal estructurado") + e.what());
    }
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
        default:
            throw std::invalid_argument("Invalid inference engine selected.");
    }
    //fin cronometro
    this-> timestamp_run_end = std::chrono::system_clock::now().time_since_epoch().count();
    // creamos el resumen y lo metemos en la carpeta con el resto de resumenes
    createResumen();
}
void InputConfiguration::runOllama() {
    ollama::setServerURL(ollama_url_);
    ollama::setConnectionTimeout(600);
    ollama::setReadTimeout(600);
    ollama::setWriteTimeout(600);
    ollama::show_requests(true);
    ollama::show_replies(true);

    OllamaTest ollamaTest(model_path_or_name_,run_path_, temperature_, batch_size_, context_size_, seed_, num_prompts_);
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
    LlamaTest llamaTest(run_path_,model_path_or_name_, temperature_, batch_size_, context_size_, seed_, num_prompts_);
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
    resumen["timestamp_run_start"] = timestamp_run_start;
    resumen["timestamp_run_end"] = timestamp_run_end;
    resumen["anotations"] = anotations;
    resumen["og_config_json"] = og_config_json;

    std::string resumen_filepath = run_path_ + "/resumen.json";
    std::ofstream file(resumen_filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file to write the summary.");
    }
    file << resumen.dump(4);
    file.flush();
    file.close();
}

