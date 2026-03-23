//
// Created by yisus on 5/3/26.
//

#include "Logger.h"

#include <iostream>

Logger::Logger(std::string filepath) {
    filepath_ = filepath;
    logfile_.open(filepath_, std::ios::app);
    if (!logfile_.is_open()) {
        //std::cerr << "ERROR opening the file " << filepath_ << std::endl;
        throw std::runtime_error("Error opening the file");
    }
}

Logger::~Logger() {
    logfile_.flush();
    logfile_.close();
}

 bool Logger::write2jsonline(metrics::promptmetrics pm) {
    nlohmann::json jsonObj = pm;
    if (!logfile_.is_open()) {
        std::cerr << "Error al abrir el archivo: " << filepath_ << std::endl;
        return false;
    }
    logfile_<< jsonObj.dump() << std::endl;
    logfile_.flush();
    //file.close();
    return true;
}

