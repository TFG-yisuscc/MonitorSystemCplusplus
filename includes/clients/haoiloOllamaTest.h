//
// Created by yisus on 16/5/26.
//

#ifndef MONITORSYSTEM_HAOILOOLLAMATEST_H
#define MONITORSYSTEM_HAOILOOLLAMATEST_H

#include <string>
#include "third_party/hailo_http_client.h"
#include "utils/promptParser.h"
#include "metrics/promptmetrics.h"

class HaoiloOllamaTest {
private:
    std::string model_name_;
    std::string filepath_;
    long long test_id_;
    float temperature_;
    int seed_;
    int num_prompts_;
    int num_predict_;
    double hardwarePeriod_;
    std::string host_;
    int port_;

    std::string create_request_json(const std::string& prompt) const;
    nlohmann::json generate(const std::string& prompt);

public:
    HaoiloOllamaTest(std::string model_name, std::string filepath, float temperature,
                     int seed, int num_prompts, int num_predict = 512,
                     double hardwarePeriod = 0.5,
                     std::string host = "raspberrypi", int port = 8000);
    explicit HaoiloOllamaTest(nlohmann::json config);

    bool runTestType0();
    bool runTestType1();
    bool runTestType1_5seg();
};

#endif //MONITORSYSTEM_HAOILOOLLAMATEST_H
