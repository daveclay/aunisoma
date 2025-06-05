//
// Created by David Clay on 6/16/23.
//

#include "TransitionAnimation.h"

#define ONE_SHOT true

TransitionAnimation::TransitionAnimation(int duration) {
    this->cycle = new Cycle(duration, ONE_SHOT, UP_ONLY_CYCLE);
    this->current_value = 0;
    this->active = false;
}


void TransitionAnimation::start(GradientValueMap* current_gradient, GradientValueMap* next_gradient) {
    this->current_gradient = current_gradient;
    this->next_gradient = next_gradient;
    // TODO: if this was already started, this will jump...
    // Note: restart() will reset the `iterations` so we can
    // determine whether the animation has run once vs not run yet.
    this->cycle->restart();
    this->active = true;
}

void TransitionAnimation::update() {
    if (!this->active) {
        return;
    }

    this->cycle->update();

    if (this->cycle->isDone()) {
        this->cycle->stop();
        this->active = false;
    }
}

bool TransitionAnimation::is_done() {
    // how do I know if it's done? It has to have _run_ to be done.
    return this->cycle->isDone();
}

/**
 * Used to reset the state of the transition animation so it can
 * be started again, vs knowing it is done and has stopped.
 */
void TransitionAnimation::reset() {
    this->cycle->reset();
}

Color TransitionAnimation::get_color(float panel_value) {
    Color from_color = this->current_gradient->getColorForValue(panel_value);
    Color to_color = this->next_gradient->getColorForValue(panel_value);

    Color color = from_color.interpolate(to_color, this->cycle->current_value);

    return color;
}
