//
// Created by Dave Clay on 6/9/25.
//
#include "Timer.h"

Timer::Timer(int duration_ms) {
    this->duration_ms = duration_ms;
    this->clock = new Clock();
    this->completed = false;
    this->current_value = 0;
    this->last_iteration_seen = 0;
}

void Timer::start() {
    this->completed = false;
    this->clock->start();
}

/**
 * Halts the timer, does not complete the timer.
 * completed = true only if this Timer is allowed
 * to run its duration.
 */
void Timer::stop() {
    this->clock->stop();
    this->current_value = 0;
    this->last_iteration_seen = 0;
}

void Timer::restart()  {
    this->restart(this->duration_ms);
}

void Timer::restart(int duration_ms) {
    this->duration_ms = duration_ms;
    this->completed = false;
    this->last_iteration_seen = 0;
    this->clock->restart();
}

void Timer::update() {
    // always update the clock. If it's stopped, it's a no-op, otherwise ensures
    // the clock is up-to-date before doing any calculations.
    this->clock->update();

    if (this->clock->isStopped()) {
        return;
    }

    // Iteration-crossing check: elapsed_ms may jump over the exact boundary,
    // so detect when (elapsed / duration) increments instead of (elapsed % duration == 0).
    unsigned long current_iteration = this->clock->elapsed_ms / this->duration_ms;
    if (current_iteration > this->last_iteration_seen) {
        this->last_iteration_seen = current_iteration;
        this->completed = true;
        this->stop();
        return;
    }

    unsigned long looping_elapsed = this->clock->elapsed_ms % this->duration_ms;
    this->current_value = static_cast<float>(looping_elapsed) / static_cast<float>(this->duration_ms);
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

void Timer::mirror_phase() {
    unsigned long current = this->clock->elapsed_ms;
    unsigned long duration = static_cast<unsigned long>(this->duration_ms);
    unsigned long mirrored = current >= duration ? 0 : duration - current;
    this->clock->shift_to(mirrored);
    // shift_to can jump elapsed_ms backwards in absolute iteration count;
    // reset the crossing tracker so the next duration boundary still fires.
    this->last_iteration_seen = mirrored / duration;
}
