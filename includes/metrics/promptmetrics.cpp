//
// Created by yisus on 3/3/26.
//

#include "promptmetrics.h"

/*void print_performance_metrics(const nlohmann::json& response_json)
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
}*/
namespace metrics {
    promptmetrics promptmetrics::from_json_ollama(nlohmann::json json,int64_t start_timestamp,int64_t finish_timestamp,int prompt_id = -1) {
        // Implementación de la función para crear un objeto promptmetrics a partir de un JSON de Ollama
        // Aquí deberías parsear el JSON y extraer los campos necesarios para construir el objeto promptmetrics

        std::string model = json.contains("model")?json["model"].get<std::string>():"DEFAULT_MODEL";
        int64_t total_duration_ns = json.contains("total_duration")? json["total_duration"].get<int64_t>():-1 ;
        int64_t prompt_eval_count = json.contains("prompt_eval_count")? json["prompt_eval_count"].get<int64_t>():-1;
        int64_t prompt_eval_duration_ns = json.contains("prompt_eval_duration")? json["prompt_eval_duration"].get<int64_t>():-1;
        int64_t eval_count = json.contains("eval_count")? json["eval_count"].get<int64_t>():-1;
        int64_t eval_duration_ns = json.contains("eval_duration")? json["eval_duration"].get<int64_t>():-1;
        int64_t load_duration_ns = json.contains("load_duration")? json["load_duration"].get<int64_t>():-1;
        std::string answer = json.contains("response")? json["response"].get<std::string>():"NONE";


        return promptmetrics(start_timestamp, finish_timestamp, model, total_duration_ns,
                             prompt_eval_count, prompt_eval_duration_ns, eval_count,
                             eval_duration_ns, load_duration_ns, answer, prompt_id);
    }
} // metrics