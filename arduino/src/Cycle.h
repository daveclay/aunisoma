//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_CYCLE_H
#define C_AUNISOMA_CYCLE_H


#include "Clock.h"

enum CycleDirection {
    CYCLE_DIRECTION_UP,
    CYCLE_DIRECTION_DOWN
};

enum CycleType {
    CYCLE_TYPE_UP,
    CYCLE_TYPE_UP_AND_DOWN
};

class CycleHandler {
public:
    virtual void value(float value, CycleDirection direction) = 0;
};

class Cycle {
public:
    int duration_ticks;
    bool oneShot;
    CycleType cycleType;
    CycleHandler* cycleHandler;
    Clock* clock;
    int iterations;

    Cycle(int duration_ticks, bool oneShot, CycleType cycleType, CycleHandler* cycleHandler);
    void next();
    void start();
    void stop();
    void jumpToDownCycle();
    void restart();
    void restart(int new_duration_ticks);
    bool isAtZeroPoint();
    bool isDone();
};


#endif //C_AUNISOMA_CYCLE_H
