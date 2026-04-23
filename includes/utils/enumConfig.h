//
// Created by yisus on 23/3/26.
//

#ifndef MONITORSYSTEM_ENUMCONFIG_H
#define MONITORSYSTEM_ENUMCONFIG_H

#include <third_party/ollama.hpp> //para que no haya conflictos con el json de vcpkg

enum TestType {TYPE_0, TYPE_1, TYPE_2};

NLOHMANN_JSON_SERIALIZE_ENUM(TestType, {
    {TYPE_0, "TYPE_0"},
    {TYPE_1, "TYPE_1"},
    {TYPE_2, "TYPE_2"},
})

#endif //MONITORSYSTEM_ENUMCONFIG_H