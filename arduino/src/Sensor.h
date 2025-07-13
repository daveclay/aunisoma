//
// Created by David Clay on 6/16/23.
//

#ifndef C_AUNISOMA_SENSOR_H
#define C_AUNISOMA_SENSOR_H
#include "Debounce.h"
#include "ValueSmoothingFn.h"


class Sensor {
public:
    bool active;
    bool last_reading;
    Sensor();
    void update(bool reading);
private:
    ValueSmoothingFn* smoothing_fn;
    Debounce* debounce;
};


#endif //C_AUNISOMA_SENSOR_H
