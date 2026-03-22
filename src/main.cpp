#include "llama.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "metrics/hardwareMetrics.h"
#include "utils/LlamaInferencer.h"

int main(int argc, char *argv[]) {

    // LlamaInferencer prueba = LlamaInferencer( "/home/yisus/Descargas/tinyllama-1.1b-chat-v1.0.Q4_0.gguf", 0.8, 512, 1024, 42, 1);
    // LlamaLoadTimestamps llt = prueba.loadModel();
    // std::cout << "Tiempo de inicialización del backend: " << (llt.finBackendInit - llt.inicioBackendInit) << " ns" << std::endl;
    // LlamaGenerateResult lgr1= prueba.generateTextCompletion("tell me three fun facts about yourself");
    // std::cout << "Respuesta: " << lgr1.answer << std::endl;
    // std::cout << "Tiempo de eval " << lgr1.perfTimings.t_eval_ms << std::endl;
    // std::cout << "Tiempo de carga" << lgr1.perfTimings.t_load_ms << std::endl;
    // std::cout << "Probabilidades"  << std::endl;
    //     for (auto eee: lgr1.probabilidades) {
    //         std::cout << eee << std::endl;
    //
    // }
    // LlamaGenerateResult lgr2 =  prueba.generateTextCompletion("tell me about taiwan");
    // std::cout << "Respuesta: " << lgr2.answer << std::endl;
    // std::cout << "Tiempo de eval " << lgr2.perfTimings.t_eval_ms << std::endl;
    // std::cout << "Tiempo de carga" << lgr2.perfTimings.t_load_ms << std::endl;
    // for (auto eee: lgr2.probabilidades) {
    //     std::cout << eee << std::endl;
    //
    // }
    // prueba.unloadModel();
    // inicializo el modelo
     //test de hardware metrics
    hardwareMetrics hm = hardwareMetrics(InferenceEngines::OLLAMA);
    printf(std::to_string(hm.timestamp_).c_str());
    hm.update();
    hm.json
    printf(std::to_string(hm.timestamp_).c_str());
    printf("\rn");


}
