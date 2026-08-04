//
// Created by David Clay on 6/16/23.
//

#include "Sensor.h"
#include "ValueSmoothingFn.h"
#define DEBOUNCE_PIR_DELAY 20 // ms debounce

Sensor::Sensor() {
    this->smoothing_fn = new ValueSmoothingFn(20);
    this->debounce = new Debounce(DEBOUNCE_PIR_DELAY, false);
    this->active = false;
    this->last_reading = false;
}

void Sensor::update(bool reading) {
    float ave = this->smoothing_fn->get_smoothed_value(reading ? 1 : 0);
    if (reading) {
        // A high reading is a real interaction — activate immediately so the
        // color transition starts the moment someone arrives, instead of
        // waiting for the average to climb (16 loops of latency).
        this->active = true;
    } else {
        // Release only after the reading has stayed low long enough to drag
        // the average down (~11 loops). Holding through brief PIR dips keeps
        // an active animation from flickering into a fade-out/reactivate
        // cycle mid-interaction.
        this->active = ave > 0.5f;
    }
    this->last_reading = reading;
}
