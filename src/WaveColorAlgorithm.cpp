#include "WaveColorAlgorithm.h"

#include <cmath>
#include "Arduino.h"

static const float WAVE_INTENSITY_EPSILON = 0.001f;
static const float FULL_CIRCLE_RADIANS = 6.28318530717958647692f;

// HSV helpers for the overlap blend. Hue is in [0, 1).
struct Hsv {
    float hue;
    float saturation;
    float value;
};

static Hsv rgb_to_hsv(Color color) {
    float red_normalized   = static_cast<float>(color.red)   / 255.0f;
    float green_normalized = static_cast<float>(color.green) / 255.0f;
    float blue_normalized  = static_cast<float>(color.blue)  / 255.0f;

    float max_channel = red_normalized;
    if (green_normalized > max_channel) max_channel = green_normalized;
    if (blue_normalized  > max_channel) max_channel = blue_normalized;

    float min_channel = red_normalized;
    if (green_normalized < min_channel) min_channel = green_normalized;
    if (blue_normalized  < min_channel) min_channel = blue_normalized;

    float delta = max_channel - min_channel;

    Hsv hsv;
    hsv.value = max_channel;
    hsv.saturation = max_channel > 0 ? delta / max_channel : 0;

    if (delta <= 0) {
        hsv.hue = 0;
    } else if (max_channel == red_normalized) {
        float sector = (green_normalized - blue_normalized) / delta;
        if (sector < 0) sector += 6.0f;
        hsv.hue = sector / 6.0f;
    } else if (max_channel == green_normalized) {
        hsv.hue = ((blue_normalized - red_normalized) / delta + 2.0f) / 6.0f;
    } else {
        hsv.hue = ((red_normalized - green_normalized) / delta + 4.0f) / 6.0f;
    }
    return hsv;
}

static Color hsv_to_rgb(float hue, float saturation, float value) {
    float chroma = value * saturation;
    float hue_prime = hue * 6.0f;
    float secondary = chroma * (1.0f - fabsf(fmodf(hue_prime, 2.0f) - 1.0f));
    float match_value = value - chroma;

    float red_prime, green_prime, blue_prime;
    if      (hue_prime < 1.0f) { red_prime = chroma;    green_prime = secondary; blue_prime = 0; }
    else if (hue_prime < 2.0f) { red_prime = secondary; green_prime = chroma;    blue_prime = 0; }
    else if (hue_prime < 3.0f) { red_prime = 0;         green_prime = chroma;    blue_prime = secondary; }
    else if (hue_prime < 4.0f) { red_prime = 0;         green_prime = secondary; blue_prime = chroma; }
    else if (hue_prime < 5.0f) { red_prime = secondary; green_prime = 0;         blue_prime = chroma; }
    else                       { red_prime = chroma;    green_prime = 0;         blue_prime = secondary; }

    int red   = static_cast<int>((red_prime   + match_value) * 255.0f + 0.5f);
    int green = static_cast<int>((green_prime + match_value) * 255.0f + 0.5f);
    int blue  = static_cast<int>((blue_prime  + match_value) * 255.0f + 0.5f);
    if (red   < 0) red   = 0;
    if (red   > 255) red   = 255;
    if (green < 0) green = 0;
    if (green > 255) green = 255;
    if (blue  < 0) blue  = 0;
    if (blue  > 255) blue  = 255;
    return Color(red, green, blue);
}

WaveColorAlgorithm::WaveColorAlgorithm(Config* config,
                                       Sensor* sensors,
                                       int number_of_panels,
                                       int number_of_sensors) {
    this->config = config;
    this->sensors = sensors;
    this->number_of_panels = number_of_panels;
    this->number_of_sensors = number_of_sensors;

    this->waves = new Wave*[number_of_sensors];
    for (int sensor_index = 0; sensor_index < number_of_sensors; sensor_index++) {
        int origin_panel_index = sensor_index / 2;
        this->waves[sensor_index] = new Wave(&sensors[sensor_index], config, origin_panel_index);
    }

    this->displayed_colors = new Color[number_of_panels];
    for (int panel_index = 0; panel_index < number_of_panels; panel_index++) {
        this->displayed_colors[panel_index] = config->wave_idle_color;
    }

    this->prev_sensor_active = new bool[number_of_sensors];
    for (int sensor_index = 0; sensor_index < number_of_sensors; sensor_index++) {
        this->prev_sensor_active[sensor_index] = false;
    }

    this->rainbow_target_active = false;
    this->rainbow_blend = 0.0f;
    this->blend_at_transition_start = 0.0f;
}

void WaveColorAlgorithm::update() {
    // Step 1: detect rising sensor edges and assign a target color (shared
    // with a nearby active wave, or freshly picked).
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        bool active_now = this->sensors[sensor_index].active;
        bool was_active = this->prev_sensor_active[sensor_index];
        this->prev_sensor_active[sensor_index] = active_now;

        // Only start a wave on a rising edge if the wave is currently idle.
        // PIR smoothing can briefly dip below the active threshold and recover
        // mid-pulse; treating that as a new activation would restart the
        // pulse_clock and snap the propagation back to the source. The
        // end-of-pulse path in Wave::update_pulse_state already re-checks the
        // sensor and restarts cleanly at the natural cycle boundary.
        if (active_now && !was_active) {
            Wave* wave = this->waves[sensor_index];
            if (!wave->is_active()) {
                int origin = wave->get_origin_panel_index();
                Color target = this->_pick_target_for_activation(sensor_index, origin);
                wave->start_activation(target);
            }
        }
    }

    // Step 2: advance each wave's pulse state. Keep this running even in
    // rainbow mode so that when we drop back to wave mode the waves are in
    // their natural positions rather than frozen in time.
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        this->waves[sensor_index]->update_pulse_state();
    }

    // Step 3: detect rainbow target changes and advance the crossfade blend.
    bool high_interaction_now = this->_is_high_interaction();
    if (high_interaction_now != this->rainbow_target_active) {
        this->blend_at_transition_start = this->rainbow_blend;
        this->transition_clock.restart();
        this->rainbow_target_active = high_interaction_now;
    }

    int transition_duration_ms = this->config->wave_rainbow_transition_duration_ms;
    if (transition_duration_ms < 1) transition_duration_ms = 1;
    this->transition_clock.update();
    float transition_progress = static_cast<float>(this->transition_clock.elapsed_ms)
                              / static_cast<float>(transition_duration_ms);
    if (transition_progress > 1.0f) transition_progress = 1.0f;
    float blend_target = this->rainbow_target_active ? 1.0f : 0.0f;
    this->rainbow_blend = this->blend_at_transition_start
                       + (blend_target - this->blend_at_transition_start) * transition_progress;

    // Rainbow scroll clock keeps ticking once rainbow has any presence so
    // mid-fade transitions don't snap the rainbow back to offset zero.
    if (this->rainbow_blend > 0.0f && this->rainbow_clock.isStopped()) {
        this->rainbow_clock.restart();
    }
    if (!this->rainbow_clock.isStopped()) {
        this->rainbow_clock.update();
    }

    // Step 4: resolve per-panel color. Always compute the wave-blended color;
    // if any rainbow is visible, lerp toward the rainbow sample.
    Color idle_color = this->config->wave_idle_color;

    for (int panel_index = 0; panel_index < this->number_of_panels; panel_index++) {

        // For ≥2 waves we sum chroma vectors in the unit disk and apply the
        // overlap rule: saturation = min(1, magnitude); excess magnitude
        // above 1 rotates the hue. Using the target color (instead of the
        // post-intensity displayed color) keeps hue stable at low
        // intensities — otherwise idle red dominates the lerp and the hue
        // reads as 0 (red) even for a cyan-target wave.
        int contributing_wave_count = 0;
        float hue_cos_sum = 0.0f;
        float hue_sin_sum = 0.0f;
        float intensity_weight_sum = 0.0f;
        float max_intensity = 0.0f;
        float strongest_intensity_at_panel = 0.0f;
        float strongest_target_hue = 0.0f;
        Color single_wave_displayed = idle_color;

        for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
            Wave* wave = this->waves[sensor_index];
            if (!wave->is_active()) continue;

            float wave_intensity = wave->get_intensity_for_panel(panel_index);
            if (wave_intensity <= WAVE_INTENSITY_EPSILON) continue;

            Color wave_target = wave->get_color_for_panel(panel_index);
            Color wave_displayed = idle_color.interpolate(wave_target, wave_intensity);

            contributing_wave_count++;
            single_wave_displayed = wave_displayed;

            Hsv target_hsv = rgb_to_hsv(wave_target);
            hue_cos_sum += cosf(target_hsv.hue * FULL_CIRCLE_RADIANS) * wave_intensity;
            hue_sin_sum += sinf(target_hsv.hue * FULL_CIRCLE_RADIANS) * wave_intensity;
            intensity_weight_sum += wave_intensity;
            if (wave_intensity > max_intensity) max_intensity = wave_intensity;
            // Stable reference hue for the coherence-fallback branch. First
            // wave to tie wins, which is deterministic across ticks because
            // sensor iteration order is fixed.
            if (wave_intensity > strongest_intensity_at_panel) {
                strongest_intensity_at_panel = wave_intensity;
                strongest_target_hue = target_hsv.hue;
            }
        }

        Color resolved;
        if (contributing_wave_count == 0) {
            resolved = idle_color;
        } else if (contributing_wave_count == 1) {
            resolved = single_wave_displayed;
        } else {
            float chroma_magnitude = sqrtf(hue_cos_sum * hue_cos_sum + hue_sin_sum * hue_sin_sum);
            float coherence = intensity_weight_sum > 0 ? chroma_magnitude / intensity_weight_sum : 0;

            // When coherence is high enough, atan2 gives a well-defined
            // direction and chroma_magnitude is the right rotation driver.
            // When the chroma sum collapses (waves on opposite sides of the
            // wheel cancel), anchor to the brightest contributing wave's
            // hue and drive rotation off total chromatic presence instead.
            // Both branches produce a saturated rotated color — no gray
            // fallback.
            float base_hue;
            float effective_magnitude;
            if (coherence >= this->config->wave_overlap_coherence_fallback_threshold) {
                base_hue = atan2f(hue_sin_sum, hue_cos_sum) / FULL_CIRCLE_RADIANS;
                if (base_hue < 0) base_hue += 1.0f;
                effective_magnitude = chroma_magnitude;
            } else {
                base_hue = strongest_target_hue;
                effective_magnitude = intensity_weight_sum;
            }

            float overshoot = effective_magnitude > 1.0f ? effective_magnitude - 1.0f : 0.0f;
            float resolved_hue = base_hue + overshoot * this->config->wave_overlap_hue_rotation_per_unit;
            resolved_hue = resolved_hue - floorf(resolved_hue);
            if (resolved_hue < 0) resolved_hue += 1.0f;

            // Saturation is always 1 in the ≥2-wave path. Tying it to
            // chroma_magnitude let low-intensity overlap collapse to gray
            // (two faint same-hue waves at the edge of their range produced
            // a near-zero chroma sum → near-zero saturation). Wave-edge
            // fade is already handled by resolved_value (max_intensity).
            float resolved_saturation = 1.0f;

            // Brightness pins to the brightest contributing wave so a faint
            // wave overlapping a bright one doesn't dim the bright one.
            // Approximate the displayed value as max_intensity * target value
            // (target value is 1 for our 6 primaries); add the small idle
            // contribution so single-wave brightness matches.
            float idle_value = static_cast<float>(idle_color.red) / 255.0f;
            if (static_cast<float>(idle_color.green) / 255.0f > idle_value) idle_value = static_cast<float>(idle_color.green) / 255.0f;
            if (static_cast<float>(idle_color.blue) / 255.0f > idle_value) idle_value = static_cast<float>(idle_color.blue) / 255.0f;
            float resolved_value = idle_value * (1.0f - max_intensity) + max_intensity;

            resolved = hsv_to_rgb(resolved_hue, resolved_saturation, resolved_value);
        }

        // Crossfade in/out of the rainbow override.
        Color final_color;
        if (this->rainbow_blend <= 0.0f) {
            final_color = resolved;
        } else if (this->rainbow_blend >= 1.0f) {
            final_color = this->_rainbow_color_for_panel(panel_index);
        } else {
            Color rainbow_color = this->_rainbow_color_for_panel(panel_index);
            final_color = resolved.interpolate(rainbow_color, this->rainbow_blend);
        }
        // Multi-wave HSV blend can hit saturation 0 at value 1 when hues
        // cancel — clamp so the panel never displays pure white.
        this->displayed_colors[panel_index] = final_color.limit();
    }
}

Color WaveColorAlgorithm::get_color_for_panel(int panel_index) const {
    return this->displayed_colors[panel_index];
}

Color WaveColorAlgorithm::_pick_target_for_activation(int activating_sensor_index, int activating_origin_panel) {
    // Adjacent = origin panels within 1 of each other (also covers the
    // sibling sensor on the same panel, distance 0). Anything farther is
    // treated as unrelated and picks a fresh random hue.
    const int neighbor_max_distance = 1;

    int closest_distance = neighbor_max_distance + 1;
    Color neighbor_color = this->config->wave_idle_color;
    bool found_neighbor = false;

    for (int other_sensor_index = 0; other_sensor_index < this->number_of_sensors; other_sensor_index++) {
        if (other_sensor_index == activating_sensor_index) continue;
        Wave* other = this->waves[other_sensor_index];
        if (!other->is_active()) continue;
        int other_origin = other->get_origin_panel_index();
        int origin_distance = other_origin - activating_origin_panel;
        if (origin_distance < 0) origin_distance = -origin_distance;
        if (origin_distance > neighbor_max_distance) continue;
        if (origin_distance < closest_distance) {
            closest_distance = origin_distance;
            neighbor_color = other->get_target_color();
            found_neighbor = true;
        }
    }

    if (found_neighbor) {
        // Shift the neighbor's hue by 1/20 of the wheel so a chain of
        // adjacent activations forms a smooth gradient instead of one
        // dominant color.
        Hsv neighbor_hsv = rgb_to_hsv(neighbor_color);
        float shifted_hue = neighbor_hsv.hue + (1.0f / 20.0f);
        if (shifted_hue >= 1.0f) shifted_hue -= 1.0f;
        return hsv_to_rgb(shifted_hue, 1.0f, 1.0f);
    }
    return this->_pick_random_saturated_color();
}

bool WaveColorAlgorithm::_is_high_interaction() const {
    int active_sensor_count = 0;
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        if (this->sensors[sensor_index].active) active_sensor_count++;
    }
    float fraction = static_cast<float>(active_sensor_count) / static_cast<float>(this->number_of_sensors);
    return fraction >= this->config->high_interaction_threshold_percent;
}

Color WaveColorAlgorithm::_rainbow_color_for_panel(int panel_index) const {
    int scroll_duration_ms = this->config->wave_rainbow_scroll_duration_ms;
    if (scroll_duration_ms <= 0) scroll_duration_ms = 8000;

    float scroll_offset = static_cast<float>(this->rainbow_clock.elapsed_ms)
                        / static_cast<float>(scroll_duration_ms);
    float panel_fraction = static_cast<float>(panel_index)
                         / static_cast<float>(this->number_of_panels);
    float hue = panel_fraction + scroll_offset;
    hue = hue - floorf(hue);
    if (hue < 0) hue += 1.0f;
    return hsv_to_rgb(hue, 1.0f, 1.0f);
}

Color WaveColorAlgorithm::_pick_random_saturated_color() const {
    float candidate_hue = static_cast<float>(random(10000)) / 10000.0f;
    return hsv_to_rgb(candidate_hue, 1.0f, 1.0f);
}
