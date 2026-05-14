//
// Created by David Clay on 6/16/23.
//

#include "Interpolation.h"

Interpolation::Interpolation(int duration) {
    this->default_duration_ms = duration;
    this->timer = new Timer(duration);
    this->active = false;
}

void Interpolation::start() {
    // TODO: if this was already started, this will jump...
    // Note: restart() will reset the `iterations` so we can
    // determine whether the animation has run once vs not run yet.
    // Always restart with the default duration in case a previous caller
    // overrode it via start(int).
    this->timer->restart(this->default_duration_ms);
    this->active = true;
}

void Interpolation::start(int duration_ms) {
    this->timer->restart(duration_ms);
    this->active = true;
}

/**
 * Re-start the interpolation clock at the duration where the current timer is at.
 * Effectively "reverses" the interpolation.
 */
void Interpolation::restart_at_tick() {
    this->timer->mirror_phase();
    // Update the timer now - in the loop, update() was called before changing
    // state and calling restart_at_tick(). The next call to get_color won't
    // reflect the reversed state of the timer. This updates the timer so
    // get_value() returns the "reversed" interpolation value based on the
    // timer state.
    this->timer->update();
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

float Interpolation::get_value() const {
    return this->timer->current_value;
}

bool Interpolation::is_done() const {
    // how do I know if it's done? It has to have _run_ to be done.
    return this->timer->is_done();
}

bool Interpolation::is_running() const {
    return this->timer->is_running();
}

/**
 * Used to reset the state of the transition animation so it can
 * be started again, vs knowing it is done and has stopped.
 */
void Interpolation::reset() {
    this->timer->reset();
}
