//
// Created by David Clay on 6/16/23.
//

#include "TransitionAnimation.h"

TransitionAnimation::TransitionAnimation(int durationTicks, TransitionAnimationCallback* transitionAnimationCallback) {
    this->transitionAnimationCallback = transitionAnimationCallback;
    this->cycle = new Cycle(durationTicks,
                            false,
                            CYCLE_TYPE_UP,
                            this);

    this->current_value = 0;
    this->active = false;
}

void TransitionAnimation::value(float value, CycleDirection direction) {
    this->current_value = value;
}

void TransitionAnimation::start(GradientValueMap* new_target_gradient) {
    this->target_gradient = new_target_gradient;
    this->cycle->restart();
    this->active = true;
}

void TransitionAnimation::update() {
    if (!this->active) {
        return;
    }

    this->cycle->next();

    if (this->current_value >= .97) {
        // Done!
        this->transitionAnimationCallback->switch_to_next_gradient();
        this->cycle->stop();
        this->active = false;
    }
}

Color TransitionAnimation::get_color(float panel_value) {
    GradientValueMap* current_gradient = this->transitionAnimationCallback->getCurrentGradient();
    Color from_color = current_gradient->getColorForValue(panel_value);
    Color to_color = this->target_gradient->getColorForValue(panel_value);
;
    Color color = from_color.interpolate(to_color, this->current_value);


    return color;
}
