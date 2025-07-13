//
// Created by David Clay on 6/16/23.
//

#include "Interpolation.h"

Interpolation::Interpolation(int duration) {
    this->timer = new Timer(duration);
    this->active = false;
}

void Interpolation::start() {
    // TODO: if this was already started, this will jump...
    // Note: restart() will reset the `iterations` so we can
    // determine whether the animation has run once vs not run yet.
    this->timer->restart();
    this->active = true;
}

void Interpolation::update() {
    if (!this->active) {
        return;
    }

    this->timer->update();

    if (this->timer->is_done()) {
        // Note/TODO: I _think_ if isDone returns true, the Timer is already stopped->
        this->timer->stop();
        this->active = false;
    }
}

float Interpolation::get_value() {
    return this->timer->current_value;
}

bool Interpolation::is_done() {
    // how do I know if it's done? It has to have _run_ to be done.
    return this->timer->is_done();
}

bool Interpolation::is_running() {
    return this->timer->is_running();
}

/**
 * Used to reset the state of the transition animation so it can
 * be started again, vs knowing it is done and has stopped.
 */
void Interpolation::reset() {
    this->timer->reset();
}
