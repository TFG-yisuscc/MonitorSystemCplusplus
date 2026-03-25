//
// Created by yisus on 23/3/26.
//

#ifndef MONITORSYSTEM_INPUTCONFIGURATION_H
#define MONITORSYSTEM_INPUTCONFIGURATION_H
#include <string>

#include "enumConfig.h"
#include "InferenceEngines.h"
#include "nlohmann/json.hpp"



class InputConfiguration {
public:
    //los miembros publicos y parametros de config
    InferenceEngines inferenceEngine_;
    TestType testType_;
    int batch_size_;
    int context_size_;
    int seed_;
    int num_prompts_;
    float temperature_;
    std::string model_path_or_name_;

    //los parametros del resumen  que se pueblan cuando se hace run
    // nio se si es la mejor opción
    long long timestamp_run_start;
    long long timestamp_run_end;
    std::string run_path_;
    //estas son un poco reduundantes creo yo
    std::string anotations; // este lo recibe del la configuración y lo mente en el resumen
    std::string og_config_json; // para poder revisarlo luego en caso de que sea necesarío o para poder acceder a campos origenales
    //campos opcionales(sobre to por llama cpp)
    // constructor con checkers
    InputConfiguration(nlohmann::json json_config);


    //un metodo run que corre ls test seleccionados
    void run();
private:
    //un "conversor" a la clase resumen
    void runOllama();
    void runLlama();
    void createResumen();


};


#endif //MONITORSYSTEM_INPUTCONFIGURATION_H