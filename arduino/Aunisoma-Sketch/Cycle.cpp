//
// Created by David Clay on 6/15/23.
//

#include "Cycle.h"
#include "Arduino.h"

Cycle::Cycle(int duration_ticks, bool oneShot, CycleType cycleType, CycleHandler* cycleHandler) {
    this->duration_ticks = duration_ticks;
    this->oneShot = oneShot;
    this->cycleType = cycleType;
    this->cycleHandler = cycleHandler;
    this->clock = new Clock();
    this->iterations = 0;
}

void Cycle::next() {
    if (this->clock->ticks > -1 && this->clock->ticks >= this->duration_ticks) {
        this->iterations += 1;
        if (!this->oneShot) {
            this->clock->restart();
        }
    }
    if (this->iterations > 0 && this->oneShot) {
        return;
    }

    this->clock->next();
    int looping_elapsed_duration = this->clock->ticks % this->duration_ticks;

    if (this->cycleType == CYCLE_TYPE_UP) {
        float current_value = (float)looping_elapsed_duration / (float)this->duration_ticks;
        this->cycleHandler->value(current_value, CYCLE_DIRECTION_UP);
    } else {
        int half_animation_loop_duration_ticks = round(this->duration_ticks / 2);
        if (looping_elapsed_duration < half_animation_loop_duration_ticks) {
            float current_value = (float)looping_elapsed_duration / (float)half_animation_loop_duration_ticks;
            this->cycleHandler->value(current_value, CYCLE_DIRECTION_UP);
        } else {
            float current_value = (float)(this->duration_ticks - looping_elapsed_duration) / (float)half_animation_loop_duration_ticks;
            this->cycleHandler->value(current_value, CYCLE_DIRECTION_DOWN);
        }
    }
}

void Cycle::start() {
    this->clock->start();
}

void Cycle::stop() {
    this->clock->stop();
}

void Cycle::jumpToDownCycle() {
    if (this->duration_ticks >= this->clock->ticks) {
        this->clock->ticks = max(0, this->duration_ticks - this->clock->ticks);
    }
}

void Cycle::restart() {
    this->restart(-1);
}

void Cycle::restart(int new_duration_ticks) {
    if (new_duration_ticks > -1) {
        this->duration_ticks = new_duration_ticks;
    }
    this->iterations = 0;
    this->clock->restart();
}

bool Cycle::isAtZeroPoint() {
    return this->clock->ticks < 0 || (this->clock->ticks % this->duration_ticks == 0);
}

bool Cycle::isDone() {
    return this->oneShot && this->iterations > 0;
}
