//
// Created by yisus on 8/3/26.
//

#include "LlamaInferencer.h"

#include <chrono>
#include <stdexcept>
#include <cstring>

// =============================================================================
// Constructor / Destructor
// =============================================================================
//TODO ajustar todos los paramtros de inferencia para que sea lo mas identico a ollama posible
// Incluir en los parametros ajustables por el usuario los parametros de sampleo (aunque en teoria son "fijos").
// mas alla de la temperatura, el top-p y el top-k,
// puesto que estos no aparencen en el .gguf


LlamaInferencer::~LlamaInferencer() {

}

LlamaLoadTimestamps LlamaInferencer::loadModel() {
    /* NO CARGA SOLAMENTE EL MODELO
     * 1)Inicicializa el backend
     * 2) Carga el modelo y sus parámetros
     * 3) Crea el contexto de inferencia
     * 4) Configura el sampler
     */
    //TODO  meterlos enlos metodos de cada funcion
    LlamaLoadTimestamps res;
    res.inicioBackendInit = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    llama_backend_init();
    res.finBackendInit = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    // 2. Parámetros del modelo
    res.inicioModelLoad= std::chrono::high_resolution_clock::now().time_since_epoch().count();
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 0;// capas en GPU (0 = solo CPU)

    // 3. Cargar el modelo
    this->model_ = llama_model_load_from_file(
        this->model_path_.c_str(), model_params);
    res.finModelLoad = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if (!this->model_) {
        throw std::runtime_error("Error cargando modelo");
    }

    // 4. Crear contexto de inferencia
    res.inicioContextCreate= std::chrono::high_resolution_clock::now().time_since_epoch().count();
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx    = this->context_size_;  // tamaño del contexto
    ctx_params.n_batch  = this->batch_size_;   // batch de procesamiento
    ctx_params.no_perf  = false;
    this->ctx_ = llama_init_from_model(this->model_, ctx_params);
    res.finContextCreate = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    // Obtener vocabulario
   this->vocab_ = llama_model_get_vocab(this->model_);
    this->sampler_ = llama_sampler_chain_init(
        llama_sampler_chain_default_params()
    );
    llama_sampler_chain_add(this->sampler_,
    llama_sampler_init_penalties(
        64,     // repeat_last_n
        1.3f,   // repeat_penalty
        0.1f,   // frequency_penalty
        0.1f    // presence_penalty
    )
);
     llama_sampler_chain_add(this->sampler_, llama_sampler_init_temp(this->temperature_));
     llama_sampler_chain_add(this->sampler_, llama_sampler_init_dist(this->seed_));
    this->initialized_ = true;
    return res;
}

 LlamaGenerateResult LlamaInferencer::generateTextCompletion(std::string prompt) {

    /* 1) Tokeniza el answer
     * 2) Crea batch y evalúa el answer
     * 3) Bucle de generación (sampling)
     * 4) Devuelve el resultado con los timestamps
     */
    //0 compruebo que hse haya inicializado el modelo
    if (!this->initialized_) {
        throw std::runtime_error("Modelo no inicializado. Llama a loadModel() antes de generar texto.");
    }
    LlamaGenerateResult res;
    //1 tokenizo el answer (no se si es exacto?)
    int n_tokens = -llama_tokenize(
        this->vocab_,
        prompt.c_str(),
        prompt.size(),
        nullptr,
        0,
        true,
        false
    );
    std::vector<llama_token> tokens(n_tokens);
    // no se si esto sirve pero bueno
    llama_tokenize(
        this->vocab_,
        prompt.c_str(),
        prompt.size(),
        tokens.data(),
        tokens.size(),
        true,
        false
    );

    //2 crear batch y prefill
    llama_perf_context_reset(this->ctx_);

    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    llama_decode(this->ctx_, batch);
    //3 bucle de generación (sampling)
    int n_cur = tokens.size();
    int n_max = 512; // máximo de tokens a generar
    std::string generated_text;
    while (n_cur < n_max) {
        llama_token token_id = llama_sampler_sample(this->sampler_, this->ctx_, -1);
        // Verificar fin de secuencia
        if (llama_vocab_is_eog(this->vocab_, token_id)) break;
        // Convertir token a texto e imprimir
        char buf[256];
        int n = llama_token_to_piece(this->vocab_, token_id, buf, sizeof(buf), 0, false);
        if (n > 0) {
            buf[n] = '\0';
            generated_text += buf;
        }
        // Evaluar el token generado
        batch = llama_batch_get_one(&token_id, 1);
        if (llama_decode(this->ctx_, batch) != 0) break;
        n_cur++;
    }
    res.answer = generated_text;
    res.probabilidades = ""; //TODO sacar las probabilidades de cada token generado
    res.perfTimings = llama_perf_context(this->ctx_);
    // reseteo de los perf y cxt
    llama_memory_clear(llama_get_memory(this->ctx_), true);
        llama_perf_context_reset(this->ctx_);
        llama_sampler_reset(this->sampler_);
    return res;
}

bool LlamaInferencer::unloadModel() {
    if (this->ctx_) {
        llama_free(this->ctx_);
        this->ctx_ = nullptr;
    }
    if (this->model_) {
        llama_model_free(this->model_);
        this->model_ = nullptr;
    }
    if (this->sampler_) {
        llama_sampler_free(this->sampler_);
        this->sampler_ = nullptr;
    }
    this->initialized_ = false;
    return true;
}
