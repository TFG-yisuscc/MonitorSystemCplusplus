//
// Created by yisus on 4/3/26.
//Header only por comodidad
//

#ifndef MONITORSYSTEM_PROMPTPARSER_H
#define MONITORSYSTEM_PROMPTPARSER_H
#include <format>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

// #include "third_party/ollama.hpp"
#include <nlohmann/json.hpp>

class promptParser {
private:
    std::string filePath_;
    std::vector<std::string> prompts;

public:
    explicit promptParser(std::string& filePath) {
        filePath_ = filePath;
        std::vector<std::string>   aux =  std::vector<std::string>();
        // 1 leemos el json
        std::ifstream file(filePath_);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file" + filePath_);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            nlohmann::json jsonObj = nlohmann::json::parse(line);
            if (jsonObj.contains("prompt")) {
                prompts.push_back(jsonObj["prompt"].get<std::string>());
            }
        }

    }

    explicit promptParser(const char * str){
        filePath_ = str;
        std::vector<std::string>   aux =  std::vector<std::string>();
        // 1 leemos el json
        std::ifstream file(filePath_);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file" + filePath_);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            nlohmann::json jsonObj = nlohmann::json::parse(line);
            if (jsonObj.contains("prompt")) {
                prompts.push_back(jsonObj["prompt"].get<std::string>());
            }
        }// constructor





    }

    [[nodiscard]] std::string file_path() const {
        return filePath_;
    }

    [[nodiscard]] std::vector<std::string> getPrompts() const {
        return prompts;
    }
    std::string getPromptI(int i) {
        return prompts.at(i);
    }
    int getNPrompts() {
        return prompts.size();
    }
};
#endif //MONITORSYSTEM_PROMPTPARSER_H