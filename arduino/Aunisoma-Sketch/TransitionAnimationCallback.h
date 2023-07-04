//
// Created by David Clay on 6/16/23.
//

#ifndef C_AUNISOMA_TRANSITIONANIMATIONCALLBACK_H
#define C_AUNISOMA_TRANSITIONANIMATIONCALLBACK_H


#include "Gradient.h"

class TransitionAnimationCallback {
public:
    virtual void switch_to_next_gradient() = 0;
    virtual GradientValueMap* getCurrentGradient() = 0;
};


#endif //C_AUNISOMA_TRANSITIONANIMATIONCALLBACK_H
