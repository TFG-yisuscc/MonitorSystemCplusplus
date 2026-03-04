//
// Created by yisus on 4/3/26.
//
#pragma once
#include "third_party/ollama.hpp"
#ifndef MONITORSYSTEM_PROMPT_UTILS_H
#define MONITORSYSTEM_PROMPT_UTILS_H
ollama::request create_request(const std::string& model_name, const std::string& prompt,int temperature, int batch_size, int
    context_size, int seed );
#endif //MONITORSYSTEM_PROMPT_UTILS_H