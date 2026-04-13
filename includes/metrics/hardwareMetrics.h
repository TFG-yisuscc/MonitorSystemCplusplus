//
// Created by yisus on 5/3/26.
//
#pragma once
#ifndef MONITORSYSTEM_HARDWAREMETRICS_H
#define MONITORSYSTEM_HARDWAREMETRICS_H
#include <cstdint>
#include <string>
#include <unistd.h>
#include <vector>

#include "utils/InferenceEngines.h"
#include <sys/resource.h>
#include "utils/cpuMonitor.h"
#include "third_party/ollama.hpp" //porque incluye la librería de nholamasn de jason e uinterfiere con la versión de vcpkg
struct throttlingInfo {
    bool under_voltage;
    bool freq_capped;
    bool throttled;
    bool soft_throttled;


    // otros bits de throttling pueden ser añadidos aquí
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(throttlingInfo, under_voltage,
        freq_capped, throttled, soft_throttled)
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(cpu_ticks, user, nice, system, idle, iowait, irq, softirq, steal)
class hardwareMetrics {
public:
    int64_t timestamp_; //hecho
    double temperature_; //hecho
    std::vector<double> frequency_; //hecho
    double voltage_; //hecho
    double fan_speed_; //hecho
    throttlingInfo throttling_; // hecho recuerda compo de bits de 20 bits
    double internalpower_; //hecho unidades en vatios
    int64_t mem_total_; // hecho
    int64_t mem_used_; // hecho
    double mem_percent_; //hecho
    //swap
    int64_t swap_total_; // hecho
    int64_t swap_used_; // hecho
    double swap_percent_; // hecho
    //cpu
    double cpu_usage_; //hecho
    cpu_ticks cpu_ticks_; // hecho
    // el conversor a json
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(hardwareMetrics,
                                   timestamp_,
                                   temperature_,
                                   frequency_,
                                   voltage_,
                                   fan_speed_,
                                   throttling_,
                                   internalpower_,
                                   mem_total_,
                                   mem_used_,
                                   mem_percent_,
                                   swap_total_,
                                   swap_used_,
                                   swap_percent_,
                                   cpu_usage_,
                                   cpu_ticks_,
                                   engine_);

    // constructor y destructor
    hardwareMetrics(const InferenceEngines engine); // creo que este va aser irrelevante

    hardwareMetrics();

    // actualiza
     void update();
    // setters y getters
        InferenceEngines getEngine() const;
        void setEngine(const InferenceEngines &engine);
private:
    InferenceEngines engine_; //TODO mirar si merece la pena mantenerlo u eliminarlo
    //metodos privados
    void fetchMemoryMetrics();
    void fetchCpuMetrics();
    void fetchSystemMetrics();
    double getSystemCpuPercent();
    static  int getCpuCores();
    std::vector<double> getCpuFrequencies();
    static double getCoreVoltage();
    static  long getFanSpeed();
    static throttlingInfo getThrottlingInfo();
    static double getPower();

    //deprecated functions
    double getCpuUsageForPid(pid_t pid);//drepecated
    pid_t getEnginePid() const; //drepecated
    cpu_ticks getEngineTicks() const; //drepecated
    void getEngineCpuPercent( ); //drepecated

};




#endif //MONITORSYSTEM_HARDWAREMETRICS_H
