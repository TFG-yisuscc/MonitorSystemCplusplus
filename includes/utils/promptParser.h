//
// Created by yisus on 4/3/26.
//Header only por comodidad
//

#ifndef MONITORSYSTEM_PROMPTPARSER_H
#define MONITORSYSTEM_PROMPTPARSER_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include "prompt_data.h"

// #include "third_party/ollama.hpp"


class promptParser {
private:
    std::string filePath_;
    std::vector<std::string> prompts;

    void parseStream(std::istream& stream) {
        std::string line;
        while (std::getline(stream, line)) {
            if (line.empty()) continue;
            nlohmann::json jsonObj = nlohmann::json::parse(line);
            if (jsonObj.contains("prompt"))
                prompts.push_back(jsonObj["prompt"].get<std::string>());
        }
    }

public:
    promptParser() {
        filePath_ = "<embedded>";
        std::string data(embedded::prompt_data);
        std::istringstream stream(data);
        parseStream(stream);
    }

    explicit promptParser(std::string& filePath) {
        filePath_ = filePath;
        std::ifstream file(filePath_);
        if (!file.is_open())
            throw std::runtime_error("Cannot open file" + filePath_);
        parseStream(file);
    }

    explicit promptParser(const char* str) {
        filePath_ = str;
        std::ifstream file(filePath_);
        if (!file.is_open())
            throw std::runtime_error("Cannot open file" + filePath_);
        parseStream(file);
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