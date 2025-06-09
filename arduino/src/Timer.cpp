//
// Created by Dave Clay on 6/9/25.
//
#include "Timer.h"

Timer::Timer(int duration_ticks) {
    this->duration_ticks = duration_ticks;
    this->clock = new Clock();
    this->completed = false;
    this->current_value = 0;
}

void Timer::start() {
    this->completed = false;
    this->clock->start();
}

/**
 * Halts the timer, does not completed the timer.
 * completed = true only if this Timer is allowed
 * run its duration.
 */
void Timer::stop() {
    this->clock->stop();
    this->current_value = 0;
}

void Timer::restart()  {
    this->restart(this->duration_ticks);
}

void Timer::restart(int duration) {
    this->duration_ticks = duration;
    this->completed = false;
    this->clock->restart();
}

void Timer::update() {
    // always update the clock. If it's stopped, it's a no-op, otherwise ensures
    // the click is up-to-date before doing any calculations.
    this->clock->update();

    if (this->clock->isStopped()) {
        // if this cycle has stopped - it's no longer actively cycling, nor is it in the release state,
        // there's nothing to do.
        return;
    }

    if (this->clock->ticks > 0 && this->clock->ticks % this->duration_ticks == 0) {
        // Mark that the timer completed, as opposed to _stopped_.
        this->completed = true;

        this->stop();
        return;
    }

    int looping_elapsed_duration = this->clock->ticks % this->duration_ticks;
    this->current_value = (float) looping_elapsed_duration / (float) this->duration_ticks;
}

bool Timer::is_running() const {
    return this->clock->running;
}

bool Timer::is_done() const {
    return this->completed;
}

void Timer::reset() {
    this->completed = false;
}
