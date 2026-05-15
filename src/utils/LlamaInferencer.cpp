//
// Created by yisus on 8/3/26.
//

#include "utils/LlamaInferencer.h"
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <cstring>

#include "third_party/ollama.hpp"
/*
 *Dado el bajo nivel de abstraccion que presenta la biblioteca de llama.h
 * Para facilitar su uso, se ha creado esta clase que abstrae su uso
 */
// =============================================================================
// Constructor / Destructor
// =============================================================================
//TODO ajustar todos los paramtros de inferencia para que sea lo mas identico a ollama posible
// Incluir en los parametros ajustables por el usuario los parametros de sampleo (aunque en teoria son "fijos").
// mas alla de la temperatura, el top-p y el top-k,
// puesto que estos no aparencen en el .gguf
// https://github.com/ollama/ollama/blob/61086083eb8c558bc14c61d6df630c3bf6e690b4/api/types.go

LlamaInferencer::~LlamaInferencer() {
    unloadModel();
}

LlamaLoadTimestamps LlamaInferencer::loadModel() {
    /* NO CARGA SOLAMENTE EL MODELO
     * 1)Inicicializa el backend
     * 2) Carga el modelo y sus parámetros
     * 3) Crea el contexto de inferencia
     * 4) Configura el sampler
     */

    LlamaLoadTimestamps res{};
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
        this->repeat_last_n_,
        this->repeat_penalty_,
        this->frequency_penalty_,
        this->presence_penalty_
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
    reset();
    LlamaGenerateResult res{};
    res.model_path = model_path_;
    res.inicioPrefill = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    //1 tokenizo el answer (no se si es exacto?)
    int n_tokens = -llama_tokenize( // este es para obtener el tamaño exacto y no mas de tokenes
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

    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    llama_decode(this->ctx_, batch);
    res.finPrefill = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    res.inicioDecode = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    //3 bucle de generación (sampling)
    int n_cur = tokens.size();
    size_t n_max = this->max_tokens_ < 0
        ? static_cast<size_t>(std::max(0, this->context_size_ - static_cast<int>(tokens.size())))
        : static_cast<size_t>(this->max_tokens_);
    std::vector<std::string>probs; //string para que no hala que hacer conversiones luego en la clase de prompmetrics
    std::string generated_text;
    while (n_cur < n_max) {
        llama_token token_id = llama_sampler_sample(this->sampler_, this->ctx_, -1);
        // Verificar fin de secuencia
        if (llama_vocab_is_eog(this->vocab_, token_id)) break;
        //
        const float* logits = llama_get_logits_ith(this->ctx_, -1);
        int n_vocab = llama_vocab_n_tokens(this->vocab_);
        float max_logit = *std::max_element(logits, logits + n_vocab);
        float sum = 0.0f;
        for (int i = 0; i < n_vocab; i++)
            sum += std::exp(logits[i] - max_logit);
        float prob = (sum > 0.0f) ? std::exp(logits[token_id] - max_logit) / sum : 0.0f;
        probs.push_back(std::to_string(prob));
        char buf[256];
        int n = llama_token_to_piece(this->vocab_, token_id, buf, sizeof(buf) - 1, 0, false);
        if (n > 0) {
            buf[n] = '\0';
            generated_text += buf;
        }
        // Evaluar el token generado
        batch = llama_batch_get_one(&token_id, 1);// no lo recomienda la api de llama
       // calcular probibilidad de generacion

        if (llama_decode(this->ctx_, batch) != 0) break;
        n_cur++;
    }
    res.finDecode = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    res.probabilidades = nlohmann::json(probs).dump();
    res.answer = generated_text;
    res.perfTimings = llama_perf_context(this->ctx_);

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

void LlamaInferencer::reset() {
    llama_memory_clear(llama_get_memory(this->ctx_), true);
    llama_perf_context_reset(this->ctx_);
    llama_sampler_reset(this->sampler_);
}
