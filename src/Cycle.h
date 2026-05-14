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
    int duration_ms;
    bool one_shot;
    CycleType cycle_type;
    bool release_phase;
    Clock* clock;
    float current_value;
    int iterations;

    Cycle(int duration_ms, bool one_shot, CycleType cycle_type);
    void start();
    void stop();
    void reset();
    void release();
    void update();
    bool isRising() const;
    void restart();
    void restart(int new_duration_ms);
    bool isAtZeroPoint() const;
    bool isDone() const;
    bool isStopped() const;
    bool isRunning() const;

private:
    unsigned long last_iteration_seen;
    bool at_zero_point;
};


#endif //C_AUNISOMA_CYCLE_H
