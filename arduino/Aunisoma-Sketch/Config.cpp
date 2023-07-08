//
// Created by David Clay on 6/15/23.
//

#include "Config.h"

Config::Config() {
}

int Config::getTriggerPanelAnimationLoopDurationTicks() const {
    return this->trigger_panel_animation_loop_duration_ticks_range->random_int_between();
}

int Config::get_reverberation_distance() const {
    return this->reverberation_distance_range->random_int_between();
}

void Config::init() {
    this->max_reverberation_distance = reverberation_distance_range->max;
    this->initial_trigger_panel_animation_loop_duration_ticks =
            (this->trigger_panel_animation_loop_duration_ticks_range->max + trigger_panel_animation_loop_duration_ticks_range->min) / 2;
}

