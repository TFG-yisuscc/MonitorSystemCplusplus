//
// Created by yisus on 5/3/26.
//Tengo un problema, digiere bastante en como es con la
//libreria de python
//

#ifndef MONITORSYSTEM_LLAMATEST_H
#define MONITORSYSTEM_LLAMATEST_H
#include <string>
#include "third_party/ollama.hpp" // por la libreria de json
#include  "llama.h"

class LlamaTest {
private:
    std::string model_path_;
    int temperature_;
    int batch_size_;
    int context_size_;
    int seed_;
    int num_prompts_;
public:
    LlamaTest(std::string model_path, int temperature, int batch_size, int context_size, int seed, int num_prompts);
    LlamaTest(nlohmann::json);
    bool runTestType1();//paralelismo an nivel de prompt individual (tras procesar un promp converge los h
    bool runTestType2(); //paralelismo a nivel de multeples prompts (
};


#endif //MONITORSYSTEM_LLAMATEST_H