//
// Created by yisus on 8/3/26.
// Probablemente lo pueda hacer mediante herencia
//de una clase padre /madre comu a la de ollama
//
#pragma once
#include <string>
#include <stdint.h>
#include "third_party/ollama.hpp"
#include "utils/InferenceEngines.h"
#ifndef MONITORSYSTEM_PROMPTMETRICSLLAMA_H
#define MONITORSYSTEM_PROMPTMETRICSLLAMA_H


class promptmetricsLlama {
    public:
    int64_t start_timestamp_ns;
    int64_t finish_timestamp_ns;
    std::string model;
    InferenceEngines engine = LLAMA;// revisar que funcione en la conversion a json
    int64_t total_duration_ns;
    int64_t prompt_eval_count;
    int64_t prompt_eval_duration_ns;
    int64_t eval_count;
    int64_t eval_duration_ns;
    int64_t load_duration_ns;
    std::string answer = "NONE";
    std::string logits = "NONE";
    int prompt_id = -1;


};


#endif //MONITORSYSTEM_PROMPTMETRICSLLAMA_H