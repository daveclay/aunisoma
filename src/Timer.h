//
// Created by Dave Clay on 6/9/25.
//

#ifndef TIMER_H
#define TIMER_H
#include "Clock.h"

class Timer {
public:
    float current_value;
    Timer(int duration_ms);
    void start();
    void stop();
    void restart();
    void restart(int duration_ms);
    void update();
    bool is_running() const;
    bool is_done() const;
    void reset();
    // Shift clock elapsed_ms to (duration_ms - current_elapsed_ms), reversing
    // the timer's progress. Used by Interpolation to flip mid-flight.
    void mirror_phase();
    Clock* clock;
    int duration_ms;

private:
    bool completed;
    unsigned long last_iteration_seen;
};

#endif //TIMER_H
