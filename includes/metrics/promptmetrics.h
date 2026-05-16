//
// Created by yisus on 3/3/26.
//
#pragma once
#ifndef MONITORSYSTEM_PROMPTMETRICS_H
#define MONITORSYSTEM_PROMPTMETRICS_H
#include <string>
#include <stdint.h>
#include "third_party/ollama.hpp"
#include "utils/InferenceEngines.h"
#include "utils/LlamaInferencer.h"
#include "utils/ProbabilityType.h"
/*
 * Esta clase combinala estructura de los datos del prompt metrics y el guardado de los mismos en un archivo jsonl,
 * cada vez que se cree un objeto de esta clase se guardara en el archivo de metrics
 */
namespace metrics {
    class promptmetrics {

    public:
        int64_t start_timestamp_ns;
        int64_t finish_timestamp_ns;
        std::string model;
        InferenceEngines engine = OLLAMA;// revisar que funcione en la conversion a json
        int64_t total_duration_ns;
        int64_t prompt_eval_count;
        int64_t prompt_eval_duration_ns;
        int64_t eval_count;
        int64_t eval_duration_ns;
        int64_t load_duration_ns;
        std::string answer = "NONE";
        ProbabilityType probType = LOG_PROBABILITY;
        std::string tokenProb = "NONE";
        int prompt_id = -1;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(promptmetrics,
                                       start_timestamp_ns,
                                       finish_timestamp_ns,
                                       model,
                                       engine,
                                       total_duration_ns,
                                       prompt_eval_count,
                                       prompt_eval_duration_ns,
                                       eval_count,
                                       eval_duration_ns,
                                       load_duration_ns,
                                       answer,
                                       probType,
                                       tokenProb,
                                       prompt_id);

        promptmetrics(int64_t start_timestamp_ns, int64_t finish_timestamp_ns, const std::string &model, InferenceEngines engine,
            int64_t total_duration_ns, int64_t prompt_eval_count, int64_t prompt_eval_duration_ns, int64_t eval_count,
            int64_t eval_duration_ns, int64_t load_duration_ns, const std::string &answer, const std::string tokenProb ,int prompt_id)
            : start_timestamp_ns(start_timestamp_ns),
              finish_timestamp_ns(finish_timestamp_ns),
              model(model),
                engine(engine),
              total_duration_ns(total_duration_ns),
              prompt_eval_count(prompt_eval_count),
              prompt_eval_duration_ns(prompt_eval_duration_ns),
              eval_count(eval_count),
              eval_duration_ns(eval_duration_ns),
              load_duration_ns(load_duration_ns),
              answer(answer),
              probType(this->engine!=OLLAMA?PROBABILITY:LOG_PROBABILITY), // Si el motor no es Ollama, se establece como OTHER
              tokenProb(tokenProb),
              prompt_id(prompt_id) {
        }

        ~promptmetrics() = default;
        static promptmetrics from_Ollama_json( nlohmann::json json,int64_t start_timestamp,int64_t finish_timestamp, int prompt_id);
        static promptmetrics from_HailoOllama_json(nlohmann::json json, int64_t start_timestamp, int64_t finish_timestamp, int prompt_id);
        static promptmetrics from_Llama(LlamaLoadTimestamps llt, LlamaGenerateResult llg, int prompt_id);
        static promptmetrics from_Llama( LlamaGenerateResult llg, int prompt_id);// esta loadtimes o
    };





};
#endif //MONITORSYSTEM_PROMPTMETRICS_H
