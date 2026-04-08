//
// Created by yisus on 8/4/26.
//

#include "hardwareMeasurements.h"

#include "metrics/hardwareMetrics.h"

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
    //TODO  segundos tome las medidas y las guarde con el logger
        running_.store(true);
        hardwareMetrics hm(InferenceEngines::LLAMA);
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

}