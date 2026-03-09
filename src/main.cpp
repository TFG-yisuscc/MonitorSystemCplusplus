#include "llama.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include "utils/LlamaAux.h"

int main(int argc, char *argv[]) {

    //probanmos cone largumento de los gatitos bonitos
    LlamaAux prueba = LlamaAux( "/home/yisus/Descargas/tinyllama-1.1b-chat-v1.0.Q4_0.gguf", 0.8, 512, 1024, 42, 1);

    LlamaLoadTimestamps llt = prueba.loadModel();
    std::cout << "Tiempo de inicialización del backend: " << (llt.finBackendInit - llt.inicioBackendInit) << " ns" << std::endl;
    LlamaGenerateResult lgr1= prueba.generateTextCompletion("Cuentame un cuento");
    std::cout << "Respuesta: " << lgr1.answer << std::endl;
    std::cout << "Tiempo de eval " << lgr1.perfTimings.t_eval_ms << std::endl;
    std::cout << "Tiempo de carga" << lgr1.perfTimings.t_load_ms << std::endl;

    LlamaGenerateResult lgr2 =  prueba.generateTextCompletion("hablame sobre taiwan");
    std::cout << "Respuesta: " << lgr2.answer << std::endl;
    std::cout << "Tiempo de eval " << lgr2.perfTimings.t_eval_ms << std::endl;
    std::cout << "Tiempo de carga" << lgr2.perfTimings.t_load_ms << std::endl;
    prueba.unloadModel();

    // inicializo el modelo
}
