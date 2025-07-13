//
// Created by David Clay on 6/15/23.
//

#include "Config.h"

Config::Config() {
}

int Config::get_single_panel_pulse_duration() const {
    return this->single_panel_pulse_duration->random_int_between();
}

int Config::get_reverberation_distance() const {
    return this->reverberation_distance_range->random_int_between();
}

void Config::init() {
}

