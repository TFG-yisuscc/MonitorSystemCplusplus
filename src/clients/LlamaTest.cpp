//
// Created by yisus on 5/3/26.
//

#include "clients/LlamaTest.h"

#include <chrono>
#include <iostream>
#include <fmt/format.h>

#include "metrics/Logger.h"
#include "metrics/promptmetrics.h"
#include "utils/hardwareMeasurements.h"
#include "utils/LlamaInferencer.h"
#include "utils/promptParser.h"
using namespace metrics;
LlamaTest::LlamaTest(std::string filepath, std::string model_path, float temperature, int batch_size, int context_size, int seed, int num_prompts, double hardwarePeriod) {
    test_id = std::chrono::system_clock::now().time_since_epoch().count();
    filepath_ = filepath;
    model_path_ = model_path;
    temperature_ = temperature;
    batch_size_ = batch_size;
    context_size_ = context_size;
    seed_ = seed;
    num_prompts_ = num_prompts;
    hardwarePeriod_ = hardwarePeriod;
}

LlamaTest::LlamaTest(nlohmann::json configLlama) {
    test_id = std::chrono::system_clock::now().time_since_epoch().count();
    model_path_= configLlama.contains("model_path")? configLlama["model_path"].get<std::string>():
    throw std::runtime_error("model_path is required in the config");
    temperature_ = configLlama.contains("temperature")? configLlama["temperature"].get<float>():throw std::runtime_error("temperature is required in the config");
    batch_size_ = configLlama.contains("batch_size")? configLlama["batch_size"].get<int>():throw std::runtime_error("batch_size is required in the config");
    context_size_ = configLlama.contains("context_size")? configLlama["context_size"].get<int>():throw std::runtime_error("context_size is required in the config");
    seed_ = configLlama.contains("seed")? configLlama["seed"].get<int>():throw std::runtime_error("seed is required in the config");
    num_prompts_ = configLlama.contains("num_prompts")? configLlama["num_prompts"].get<int>():throw std::runtime_error("num_prompts is required in the config");
}

bool LlamaTest::runTestType0() {
//parser y logger ( //)
    promptParser parser2 = promptParser();
    std::vector<std::string> prompts = parser2.getPrompts();
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_test1.jsonl",test_id,getCleanModelPath());
    Logger promptLogger(log_prompt_file);
//carga
    LlamaInferencer inferencer(model_path_, temperature_, batch_size_, context_size_, seed_);


// bucle
    for (int i = 0; i < num_prompts_; i++) {
        if (i == 0) {
            /*No me molseto en sacarlo del bucle esta condición inicial porque en tería
             *el compilador es lo suficientemente listo como para hacerlo el solito
             *¿y quien soy yo para contradecirle?
             *Bueno si me lo piden los jefes lo hago claro
             */
            auto llt = inferencer.loadModel();//con ollama se carga directamente en el primer punto así que no me preocupa
           auto llg = inferencer.generateTextCompletion(prompts.at(i));
            promptLogger.write2jsonline(promptmetrics::from_Llama(llt, llg, i));
        }else {
            auto llg = inferencer.generateTextCompletion(prompts.at(i));
            promptLogger.write2jsonline(promptmetrics::from_Llama(llg, i));
        }
    //std::cout << inferencer.generateTextCompletion(prompts.at(i)).answer << std::endl;
}
//cierre
inferencer.unloadModel();
return true;
}
bool LlamaTest::runTestType1() {
    promptParser parser2 = promptParser();
    std::vector<std::string> prompts = parser2.getPrompts();
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_test1.jsonl",test_id,getCleanModelPath());
    Logger promptLogger(log_prompt_file);
    std::string log_hw_file = filepath_ + fmt::format("/{}_hw_metrics_{}_test1.jsonl", test_id, getCleanModelPath());
    LlamaInferencer inferencer(model_path_, temperature_, batch_size_, context_size_, seed_);
    HardwareMeasurements hwMonitor(log_hw_file, hardwarePeriod_);

    std::thread hwThread([&hwMonitor]() {
        try { hwMonitor.start(); }
        catch (const std::exception& e) { std::cerr << "HW monitor error: " << e.what() << std::endl; }
        catch (...) { std::cerr << "HW monitor: unknown error" << std::endl; }
    });
    try {
        for (int i = 0; i < num_prompts_; i++) {
            if (i == 0) {
                auto llt = inferencer.loadModel();
                auto llg = inferencer.generateTextCompletion(prompts.at(i));
                promptLogger.write2jsonline(promptmetrics::from_Llama(llt, llg, i));
            } else {
                auto llg = inferencer.generateTextCompletion(prompts.at(i));
                promptLogger.write2jsonline(promptmetrics::from_Llama(llg, i));
            }
        }
        inferencer.unloadModel();
    } catch (...) {
        hwMonitor.stop();
        hwThread.join();
        throw;
    }
    hwMonitor.stop();
    hwThread.join();
    return true;
}
bool LlamaTest::runTestType1_5seg() {
    promptParser parser2 = promptParser();
    std::vector<std::string> prompts = parser2.getPrompts();
    std::string log_prompt_file = filepath_ + fmt::format("/{}_prompt_metrics_{}_test1.jsonl",test_id,getCleanModelPath());
    Logger promptLogger(log_prompt_file);
    std::string log_hw_file = filepath_ + fmt::format("/{}_hw_metrics_{}_test1.jsonl", test_id, getCleanModelPath());
    LlamaInferencer inferencer(model_path_, temperature_, batch_size_, context_size_, seed_);
    HardwareMeasurements hwMonitor(log_hw_file, hardwarePeriod_);

    std::thread hwThread([&hwMonitor]() {
        try { hwMonitor.start(); }
        catch (const std::exception& e) { std::cerr << "HW monitor error: " << e.what() << std::endl; }
        catch (...) { std::cerr << "HW monitor: unknown error" << std::endl; }
    });
    try {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        for (int i = 0; i < num_prompts_; i++) {
            if (i == 0) {
                auto llt = inferencer.loadModel();
                auto llg = inferencer.generateTextCompletion(prompts.at(i));
                promptLogger.write2jsonline(promptmetrics::from_Llama(llt, llg, i));
            } else {
                auto llg = inferencer.generateTextCompletion(prompts.at(i));
                promptLogger.write2jsonline(promptmetrics::from_Llama(llg, i));
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        inferencer.unloadModel();
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

// privadas
std::string LlamaTest::getCleanModelPath() {
    std::filesystem::path p(model_path_);
    if (p.extension() == ".gguf")
        return p.stem().string();
    return p.filename().string();
}