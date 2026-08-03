#include "Glow.h"

#include <cmath>
#include "Arduino.h"

static const float GLOW_FULL_CIRCLE_RADIANS = 6.28318530717958647692f;

Glow::Glow(Sensor* sensor, Config* config, int panel_index) {
    this->sensor = sensor;
    this->config = config;
    this->panel_index = panel_index;

    this->state = GLOW_IDLE;
    this->target_color = config->wave_idle_color;
    this->pulse_period_ms = config->glow_pulse_min_period_ms;
    this->previous_pulse_period_ms = 0;
    this->fade_in_start_intensity = 0.0f;
    this->fade_out_start_intensity = 0.0f;

    int ripple_distance = config->glow_ripple_distance;
    if (ripple_distance < 0) ripple_distance = 0;
    this->ripple_fade_in_start = new float[ripple_distance + 1];
    this->ripple_fade_out_start = new float[ripple_distance + 1];
    for (int distance = 0; distance <= ripple_distance; distance++) {
        this->ripple_fade_in_start[distance] = 0.0f;
        this->ripple_fade_out_start[distance] = 0.0f;
    }
}

void Glow::start_activation(Color target) {
    this->target_color = target;
    this->fade_in_start_intensity = 0.0f;
    for (int distance = 1; distance <= this->config->glow_ripple_distance; distance++) {
        this->ripple_fade_in_start[distance] = 0.0f;
    }
    this->state = GLOW_FADING_IN;
    this->fade_clock.restart();
    this->_begin_pulse_cycle();
    this->previous_pulse_period_ms = 0;
}

void Glow::reactivate() {
    this->fade_in_start_intensity = this->get_intensity();
    for (int distance = 1; distance <= this->config->glow_ripple_distance; distance++) {
        this->ripple_fade_in_start[distance] = this->_ripple_envelope_at_distance(distance);
    }
    this->state = GLOW_FADING_IN;
    this->fade_clock.restart();
    this->_begin_pulse_cycle();
    this->previous_pulse_period_ms = 0;
}

void Glow::update() {
    if (this->state == GLOW_IDLE) {
        return;
    }

    this->fade_clock.update();

    if (this->state == GLOW_FADING_IN || this->state == GLOW_PULSING) {
        this->pulse_clock.update();

        // Falling edge: capture the current modulated intensity (source and
        // each neighbor, at its own lag) and fall off from there so a
        // mid-pulse release doesn't jump in brightness.
        if (!this->sensor->active) {
            this->fade_out_start_intensity = this->get_intensity();
            for (int distance = 1; distance <= this->config->glow_ripple_distance; distance++) {
                long pulse_lag_ms = static_cast<long>(distance) * this->config->glow_ripple_peak_lag_ms;
                this->ripple_fade_out_start[distance] = this->_ripple_envelope_at_distance(distance)
                                                      * this->_pulse_modulation_at_lag(pulse_lag_ms);
            }
            this->state = GLOW_FADING_OUT;
            this->fade_clock.restart();
            this->pulse_clock.stop();
            return;
        }

        // Pulse cycle boundary: modulation is back at 100%, so restarting
        // the clock with a fresh random period is seamless. Remember the
        // finished cycle's period for lagged neighbor modulation.
        if (static_cast<long>(this->pulse_clock.elapsed_ms) >= this->pulse_period_ms) {
            this->previous_pulse_period_ms = this->pulse_period_ms;
            this->_begin_pulse_cycle();
        }

        if (this->state == GLOW_FADING_IN
            && static_cast<long>(this->fade_clock.elapsed_ms) >= this->config->glow_fade_in_duration_ms) {
            this->state = GLOW_PULSING;
        }
    } else if (this->state == GLOW_FADING_OUT) {
        // The outermost neighbor trails the source's fade-out by
        // ripple_distance * peak_lag; hold FADING_OUT (source envelope is
        // already clamped to 0) until that tail finishes so neighbors keep
        // this glow's color instead of snapping to idle.
        long fade_out_total_ms = static_cast<long>(this->config->glow_fade_out_duration_ms)
            + static_cast<long>(this->config->glow_ripple_distance) * this->config->glow_ripple_peak_lag_ms;
        if (static_cast<long>(this->fade_clock.elapsed_ms) >= fade_out_total_ms) {
            this->state = GLOW_IDLE;
            this->fade_clock.stop();
            this->target_color = this->config->wave_idle_color;
        }
    }
}

float Glow::get_intensity() const {
    if (this->state == GLOW_IDLE) {
        return 0.0f;
    }
    if (this->state == GLOW_FADING_OUT) {
        // fade_out_start_intensity already includes the pulse modulation
        // captured at the falling edge.
        return this->_fade_envelope();
    }
    return this->_fade_envelope() * this->_pulse_modulation();
}

float Glow::get_intensity_for_panel(int target_panel_index) const {
    int distance = target_panel_index - this->panel_index;
    if (distance < 0) distance = -distance;
    if (distance == 0) {
        return this->get_intensity();
    }
    int ripple_distance = this->config->glow_ripple_distance;
    if (distance > ripple_distance) {
        return 0.0f;
    }
    // Linear peak falloff: zero one panel past ripple_distance, so the
    // outermost neighbor still gets a visible fraction.
    float peak = 1.0f - static_cast<float>(distance) / static_cast<float>(ripple_distance + 1);
    float envelope = this->_ripple_envelope_at_distance(distance);
    // While the source pulses, the neighbor repeats its dips delayed by the
    // per-panel lag so each pulse travels outward. During fade-out the
    // captured start value already includes the modulation.
    if (this->state == GLOW_FADING_IN || this->state == GLOW_PULSING) {
        long pulse_lag_ms = static_cast<long>(distance) * this->config->glow_ripple_peak_lag_ms;
        envelope *= this->_pulse_modulation_at_lag(pulse_lag_ms);
    }
    return peak * envelope;
}

bool Glow::is_active() const {
    return this->state != GLOW_IDLE;
}

int Glow::get_panel_index() const {
    return this->panel_index;
}

Color Glow::get_target_color() const {
    return this->target_color;
}

void Glow::_begin_pulse_cycle() {
    this->pulse_period_ms = random(this->config->glow_pulse_min_period_ms,
                                   this->config->glow_pulse_max_period_ms + 1);
    if (this->pulse_period_ms < 1) this->pulse_period_ms = 1;
    this->pulse_clock.restart();
}

float Glow::_fade_envelope() const {
    if (this->state == GLOW_FADING_IN) {
        int fade_in_duration_ms = this->config->glow_fade_in_duration_ms;
        if (fade_in_duration_ms < 1) fade_in_duration_ms = 1;
        float progress = static_cast<float>(this->fade_clock.elapsed_ms)
                       / static_cast<float>(fade_in_duration_ms);
        if (progress > 1.0f) progress = 1.0f;
        return this->fade_in_start_intensity
             + (1.0f - this->fade_in_start_intensity) * progress;
    }
    if (this->state == GLOW_PULSING) {
        return 1.0f;
    }
    if (this->state == GLOW_FADING_OUT) {
        int fade_out_duration_ms = this->config->glow_fade_out_duration_ms;
        if (fade_out_duration_ms < 1) fade_out_duration_ms = 1;
        float progress = static_cast<float>(this->fade_clock.elapsed_ms)
                       / static_cast<float>(fade_out_duration_ms);
        if (progress > 1.0f) progress = 1.0f;
        return this->fade_out_start_intensity * (1.0f - progress);
    }
    return 0.0f;
}

// A neighbor at distance N mirrors the source's fade, shifted and stretched:
// it starts N * glow_ripple_start_delay_ms after the source's edge and
// completes N * glow_ripple_peak_lag_ms after the source completes, both in
// and out. With the sketch defaults (fade-in 200, start delay 100, peak lag
// 200) a distance-1 neighbor starts brightening at 100 ms and hits its peak
// at 400 ms — 200 ms after the source's 200 ms peak.
float Glow::_ripple_envelope_at_distance(int distance) const {
    long start_offset_ms = static_cast<long>(distance) * this->config->glow_ripple_start_delay_ms;
    long end_lag_ms = static_cast<long>(distance) * this->config->glow_ripple_peak_lag_ms;
    long elapsed_at_distance = static_cast<long>(this->fade_clock.elapsed_ms) - start_offset_ms;

    if (this->state == GLOW_FADING_IN || this->state == GLOW_PULSING) {
        float start_value = this->ripple_fade_in_start[distance];
        if (elapsed_at_distance <= 0) return start_value;
        long rise_duration_ms = this->config->glow_fade_in_duration_ms + end_lag_ms - start_offset_ms;
        if (rise_duration_ms < 1) rise_duration_ms = 1;
        float progress = static_cast<float>(elapsed_at_distance)
                       / static_cast<float>(rise_duration_ms);
        if (progress > 1.0f) progress = 1.0f;
        return start_value + (1.0f - start_value) * progress;
    }
    if (this->state == GLOW_FADING_OUT) {
        float start_value = this->ripple_fade_out_start[distance];
        if (elapsed_at_distance <= 0) return start_value;
        long fall_duration_ms = this->config->glow_fade_out_duration_ms + end_lag_ms - start_offset_ms;
        if (fall_duration_ms < 1) fall_duration_ms = 1;
        float progress = static_cast<float>(elapsed_at_distance)
                       / static_cast<float>(fall_duration_ms);
        if (progress > 1.0f) progress = 1.0f;
        return start_value * (1.0f - progress);
    }
    return 0.0f;
}

float Glow::_pulse_modulation() const {
    return this->_pulse_modulation_at_lag(0);
}

float Glow::_pulse_modulation_at_lag(long lag_ms) const {
    long lagged_elapsed_ms = static_cast<long>(this->pulse_clock.elapsed_ms) - lag_ms;
    long period_ms = this->pulse_period_ms;

    if (lagged_elapsed_ms < 0) {
        // The lag reaches into the previous pulse cycle. If there isn't one
        // (first cycle of this activation), the pulse hadn't started yet and
        // modulation was at its 100% resting value. Same clamp if the lag
        // reaches beyond the one remembered cycle (only possible when
        // ripple_distance * peak_lag exceeds the minimum pulse period).
        if (this->previous_pulse_period_ms < 1) {
            return 1.0f;
        }
        period_ms = this->previous_pulse_period_ms;
        lagged_elapsed_ms += period_ms;
        if (lagged_elapsed_ms < 0) {
            return 1.0f;
        }
    }

    // Cosine dip: 1.0 at the cycle boundaries, glow_pulse_min_value at the
    // midpoint.
    float phase = static_cast<float>(lagged_elapsed_ms) / static_cast<float>(period_ms);
    if (phase > 1.0f) phase = 1.0f;
    float dip = 0.5f + 0.5f * cosf(phase * GLOW_FULL_CIRCLE_RADIANS);
    float min_value = this->config->glow_pulse_min_value;
    return min_value + (1.0f - min_value) * dip;
}
