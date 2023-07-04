//
// Created by David Clay on 6/15/23.
//

#include "Clock.h"

Clock::Clock() {
    this->running = false;
    this->ticks = -1;
}

void Clock::stop() {
    this->running = false;
    this->ticks = -1;
}

void Clock::start() {
    this->running = true;
}

void Clock::restart() {
    this->running = true;
    this->ticks = -1;
}

void Clock::next() {
    if (!this->running) {
        return;
    }

    if (this->ticks < 0) {
        this->ticks = 0;
    } else {
        this->ticks += 1;
    }
}
