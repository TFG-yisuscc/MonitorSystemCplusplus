#include "../includes/third_party/ollama.hpp"


#include <iostream>
#include <string>
#include <functional>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>
#include "../includes/metrics/promptmetrics.h"
#include "../includes/utils/promptParser.h"
#include "clients/OllamaTest.h"


std::atomic<bool> done(false);
// Función auxiliar para mostrar datos de rendimiento
void print_performance_metrics(const nlohmann::json& response_json)
{
    std::cout << "\n=== MÉTRICAS DE RENDIMIENTO ===" << std::endl;

    // Total duration (duración total en nanosegundos)
    if (response_json.contains("total_duration")) {
        auto total_ns = response_json["total_duration"].get<long long>();
        double total_ms = total_ns / 1000000.0;
        double total_s = total_ns / 1000000000.0;
        std::cout << "Duración total: " << std::fixed << std::setprecision(2)
                  << total_ms << " ms (" << total_s << " s)" << std::endl;
    }

    // Load duration (tiempo para cargar el modelo)
    if (response_json.contains("load_duration")) {
        auto load_ns = response_json["load_duration"].get<long long>();
        double load_ms = load_ns / 1000000.0;
        std::cout << "Tiempo de carga del modelo: " << std::fixed << std::setprecision(2)
                  << load_ms << " ms" << std::endl;
    }

    // Prompt eval duration (tiempo para procesar el prompt)
    if (response_json.contains("prompt_eval_duration")) {
        auto prompt_eval_ns = response_json["prompt_eval_duration"].get<long long>();
        double prompt_eval_ms = prompt_eval_ns / 1000000.0;
        std::cout << "Tiempo de evaluación del prompt: " << std::fixed << std::setprecision(2)
                  << prompt_eval_ms << " ms" << std::endl;
    }

    // Prompt eval count (número de tokens en el prompt)
    if (response_json.contains("prompt_eval_count")) {
        auto prompt_tokens = response_json["prompt_eval_count"].get<int>();
        std::cout << "Tokens en prompt: " << prompt_tokens << std::endl;
    }

    // Eval duration (tiempo para generar los tokens)
    if (response_json.contains("eval_duration")) {
        auto eval_ns = response_json["eval_duration"].get<long long>();
        double eval_ms = eval_ns / 1000000.0;
        std::cout << "Tiempo de generación: " << std::fixed << std::setprecision(2)
                  << eval_ms << " ms" << std::endl;
    }

    // Eval count (número de tokens generados)
    if (response_json.contains("eval_count")) {
        auto generated_tokens = response_json["eval_count"].get<int>();
        std::cout << "Tokens generados: " << generated_tokens << std::endl;
    }

    // Calcular tokens por segundo
    if (response_json.contains("eval_duration") && response_json.contains("eval_count")) {
        auto eval_ns = response_json["eval_duration"].get<long long>();
        auto eval_count = response_json["eval_count"].get<int>();
        if (eval_ns > 0) {
            double tokens_per_second = (eval_count / (eval_ns / 1000000000.0));
            std::cout << "Velocidad: " << std::fixed << std::setprecision(2)
                      << tokens_per_second << " tokens/segundo" << std::endl;
        }
    }

    std::cout << "==============================\n" << std::endl;
}

bool on_receive_response(const ollama::response& response)
{
    std::cout << response << std::flush;

    if (response.as_json()["done"]==true) {
        done=true;
        // Mostrar métricas de rendimiento cuando la respuesta esté completa
        print_performance_metrics(response.as_json());
        std::cout << std::endl;
    }

    return !done; // Return true to continue streaming this response; return false to stop immediately.
}


int main(){
//    nlohmann::json opts;
//     opts["options"]["temperature"] = 0;
//     promptParser aux = promptParser("../prompt_list/instruction_following_eval_promt.jsonl");
//     printf(aux.getPromptI(69).c_str());
//     ollama::request req("granite4:micro-h", aux.getPromptI(69),
//                         opts,
//                         false);
//
//     // Añadir campos extra directamente (hereda de json)
//     req["logprobs"] = true;
//     req["verbose"]  = true;
//     ollama::show_requests(true);
//     ollama::show_replies(true);
//
//     // Exceptions can be dynamically enabled and disabled through this call.
//     // If exceptions are true, ollama::exception will be thrown in the event of errors. If exceptions are false, functions will either return false or empty values.
//     ollama::allow_exceptions(true);
//
//
//
// int64_t tiinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
//     ollama::response respuesta = ollama::generate(req);
// int64_t tfinish = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
//     auto pm = metrics::promptmetrics::from_json_ollama(respuesta.as_json(),tiinicio,tfinish,-1);
//     std::cout << "model: " << pm.model << std::endl;
//     std::cout << "answer: " << pm.answer << std::endl;
//     std::cout << "total_duration_ns: " << pm.total_duration_ns << std::endl;
//     std::cout << "prompt_eval_count: " << pm.prompt_eval_count << std::endl;
//     std::cout << "prompt_eval_duration_ns: " << pm.prompt_eval_duration_ns << std::endl;
//     std::cout << "eval_count: " << pm.eval_count << std::endl;
//     std::cout << "eval_duration_ns: " << pm.eval_duration_ns << std::endl;
//     std::cout << "load_duration_ns: " << pm.logprobs << std::endl;
//     pm.write2jsonline("../results/metrics.jsonl");
 OllamaTest ot = OllamaTest("granite4:micro-h",0,512,2048,42,3);
    ot.runTestType1();

}