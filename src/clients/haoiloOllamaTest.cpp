//
// Created by yisus on 16/5/26.
//

#include "clients/haoiloOllamaTest.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <fmt/format.h>
#include "metrics/Logger.h"
#include "utils/hardwareMeasurements.h"

HaoiloOllamaTest::HaoiloOllamaTest(std::string model_name, std::string filepath, float temperature,
                                   int seed, int num_prompts, int num_predict,
                                   double hardwarePeriod, std::string host, int port)
    : model_name_(std::move(model_name)), filepath_(std::move(filepath)),
      temperature_(temperature), seed_(seed), num_prompts_(num_prompts),
      num_predict_(num_predict), hardwarePeriod_(hardwarePeriod),
      host_(std::move(host)), port_(port) {
    test_id_ = std::chrono::system_clock::now().time_since_epoch().count();
}

HaoiloOllamaTest::HaoiloOllamaTest(nlohmann::json config) {
    test_id_ = std::chrono::system_clock::now().time_since_epoch().count();
    model_name_ = config.at("model_name").get<std::string>();
    filepath_   = config.at("filepath").get<std::string>();
    temperature_ = config.at("temperature").get<float>();
    seed_        = config.at("seed").get<int>();
    num_prompts_ = config.at("num_prompts").get<int>();
    num_predict_ = config.value("num_predict", 512);
    hardwarePeriod_ = config.value("hardware_period", 0.5);
    host_ = config.value("host", std::string("raspberrypi"));
    port_ = config.value("port", 8000);
}

std::string HaoiloOllamaTest::create_request_json(const std::string& prompt) const {
    nlohmann::json req;
    req["model"]    = model_name_;
    req["messages"] = nlohmann::json::array({{{"role", "user"}, {"content", prompt}}});
    req["stream"]   = false;
    req["options"]["temperature"] = temperature_;
    req["options"]["seed"]        = seed_;
    req["options"]["num_predict"] = num_predict_;
    req["keep_alive"] = -1;
    return req.dump();
}

nlohmann::json HaoiloOllamaTest::generate(const std::string& prompt) {
    hailo_http::Client client(host_, port_);
    auto resp = client.post("/api/chat", create_request_json(prompt));
    if (resp.status_code != 200)
        throw std::runtime_error("hailo-ollama: HTTP " + std::to_string(resp.status_code) + " — " + resp.body);
    return nlohmann::json::parse(resp.body);
}

bool HaoiloOllamaTest::runTestType0() {
    promptParser parser;
    auto prompts = parser.getPrompts();
    std::string log_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_hailo_test0.jsonl", test_id_, model_name_);
    Logger logger(log_file);

    for (int i = 0; i < num_prompts_ && i < (int)prompts.size(); ++i) {
        int64_t t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        auto result = generate(prompts.at(i));
        int64_t t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        logger.write2jsonline(metrics::promptmetrics::from_HailoOllama_json(result, t_start, t_end, i));
    }
    return true;
}

bool HaoiloOllamaTest::runTestType1() {
    promptParser parser;
    auto prompts = parser.getPrompts();
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_hailo_test1.jsonl", test_id_, model_name_);
    std::string log_hw_file     = filepath_ + fmt::format("/{}_hw_metrics_{}_hailo_test1.jsonl",     test_id_, model_name_);
    Logger logger(log_prompt_file);
    HardwareMeasurements hwMonitor(log_hw_file, hardwarePeriod_);

    std::thread hwThread([&hwMonitor]() {
        try { hwMonitor.start(); }
        catch (const std::exception& e) { std::cerr << "HW monitor error: " << e.what() << "\n"; }
        catch (...) { std::cerr << "HW monitor: unknown error\n"; }
    });
    try {
        for (int i = 0; i < num_prompts_ && i < (int)prompts.size(); ++i) {
            int64_t t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            auto result = generate(prompts.at(i));
            int64_t t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            logger.write2jsonline(metrics::promptmetrics::from_HailoOllama_json(result, t_start, t_end, i));
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

bool HaoiloOllamaTest::runTestType1_5seg() {
    promptParser parser;
    auto prompts = parser.getPrompts();
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_hailo_test1_5seg.jsonl", test_id_, model_name_);
    std::string log_hw_file     = filepath_ + fmt::format("/{}_hw_metrics_{}_hailo_test1_5seg.jsonl",     test_id_, model_name_);
    Logger logger(log_prompt_file);
    HardwareMeasurements hwMonitor(log_hw_file, hardwarePeriod_);

    std::thread hwThread([&hwMonitor]() {
        try { hwMonitor.start(); }
        catch (const std::exception& e) { std::cerr << "HW monitor error: " << e.what() << "\n"; }
        catch (...) { std::cerr << "HW monitor: unknown error\n"; }
    });
    try {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        for (int i = 0; i < num_prompts_ && i < (int)prompts.size(); ++i) {
            int64_t t_start = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            auto result = generate(prompts.at(i));
            int64_t t_end = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            logger.write2jsonline(metrics::promptmetrics::from_HailoOllama_json(result, t_start, t_end, i));
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
