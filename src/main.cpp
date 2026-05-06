#include <iostream>
#include <fstream>
#include <string>

#include "utils/inputConfiguration.h"

static void printUsage(const char* program) {
    std::cerr << "Uso:\n"
              << "  " << program << " <config.json>           carga configuracion desde archivo JSON\n"
              << "  " << program << " --json '<json_string>'  pasa el JSON como argumento de consola\n"
              << "\nCampos requeridos en el JSON:\n"
              << "  inference_engine   : \"LLAMA\" | \"OLLAMA\"\n"
              << "  test_type          : \"TYPE_0\" | \"TYPE_1\" | \"TYPE_2\"\n"
              << "  batch_size         : int\n"
              << "  context_size       : int\n"
              << "  seed               : int\n"
              << "  num_prompts        : int\n"
              << "  temperature        : float\n"
              << "  model_path_or_name : string\n"
              << "  hardware_period    : float (segundos entre mediciones)\n"
              << "  anotations         : string (opcional)\n"
              << "  ollama_url         : string (opcional, default: http://localhost:11434)\n";
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    nlohmann::json json_config;

    try {
        std::string first_arg(argv[1]);
        if (first_arg == "--json") {
            if (argc < 3) {
                std::cerr << "Error: --json requiere un argumento con el contenido JSON.\n";
                printUsage(argv[0]);
                return 1;
            }
            json_config = nlohmann::json::parse(argv[2]);
        } else {
            std::ifstream config_file(first_arg);
            if (!config_file.is_open()) {
                std::cerr << "Error: no se pudo abrir el archivo: " << first_arg << "\n";
                return 1;
            }
            config_file >> json_config;
            if (config_file.bad()) {
                std::cerr << "Error: fallo de I/O leyendo el archivo: " << first_arg << "\n";
                return 1;
            }
        }
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Error al parsear el JSON: " << e.what() << "\n";
        return 1;
    }

    try {
        InputConfiguration config(json_config);
        config.run();
    } catch (const std::exception& e) {
        std::cerr << "Error durante la ejecucion: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

