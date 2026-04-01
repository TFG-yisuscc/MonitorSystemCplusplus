//
// Created by yisus on 5/3/26.
//

#include "clients/LlamaTest.h"

#include "utils/LlamaInferencer.h"
#include "utils/promptParser.h"

LlamaTest::LlamaTest(std::string model_path, int temperature, int batch_size, int context_size, int seed, int num_prompts) {
    model_path_ = model_path;
    temperature_ = temperature;
    batch_size_ = batch_size;
    context_size_ = context_size;
    seed_ = seed;
    num_prompts_ = num_prompts;
}

LlamaTest::LlamaTest(nlohmann::json configLlama) {
    model_path_= configLlama.contains("model_path")? configLlama["model_path"].get<std::string>():
    throw std::runtime_error("model_path is required in the config");
    temperature_ = configLlama.contains("temperature")? configLlama["temperature"].get<int>():throw std::runtime_error("temperature is required in the config");
    batch_size_ = configLlama.contains("batch_size")? configLlama["batch_size"].get<int>():throw std::runtime_error("batch_size is required in the config");
    context_size_ = configLlama.contains("context_size")? configLlama["context_size"].get<int>():throw std::runtime_error("context_size is required in the config");
    seed_ = configLlama.contains("seed")? configLlama["seed"].get<int>():throw std::runtime_error("seed is required in the config");
    num_prompts_ = configLlama.contains("num_prompts")? configLlama["num_prompts"].get<int>():throw std::runtime_error("num_prompts is required in the config");
}

bool LlamaTest::runTestType0() {
//parser y logger ( //TODO)
    promptParser parser2 = promptParser("../prompt_list/instruction_following_eval_promt.jsonl");
    std::vector<std::string> prompts = parser2.getPrompts();
//carga
    LlamaInferencer inferencer(model_path_, temperature_, batch_size_, context_size_, seed_);
    inferencer.loadModel();

// bucle
    for (int i = 0; i < num_prompts_; i++) {
    std::cout << inferencer.generateTextCompletion(prompts.at(i)).answer << std::endl;
}
//cierre
inferencer.unloadModel();
return true;
}
bool LlamaTest::runTestType1() {
    // creamos esl llama model y lo cargamos con los parametros de la clase
    llama_backend_init();
    // configuaracion y seleccion del modelo
    llama_model_params model_params = llama_model_default_params();
    llama_model* model = llama_model_load_from_file(model_path_.c_str(), model_params);
    if (!model) {
        llama_backend_free();
        return false;
    }

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = context_size_;
    ctx_params.n_batch = batch_size_;
    //ctx_params.seed = seed_; no es aquí

    llama_context* ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
        llama_model_free(model);
        llama_backend_free();
        return false;
    }
    // establecemos el sampling



    // ejecucion del test de llma  model
return true;

}
bool LlamaTest::runTestType2() {
    //TODO implementar el test de tipo 2 para llama
    throw std::runtime_error("Test type 2 for LLAMA is not implemented yet.");
    return true;
}