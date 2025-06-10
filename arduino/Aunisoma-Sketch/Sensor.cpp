//
// Created by David Clay on 6/16/23.
//

#include "Sensor.h"
#include "ValueSmoothingFn.h"
#define DEBOUNCE_PIR_DELAY 20 // ms debounce

Sensor::Sensor() {
    this->smoothing_fn = new ValueSmoothingFn(20);
    this->debounce = new Debounce(DEBOUNCE_PIR_DELAY);
    this->active = false;
    this->last_reading = false;
}

void Sensor::update(bool reading) {
    float ave = this->smoothing_fn->get_smoothed_value(reading ? 1 : 0);
    this->active = ave > .75;// this->debounce->update(reading);
    this->last_reading = reading;
}
