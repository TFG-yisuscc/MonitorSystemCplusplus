//
// Created by yisus on 8/4/26.
//

#ifndef MONITORSYSTEM_HARDWAREMEASUREMENTS_H
#define MONITORSYSTEM_HARDWAREMEASUREMENTS_H
#include <string>
#include <chrono>

#include "metrics/Logger.h"
/* Clase que controlo la freuencia de las tomas de medidas hardware y software
 * recibe el filename y la frecuencia de operación
 *
 *
 */


class HardwareMeasurements {
private:
    Logger logger_;
    double periodo_s;
    std::atomic<bool> running_{false};


public:
 //   explicit HardwareMeasurements(std::string& filepath,double periodo_s); // el explicit creo que sobra en este caso
    HardwareMeasurements(const std::string &filepath, double periodo_s);


    ~HardwareMeasurements();
     void start();
    void stop();
     void close(); //inutil

};



#endif //MONITORSYSTEM_HARDWAREMEASUREMENTS_H
