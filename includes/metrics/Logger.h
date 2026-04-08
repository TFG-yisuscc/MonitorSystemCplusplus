//
// Created by yisus on 5/3/26.
//

#ifndef MONITORSYSTEM_PROMPTMETRICLOGGER_H
#define MONITORSYSTEM_PROMPTMETRICLOGGER_H
#include <fstream>
#include <iosfwd>

#include "hardwareMetrics.h"
#include "promptmetrics.h"


class Logger {
private:
    std::ofstream logfile_;
    std::string filepath_;

public:
    Logger(std::string filepath);
    ~Logger();


    bool write2jsonline(metrics::promptmetrics);
    bool write2jsonline(hardwareMetrics);

};


#endif //MONITORSYSTEM_PROMPTMETRICLOGGER_H