//
// Created by Dave Clay on 6/3/25.
//
#include "Pulse.h"

Pulse::Pulse(float amplitude, int duration, bool one_shot) {
    this->amplitude = amplitude;
    this->active = false;
    this->current_value = 0;
    this->cycle = new Cycle(duration, one_shot, UP_DOWN_CYCLE);
}

void Pulse::update(bool active) {
    if (this->active && !active) {
        // interaction was active, and now is not->
        this->stop();
    } else if (!this->active && active) {
        // interaction was not active, and now is
        this->start();
    }

    // no matter if it's stopped or not, we're updating its tick
    this->cycle->update();
    this->current_value = this->cycle->current_value * this->amplitude;
}

void Pulse::start() {
    this->active = true;
    this->cycle->start();
}

void Pulse::stop() {
    this->active = false;
    this->cycle->release();
}

bool Pulse::isAtZeroPoint() {
    return this->cycle->isAtZeroPoint();
}

bool Pulse::isStopped() {
    return this->cycle->isStopped();
}
