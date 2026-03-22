//
// Created by yisus on 12/3/26.
//

#ifndef MONITORSYSTEM_CPUTICKS_H
#define MONITORSYSTEM_CPUTICKS_H
#include <cstdint>
struct  cpu_ticks {
    int64_t user;     // tiempo en modo usuario
    int64_t nice;     // tiempo en modo usuario con baja prioridad
    int64_t system;   // tiempo en modo kernel
    int64_t idle;     // tiempo inactivo
    int64_t iowait;   // tiempo esperando I/O
    int64_t irq;      // tiempo atendiendo interrupciones hardware
    int64_t softirq;  // tiempo atendiendo interrupciones software
    int64_t steal;    // tiempo robado por otras VMs (en entornos virtualizados)

    int64_t total() const {
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }

    int64_t active() const {
        return user + nice + system + irq + softirq + steal;
    }
};
struct cpu_ticks_pid {

};
inline cpu_ticks getCpuTimes() {
    cpu_ticks ct{0, 0, 0, 0, 0, 0, 0, 0};
    std::ifstream file("/proc/stat");
    std::string cpu;
    // primera línea: "cpu  user nice system idle iowait irq softirq steal"
    file >> cpu
         >> ct.user >> ct.nice >> ct.system >> ct.idle
         >> ct.iowait >> ct.irq >> ct.softirq >> ct.steal;
    if (!file.good()) {
        throw std::runtime_error("Failed to read CPU times");
    }
    return ct;
}
inline cpu_ticks getCpuTimesPid(pid_t pid) {
    //TODO tiempos de cpu por proceso
    return cpu_ticks{0, 0, 0, 0, 0, 0, 0, 0};
}
inline cpu_ticks getCpuTimesUser(pid_t pid) {
    //TODO tiempos de cpu en modo usuario por proceso
    return cpu_ticks{0, 0, 0, 0, 0, 0, 0, 0};
}
#endif //MONITORSYSTEM_CPUTICKS_H