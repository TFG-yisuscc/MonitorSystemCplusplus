//
// Created by yisus on 5/3/26.
//

#include "LlamaTest.h"

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

bool LlamaTest::runTestType1() {
    // creamos esl llama model y lo cargamos con los parametros de la clase
    llama_backend_init();
    // configuaracion y seleccion del modelo
    llama_model_params model_params = llama_model_default_params();
    llama_model* model = llama_load_model_from_file(model_path_.c_str(), model_params);
    if (!model) {
        llama_backend_free();
        return false;
    }

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = context_size_;
    ctx_params.n_batch = batch_size_;
    //ctx_params.seed = seed_; no es aquí

    llama_context* ctx = llama_new_context_with_model(model, ctx_params);
    if (!ctx) {
        llama_free_model(model);
        llama_backend_free();
        return false;
    }
    // establecemos el sampling



    // ejecucion del test de llma  model


}