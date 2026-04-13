//
// Created by yisus on 5/3/26.
//


#include "clients/OllamaTest.h"
#include <fmt/format.h>
#include "metrics/Logger.h"
#include "utils/hardwareMeasurements.h"


// constructores
OllamaTest::OllamaTest(std::string model_name,std::string filepath,float temperature, int batch_size, int context_size, int seed, int num_prompts)
{
    model_name_ = model_name;
    filepath_ = filepath;
    temperature_ = temperature;
    batch_size_ = batch_size;
    context_size_ = context_size;
    seed_ = seed;
    num_prompts_ = num_prompts;
}

OllamaTest::OllamaTest(nlohmann::json testConfig) {
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



// funciones de test
bool OllamaTest::runTestType0() {
    // obtenemos los prompts
    promptParser parser2 = promptParser("../prompt_list/instruction_following_eval_promt.jsonl");
    std::vector<std::string> prompts = parser2.getPrompts();
    //cramops el lloger de prompts
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_test1.jsonl",test_id,model_name_);
    Logger promptLogger(log_prompt_file);
    // iteramos sobre los prompts
    for (int i = 0; i < num_prompts_ && i < prompts.size(); i++) {
        std::string prompt = prompts.at(i);
        // creamos la request
        ollama::request req = create_request( prompt);
        // enviamos la request y obtenemos la respuesta
        int64_t tinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        ollama::response response =  ollama::generate(req);
        int64_t tfinal = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        // LO  CNVETRIMOS EN UN PROMPTETRICS Y LO EJECUTAMOS
        auto pm = metrics::promptmetrics::from_Ollama_json(response.as_json(), tinicio, tfinal, i);
        promptLogger.write2jsonline(pm);
        //pm.write2jsonline(filepath);

    }
    ollamaClose();
    return true;
}

bool OllamaTest::runTestType1() {
    // obtenemos los prompts
    promptParser parser2 = promptParser("../prompt_list/instruction_following_eval_promt.jsonl");
    std::vector<std::string> prompts = parser2.getPrompts();
    //cramops el lloger de prompts
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_test1.jsonl",test_id,model_name_);
    Logger promptLogger(log_prompt_file);
    std::string log_hw_file = filepath_ + fmt::format("/{}_hw_metrics_{}_test1.jsonl", test_id, model_name_);
    HardwareMeasurements hwMonitor(log_hw_file, 1.0); // muestrea cada 1 segundo
    std::thread hwThread([&hwMonitor]() {
        hwMonitor.start(); // bloquea internamente hasta que se llame stop()
    });
    // iteramos sobre los prompts
    for (int i = 0; i < num_prompts_ && i < prompts.size(); i++) {
        std::string prompt = prompts.at(i);
        // creamos la request
        ollama::request req = create_request( prompt);
        // enviamos la request y obtenemos la respuesta
        int64_t tinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        ollama::response response =  ollama::generate(req);
        int64_t tfinal = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        // LO  CNVETRIMOS EN UN PROMPTETRICS Y LO EJECUTAMOS
        auto pm = metrics::promptmetrics::from_Ollama_json(response.as_json(), tinicio, tfinal, i);
        promptLogger.write2jsonline(pm);
        //pm.write2jsonline(filepath);

    }
    ollamaClose();
    hwMonitor.stop();
    hwThread.join();
    return true;
}
bool OllamaTest::runTestType1_5seg() {
    // obtenemos los prompts
    promptParser parser2 = promptParser("../prompt_list/instruction_following_eval_promt.jsonl");
    std::vector<std::string> prompts = parser2.getPrompts();
    //cramops el lloger de prompts
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_test1.jsonl",test_id,model_name_);
    Logger promptLogger(log_prompt_file);
    std::string log_hw_file = filepath_ + fmt::format("/{}_hw_metrics_{}_test1.jsonl", test_id, model_name_);
    HardwareMeasurements hwMonitor(log_hw_file, 1.0); // muestrea cada 1 segundo
    std::thread hwThread([&hwMonitor]() {
        hwMonitor.start(); // bloquea internamente hasta que se llame stop()
    });
    std::this_thread::sleep_for(std::chrono::seconds(5));
    // iteramos sobre los prompts
    for (int i = 0; i < num_prompts_ && i < prompts.size(); i++) {
        std::string prompt = prompts.at(i);
        // creamos la request
        ollama::request req = create_request( prompt);
        // enviamos la request y obtenemos la respuesta
        int64_t tinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        ollama::response response =  ollama::generate(req);
        int64_t tfinal = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        // LO  CNVETRIMOS EN UN PROMPTETRICS Y LO EJECUTAMOS
        auto pm = metrics::promptmetrics::from_Ollama_json(response.as_json(), tinicio, tfinal, i);
        promptLogger.write2jsonline(pm);
        //pm.write2jsonline(filepath);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    ollamaClose();
    std::this_thread::sleep_for(std::chrono::seconds(5));
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
    options["options"]["keep_alive"] =-1;

    ollama::request req(model_name_, prompt, options,false);
    req["logprobs"] = true;
    req["verbose"]  = true;
    return req;
}
bool OllamaTest::ollamaClose() {
    nlohmann::json options;
    //menos el keep alive el resto de opciones dan igual
    options["options"]["temperature"] = temperature_;
    options["options"]["num_ctx"] = context_size_;
    options["options"]["num_batch"] = batch_size_;
    options["options"]["seed"] = seed_;
    options["options"]["keep_alive"] =-1;
    //e campo de prompt tien que estar vacío para que se cierre el modelo
    ollama::request req(model_name_, "", options,false);
    req["logprobs"] = true;
    req["verbose"]  = true;
    ollama::response response =  ollama::generate(req);
    return true;
}