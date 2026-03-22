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
#include "utils/misc.h"
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
//TODO resolver todo el pifostio de las cpu  que me esta hasta doliendo la cabeza
void hardwareMetrics::fetchCpuMetrics() {
    //TODO metricas de cpu

    this->cpu_ticks_ = getCpuTimes();
     this->cpu_usage_ = CpuMonitor::getSystemCpuPercent();
     //this->cpu_usage_engine_ = CpuMonitor::getEngineCpuPercent();
}



// ─── Sistema (temperatura, frecuencia, voltaje, fan, throttling) ───────────────
void hardwareMetrics::fetchSystemMetrics() {
        //TODO metricas de sistema

    std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
    if (!file.is_open())
        throw std::runtime_error("No se pudo abrir el sensor de temperatura");
    int auxTemp;
    file >> auxTemp;
    this->temperature_ = auxTemp / 1000.0;

    // frecuencia
   this->frequency_ = getCpuFrequencies();


    //voltaje
    //TODO  ver sie es posible hacerlo agnostico al la plataforma hardware
    /*
    * cmd_output = subprocess.check_output(["vcgencmd", "measure_volts", "core"]).decode("utf-8")
    self.voltage = float(cmd_output.split("=")[1][:-2])
     */

        this->voltage_ = getCoreVoltage();

    // ventilador
    this->fan_speed_ = getFanSpeed();

    // throttling
    /*
    *        cmd_output = subprocess.check_output(["vcgencmd", "get_throttled"]).decode("utf-8")
    *self.throttling = int(cmd_output.strip().split("=")[1].split("\"")[0], 16)
    *
    */
    this->throttling_ = getThrottlingInfo();
}

// ─── Helpers ───────────────────────────────────────────────────────────────────
pid_t hardwareMetrics::getEnginePid() const {
    //TODO obtener el PID del proceso del motor de inferencia
    return 0;
}

   void hardwareMetrics::getEngineCpuPercent(){
    static cpu_ticks engine_ticks_prev{};
    switch (this->engine_) {
        case InferenceEngines::LLAMA:
            //
            //

            break;
        case InferenceEngines::OLLAMA:
            //TODO cpu percent para ollama
            break;
        default:
            //std::cerr << "Unknown engine type" << std::endl;
            throw std::runtime_error("Unknown engine type");
    }
}

double hardwareMetrics::getSystemCpuPercent() {
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

int hardwareMetrics::getCpuCores() {
    std::ifstream file("/proc/cpuinfo");
    if (!file.is_open()) throw std::runtime_error("Cannot open /proc/cpuinfo");
    int cores = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("processor") == 0) cores++;
    }
    return cores;
}
std::vector<double> hardwareMetrics::getCpuFrequencies() {
    const static int cores = getCpuCores();
    std::vector<double> freqs(cores);
    for (int i = 0; i < cores; ++i) {
        std::string path = "/sys/devices/system/cpu/cpu"
                         + std::to_string(i)
                         + "/cpufreq/scaling_cur_freq";
        std::ifstream f(path);
        if (f.is_open()) {
            long val = 0;
            f >> val;
            freqs[i] = val / 1e6; // MHz
        } else {
            freqs[i] = 0.0;
        }
    }
    return freqs;
}
double hardwareMetrics::getCoreVoltage() {
    // Run vcgencmd and open a pipe to read its output
    FILE* pipe = popen("vcgencmd measure_volts core", "r");
    if (!pipe)
        throw std::runtime_error("popen() failed");

    char buffer[64];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe))
        output += buffer;
    pclose(pipe);

    // Parse "volt=1.2000V\n" → 1.2
    auto pos = output.find('=');
    if (pos == std::string::npos)
        throw std::runtime_error("Unexpected vcgencmd output: " + output);

    std::string valueStr = output.substr(pos + 1);  // "1.2000V\n"

    // Strip trailing non-numeric characters (e.g. 'V', '\n')
    while (!valueStr.empty() && !std::isdigit(valueStr.back()))
        valueStr.pop_back();

    return std::stod(valueStr);
}

long hardwareMetrics::getFanSpeed() {
        for (const auto& entry : std::filesystem::directory_iterator("/sys/class/hwmon")) {
            // fan1_input contiene las RPM directamente
            auto path = entry.path() / "fan1_input";
            if (std::filesystem::exists(path)) {
                std::ifstream f(path);
                long rpm;
                f >> rpm;
                return rpm;
            }
        }
        return 0;
    }

throttlingInfo hardwareMetrics::getThrottlingInfo() {
    // con exec command
    std:std::string  cmd_output = execCommand("vcgencmd get_throttled");
    size_t eq_pos = cmd_output.find('=');
    if (eq_pos == std::string::npos)
        throw std::runtime_error("Formato inesperado en la salida de vcgencmd");

    std::string hex_str = cmd_output.substr(eq_pos + 1);
    hex_str.erase(hex_str.find_last_not_of(" \n\r\t\"") + 1);
    hex_str.erase(0, hex_str.find_first_not_of(" \t\""));
    int raw = std::stoi(hex_str, nullptr, 16);
    throttlingInfo info;
    info.under_voltage  = (raw & (1 << 0)) != 0;  // 0x00001
    info.freq_capped    = (raw & (1 << 1)) != 0;  // 0x00002
    info.throttled      = (raw & (1 << 2)) != 0;  // 0x00004
    info.soft_throttled = (raw & (1 << 3)) != 0;  // 0x00008
    return info;
}
