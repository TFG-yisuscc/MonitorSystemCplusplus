//
// Created by yisus on 3/3/26.
//

#include "promptmetrics.h"


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

        //TODO REHACER
        return promptmetrics(start_timestamp, finish_timestamp, model,InferenceEngines::OLLAMA, total_duration_ns,
                             prompt_eval_count, prompt_eval_duration_ns, eval_count,
                             eval_duration_ns, load_duration_ns, answer,logprobs, prompt_id);
    }
}
// metrics