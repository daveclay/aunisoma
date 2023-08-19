//
// Created by David Clay on 6/16/23.
//

#ifndef C_AUNISOMA_SENSOR_H
#define C_AUNISOMA_SENSOR_H


class Sensor {
public:
    Sensor();
    int panelIndex;
    bool active;
    void update(bool reading);
    unsigned long lastDebounceTime;
    bool lastReading;
};


#endif //C_AUNISOMA_SENSOR_H
