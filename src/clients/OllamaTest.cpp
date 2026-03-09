//
// Created by yisus on 5/3/26.
//


#include "clients/OllamaTest.h"


// constructores
OllamaTest::OllamaTest(std::string model_name, int temperature, int batch_size, int context_size, int seed, int num_prompts)
{
    model_name_ = model_name;
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


// funciones auxiliares
ollama::request create_request(const std::string& model_name, const std::string& prompt,int temperature, int batch_size, int
                               context_size, int seed)
{
    nlohmann::json options;
    /*¡¡¡ATENCION!!!!!
     * Revisa bien si ollama hace caso a los parametros especificados o los ignora
     * Que ollama tiene fama de ignorar parametros que se le especifican.
     */
    options["options"]["temperature"] = temperature;
    options["options"]["num_ctx"] = context_size;
    options["options"]["num_batch"] = batch_size;
    options["options"]["seed"] = seed;

    ollama::request req(model_name, prompt, options,false);
    req["logprobs"] = true;
    req["verbose"]  = true;
    return req;
}
// funciones de test
bool OllamaTest::runTestType1() {
    //TODO implementar el logger
    std::cout << "Test type1" << std::endl;
    std::string filepath = "../results/metrics.jsonl";
    // obtenemos los prompts
    promptParser parser2 = promptParser("../prompt_list/instruction_following_eval_promt.jsonl");
    std::vector<std::string> prompts = parser2.getPrompts();
    // iteramos sobre los prompts
    for (int i = 0; i < num_prompts_ && i < prompts.size(); i++) {
        std::string prompt = prompts.at(i);
        // creamos la request
        ollama::request req = create_request(model_name_, prompt, temperature_, batch_size_, context_size_, seed_);
        // enviamos la request y obtenemos la respuesta
        int64_t tinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        ollama::response response =  ollama::generate(req);
        int64_t tfinal = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        // LO  CNVETRIMOS EN UN PROMPTETRICS Y LO EJECUTAMOS
        auto pm = metrics::promptmetrics::from_Ollama_json(response.as_json(), tinicio, tfinal, i);
        //pm.write2jsonline(filepath);

    }
    return true;
}
bool OllamaTest::runTestType2() {
    //TODO implementar
    return true;
}

