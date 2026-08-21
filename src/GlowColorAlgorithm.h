#ifndef C_AUNISOMA_GLOWCOLORALGORITHM_H
#define C_AUNISOMA_GLOWCOLORALGORITHM_H

#include "Clock.h"
#include "ColorAlgorithm.h"
#include "Config.h"
#include "Glow.h"
#include "KnightRiderAnimation.h"
#include "Sensor.h"

// Per-panel pulsing. Each sensor (front and back are independent
// interactions) drives its own Glow on its own panel; neighbors within
// glow_ripple_distance follow the source's fade envelope with a
// per-distance lag, so activation and release visibly propagate outward.
// A panel whose own two sensors are both active dances between the two
// glows' colors on glow_dance_period_ms; other multi-glow panels blend
// with the same chroma-vector overlap rule WaveColorAlgorithm uses. The
// high-interaction rainbow override is carried over unchanged. A sustained
// crowd — multiple panels active with the sensors still changing for
// a random glow_flicker_trigger_min_delay_ms..glow_flicker_trigger_max_delay_ms
// — starts a flicker that builds up one
// random sparking panel at a time over glow_flicker_ramp_duration_ms,
// holds the whole sculpture flickering random colors — drawn from a hue
// neighborhood (± glow_flicker_hue_range) around a base hue rolled fresh
// each run — for glow_flicker_run_duration_ms, then fades out.
class GlowColorAlgorithm : public ColorAlgorithm {
public:
    GlowColorAlgorithm(Config* config,
                       Sensor* sensors,
                       int number_of_panels,
                       int number_of_sensors,
                       GradientValueMap* knight_rider_gradient);
    void update() override;
    Color get_color_for_panel(int panel_index) const override;

private:
    Config* config;
    Sensor* sensors;
    int number_of_panels;
    int number_of_sensors;
    Glow** glows;
    Color* displayed_colors;
    bool* prev_sensor_active;
    // Two-sided dance state. dance_clocks runs per panel while both of the
    // panel's own glows are active; the anchor is the glow that was already
    // running when the second one arrived, so the cycle departs from the
    // color the panel is currently showing. prev_glow_active is glow (not
    // sensor) activity, which outlives the sensor through fade-out.
    Clock* dance_clocks;
    bool* dance_active;
    bool* dance_anchor_is_front;
    bool* prev_glow_active;
    // Per-sensor hue of the glow's target color and its unit chroma vector,
    // cached at activation (the target is constant while active) so the
    // per-panel blend loop is multiply-adds instead of rgb_to_hsv + trig.
    float* glow_target_hue;
    float* glow_hue_cos;
    float* glow_hue_sin;
    bool rainbow_target_active;
    float rainbow_blend;
    float blend_at_transition_start;
    Clock rainbow_clock;
    Clock transition_clock;

    // Idle attract: after glow_knight_rider_idle_delay_ms with every glow
    // idle, the knight rider sweep crossfades in for
    // glow_knight_rider_run_duration_ms. idle_clock runs only while the
    // whole sculpture is dark; any interaction stops it (and fades the
    // sweep out if it is showing).
    KnightRiderAnimation* knight_rider_animation;
    bool knight_rider_target_active;
    float knight_rider_blend;
    float knight_rider_blend_at_transition_start;
    Clock idle_clock;
    Clock knight_rider_run_clock;
    Clock knight_rider_transition_clock;

    // Sustained-crowd flicker: multi_panel_clock runs while at least
    // glow_flicker_min_active_panels panels have active glows AND the sensor
    // state changed within glow_flicker_change_timeout_ms (tracked by
    // sensor_change_clock); when it reaches flicker_trigger_delay_ms — the
    // current countdown target, re-rolled between
    // glow_flicker_trigger_min_delay_ms and glow_flicker_trigger_max_delay_ms
    // after each trigger — the flicker ramps up in single-panel sparks over
    // glow_flicker_ramp_duration_ms, holds every panel flickering for
    // glow_flicker_run_duration_ms, then crossfades out like the other
    // overrides. A flickering panel fades through its colors rather than
    // snapping: each hold period crossfades flicker_from_colors →
    // flicker_to_colors, and flicker_colors is the mid-fade color displayed
    // this tick. flicker_change_started_ms/flicker_next_change_ms bound the
    // current hold and flicker_spark_end_ms is each panel's spark expiry,
    // all on the flicker_run_clock timeline. flicker_panel_strength is
    // each panel's share of the override this tick: sparking panels only
    // during the ramp (easing back to idle over glow_flicker_spark_fade_ms
    // when their spark expires), everyone afterward. Each run anchors to a
    // fresh base hue and every color re-roll lands within
    // glow_flicker_hue_range of it (flicker_base_wheel_position), so one
    // run reads as a distinct palette rather than pure noise. The base
    // advances by a golden-ratio step in unwarped HSV space per run
    // (flicker_base_hue): unwarped so the perceptual warp's wide warm band
    // doesn't over-pick reds, golden-stepped so consecutive runs never
    // land on similar palettes.
    Color* flicker_colors;
    Color* flicker_from_colors;
    Color* flicker_to_colors;
    unsigned long* flicker_change_started_ms;
    unsigned long* flicker_next_change_ms;
    bool* flicker_spark_active;
    unsigned long* flicker_spark_end_ms;
    float* flicker_panel_strength;
    bool flicker_target_active;
    long flicker_trigger_delay_ms;
    float flicker_base_hue;
    float flicker_base_wheel_position;
    float flicker_blend;
    float flicker_blend_at_transition_start;
    Clock multi_panel_clock;
    Clock sensor_change_clock;
    Clock flicker_run_clock;
    Clock flicker_transition_clock;

    Color _pick_target_for_activation(int activating_sensor_index, int activating_panel_index);
    Color _dance_color_for_panel(int panel_index, float idle_value) const;
    Color _pick_random_saturated_color() const;
    Color _pick_flicker_color() const;
    long _roll_flicker_trigger_delay() const;
    bool _is_high_interaction() const;
    Color _rainbow_color_for_panel(int panel_index) const;
};

#endif //C_AUNISOMA_GLOWCOLORALGORITHM_H
