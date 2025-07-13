//
// Created by Dave Clay on 6/3/25.
//

#include "Debounce.h"

#include "Arduino.h"

Debounce::Debounce(int debounce_ms) {
    this->debounce_ms = debounce_ms;
}

bool Debounce::update(bool new_reading) {
    long currentMillis = millis();
    if (this->previous_reading != new_reading) {
        this->last_debounce_time = currentMillis;
    }

    if ((currentMillis - this->last_debounce_time) >= this->debounce_ms) {
        // whatever the reading is at, it's been there for longer than the debounce
        // delay, so take it as the actual current state:
        if (this->reading != new_reading) {
            this->reading = new_reading;
        }
    }

    this->previous_reading = new_reading;

    return this->reading;
}
