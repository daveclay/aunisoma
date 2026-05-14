//
// Created by David Clay on 6/15/23.
//

#include "Clock.h"
#include "Arduino.h"

Clock::Clock() {
    this->running = false;
    this->started = false;
    this->elapsed_ms = 0;
    this->start_ms = 0;
}

void Clock::start() {
    this->running = true;
    if (!this->started) {
        this->start_ms = millis();
        this->elapsed_ms = 0;
        this->started = true;
    }
}

void Clock::stop() {
    this->running = false;
    this->started = false;
    this->elapsed_ms = 0;
}

void Clock::restart() {
    this->running = true;
    this->started = true;
    this->start_ms = millis();
    this->elapsed_ms = 0;
}

void Clock::update() {
    if (!this->running) {
        return;
    }
    this->elapsed_ms = millis() - this->start_ms;
}

bool Clock::isStopped() const {
    return !this->running && !this->started;
}

void Clock::shift_to(unsigned long new_elapsed_ms) {
    // Move elapsed_ms to a new value by shifting the reference point.
    // start_ms = now - new_elapsed_ms means update() will compute
    // elapsed = now - (now - new_elapsed_ms) = new_elapsed_ms.
    this->start_ms = millis() - new_elapsed_ms;
    this->elapsed_ms = new_elapsed_ms;
}
