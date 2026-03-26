//
// Created by yisus on 23/3/26.
//

#ifndef MONITORSYSTEM_INPUTCONFIGURATION_H
#define MONITORSYSTEM_INPUTCONFIGURATION_H
#include <string>
#include "enumConfig.h"
#include "InferenceEngines.h"




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
    long long timestamp_run_start = 0L;
    long long timestamp_run_end  = 0L;
    std::string run_path_ = "EMPTY";
    //estas son un poco reduundantes creo yo
    std::string anotations ="EMPTY"; // este lo recibe del la configuración y lo mente en el resumen
    std::string og_config_json = "EMPTY"; // para poder revisarlo luego en caso de que sea necesarío o para poder acceder a campos origenales
    //campos opcionales(sobre to por llama cpp)
    // constructor con checkers
    //InputConfiguration(nlohmann::json json_config);
    InputConfiguration(InferenceEngines inference_engine, TestType test_type, int batch_size, int context_size,
        int seed, int num_prompts, float temperature, const std::string &model_path_or_name)
        : inferenceEngine_(inference_engine),
          testType_(test_type),
          batch_size_(batch_size),
          context_size_(context_size),
          seed_(seed),
          num_prompts_(num_prompts),
          temperature_(temperature),
          model_path_or_name_(model_path_or_name) {
    };


    //un metodo run que corre ls test seleccionados
    void run();
private:
    //un "conversor" a la clase resumen
    void runOllama();
    void runLlama();
    void createResumen();


};


#endif //MONITORSYSTEM_INPUTCONFIGURATION_H