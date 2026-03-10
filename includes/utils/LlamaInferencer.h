//
// Created by yisus on 8/3/26.
//
#ifndef MONITORSYSTEM_LLAMAAUX_H
#define MONITORSYSTEM_LLAMAAUX_H

#include "llama.h"
#include <functional>
#include <string>
#include <vector>

/**
 * LlamaInferencer - Clase auxiliar que encapsula el ciclo de vida completo de un modelo llama.cpp:
 *  - Inicialización del backend
 *  - Carga del modelo y sus parámetros
 *  - Creación y destrucción del contexto
 *  - Configuración del sampler
 *  - Generación de texto (tokenización, decode, sample, detokenización)
 *  todos los timestamps son en nanosegundos (ns) para poderc omparar con ollama
 */

struct LlamaGenerateResult{
    std::string answer;
    std::string probabilidades;
    llama_perf_context_data perfTimings;
    int64_t inicioPrefill;
    int64_t finPrefill;
    int64_t inicioDecode;
    int64_t finDecode;
    int64_t inicioSample;
    int64_t finSample;
};
struct  LlamaLoadTimestamps {
    int64_t inicioBackendInit;
    int64_t finBackendInit;
    int64_t inicioModelLoad;
    int64_t finModelLoad;
    int64_t inicioContextCreate;
    int64_t finContextCreate;

};
class LlamaInferencer {
public:
    // -------------------------------------------------------------------------
    // Parámetros de configuración
    // -------------------------------------------------------------------------
    std::string model_path_;
    int temperature_;
    int batch_size_;
    int context_size_;
    int seed_;
    int num_prompts_;
    bool initialized_ = false;
private:
    llama_model* model_ = nullptr;
    llama_context* ctx_ = nullptr;
    llama_sampler* sampler_ = nullptr;
    const struct llama_vocab *vocab_ = nullptr;


    // -------------------------------------------------------------------------
    // Constructor / Destructor
    // -------------------------------------------------------------------------
public:
    LlamaInferencer(const std::string &model_path, int temperature, int batch_size, int context_size, int seed,
        int num_prompts)
        : model_path_(model_path),
          temperature_(temperature),
          batch_size_(batch_size),
          context_size_(context_size),
          seed_(seed),
          num_prompts_(num_prompts) {//TODO ELIMinar numero de Prompts porque no atañe a esta clase
    }
    ~LlamaInferencer();
    LlamaLoadTimestamps loadModel();
    LlamaGenerateResult generateTextCompletion(std::string prompt);
    bool unloadModel();
private:
    bool initializeBackend();
    bool createContext();
    bool setupSampler();
    bool reset();

};
#endif //MONITORSYSTEM_LLAMAAUX_H

