//
// Created by yisus on 5/3/26.
//
/*En esta calse se va a definir el funcionamiento del test y las diversas funciones auxiliares relacionadas con ollama
 *de forma similar a como se hace  en la version de python de monitorSyastem
 *
 *
 */
#ifndef MONITORSYSTEM_OLLAMATEST_H
#define MONITORSYSTEM_OLLAMATEST_H
#include <string>
#include "third_party/ollama.hpp"
#include "utils/promptParser.h"

class OllamaTest{
private:
 //aqui definimos los parametros de ejecucion temp, modelos...
    std::string model_name_;
    int temperature_;
    int batch_size_;
    int context_size_;
    int seed_;
    int num_prompts_;
    // TODO no se si añadir los parametros TOP-p y TOP k
public:
    OllamaTest(std::string model_name, int temperature, int batch_size, int context_size, int seed, int num_prompts);
    OllamaTest(nlohmann::json);
    bool runTestType1();//paralelismo an nivel de prompt individual (tras procesar un promp converge los hilos)
    bool runTestType2(); //paralelismo a nivel de multeples prompts (los hilos convergen al final de todos los prompts)
};


#endif //MONITORSYSTEM_OLLAMATEST_H