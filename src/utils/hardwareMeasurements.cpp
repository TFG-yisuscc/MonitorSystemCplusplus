//
// Created by yisus on 8/4/26.
//

#include "../../includes/utils/hardwareMeasurements.h"

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
    //TODO adaptar harwaremetrics para quitar el inference engines o ponerlo de otro modo
        running_.store(true);
        hardwareMetrics hm(InferenceEngines::OTHER);
        while (running_.load()) {
           hm.update();
            logger_.write2jsonline(hm);
            std::this_thread::sleep_for(std::chrono::duration<double>(periodo_s));
        }

}
void HardwareMeasurements::stop() {
 running_.store(false);
}
void HardwareMeasurements::close() {
// no hace ná
}