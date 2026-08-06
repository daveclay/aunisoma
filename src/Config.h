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
    // its source; intensity falls off exponentially (decay constant
    // wave_spatial_decay_constant), normalized so it is 1.0 at the origin and
    // 0.0 at max_distance. Larger decay constant -> sharper drop-off.
    // wave_rainbow_scroll_duration_ms is the time for the rainbow override to
    // scroll one full revolution along the strip during high-interaction mode.
    int wave_rise_duration_ms;
    int wave_fall_duration_ms;
    int wave_panel_delay_ms;
    int wave_inter_pulse_delay_ms;
    int wave_max_propagation_distance;
    float wave_spatial_decay_constant;
    int wave_rainbow_scroll_duration_ms;
    int wave_rainbow_transition_duration_ms;
    Color wave_idle_color;

    // Overlap blend tuning. Above magnitude 1.0 on the chroma sum, saturation
    // clamps and the excess rotates the hue by this fraction of the wheel per
    // unit of overshoot (1/12 ≈ 30°/unit). The coherence threshold gates
    // whether the chroma sum's direction is trusted or we anchor to the
    // brightest contributing wave's hue (numerical stability, not a "go gray"
    // gate — both branches produce a saturated color).
    float wave_overlap_hue_rotation_per_unit;
    float wave_overlap_coherence_fallback_threshold;

    // Glow algorithm parameters (Glow-Sketch). No propagation: each sensor
    // lights only its own panel. Fade in over glow_fade_in_duration_ms on the
    // rising edge, fall off over glow_fade_out_duration_ms on release. While
    // active, brightness pulses from 100% down to glow_pulse_min_value and
    // back; each pulse cycle draws a fresh random period from
    // [glow_pulse_min_period_ms, glow_pulse_max_period_ms].
    int glow_fade_in_duration_ms;
    int glow_fade_out_duration_ms;
    int glow_pulse_min_period_ms;
    int glow_pulse_max_period_ms;
    float glow_pulse_min_value;

    // Ripple: neighbors within glow_ripple_distance panels on each side
    // follow the source with a per-distance lag. A neighbor starts
    // brightening glow_ripple_start_delay_ms per panel of distance after
    // the source's rising edge, reaches its peak glow_ripple_peak_lag_ms
    // per panel after the source peaks, repeats the source's pulse dips
    // delayed glow_ripple_peak_lag_ms per panel while the sensor stays
    // active, and trails the fade-out by the same lags. Peak intensity
    // falls off linearly with distance, reaching zero one panel past
    // glow_ripple_distance.
    int glow_ripple_distance;
    int glow_ripple_start_delay_ms;
    int glow_ripple_peak_lag_ms;

    // Two-sided dance: when both of a panel's own sensors have active glows
    // (one person on each side), the panel cycles between the two glows'
    // colors instead of statically blending them. One full
    // anchor → other → anchor color cycle takes glow_dance_period_ms. Each
    // color's pull is weighted by its glow's live intensity, so the dance
    // eases in while the second glow fades in and hands off smoothly to the
    // surviving color when either side releases.
    int glow_dance_period_ms;
    // Hue offset (fraction of the wheel) given to a sensor that activates
    // while its own panel's other side is already lit. Adjacent-panel
    // activations keep the small chain-gradient shift; the same-panel
    // partner gets this larger offset so the dance between the two colors
    // reads clearly.
    float glow_dance_partner_hue_offset;

    // Idle attract (Glow-Sketch): after glow_knight_rider_idle_delay_ms with
    // no glow activity anywhere, run the knight rider sweep for
    // glow_knight_rider_run_duration_ms, then return to idle; while idleness
    // continues it comes back every glow_knight_rider_idle_delay_ms. One full
    // left-right-left sweep takes glow_knight_rider_sweep_duration_ms. The
    // sweep crossfades in and out over glow_knight_rider_fade_ms; any
    // interaction fades it out immediately in favor of the normal glow
    // animation.
    int glow_knight_rider_idle_delay_ms;
    int glow_knight_rider_run_duration_ms;
    int glow_knight_rider_sweep_duration_ms;
    int glow_knight_rider_fade_ms;

    // This is here to make it available to the C++ files, since importing Aunisoma-Sketch isn't good.
    int number_of_panels;

    Config();

    void init();
    int get_single_panel_pulse_duration() const;
    int get_reverberation_distance() const;
};


#endif //C_AUNISOMA_CONFIG_H
