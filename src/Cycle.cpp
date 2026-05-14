//
// Created by David Clay on 6/15/23.
//

#include "Cycle.h"
#include "Arduino.h"

Cycle::Cycle(int duration_ticks, bool one_shot, CycleType cycle_type) {
    this->duration_ticks = duration_ticks;
    this->one_shot = one_shot;
    this->cycle_type = cycle_type;
    this->clock = new Clock();
    this->iterations = 0;
    this->release_phase = false;
    this->current_value = 0;
}

void Cycle::start() {
    this->release_phase = false;
    // if the clock is on the falling side,
    // set it to the equidistant rising side
    if (!this->isRising()) {
        this->clock->ticks = max(0, this->clock->ticks = this->duration_ticks - this->clock->ticks % this->duration_ticks);
    }
    this->clock->start();
}

void Cycle::stop() {
    this->clock->stop();
    this->current_value = 0;
    this->release_phase = false;
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
        // If we're on the rising side of the cycle, jump the clock ticks
        // to the next down cycle at the same point: if we're at tick 23
        // of a duration of 100, then we're jumping to clock tick 67
        // (100 - 23) and continuing on to finish the remaining duration
        // cycle of 100.
        //
        // If we're on the falling side of the cycle, just let it finish
        this->clock->ticks = max(0, this->clock->ticks = this->duration_ticks - this->clock->ticks % this->duration_ticks);
    }
}

/**
 * Update this Cycle to the next iteration of its clock. Even if
 * a Cycle is stopped, it may have a release duration.
 */
void Cycle::update() {
    // always update the clock. If it's stopped, it's a no-op, otherwise ensures
    // the click is up-to-date before doing any calculations.
    this->clock->update();

    if (this->clock->isStopped()) {
        // if this cycle has stopped - it's no longer actively cycling, nor is it in the release state,
        // there's nothing to do.
        return;
    }

    if (this->clock->ticks > 0 && this->clock->ticks % this->duration_ticks == 0) {
        // we've hit run the entire duration length. Increment the number
        // of iterations.
        this->iterations += 1;

        if (this->one_shot || this->release_phase) {
            // If this is a one-shot or the "release" is done, the Cycle is done.
            // Stop the clock, and return.
            this->stop();
            return;
        }
    }

    int looping_elapsed_duration = this->clock->ticks % this->duration_ticks;
    if (this->cycle_type == UP_ONLY_CYCLE) {
        this->current_value = (float) looping_elapsed_duration / (float) this->duration_ticks;
    } else {
        int half_animation_loop_duration_ticks = (int) round(this->duration_ticks / 2);
        if (looping_elapsed_duration <= half_animation_loop_duration_ticks) {
            this->current_value = (float) looping_elapsed_duration / (float) half_animation_loop_duration_ticks;
        } else {
            this->current_value = ((float) this->duration_ticks - 1 - (float) looping_elapsed_duration) / (float) half_animation_loop_duration_ticks;
        }
    }
}

bool Cycle::isRising() {
    if (this->cycle_type == UP_ONLY_CYCLE) {
        return true;
    }
    int relative_ticks = this->clock->ticks % this->duration_ticks;
    return relative_ticks < (this->duration_ticks / 2);
}

void Cycle::restart() {
    this->restart(-1);
}

void Cycle::restart(int new_duration_ticks) {
    if (new_duration_ticks > -1) {
        this->duration_ticks = new_duration_ticks;
    }
    this->release_phase = false;
    this->iterations = 0;
    this->clock->restart();
}

bool Cycle::isAtZeroPoint() {
    return this->clock->ticks % this->duration_ticks == 0;
}

bool Cycle::isDone() {
    return this->one_shot && this->iterations > 0;
}

bool Cycle::isStopped() {
    return this->clock->isStopped();
}

bool Cycle::isRunning() {
    return this->clock->running;
}
