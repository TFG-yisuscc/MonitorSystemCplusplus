//
// Created by yisus on 3/3/26.
//
#pragma once
#ifndef MONITORSYSTEM_PROMPTMETRICS_H
#define MONITORSYSTEM_PROMPTMETRICS_H
#include <string>
#include <stdint.h>
#include "third_party/ollama.hpp"
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
        int64_t total_duration_ns;
        int64_t prompt_eval_count;
        int64_t prompt_eval_duration_ns;
        int64_t eval_count;
        int64_t eval_duration_ns;
        int64_t load_duration_ns;
        std::string answer = "NONE";
        std::string logprobs = "NONE";
        int prompt_id = -1;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(promptmetrics,
                                       start_timestamp_ns,
                                       finish_timestamp_ns,
                                       model,
                                       total_duration_ns,
                                       prompt_eval_count,
                                       prompt_eval_duration_ns,
                                       eval_count,
                                       eval_duration_ns,
                                       load_duration_ns,
                                       answer,
                                       logprobs,
                                       prompt_id);

        promptmetrics(int64_t start_timestamp_ns, int64_t finish_timestamp_ns, const std::string &model,
            int64_t total_duration_ns, int64_t prompt_eval_count, int64_t prompt_eval_duration_ns, int64_t eval_count,
            int64_t eval_duration_ns, int64_t load_duration_ns, const std::string &answer, const std::string logprobs ,int prompt_id)
            : start_timestamp_ns(start_timestamp_ns),
              finish_timestamp_ns(finish_timestamp_ns),
              model(model),
              total_duration_ns(total_duration_ns),
              prompt_eval_count(prompt_eval_count),
              prompt_eval_duration_ns(prompt_eval_duration_ns),
              eval_count(eval_count),
              eval_duration_ns(eval_duration_ns),
              load_duration_ns(load_duration_ns),
              answer(answer),
              logprobs(logprobs),
              prompt_id(prompt_id) {
        }

        ~promptmetrics() = default;
        static promptmetrics from_json_ollama( nlohmann::json json,int64_t start_timestamp,int64_t finish_timestamp, int prompt_id);
        static promptmetrics from_json_llama(std::string json);
        bool write2jsonline();
        bool closefile();// cierra los archivos de metricas
    };





};
#endif //MONITORSYSTEM_PROMPTMETRICS_H
