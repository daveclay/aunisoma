//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_CYCLE_H
#define C_AUNISOMA_CYCLE_H


#include "Clock.h"

enum CycleType {
    UP_ONLY_CYCLE,
    UP_DOWN_CYCLE
};

class Cycle {
public:
    int duration_ticks;
    bool one_shot;
    CycleType cycle_type;
    bool release_phase;
    Clock* clock;
    float current_value;
    int iterations;

    Cycle(int duration_ticks, bool one_shot, CycleType cycle_type);
    void start();
    void stop();
    void reset();
    void release();
    void update();
    bool isRising();
    void restart();
    void restart(int new_duration_ticks);
    bool isAtZeroPoint();
    bool isDone();
    bool isStopped();
    bool isRunning();
};


#endif //C_AUNISOMA_CYCLE_H
