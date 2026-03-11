//
// Created by yisus on 5/3/26.
//

#ifndef MONITORSYSTEM_HARDWAREMETRICS_H
#define MONITORSYSTEM_HARDWAREMETRICS_H
#include <cstdint>
#include <string>
#include <unistd.h>
#include "utils/InferenceEngines.h"
#include <nlohmann/json.hpp>

class hardwareMetrics {
private:
    InferenceEngines engine_;   // ← quitar static

    void fetchMemoryMetrics();
    void fetchCpuMetrics();
    void fetchSystemMetrics();
    double getCpuUsageForPid(pid_t pid);
    pid_t getEnginePid() const;

public:
    int64_t timestamp_;
    //int promptID; //esta creo que es mjor quitarla ya q ue tenemos los tiimestamps
    //operacion
    double temperature_;
    double frequency_;
    double voltage_;
    double fan_speed_;
    uint32_t throttling_; //recuerda compo de bits de 20 bits
    // memorias
    int64_t mem_total_;
    int64_t mem_used_;
    double mem_percent_;
    double mem_percent_engine_;
    //swap
    int64_t swap_total_;
    int64_t swap_used_;
    double swap_percent_;
    //cpu
    double cpu_usage_;
    double cpu_usage_engine_;
    // el conversor a json
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(hardwareMetrics,
                                   timestamp_,
                                   temperature_,
                                   frequency_,
                                   voltage_,
                                   fan_speed_,
                                   throttling_,
                                   mem_total_,
                                   mem_used_,
                                   mem_percent_,
                                   mem_percent_engine_,
                                   swap_total_,
                                   swap_used_,
                                   swap_percent_,
                                   cpu_usage_,
                                   cpu_usage_engine_,
                                   engine_);

    // constructor y destructor
    hardwareMetrics(const InferenceEngines engine);
    // actualiza
     void update();
    // setters y getters
        InferenceEngines getEngine() const;
        void setEngine(const InferenceEngines &engine);
};



#endif //MONITORSYSTEM_HARDWAREMETRICS_H
