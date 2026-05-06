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

    std::string filepath_; //ruta para guardar los resultados
    long long test_id; // el timestammp de comienczo en la practica
    std::string model_path_;
    int temperature_;
    int batch_size_;
    int context_size_;
    int seed_;
    int num_prompts_;
    double hardwarePeriod_;
    std::string getCleanModelPath();
public:
    LlamaTest(std::string filepath_, std::string model_path, int temperature, int batch_size, int context_size, int seed, int num_prompts, double hardwarePeriod = 0.5);
    LlamaTest(nlohmann::json);
    bool runTestType0(); // solo procesa los prompts, no toma medidas hardware
    bool runTestType1();//paralelismo an nivel de prompt individual (tras procesar un promp converge los h
    bool runTestType1_5seg(); //paralelismo a nivel de multiples prompts espaciado 5 segundos
};


#endif //MONITORSYSTEM_LLAMATEST_H