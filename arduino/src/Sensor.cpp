//
// Created by David Clay on 6/16/23.
//

#include "Sensor.h"
#define DEBOUNCE_PIR_DELAY 20 // ms debounce
#include "Arduino.h"

Sensor::Sensor() {
}

void Sensor::update(bool reading) {
    long currentMillis = millis();
    if (this->lastReading != reading) {
        this->lastDebounceTime = currentMillis;
    }
    if ((currentMillis - lastDebounceTime) > DEBOUNCE_PIR_DELAY) {
        // whatever the reading is at, it's been there for longer than the debounce
        // delay, so take it as the actual current state:
        if (this->active != reading) {
            this->active = reading;
        }
    }

    this->lastReading = reading;
}
