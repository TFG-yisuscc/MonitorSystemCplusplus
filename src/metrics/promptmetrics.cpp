//
// Created by yisus on 3/3/26.
//

#include "../../includes/metrics/promptmetrics.h"


namespace metrics {
    promptmetrics promptmetrics::from_Ollama_json(nlohmann::json json, int64_t start_timestamp,
                                                  int64_t finish_timestamp, int prompt_id) {
        // Implementación de la función para crear un objeto promptmetrics a partir de un JSON de Ollama
        // Aquí deberías parsear el JSON y extraer los campos necesarios para construir el objeto promptmetrics

        std::string model = json.contains("model")?json["model"].get<std::string>():"DEFAULT_MODEL";
        int64_t total_duration_ns = json.contains("total_duration")? json["total_duration"].get<int64_t>():-1 ;
        int64_t prompt_eval_count = json.contains("prompt_eval_count")? json["prompt_eval_count"].get<int64_t>():-1;
        int64_t prompt_eval_duration_ns = json.contains("prompt_eval_duration")? json["prompt_eval_duration"].get<int64_t>():-1;
        int64_t eval_count = json.contains("eval_count")? json["eval_count"].get<int64_t>():-1;
        int64_t eval_duration_ns = json.contains("eval_duration")? json["eval_duration"].get<int64_t>():-1;
        int64_t load_duration_ns = json.contains("load_duration")? json["load_duration"].get<int64_t>():-1;
        std::string answer = json.contains("response")? json["response"].get<std::string>():"NONE";
        std::string logprobs = json.contains("logprobs")? json["logprobs"].dump():"NONE";


        return promptmetrics(start_timestamp, finish_timestamp, model,InferenceEngines::OLLAMA, total_duration_ns,
                             prompt_eval_count, prompt_eval_duration_ns, eval_count,
                             eval_duration_ns, load_duration_ns, answer,logprobs, prompt_id);
    }

    promptmetrics promptmetrics::from_Llama(LlamaLoadTimestamps llt, LlamaGenerateResult llg, int prompt_id) {
        /*
         *Tiene en cuenta el tiempo de carga
         * se usa cuando se carga el modelo por primeravez
         */
        std::string model = llg.model_path;
        int64_t start_timestamp_ns = llt.inicioBackendInit;
        int64_t finish_timestamp_ns = llg.finDecode;
        int64_t total_duration_ns = llg.finDecode - llt.inicioBackendInit;
        int64_t prompt_eval_count = llg.perfTimings.n_p_eval;
        int64_t prompt_eval_duration_ns = llg.perfTimings.t_p_eval_ms * 1'000'000;
        int64_t eval_count = llg.perfTimings.n_eval;
        int64_t eval_duration_ns = llg.perfTimings.t_eval_ms * 1'000'000;
        int64_t load_duration_ns = 0; //llg.perfTimings.t_load_ms * 1'000'000;
        std::string answer = llg.answer;
         InferenceEngines engine = InferenceEngines::LLAMA;
         std::string probs = llg.probabilidades;
        return promptmetrics(start_timestamp_ns, finish_timestamp_ns, model, engine, total_duration_ns,
                             prompt_eval_count, prompt_eval_duration_ns, eval_count,
                             eval_duration_ns, load_duration_ns, answer,probs, prompt_id);

    }

    promptmetrics promptmetrics::from_Llama(LlamaGenerateResult llg, int prompt_id) {
        std::string model = llg.model_path;
        int64_t start_timestamp_ns = llg.inicioDecode;
        int64_t finish_timestamp_ns = llg.finDecode;
        int64_t total_duration_ns = llg.finDecode - llg.inicioDecode;
        int64_t prompt_eval_count = llg.perfTimings.n_p_eval;
        int64_t prompt_eval_duration_ns = llg.perfTimings.t_p_eval_ms * 1'000'000;
        int64_t eval_count = llg.perfTimings.n_eval;
        int64_t eval_duration_ns = llg.perfTimings.t_eval_ms * 1'000'000;
        int64_t load_duration_ns = 0;
        std::string answer = llg.answer;
        InferenceEngines engine = InferenceEngines::LLAMA;
        std::string probs = llg.probabilidades;
    return promptmetrics(start_timestamp_ns, finish_timestamp_ns, model,LLAMA, total_duration_ns,
                         prompt_eval_count, prompt_eval_duration_ns, eval_count,
                         eval_duration_ns, load_duration_ns, answer,probs, prompt_id);
    }
}
// metrics