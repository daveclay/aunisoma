//
// Created by David Clay on 6/15/23.
//

#include "Cycle.h"
#include "Arduino.h"
#include <cmath>

Cycle::Cycle(int duration_ms, bool one_shot, CycleType cycle_type) {
    this->duration_ms = duration_ms;
    this->one_shot = one_shot;
    this->cycle_type = cycle_type;
    this->clock = new Clock();
    this->iterations = 0;
    this->release_phase = false;
    this->current_value = 0;
    this->last_iteration_seen = 0;
    this->at_zero_point = false;
}

void Cycle::start() {
    this->release_phase = false;
    // if the clock is on the falling side, set it to the equidistant rising side
    if (!this->isRising()) {
        unsigned long phase = this->clock->elapsed_ms % this->duration_ms;
        this->clock->shift_to(this->duration_ms - phase);
        // shift_to jumps elapsed_ms backwards relative to absolute iteration
        // count, so reset the crossing tracker to match the new phase.
        this->last_iteration_seen = 0;
    }
    this->clock->start();
}

void Cycle::stop() {
    this->clock->stop();
    this->current_value = 0;
    this->release_phase = false;
    this->last_iteration_seen = 0;
    this->at_zero_point = false;
}

void Cycle::reset() {
    // The `iterations` tracks whether this cycle has run or not.
    this->iterations = 0;
}

/**
 * Triggers the "release" phase of the cycle, dropping the
 * value down to its minimum which will then stop the cycle.
 */
void Cycle::release() {
    this->release_phase = true;
    if (this->isRising() && this->cycle_type == UP_DOWN_CYCLE) {
        // If we're on the rising side of the cycle, jump to the mirrored point
        // on the falling side at the same wave height, and continue to finish
        // the cycle. If we're on the falling side already, just let it finish.
        unsigned long phase = this->clock->elapsed_ms % this->duration_ms;
        this->clock->shift_to(this->duration_ms - phase);
        // shift_to jumps elapsed_ms backwards relative to absolute iteration
        // count, so reset the crossing tracker so the next boundary at
        // duration_ms is detected as a crossing.
        this->last_iteration_seen = 0;
    }
}

/**
 * Update this Cycle to the next iteration of its clock. Even if
 * a Cycle is stopped, it may have a release duration.
 */
void Cycle::update() {
    // always update the clock. If it's stopped, it's a no-op, otherwise ensures
    // the clock is up-to-date before doing any calculations.
    this->clock->update();
    this->at_zero_point = false;

    if (this->clock->isStopped()) {
        return;
    }

    // Iteration-crossing check: elapsed_ms may jump over the exact boundary,
    // so detect when (elapsed / duration) increments instead of (elapsed % duration == 0).
    unsigned long current_iteration = this->clock->elapsed_ms / this->duration_ms;
    if (current_iteration > this->last_iteration_seen) {
        this->iterations += static_cast<int>(current_iteration - this->last_iteration_seen);
        this->last_iteration_seen = current_iteration;
        this->at_zero_point = true;

        if (this->one_shot || this->release_phase) {
            this->stop();
            return;
        }
    } else if (this->clock->elapsed_ms == 0) {
        // first update after restart: original tick-based behavior also reported
        // ticks==0 as "at zero point" on the first update.
        this->at_zero_point = true;
    }

    unsigned long looping_elapsed = this->clock->elapsed_ms % this->duration_ms;
    if (this->cycle_type == UP_ONLY_CYCLE) {
        this->current_value = static_cast<float>(looping_elapsed) / static_cast<float>(this->duration_ms);
    } else {
        unsigned long half_duration = static_cast<unsigned long>(std::lround(this->duration_ms / 2.0));
        if (looping_elapsed <= half_duration) {
            this->current_value = static_cast<float>(looping_elapsed) / static_cast<float>(half_duration);
        } else {
            this->current_value = (static_cast<float>(this->duration_ms) - 1 - static_cast<float>(looping_elapsed)) / static_cast<float>(half_duration);
        }
    }
}

bool Cycle::isRising() const {
    if (this->cycle_type == UP_ONLY_CYCLE) {
        return true;
    }
    unsigned long relative = this->clock->elapsed_ms % this->duration_ms;
    return relative < static_cast<unsigned long>(this->duration_ms / 2);
}

void Cycle::restart() {
    this->restart(-1);
}

void Cycle::restart(int new_duration_ms) {
    if (new_duration_ms > -1) {
        this->duration_ms = new_duration_ms;
    }
    this->release_phase = false;
    this->iterations = 0;
    this->last_iteration_seen = 0;
    this->at_zero_point = false;
    this->clock->restart();
}

bool Cycle::isAtZeroPoint() const {
    return this->at_zero_point;
}

bool Cycle::isDone() const {
    return this->one_shot && this->iterations > 0;
}

bool Cycle::isStopped() const {
    return this->clock->isStopped();
}

bool Cycle::isRunning() const {
    return this->clock->running;
}
