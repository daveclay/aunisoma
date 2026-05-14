//
// Created by Dave Clay on 6/3/25.
//

#include "ValueSmoothingFn.h"

ValueSmoothingFn::ValueSmoothingFn(int window_size) {
    this->window_size = window_size;
    this->current_value_index = 0;
    this->initializing = true;
    for (int i = 0; i < this->window_size; i++) {
        this->values[i] = 0;
    }
}

float ValueSmoothingFn::get_smoothed_value(float new_value) {
    if (this->initializing) {
        // initialize the array to the new value - the first
        // time we call this method, it returns the value.
        for (int i = 0; i < this->window_size; i++) {
            this->values[i] = new_value;
        }
        this->initializing = false;
        // optimize: there's no need to do the rest of the buffer
        // math since it returns the new_value anyways.
        return new_value;
    }

    // to use the latest window_size values, plus the new_value,
    // this overwrites the previous value at the
    // current_value_index with the new_value. The
    // current_value_index represents the _oldest_ value
    // in the window.
    this->values[this->current_value_index] = new_value;

    // Once the oldest value is overwritten with the latest value,
    // the current_value_index is incremented to the _next_
    // oldest value, so that the next value will overwrite
    // the oldest value->
    this->current_value_index++;
    if (this->current_value_index >= this->window_size) {
        // write into the first
        this->current_value_index = 0;
    }

    float smoothed_value = 0;
    for (int i = 0; i < this->window_size; i++) {
        smoothed_value += this->values[i];
    }

    return smoothed_value / (float) this->window_size;
}
