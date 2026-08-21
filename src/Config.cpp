//
// Created by David Clay on 6/15/23.
//

#include "Config.h"

Config::Config() : initial_trigger_panel_animation_loop_duration_ms(0), reverberation_distance_range(nullptr),
                   reverberation_panel_delay_ms(0),
                   single_panel_pulse_duration(nullptr),
                   high_interaction_threshold_percent(0),
                   intermediate_interaction_threshold_percent(0),
                   min_max_interaction_gradient_transition_duration(0),
                   default_gradient_delay_duration_range(nullptr),
                   no_interaction_knight_rider_delay_range(nullptr),
                   delay_for_gradient_transition_duration(0),
                   gradient_transition_animation_duration(0),
                   transition_to_default_gradient_duration(0),
                   smoothing_fn_window_size(0),
                   watchdog_state_duration_limit_ms(0),
                   knight_rider_interaction_debounce_ms(0),
                   no_interaction_debounce_ms(0),
                   low_interaction_debounce_ms(0),
                   med_interaction_debounce_ms(0),
                   high_interaction_debounce_ms(0),
                   wave_overlap_hue_rotation_per_unit(0),
                   wave_overlap_coherence_fallback_threshold(0),
                   glow_fade_in_duration_ms(0),
                   glow_fade_out_duration_ms(0),
                   glow_pulse_min_period_ms(0),
                   glow_pulse_max_period_ms(0),
                   glow_pulse_min_value(0),
                   glow_ripple_distance(0),
                   glow_ripple_start_delay_ms(0),
                   glow_ripple_peak_lag_ms(0),
                   glow_dance_period_ms(0),
                   glow_dance_partner_hue_offset(0),
                   glow_knight_rider_idle_delay_ms(0),
                   glow_knight_rider_run_duration_ms(0),
                   glow_knight_rider_sweep_duration_ms(0),
                   glow_knight_rider_fade_ms(0),
                   glow_flicker_min_active_panels(0),
                   glow_flicker_trigger_delay_ms(0),
                   glow_flicker_change_timeout_ms(0),
                   glow_flicker_ramp_duration_ms(0),
                   glow_flicker_spark_min_duration_ms(0),
                   glow_flicker_spark_max_duration_ms(0),
                   glow_flicker_spark_fade_ms(0),
                   glow_flicker_run_duration_ms(0),
                   glow_flicker_min_hold_ms(0),
                   glow_flicker_max_hold_ms(0),
                   glow_flicker_fade_ms(0),
                   number_of_panels(0) {
}

int Config::get_single_panel_pulse_duration() const {
    return this->single_panel_pulse_duration->random_int_between();
}

int Config::get_reverberation_distance() const {
    return this->reverberation_distance_range->random_int_between();
}

void Config::init() {
}

