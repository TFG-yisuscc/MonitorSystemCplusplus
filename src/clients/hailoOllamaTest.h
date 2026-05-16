//
// Created by yisus on 15/5/26.
//

#ifndef MONITORSYSTEM_HAILOOLLAMATEST_H
#define MONITORSYSTEM_HAILOOLLAMATEST_H

#include <string>
#include "third_party/ollama.hpp"   // solo por nlohmann::json (vía promptmetrics.h)
#include "metrics/promptmetrics.h"

// Equivalent to OllamaTest but communicates directly with Hailo-Ollama
// via hailo_http_client (plain POSIX sockets). Uses /api/chat (not /api/generate,
// which is broken on Hailo-Ollama v0.5.1 — it leaks raw template tokens).
class HailoOllamaTest {
private:
    std::string model_name_;
    std::string filepath_;
    long long   test_id_;
    float       temperature_;
    int         batch_size_;
    int         context_size_;
    int         seed_;
    int         num_prompts_;
    double      hardwarePeriod_;
    std::string server_host_;   // e.g. "localhost" or "raspberrypi"
    int         server_port_;   // 8000

    void ensureModelAvailable();
    std::string build_chat_body(const std::string& prompt, bool stream) const;
    // Returns the done-line JSON parsed from /api/chat stream:false
    nlohmann::json chat(const std::string& prompt);

public:
    // server_host / server_port: "localhost"/8000 si el binario corre en la Pi,
    //                             "raspberrypi"/8000 para desarrollo remoto.
    HailoOllamaTest(std::string model_name, std::string filepath, float temperature,
                    int batch_size, int context_size, int seed, int num_prompts,
                    double hardwarePeriod = 0.5,
                    std::string server_host = "localhost", int server_port = 8000);
    HailoOllamaTest(nlohmann::json config);

    bool runTestType0();
    bool runTestType1();
    bool runTestType1_5seg();
};

#endif //MONITORSYSTEM_HAILOOLLAMATEST_H
