//
// Created by yisus on 8/4/26.
//

#include "../../includes/utils/hardwareMeasurements.h"

#include <thread>
#include "metrics/hardwareMetrics.h"
/*
 *Clase que controla la tomas de medidas */
HardwareMeasurements::HardwareMeasurements(const std::string& filepath, double periodo_s)
    : logger_(filepath),    // ✅ construye directamente, no default-construye
      periodo_s(periodo_s)
{
    // cuerpo vacío
}

HardwareMeasurements::~HardwareMeasurements() {
    stop();
    close();
}

void HardwareMeasurements::start() {
    // Auténticamente períodica, el profe de SETR estarían orgullosos de mi./
    running_.store(true);
    hardwareMetrics hm(InferenceEngines::OTHER);
    const auto period = std::chrono::duration<double>(periodo_s);
    while (running_.load()) {
        auto next_tick = std::chrono::steady_clock::now() + period;
        hm.update();
        logger_.write2jsonline(hm);
        std::this_thread::sleep_until(next_tick);
    }
}
void HardwareMeasurements::stop() {
 running_.store(false);
}
void HardwareMeasurements::close() {
// no hace ná
}