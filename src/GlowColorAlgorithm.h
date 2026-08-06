#ifndef C_AUNISOMA_GLOWCOLORALGORITHM_H
#define C_AUNISOMA_GLOWCOLORALGORITHM_H

#include "Clock.h"
#include "ColorAlgorithm.h"
#include "Config.h"
#include "Glow.h"
#include "Sensor.h"

// Per-panel pulsing. Each sensor (front and back are independent
// interactions) drives its own Glow on its own panel; neighbors within
// glow_ripple_distance follow the source's fade envelope with a
// per-distance lag, so activation and release visibly propagate outward.
// A panel whose own two sensors are both active dances between the two
// glows' colors on glow_dance_period_ms; other multi-glow panels blend
// with the same chroma-vector overlap rule WaveColorAlgorithm uses. The
// high-interaction rainbow override is carried over unchanged.
class GlowColorAlgorithm : public ColorAlgorithm {
public:
    GlowColorAlgorithm(Config* config,
                       Sensor* sensors,
                       int number_of_panels,
                       int number_of_sensors);
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

    Color _pick_target_for_activation(int activating_sensor_index, int activating_panel_index);
    Color _dance_color_for_panel(int panel_index, float idle_value) const;
    Color _pick_random_saturated_color() const;
    bool _is_high_interaction() const;
    Color _rainbow_color_for_panel(int panel_index) const;
};

#endif //C_AUNISOMA_GLOWCOLORALGORITHM_H
