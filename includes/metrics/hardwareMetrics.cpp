//
// Created by yisus on 5/3/26.
//

#include "hardwareMetrics.h"
#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>
#include <filesystem>
#include <algorithm>
#include <cstdio>

// Definición del miembro estático


hardwareMetrics::hardwareMetrics(const InferenceEngines engine) {
    this->engine_ = engine;
    update();
}

void hardwareMetrics::update() {
    timestamp_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    fetchMemoryMetrics();
    fetchCpuMetrics();
    fetchSystemMetrics();
}

// ─── RAM & Swap ────────────────────────────────────────────────────────────────
void hardwareMetrics::fetchMemoryMetrics() {
    //TODO metricas de memoria

}

// ─── CPU ───────────────────────────────────────────────────────────────────────
void hardwareMetrics::fetchCpuMetrics() {
    //TODO metricas de cpu
}

double hardwareMetrics::getCpuUsageForPid(pid_t pid) {
        //TODO uso de cpu por proceso
        return 0.0;
}

// ─── Sistema (temperatura, frecuencia, voltaje, fan, throttling) ───────────────
void hardwareMetrics::fetchSystemMetrics() {
        //TODO metricas de sistema
}

// ─── Helpers ───────────────────────────────────────────────────────────────────
pid_t hardwareMetrics::getEnginePid() const {
    //TODO obtener el PID del proceso del motor de inferencia
    return 0;
}
