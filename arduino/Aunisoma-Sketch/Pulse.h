//
// Created by Dave Clay on 6/3/25.
//

#ifndef PULSE_H
#define PULSE_H
#include "Cycle.h"

class Pulse {
public:
    float amplitude;
    bool active;
    float current_value;
    Cycle* cycle;

    Pulse(float amplitude, int duration, bool one_shot);
    void update(bool active);
    void start();
    void stop();
    bool isAtZeroPoint();
    bool isStopped();
};

#endif //PULSE_H
