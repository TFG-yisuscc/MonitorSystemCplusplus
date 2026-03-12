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
    //TODO metricas de memoria especificas al engine
    // que como ya adelantamos de monitoer system por culpa  ollama es algo mas complejo

    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) throw std::runtime_error("Cannot open /proc/meminfo");
    int64_t mem_free = 0, mem_available = 0, mem_buffers = 0, mem_cached = 0;
    int64_t swap_free = 0;
    std::string key;
    int64_t value;
    std::string unit;
    while (file >> key >> value >> unit) {
        if      (key == "MemTotal:")     this->mem_total_ = value * 1024;
        else if (key == "MemAvailable:") mem_available     = value * 1024;
        else if (key == "Buffers:")      mem_buffers       = value * 1024;
        else if (key == "Cached:")       mem_cached        = value * 1024;
        else if (key == "MemFree:")      mem_free          = value * 1024;
        else if (key == "SwapTotal:")    this->swap_total_ = value * 1024;
        else if (key == "SwapFree:")     swap_free         = value * 1024;
    }
    this->mem_used_     =  this->mem_total_ - mem_available;
    this->mem_percent_  = (this->mem_total_ > 0)
                          ? 100.0 * this->mem_used_ / this->mem_total_
                          : 0.0;

    this->swap_used_    = this->swap_total_ - swap_free;
    this->swap_percent_ = (this->swap_total_ > 0)
                          ? 100.0 * this->swap_used_ / this->swap_total_
                          : 0.0;

}

// ─── CPU ───────────────────────────────────────────────────────────────────────
void hardwareMetrics::fetchCpuMetrics() {
    //TODO metricas de cpu
    // recordar lo de la 2 llamada por los deltas
    this->cpu_ticks_ = readCpuTimes();
     this->cpu_usage_ = CpuMonitor::getSystemCpuPercent();
     this->cpu_usage_engine_ = CpuMonitor::getEngineCpuPercent();
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
