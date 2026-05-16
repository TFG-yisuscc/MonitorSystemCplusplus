

// Created by yisus on 4/3/26.
//

#ifndef MONITORSYSTEM_INFERENCEENGINES_H
#define MONITORSYSTEM_INFERENCEENGINES_H

#include <third_party/ollama.hpp> //para que no haya conflictos con el json de vcpkg

enum InferenceEngines {LLAMA, OLLAMA, HAILO_OLLAMA, OTHER};

NLOHMANN_JSON_SERIALIZE_ENUM(InferenceEngines, {
    {LLAMA,        "LLAMA"},
    {OLLAMA,       "OLLAMA"},
    {HAILO_OLLAMA, "HAILO_OLLAMA"},
    {OTHER,        "OTHER"},
})

#endif //MONITORSYSTEM_INFERENCEENGINES_H