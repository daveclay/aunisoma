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
    void update();
    float get_value();
    bool is_done();
    bool is_running();
    void reset();

private:
    bool active;
    Timer* timer;
};

#endif //C_AUNISOMA_TRANSITIONANIMATION_H
