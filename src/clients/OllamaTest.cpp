//
// Created by yisus on 5/3/26.
//


#include "clients/OllamaTest.h"
#include <chrono>
#include <iostream>
#include <fmt/format.h>
#include "metrics/Logger.h"
#include "utils/hardwareMeasurements.h"


// constructores
OllamaTest::OllamaTest(std::string model_name, std::string filepath, float temperature, int batch_size, int context_size, int seed, int num_prompts, double hardwarePeriod)
{
    test_id = std::chrono::system_clock::now().time_since_epoch().count();
    model_name_ = model_name;
    filepath_ = filepath;
    temperature_ = temperature;
    batch_size_ = batch_size;
    context_size_ = context_size;
    seed_ = seed;
    num_prompts_ = num_prompts;
    hardwarePeriod_ = hardwarePeriod;
}

OllamaTest::OllamaTest(nlohmann::json testConfig) {
    test_id = std::chrono::system_clock::now().time_since_epoch().count();
    model_name_ = testConfig.contains("model_name")
                      ? testConfig["model_name"].get<std::string>()
                      : throw std::runtime_error("model_name is required in testConfig");
    temperature_ = testConfig.contains("temperature")
                       ? testConfig["temperature"].get<int>()
                       : throw std::runtime_error("temperature is required in testConfig");
    batch_size_ =  testConfig.contains("batch_size")? testConfig["batch_size"].get<int>() : throw std::runtime_error("batch_size is required in testConfig");
    context_size_ = testConfig.contains("context_size")? testConfig["context_size"].get<int>() : throw std::runtime_error("context_size is required in testConfig");
    seed_ = testConfig.contains("seed")? testConfig["seed"].get<int>() : throw std::runtime_error("seed is required in testConfig");
    num_prompts_ = testConfig.contains("num_prompts")? testConfig["num_prompts"].get<int>() : throw std::runtime_error("num_prompts is required in testConfig");

}


// destructores



void OllamaTest::ensureModelAvailable() {
    auto models = ollama::list_models();
    bool found = false;
    for (const auto& m : models) {
        if (m == model_name_) { found = true; break; }
        // si el usuario no especificó tag, comparamos con ":latest"
        if (model_name_.find(':') == std::string::npos && m == model_name_ + ":latest") {
            found = true; break;
        }
    }
    if (!found) {
        fmt::print("Modelo '{}' no encontrado localmente, descargando...\n", model_name_);
        if (!ollama::pull_model(model_name_))
            throw std::runtime_error("No se pudo descargar el modelo: " + model_name_);
        fmt::print("Modelo '{}' descargado correctamente.\n", model_name_);
    }
}

// funciones de test
bool OllamaTest::runTestType0() {
    ollama::setConnectionTimeout(3600);
    ollama::setReadTimeout(3600);
    ollama::setWriteTimeout(3600);
    ensureModelAvailable();
    ollama::show_requests(true);
    ollama::show_replies(true);
    // obtenemos los prompts
    promptParser parser2 = promptParser();
    std::vector<std::string> prompts = parser2.getPrompts();
    //cramops el lloger de prompts
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_test1.jsonl",test_id,model_name_);
    Logger promptLogger(log_prompt_file);
    for (int i = 0; i < num_prompts_; ++i) {
        std::string prompt = prompts.at(i);
        // creamos la request
        ollama::request req = create_request( prompt);
        int64_t tinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        ollama::response response = ollama::generate(req);
        int64_t tfinal = std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        promptLogger.write2jsonline(metrics::promptmetrics::from_Ollama_json(response.as_json(), tinicio, tfinal, i));

    }
    // iteramos sobre los prompts
    ollamaClose();
    return true;
}

bool OllamaTest::runTestType1() {
    ollama::setConnectionTimeout(3600);
    ollama::setReadTimeout(3600);
    ollama::setWriteTimeout(3600);
    ensureModelAvailable();
    promptParser parser2 = promptParser();
    std::vector<std::string> prompts = parser2.getPrompts();
    //cramops el lloger de prompts
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_test1.jsonl",test_id,model_name_);
    Logger promptLogger(log_prompt_file);
    std::string log_hw_file = filepath_ + fmt::format("/{}_hw_metrics_{}_test1.jsonl", test_id, model_name_);
    HardwareMeasurements hwMonitor(log_hw_file, hardwarePeriod_);
    std::thread hwThread([&hwMonitor]() {
        try { hwMonitor.start(); }
        catch (const std::exception& e) { std::cerr << "HW monitor error: " << e.what() << std::endl; }
        catch (...) { std::cerr << "HW monitor: unknown error" << std::endl; }
    });
    try {
        for (int i = 0; i < num_prompts_ && i < (int)prompts.size(); i++) {
            ollama::request req = create_request(prompts.at(i));
            int64_t tinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            ollama::response response = ollama::generate(req);
            int64_t tfinal = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            auto pm = metrics::promptmetrics::from_Ollama_json(response.as_json(), tinicio, tfinal, i);
            promptLogger.write2jsonline(pm);
        }
        ollamaClose();
    } catch (...) {
        hwMonitor.stop();
        hwThread.join();
        throw;
    }
    hwMonitor.stop();
    hwThread.join();
    return true;
}
bool OllamaTest::runTestType1_5seg() {
    ollama::setConnectionTimeout(3600);
    ollama::setReadTimeout(3600);
    ollama::setWriteTimeout(3600);
    ensureModelAvailable();
    promptParser parser2 = promptParser();
    std::vector<std::string> prompts = parser2.getPrompts();
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_test1.jsonl",test_id,model_name_);
    Logger promptLogger(log_prompt_file);
    std::string log_hw_file = filepath_ + fmt::format("/{}_hw_metrics_{}_test1.jsonl", test_id, model_name_);
    HardwareMeasurements hwMonitor(log_hw_file, hardwarePeriod_);
    std::thread hwThread([&hwMonitor]() {
        try { hwMonitor.start(); }
        catch (const std::exception& e) { std::cerr << "HW monitor error: " << e.what() << std::endl; }
        catch (...) { std::cerr << "HW monitor: unknown error" << std::endl; }
    });
    try {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        for (int i = 0; i < num_prompts_ && i < (int)prompts.size(); i++) {
            ollama::request req = create_request(prompts.at(i));
            int64_t tinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
          //  ollama::response response = ollama::generate(req);
                ollama::response response =  ollama::generate(req);
            int64_t tfinal = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            auto pm = metrics::promptmetrics::from_Ollama_json(response.as_json(), tinicio, tfinal, i);
            promptLogger.write2jsonline(pm);
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        ollamaClose();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    } catch (...) {
        hwMonitor.stop();
        hwThread.join();
        throw;
    }
    hwMonitor.stop();
    hwThread.join();
    return true;
}


// funciones auxiliares
ollama::request OllamaTest::create_request( const std::string& prompt)
{
    nlohmann::json options;
    /*¡¡¡ATENCION!!!!!
     * Revisa bien si ollama hace caso a los parametros especificados o los ignora
     * Que ollama tiene fama de ignorar parametros que se le especifican.
     * OJO esta matniene el keepalive en memoria de forma permanente
     */
    options["options"]["temperature"] = temperature_;
    options["options"]["num_ctx"] = context_size_;
    options["options"]["num_batch"] = batch_size_;
    options["options"]["seed"] = seed_;
  

    ollama::request req(model_name_, prompt, options,false);
    req["logprobs"] = true;
    req["verbose"]  = true;
    req["keep_alive"] = -1;
    return req;
}
bool OllamaTest::ollamaClose() {
    nlohmann::json options;
    options["options"]["temperature"] = temperature_;
    options["options"]["num_ctx"] = context_size_;
    options["options"]["num_batch"] = batch_size_;
    options["options"]["seed"] = seed_;
    ollama::request req(model_name_, "", options, false);
    req["keep_alive"] = 0;
    ollama::generate(req);
    return true;
}