#include "llama.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "metrics/hardwareMetrics.h"
#include "utils/LlamaInferencer.h"
#include "utils/inputConfiguration.h"
#include "utils/enumConfig.h"

int main(int argc, char *argv[]) {
    //std::cout << ollama::generate("granite4:micro-h", "Why is the sky blue?") << std::endl;
    // creamos una configuración de entrada
    InputConfiguration input_configuration(InferenceEngines::LLAMA, TestType::TYPE_0, 512,
        2048, 42, 9, 0, "/home/user1/tinyllama-1.1b-chat-v1.0.Q4_0.gguf");

    //  ejecutamos el test tipo 1 de ollama
    input_configuration.run();


    //verificamos el resultado

}
