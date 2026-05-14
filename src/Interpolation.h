//
// Created by David Clay on 6/16/23.
//

#ifndef C_AUNISOMA_TRANSITIONANIMATION_H
#define C_AUNISOMA_TRANSITIONANIMATION_H

#include "Timer.h"

class Interpolation {
public:
    Interpolation(int duration);
    void start();
    void start(int duration_ms);
    void restart_at_tick();
    void update();
    float get_value() const;
    bool is_done() const;
    bool is_running() const;
    void reset();

private:
    bool active;
    int default_duration_ms;
    Timer* timer;
};

#endif //C_AUNISOMA_TRANSITIONANIMATION_H
