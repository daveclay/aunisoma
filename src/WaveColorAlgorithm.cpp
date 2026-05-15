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
}

void WaveColorAlgorithm::update() {
    // Step 1: detect rising sensor edges and assign a target color (shared
    // with a nearby active wave, or freshly picked).
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        bool active_now = this->sensors[sensor_index].active;
        bool was_active = this->prev_sensor_active[sensor_index];
        this->prev_sensor_active[sensor_index] = active_now;

        if (active_now && !was_active) {
            Wave* wave = this->waves[sensor_index];
            int origin = wave->get_origin_panel_index();
            Color target = this->_pick_shared_or_new_target(sensor_index, origin);
            wave->start_activation(target);
        }
    }

    // Step 2: advance each wave's pulse state.
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        this->waves[sensor_index]->update_pulse_state();
    }

    // Step 3: resolve per-panel color from contributing waves.
    Color idle_color = this->config->wave_idle_color;

    for (int panel_index = 0; panel_index < this->number_of_panels; panel_index++) {
        // For ≥2 waves we blend in HSV: intensity-weighted circular mean of
        // target hues, intensity-weighted mean of target saturations, and
        // max of intensity for brightness. Using the target color (instead of
        // the post-intensity displayed color) keeps hue stable at low
        // intensities — otherwise idle red dominates the lerp and the hue
        // reads as 0 (red) even for a cyan-target wave.
        int contributing_wave_count = 0;
        float hue_cos_sum = 0.0f;
        float hue_sin_sum = 0.0f;
        float saturation_weighted_sum = 0.0f;
        float intensity_weight_sum = 0.0f;
        float max_intensity = 0.0f;
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
            saturation_weighted_sum += target_hsv.saturation * wave_intensity;
            intensity_weight_sum += wave_intensity;
            if (wave_intensity > max_intensity) max_intensity = wave_intensity;
        }

        Color resolved;
        if (contributing_wave_count == 0) {
            resolved = idle_color;
        } else if (contributing_wave_count == 1) {
            resolved = single_wave_displayed;
        } else {
            // Coherence = how aligned the contributing target hues are.
            // 1.0 = all waves on the same hue, 0.0 = perfectly cancelling
            // (e.g. complementary primaries with equal intensity).
            float magnitude = sqrtf(hue_cos_sum * hue_cos_sum + hue_sin_sum * hue_sin_sum);
            float coherence = intensity_weight_sum > 0 ? magnitude / intensity_weight_sum : 0;

            float resolved_hue = atan2f(hue_sin_sum, hue_cos_sum) / FULL_CIRCLE_RADIANS;
            if (resolved_hue < 0) resolved_hue += 1.0f;

            float mean_target_saturation = intensity_weight_sum > 0
                ? saturation_weighted_sum / intensity_weight_sum
                : 0;

            // When hues sit on opposite sides of the wheel and cancel (e.g.
            // red + cyan), the resulting direction is unstable. Fall back to
            // a neutral gray at the brighter wave's intensity — the natural
            // midpoint of two complements.
            float resolved_saturation = coherence < 0.1f ? 0.0f : mean_target_saturation;

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

        this->displayed_colors[panel_index] = resolved;
    }
}

Color WaveColorAlgorithm::get_color_for_panel(int panel_index) const {
    return this->displayed_colors[panel_index];
}

Color WaveColorAlgorithm::_pick_shared_or_new_target(int activating_sensor_index, int activating_origin_panel) {
    // "Gap of 2 inactive panels" between two origin panels P1 and P2 means
    // |P1 - P2| - 1 == 2, i.e. |P1 - P2| == 3. We share when the gap is 0
    // or 1 inactive panels between, so |P1 - P2| <= 2.
    const int max_share_origin_distance = 2;

    int best_distance = max_share_origin_distance + 1;
    Color best_color = this->config->wave_idle_color;
    bool found_shared = false;

    for (int other_sensor_index = 0; other_sensor_index < this->number_of_sensors; other_sensor_index++) {
        if (other_sensor_index == activating_sensor_index) continue;
        Wave* other = this->waves[other_sensor_index];
        if (!other->is_active()) continue;
        int other_origin = other->get_origin_panel_index();
        int origin_distance = other_origin - activating_origin_panel;
        if (origin_distance < 0) origin_distance = -origin_distance;
        if (origin_distance > max_share_origin_distance) continue;
        if (origin_distance < best_distance) {
            best_distance = origin_distance;
            best_color = other->get_target_color();
            found_shared = true;
        }
    }

    if (found_shared) {
        return best_color;
    }
    return this->_pick_random_target(this->displayed_colors[activating_origin_panel]);
}

Color WaveColorAlgorithm::_pick_random_target(Color /*exclude_color*/) const {
    // Pick a continuous hue rather than one of 6 primaries. Constraints
    // (rejection sampled):
    //   - circular distance to any in-use wave hue >= min_same_distance
    //     (so unrelated interactions are visibly distinct)
    //   - circular distance to any in-use wave hue <= 0.5 - min_opposite_distance
    //     (so two overlapping waves never cancel to gray)
    //   - circular distance to the idle color hue >= min_same_distance
    //     (so the activation is visible against the background)
    // Saturation and value are pinned to 1 so the picked color is a bright,
    // saturated point on the rainbow.
    constexpr float min_same_distance     = 0.06f;
    constexpr float min_opposite_distance = 0.06f;
    constexpr int   max_attempts          = 32;

    Hsv idle_hsv = rgb_to_hsv(this->config->wave_idle_color);
    float idle_hue = idle_hsv.hue;

    float in_use_hues[40];  // sized for NUMBER_OF_SENSORS upper bound
    int in_use_count = 0;
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        Wave* wave = this->waves[sensor_index];
        if (!wave->is_active()) continue;
        Hsv target_hsv = rgb_to_hsv(wave->get_target_color());
        in_use_hues[in_use_count++] = target_hsv.hue;
    }

    float candidate_hue = 0.0f;
    for (int attempt = 0; attempt < max_attempts; attempt++) {
        candidate_hue = static_cast<float>(random(10000)) / 10000.0f;

        // Same-hue distance from idle.
        float distance_to_idle = candidate_hue - idle_hue;
        if (distance_to_idle < 0) distance_to_idle = -distance_to_idle;
        if (distance_to_idle > 0.5f) distance_to_idle = 1.0f - distance_to_idle;
        if (distance_to_idle < min_same_distance) continue;

        // Same-hue and opposite-hue distance from each in-use wave.
        bool conflict = false;
        for (int in_use_index = 0; in_use_index < in_use_count; in_use_index++) {
            float distance = candidate_hue - in_use_hues[in_use_index];
            if (distance < 0) distance = -distance;
            if (distance > 0.5f) distance = 1.0f - distance;
            if (distance < min_same_distance) { conflict = true; break; }
            if (distance > 0.5f - min_opposite_distance) { conflict = true; break; }
        }
        if (conflict) continue;

        return hsv_to_rgb(candidate_hue, 1.0f, 1.0f);
    }

    // Couldn't satisfy all constraints in time (e.g. too many in-use hues);
    // accept whatever was last sampled rather than spin forever.
    return hsv_to_rgb(candidate_hue, 1.0f, 1.0f);
}
