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
#include "metrics/promptmetrics.h"

class OllamaTest{
private:
 //aqui definimos los parametros de ejecucion temp, modelos...
    std::string model_name_;
    std::string filepath_; //ruta para guardar los resultados
    long long test_id; // el timestammp de comienczo en la practica
    float temperature_;
    int batch_size_;
    int context_size_;
    int seed_;
    int num_prompts_;


    bool ollamaClose();
    ollama::request create_request(const std::string& prompt);
    nlohmann::json generate_streaming(ollama::request& req);
public:
    OllamaTest(std::string model_name,std::string filepath, float temperature, int batch_size, int context_size, int seed, int num_prompts);
    OllamaTest(nlohmann::json);
    bool runTestType0(); //solo se procesan los prompts y no se toman métricas de hardware.
    bool runTestType1();//paralelismo a nivel de multeples prompts (los hilos convergen al final de todos los prompts)
    bool runTestType1_5seg(); //5 seg entre prompts
};


#endif //MONITORSYSTEM_OLLAMATEST_H