//
// Created by yisus on 15/5/26.
//

#include "../../src/clients/hailoOllamaTest.h"
#include "third_party/hailo_http_client.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <fmt/format.h>
#include "metrics/Logger.h"
#include "utils/hardwareMeasurements.h"
#include "utils/promptParser.h"

// ── Constructores ────────────────────────────────────────────────────────────

HailoOllamaTest::HailoOllamaTest(std::string model_name, std::string filepath, float temperature,
                                   int batch_size, int context_size, int seed, int num_prompts,
                                   double hardwarePeriod, std::string server_host, int server_port)
{
    test_id_        = std::chrono::system_clock::now().time_since_epoch().count();
    model_name_     = std::move(model_name);
    filepath_       = std::move(filepath);
    temperature_    = temperature;
    batch_size_     = batch_size;
    context_size_   = context_size;
    seed_           = seed;
    num_prompts_    = num_prompts;
    hardwarePeriod_ = hardwarePeriod;
    server_host_    = std::move(server_host);
    server_port_    = server_port;
}

HailoOllamaTest::HailoOllamaTest(nlohmann::json cfg)
{
    test_id_      = std::chrono::system_clock::now().time_since_epoch().count();
    model_name_   = cfg.contains("model_name")   ? cfg["model_name"].get<std::string>()   : throw std::runtime_error("model_name is required");
    temperature_  = cfg.contains("temperature")  ? cfg["temperature"].get<float>()         : throw std::runtime_error("temperature is required");
    batch_size_   = cfg.contains("batch_size")   ? cfg["batch_size"].get<int>()            : throw std::runtime_error("batch_size is required");
    context_size_ = cfg.contains("context_size") ? cfg["context_size"].get<int>()          : throw std::runtime_error("context_size is required");
    seed_         = cfg.contains("seed")         ? cfg["seed"].get<int>()                  : throw std::runtime_error("seed is required");
    num_prompts_  = cfg.contains("num_prompts")  ? cfg["num_prompts"].get<int>()           : throw std::runtime_error("num_prompts is required");
    hardwarePeriod_ = cfg.contains("hardwarePeriod") ? cfg["hardwarePeriod"].get<double>() : 0.5;
    server_host_  = cfg.contains("server_host")  ? cfg["server_host"].get<std::string>()  : "localhost";
    server_port_  = cfg.contains("server_port")  ? cfg["server_port"].get<int>()          : 8000;
}

// ── Helpers privados ─────────────────────────────────────────────────────────

void HailoOllamaTest::ensureModelAvailable() {
    hailo_http::Client http(server_host_, server_port_);
    auto resp = http.get("/api/tags");
    if (resp.status_code != 200)
        throw std::runtime_error("Hailo-Ollama /api/tags returned HTTP " +
                                 std::to_string(resp.status_code));

    auto j = nlohmann::json::parse(resp.body);
    for (const auto& m : j.value("models", nlohmann::json::array())) {
        std::string name = m.value("name", "");
        if (name == model_name_) return;
        if (model_name_.find(':') == std::string::npos && name == model_name_ + ":latest") return;
    }
    throw std::runtime_error(
        "Modelo '" + model_name_ + "' no encontrado en Hailo-Ollama (" +
        server_host_ + ":" + std::to_string(server_port_) + "). "
        "Instálalo con el paquete hailo_gen_ai_model_zoo.");
}

std::string HailoOllamaTest::build_chat_body(const std::string& prompt, bool stream) const {
    // num_ctx y num_batch NO se envían: en Hailo están fijados en el HEF en tiempo de compilación.
    // Solo temperature y seed son parámetros de sampling que el servidor puede honrar en runtime.
    nlohmann::json body;
    body["model"]    = model_name_;
    body["stream"]   = stream;
    body["messages"] = nlohmann::json::array({
        nlohmann::json{{"role", "user"}, {"content", prompt}}
    });
    body["options"]["temperature"] = temperature_;
    body["options"]["seed"]        = seed_;
    return body.dump();
}

// Uses streaming to measure prompt_eval_duration and eval_duration client-side.
// Returns a JSON object with the same fields as the Hailo-Ollama done-line,
// augmented with prompt_eval_duration and eval_duration from measured timestamps.
nlohmann::json HailoOllamaTest::chat(const std::string& prompt) {
    hailo_http::Client http(server_host_, server_port_, 3600);

    int64_t t_first = 0;   // timestamp of first token line
    int64_t t_last  = 0;   // timestamp of the done:true line

    nlohmann::json done_line;
    std::string full_content;
    int64_t stream_eval_count = 0;

    http.post_streaming("/api/chat", build_chat_body(prompt, /*stream=*/true),
        [&](const hailo_http::StreamLine& sl) -> bool {
            auto j = nlohmann::json::parse(sl.text, nullptr, /*throw=*/false);
            if (j.is_discarded()) return true;  // línea rota, seguir

            if (j.value("done", false)) {
                t_last    = sl.ts_ns;
                done_line = j;
                return false;  // cerrar socket inmediatamente
            }
            if (t_first == 0) t_first = sl.ts_ns;
            if (j.contains("message") && j["message"].contains("content"))
                full_content += j["message"]["content"].get<std::string>();
            ++stream_eval_count;
            return true;
        });

    if (done_line.is_null())
        throw std::runtime_error("Hailo-Ollama: no done line received for prompt");

    done_line["message"] = {{"role", "assistant"}, {"content", full_content}};
    // eval_duration: resta de relojes iguales (CLOCK_MONOTONIC), precisa.
    // prompt_eval_duration/count y load_duration no los reporta el servidor: se dejan a -1 en from_HailoOllama_json.
    done_line["eval_duration"] = (t_first > 0 && t_last > t_first) ? (t_last - t_first) : 0;
    if (!done_line.contains("eval_count"))
        done_line["eval_count"] = stream_eval_count;

    return done_line;
}

// ── Tests ────────────────────────────────────────────────────────────────────

bool HailoOllamaTest::runTestType0() {
    ensureModelAvailable();

    promptParser parser;
    std::vector<std::string> prompts = parser.getPrompts();
    std::string log_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_hailo_test0.jsonl",
                                                    test_id_, model_name_);
    Logger promptLogger(log_file);

    for (int i = 0; i < num_prompts_ && i < (int)prompts.size(); ++i) {
        int64_t tinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        auto json_resp = chat(prompts.at(i));
        int64_t tfinal = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        promptLogger.write2jsonline(
            metrics::promptmetrics::from_HailoOllama_json(json_resp, tinicio, tfinal, i));
    }
    return true;
}

bool HailoOllamaTest::runTestType1() {
    ensureModelAvailable();

    promptParser parser;
    std::vector<std::string> prompts = parser.getPrompts();
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_hailo_test1.jsonl",
                                                           test_id_, model_name_);
    std::string log_hw_file     = filepath_ + fmt::format("/{}_hw_metrics_{}_hailo_test1.jsonl",
                                                           test_id_, model_name_);
    Logger promptLogger(log_prompt_file);
    HardwareMeasurements hwMonitor(log_hw_file, hardwarePeriod_);

    std::thread hwThread([&hwMonitor]() {
        try { hwMonitor.start(); }
        catch (const std::exception& e) { std::cerr << "HW monitor error: " << e.what() << "\n"; }
        catch (...) { std::cerr << "HW monitor: unknown error\n"; }
    });

    try {
        for (int i = 0; i < num_prompts_ && i < (int)prompts.size(); ++i) {
            int64_t tinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            auto json_resp = chat(prompts.at(i));
            int64_t tfinal = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            promptLogger.write2jsonline(
                metrics::promptmetrics::from_HailoOllama_json(json_resp, tinicio, tfinal, i));
        }
    } catch (...) {
        hwMonitor.stop();
        hwThread.join();
        throw;
    }
    hwMonitor.stop();
    hwThread.join();
    return true;
}

bool HailoOllamaTest::runTestType1_5seg() {
    ensureModelAvailable();

    promptParser parser;
    std::vector<std::string> prompts = parser.getPrompts();
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_hailo_test1_5seg.jsonl",
                                                           test_id_, model_name_);
    std::string log_hw_file     = filepath_ + fmt::format("/{}_hw_metrics_{}_hailo_test1_5seg.jsonl",
                                                           test_id_, model_name_);
    Logger promptLogger(log_prompt_file);
    HardwareMeasurements hwMonitor(log_hw_file, hardwarePeriod_);

    std::thread hwThread([&hwMonitor]() {
        try { hwMonitor.start(); }
        catch (const std::exception& e) { std::cerr << "HW monitor error: " << e.what() << "\n"; }
        catch (...) { std::cerr << "HW monitor: unknown error\n"; }
    });

    try {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        for (int i = 0; i < num_prompts_ && i < (int)prompts.size(); ++i) {
            int64_t tinicio = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            auto json_resp = chat(prompts.at(i));
            int64_t tfinal = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            promptLogger.write2jsonline(
                metrics::promptmetrics::from_HailoOllama_json(json_resp, tinicio, tfinal, i));
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    } catch (...) {
        hwMonitor.stop();
        hwThread.join();
        throw;
    }
    hwMonitor.stop();
    hwThread.join();
    return true;
}
