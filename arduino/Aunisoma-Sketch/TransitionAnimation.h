//
// Created by David Clay on 6/16/23.
//

#ifndef C_AUNISOMA_TRANSITIONANIMATION_H
#define C_AUNISOMA_TRANSITIONANIMATION_H


#include "Cycle.h"
#include "Gradient.h"

class TransitionAnimation {
public:
    bool active;
    TransitionAnimation(int duration);
    void start(GradientValueMap* current_gradient, GradientValueMap* next_gradient);
    void update();
    bool is_done();
    void reset();
    Color get_color(float panel_value);

private:
    float current_value;
    Cycle* cycle;
    GradientValueMap* current_gradient;
    GradientValueMap* next_gradient;
};

#endif //C_AUNISOMA_TRANSITIONANIMATION_H
