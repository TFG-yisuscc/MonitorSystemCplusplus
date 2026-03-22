//
// Created by yisus on 13/3/26.
//

#ifndef MONITORSYSTEM_MISC_H
#define MONITORSYSTEM_MISC_H
#include <array>
#include <stdexcept>
#include <string>

/*
std::string output;

if (FILE* pipe = popen("vcgencmd measure_temp", "r")) {
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    pclose(pipe);
}

const auto eq_pos = output.find('=');
const auto c_pos = output.find("'C");
if (eq_pos != std::string::npos && c_pos != std::string::npos && c_pos > eq_pos +1) {
    this->temperature_ = std::stod(output.substr(eq_pos +1, c_pos - (eq_pos +1)));
}
*/

inline std::string execCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    try {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

#endif //MONITORSYSTEM_MISC_H