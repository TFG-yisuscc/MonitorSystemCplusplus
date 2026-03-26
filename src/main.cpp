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
    // creamos una configuración de entrada
    InputConfiguration input_configuration(InferenceEngines::OLLAMA, TestType::TYPE_1, 4,
        2048, 42, 10, 0, "granite4:micro-h");

    //  ejecutamos el test tipo 1 de ollama
    input_configuration.run();


    //verificamos el resultado

}
