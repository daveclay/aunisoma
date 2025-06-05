//
// Created by David Clay on 6/16/23.
//

#include "Sensor.h"
#define DEBOUNCE_PIR_DELAY 20 // ms debounce
#include "Arduino.h"

Sensor::Sensor() {
    this->debounce = new Debounce(DEBOUNCE_PIR_DELAY);
    this->active = false;
}

void Sensor::update(bool reading) {
    this->active = this->debounce->update(reading);
}
