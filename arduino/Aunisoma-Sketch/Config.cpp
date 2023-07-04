//
// Created by David Clay on 6/15/23.
//

#include "Config.h"

Config::Config(int number_of_panels,
               int min_reverberation_distance,
               int max_reverberation_distance,
               int reverberation_panel_delay_ticks,
               int min_trigger_panel_animation_loop_duration_ticks,
               int max_trigger_panel_animation_loop_duration_ticks,
               float max_interaction_threshold_percent,
               int max_interaction_duration_ticks,
               float max_interaction_amount_of_reverberation,
               float max_interaction_value_multiplier) {
    this->number_of_panels = number_of_panels;
    this->max_reverberation_distance = max_reverberation_distance;
    this->reverberation_distance_range = new Range(
            min_reverberation_distance,
            max_reverberation_distance);
    this->reverberation_panel_delay_ticks = reverberation_panel_delay_ticks;
    this->trigger_panel_animation_loop_duration_ticks_range = new Range(
            min_trigger_panel_animation_loop_duration_ticks,
            max_trigger_panel_animation_loop_duration_ticks);
    this->initial_trigger_panel_animation_loop_duration_ticks =
            (max_trigger_panel_animation_loop_duration_ticks + min_trigger_panel_animation_loop_duration_ticks) / 2;
    this->max_interaction_threshold_percent = max_interaction_threshold_percent;
    this->max_interaction_duration_ticks = max_interaction_duration_ticks;
    this->max_interaction_amount_of_reverberation = max_interaction_amount_of_reverberation;
    this->max_interaction_value_multiplier = max_interaction_value_multiplier;

};

int Config::getTriggerPanelAnimationLoopDurationTicks() const {
    return this->trigger_panel_animation_loop_duration_ticks_range->random_int_between();
}

int Config::get_reverberation_distance() const {
    return this->reverberation_distance_range->random_int_between();
}

