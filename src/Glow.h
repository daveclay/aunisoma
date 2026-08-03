#ifndef C_AUNISOMA_GLOW_H
#define C_AUNISOMA_GLOW_H

#include "Clock.h"
#include "Color.h"
#include "Config.h"
#include "Sensor.h"

enum GlowState {
    GLOW_IDLE,
    GLOW_FADING_IN,
    GLOW_PULSING,
    GLOW_FADING_OUT
};

// One glow per sensor. Lifecycle: fade in fast on the sensor's rising edge,
// pulse brightness between 100% and glow_pulse_min_value while the sensor
// stays active (each pulse cycle gets a fresh random period), then fall off
// slowly when the sensor releases. Because the pulse clock starts on the
// rising edge, the front and back glows of the same panel naturally run out
// of phase with each other.
//
// Neighbors within glow_ripple_distance panels follow the source with a
// per-distance lag, so every pulse visibly propagates outward for as long
// as the sensor stays active: a neighbor starts brightening
// glow_ripple_start_delay_ms per panel of distance after the source starts,
// reaches its (distance-attenuated) peak glow_ripple_peak_lag_ms per panel
// after the source peaks, repeats the source's pulse dips delayed by the
// same per-panel lag, and trails the fade-out by the same lags.
class Glow {
public:
    Glow(Sensor* sensor, Config* config, int panel_index);

    // Rising edge while idle. The algorithm has already chosen the target
    // color (shared with a nearby active glow, or freshly picked).
    void start_activation(Color target);

    // Rising edge while fading out: keep the current color, fade back in
    // from the current intensity, and restart the pulse phase.
    void reactivate();

    // Advance the fade/pulse state machine and watch the sensor for the
    // falling edge.
    void update();

    // Displayed intensity of the source panel in [0, 1]: fade envelope times
    // pulse modulation.
    float get_intensity() const;

    // Per-panel contribution: the pulsing glow itself at distance 0, the
    // lagged neighbor envelope within glow_ripple_distance, zero beyond.
    float get_intensity_for_panel(int panel_index) const;

    bool is_active() const;
    int get_panel_index() const;
    Color get_target_color() const;

private:
    Sensor* sensor;
    Config* config;
    int panel_index;

    GlowState state;
    Color target_color;

    Clock fade_clock;
    Clock pulse_clock;
    int pulse_period_ms;
    // Period of the pulse cycle before the current one; 0 when the current
    // cycle is the activation's first. Lagged neighbor modulation reaches
    // back into it when the lag crosses the cycle boundary.
    int previous_pulse_period_ms;

    // Fading in resumes from wherever the intensity currently is (nonzero
    // when a fade-out gets interrupted); fading out starts from the full
    // modulated intensity captured at the falling edge so there is no jump
    // mid-pulse. Neighbors get the same treatment per distance (index 1..
    // glow_ripple_distance; index 0 unused).
    float fade_in_start_intensity;
    float fade_out_start_intensity;
    float* ripple_fade_in_start;
    float* ripple_fade_out_start;

    void _begin_pulse_cycle();
    float _fade_envelope() const;
    float _pulse_modulation() const;
    // Pulse modulation as it was lag_ms ago, for lagged neighbor pulses.
    float _pulse_modulation_at_lag(long lag_ms) const;
    // Neighbor envelope in [0, 1] before distance attenuation and pulse
    // modulation.
    float _ripple_envelope_at_distance(int distance) const;
};

#endif //C_AUNISOMA_GLOW_H
