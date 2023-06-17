//
// Created by David Clay on 6/16/23.
//

#ifndef C_AUNISOMA_TRANSITIONANIMATION_H
#define C_AUNISOMA_TRANSITIONANIMATION_H


#include "Cycle.h"
#include "PanelContext.h"
#include "Gradient.h"
#include "TransitionAnimationCallback.h"

class TransitionAnimation: public CycleHandler {
public:
    TransitionAnimation(int durationTicks, TransitionAnimationCallback* transitionAnimationCallback);
    bool active;
    Cycle* cycle;
    void value(float value, CycleDirection direction);
    void start(GradientValueMap* target_gradient);
    void update();
    Color get_color(float panel_value);

private:
    TransitionAnimationCallback* transitionAnimationCallback;
    float current_value;
    GradientValueMap* target_gradient;
};


#endif //C_AUNISOMA_TRANSITIONANIMATION_H
