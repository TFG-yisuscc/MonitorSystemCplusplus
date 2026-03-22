//
// Created by yisus on 12/3/26.
//
#pragma once
#ifndef MONITORSYSTEM_CPUMONITOR_H
#define MONITORSYSTEM_CPUMONITOR_H
#include <fstream>
#include "utils/cpuTicks.h"
#include <unistd.h>

class CpuMonitor {// no sde si me merece la pena organizarla en un a clase aparte
public:
    static double getSystemCpuPercent() {

        static cpu_ticks ct_prev{};
        cpu_ticks actual = getCpuTimes();

        double cpu_percent = 0.0;
        int64_t total_diff = actual.total() - ct_prev.total();
        int64_t active_diff = actual.active() - ct_prev.active();
        if (total_diff > 0) {
            cpu_percent = 100.0 * active_diff / total_diff;
    }
        ct_prev = actual;
        return cpu_percent;
    }

};


#endif //MONITORSYSTEM_CPUMONITOR_H