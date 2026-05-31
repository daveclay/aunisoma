#include "Wave.h"

#include <cmath>
#include "Arduino.h"

Wave::Wave(Sensor* sensor, Config* config, int origin_panel_index) {
    this->sensor = sensor;
    this->config = config;
    this->origin_panel_index = origin_panel_index;
    this->max_distance = config->wave_max_propagation_distance;
    if (this->max_distance < 1) this->max_distance = 1;

    this->state = WAVE_IDLE;
    this->target_color = config->wave_idle_color;
}

void Wave::start_activation(Color target) {
    this->target_color = target;
    this->_start_new_pulse();
}

void Wave::update_pulse_state() {
    if (this->state == WAVE_PROPAGATING) {
        this->pulse_clock.update();
        int total_pulse_ms = this->_pulse_total_duration_ms();
        int inter_pulse_delay_ms = this->config->wave_inter_pulse_delay_ms;

        if (static_cast<long>(this->pulse_clock.elapsed_ms) >= total_pulse_ms + inter_pulse_delay_ms) {
            if (this->sensor->active) {
                // Same target color, just restart the propagation.
                this->_start_new_pulse();
            } else {
                this->state = WAVE_IDLE;
                this->pulse_clock.stop();
                this->target_color = this->config->wave_idle_color;
            }
        }
    }
}

Color Wave::get_color_for_panel(int /*panel_index*/) const {
    return this->target_color;
}

float Wave::get_intensity_for_panel(int panel_index) const {
    if (this->state != WAVE_PROPAGATING) {
        return 0.0f;
    }

    int distance = panel_index - this->origin_panel_index;
    if (distance < 0) distance = -distance;
    if (distance > this->max_distance) {
        return 0.0f;
    }

    // Exponential spatial falloff: exp(-k*distance). 1.0 at the origin,
    // decays with configurable constant k. max_distance is a hard cutoff
    // (early-returned above) rather than a normalization edge, so the panel
    // at max_distance still contributes exp(-k*max_distance) instead of 0.
    float decay = this->config->wave_spatial_decay_constant;
    float falloff = expf(-decay * static_cast<float>(distance));
    float envelope = this->_panel_envelope_value(panel_index);
    return falloff * envelope;
}

bool Wave::is_active() const {
    return this->state != WAVE_IDLE;
}

int Wave::get_origin_panel_index() const {
    return this->origin_panel_index;
}

Color Wave::get_target_color() const {
    return this->target_color;
}

void Wave::_start_new_pulse() {
    this->pulse_clock.restart();
    this->state = WAVE_PROPAGATING;
}

int Wave::_pulse_total_duration_ms() const {
    return this->max_distance * this->config->wave_panel_delay_ms
         + this->config->wave_rise_duration_ms
         + this->config->wave_fall_duration_ms;
}

float Wave::_panel_envelope_value(int panel_index) const {
    int distance = panel_index - this->origin_panel_index;
    if (distance < 0) distance = -distance;

    int panel_delay_ms = distance * this->config->wave_panel_delay_ms;
    long elapsed_in_panel = static_cast<long>(this->pulse_clock.elapsed_ms) - panel_delay_ms;
    if (elapsed_in_panel <= 0) return 0.0f;

    int rise_duration_ms = this->config->wave_rise_duration_ms;
    int fall_duration_ms = this->config->wave_fall_duration_ms;

    if (elapsed_in_panel < rise_duration_ms) {
        if (rise_duration_ms <= 0) return 1.0f;
        return static_cast<float>(elapsed_in_panel) / static_cast<float>(rise_duration_ms);
    }

    long elapsed_in_fall = elapsed_in_panel - rise_duration_ms;
    if (elapsed_in_fall < fall_duration_ms) {
        if (fall_duration_ms <= 0) return 0.0f;
        return 1.0f - static_cast<float>(elapsed_in_fall) / static_cast<float>(fall_duration_ms);
    }

    return 0.0f;
}
