//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_CONFIG_H
#define C_AUNISOMA_CONFIG_H


#include "Color.h"
#include "Range.h"

// All duration fields below are in milliseconds (wall-clock time, from millis()).
class Config {
public:
    int initial_trigger_panel_animation_loop_duration_ms;
    Range* reverberation_distance_range;
    int reverberation_panel_delay_ms;
    Range* single_panel_pulse_duration;  // ms
    float high_interaction_threshold_percent;
    float intermediate_interaction_threshold_percent;
    int min_max_interaction_gradient_transition_duration;
    Range* default_gradient_delay_duration_range;  // ms
    Range* no_interaction_knight_rider_delay_range;  // ms

    int delay_for_gradient_transition_duration;  // ms
    int gradient_transition_animation_duration;  // ms
    int transition_to_default_gradient_duration;  // ms
    int smoothing_fn_window_size;  // sample count, NOT a duration
    int watchdog_state_duration_limit_ms;

    // Debounce durations (ms) for interaction-level readings. Don't flicker the
    // number of interactions: a reading has to stay high (or low) for these
    // many ms before it's accepted.
    int knight_rider_interaction_debounce_ms;
    int no_interaction_debounce_ms;
    int low_interaction_debounce_ms;
    int med_interaction_debounce_ms;

    int high_interaction_debounce_ms;

    // Wave algorithm parameters (see plan.md). Each panel does a full up-down
    // animation per pulse: rises from idle to attenuated target over
    // wave_rise_duration_ms, then falls back over wave_fall_duration_ms.
    // wave_max_propagation_distance caps how many panels a wave reaches from
    // its source; intensity falls off linearly to zero at that distance.
    int wave_rise_duration_ms;
    int wave_fall_duration_ms;
    int wave_panel_delay_ms;
    int wave_inter_pulse_delay_ms;
    int wave_max_propagation_distance;
    Color wave_idle_color;

    // This is here to make it available to the C++ files, since importing Aunisoma-Sketch isn't good.
    int number_of_panels;

    Config();

    void init();
    int get_single_panel_pulse_duration() const;
    int get_reverberation_distance() const;
};


#endif //C_AUNISOMA_CONFIG_H
