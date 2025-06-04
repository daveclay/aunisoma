//
// Created by David Clay on 6/16/23.
//

#ifndef C_AUNISOMA_SENSOR_H
#define C_AUNISOMA_SENSOR_H
#include "Debounce.h"


class Sensor {
public:
    bool active;
    Sensor();
    void update(bool reading);
private:
    Debounce* debounce;
};


#endif //C_AUNISOMA_SENSOR_H
