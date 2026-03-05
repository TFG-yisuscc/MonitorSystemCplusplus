//
// Created by yisus on 5/3/26.
//

#ifndef MONITORSYSTEM_PROMPTMETRICLOGGER_H
#define MONITORSYSTEM_PROMPTMETRICLOGGER_H
#include <fstream>
#include <iosfwd>

#include "promptmetrics.h"


class promptmetriclogger {
private:
    std::ofstream logfile_;
    std::string filepath_;

public:
    promptmetriclogger(std::string filepath);
    ~promptmetriclogger();
    bool write2jsonline(metrics::promptmetrics pm);
};


#endif //MONITORSYSTEM_PROMPTMETRICLOGGER_H