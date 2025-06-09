//
// Created by Dave Clay on 6/9/25.
//

#ifndef TIMER_H
#define TIMER_H
#include "Clock.h"

class Timer {
public:
    float current_value;
    Timer(int duration_ticks);
    void start();
    void stop();
    void restart();
    void restart(int duration);
    void update();
    bool is_running() const;
    bool is_done() const;
    void reset();

private:
    int duration_ticks;
    Clock* clock;
    bool completed;
};

#endif //TIMER_H
