#include "GlowColorAlgorithm.h"

#include <cmath>
#include "Arduino.h"

static const float GLOW_INTENSITY_EPSILON = 0.001f;
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

GlowColorAlgorithm::GlowColorAlgorithm(Config* config,
                                       Sensor* sensors,
                                       int number_of_panels,
                                       int number_of_sensors) {
    this->config = config;
    this->sensors = sensors;
    this->number_of_panels = number_of_panels;
    this->number_of_sensors = number_of_sensors;

    this->glows = new Glow*[number_of_sensors];
    for (int sensor_index = 0; sensor_index < number_of_sensors; sensor_index++) {
        int panel_index = sensor_index / 2;
        this->glows[sensor_index] = new Glow(&sensors[sensor_index], config, panel_index);
    }

    this->displayed_colors = new Color[number_of_panels];
    for (int panel_index = 0; panel_index < number_of_panels; panel_index++) {
        this->displayed_colors[panel_index] = config->wave_idle_color;
    }

    this->prev_sensor_active = new bool[number_of_sensors];
    this->glow_target_hue = new float[number_of_sensors];
    this->glow_hue_cos = new float[number_of_sensors];
    this->glow_hue_sin = new float[number_of_sensors];
    for (int sensor_index = 0; sensor_index < number_of_sensors; sensor_index++) {
        this->prev_sensor_active[sensor_index] = false;
        this->glow_target_hue[sensor_index] = 0.0f;
        this->glow_hue_cos[sensor_index] = 1.0f;
        this->glow_hue_sin[sensor_index] = 0.0f;
    }

    this->rainbow_target_active = false;
    this->rainbow_blend = 0.0f;
    this->blend_at_transition_start = 0.0f;
}

void GlowColorAlgorithm::update() {
    // Step 1: detect rising sensor edges. A fresh activation picks a target
    // color (shared with a nearby active glow, or freshly picked); a rising
    // edge during fall-off keeps the color and fades back in from wherever
    // the intensity currently is. Either way the pulse phase restarts on the
    // rising edge, which is what keeps a panel's front and back pulses
    // intermingled instead of in sync.
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        bool active_now = this->sensors[sensor_index].active;
        bool was_active = this->prev_sensor_active[sensor_index];
        this->prev_sensor_active[sensor_index] = active_now;

        if (active_now && !was_active) {
            Glow* glow = this->glows[sensor_index];
            if (!glow->is_active()) {
                int panel_index = glow->get_panel_index();
                Color target = this->_pick_target_for_activation(sensor_index, panel_index);
                glow->start_activation(target);
                // Cache the target's hue and unit chroma vector for the
                // blend loop; a reactivation keeps its color, so the cached
                // values stay valid for the glow's whole lifetime.
                Hsv target_hsv = rgb_to_hsv(target);
                this->glow_target_hue[sensor_index] = target_hsv.hue;
                this->glow_hue_cos[sensor_index] = cosf(target_hsv.hue * FULL_CIRCLE_RADIANS);
                this->glow_hue_sin[sensor_index] = sinf(target_hsv.hue * FULL_CIRCLE_RADIANS);
            } else {
                glow->reactivate();
            }
        }
    }

    // Step 2: advance each glow's fade/pulse state. Keep this running even in
    // rainbow mode so that when we drop back to glow mode the pulses are in
    // their natural positions rather than frozen in time.
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        this->glows[sensor_index]->update();
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

    // Step 4: resolve per-panel color. A panel can be touched by its own two
    // sensors' glows and by the lagged neighbor envelopes of glows within
    // glow_ripple_distance. One contributor lerps idle toward its target;
    // two or more blend chroma vectors in the unit disk with the overlap
    // rule: saturation = min(1, magnitude), and excess magnitude above 1
    // rotates the hue.
    Color idle_color = this->config->wave_idle_color;

    // Brightest idle channel, used to pin multi-glow brightness; constant
    // per tick, so computed once here instead of per panel.
    float idle_value = static_cast<float>(idle_color.red) / 255.0f;
    if (static_cast<float>(idle_color.green) / 255.0f > idle_value) idle_value = static_cast<float>(idle_color.green) / 255.0f;
    if (static_cast<float>(idle_color.blue) / 255.0f > idle_value) idle_value = static_cast<float>(idle_color.blue) / 255.0f;

    for (int panel_index = 0; panel_index < this->number_of_panels; panel_index++) {

        int contributing_glow_count = 0;
        float hue_cos_sum = 0.0f;
        float hue_sin_sum = 0.0f;
        float intensity_weight_sum = 0.0f;
        float max_intensity = 0.0f;
        float strongest_intensity_at_panel = 0.0f;
        float strongest_target_hue = 0.0f;
        Color single_glow_displayed = idle_color;

        for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
            Glow* glow = this->glows[sensor_index];
            if (!glow->is_active()) continue;

            float glow_intensity = glow->get_intensity_for_panel(panel_index);
            if (glow_intensity <= GLOW_INTENSITY_EPSILON) continue;

            Color glow_target = glow->get_target_color();
            Color glow_displayed = idle_color.interpolate(glow_target, glow_intensity);

            contributing_glow_count++;
            single_glow_displayed = glow_displayed;

            hue_cos_sum += this->glow_hue_cos[sensor_index] * glow_intensity;
            hue_sin_sum += this->glow_hue_sin[sensor_index] * glow_intensity;
            intensity_weight_sum += glow_intensity;
            if (glow_intensity > max_intensity) max_intensity = glow_intensity;
            // Stable reference hue for the coherence-fallback branch. First
            // glow to tie wins, which is deterministic across ticks because
            // sensor iteration order is fixed.
            if (glow_intensity > strongest_intensity_at_panel) {
                strongest_intensity_at_panel = glow_intensity;
                strongest_target_hue = this->glow_target_hue[sensor_index];
            }
        }

        Color resolved;
        if (contributing_glow_count == 0) {
            resolved = idle_color;
        } else if (contributing_glow_count == 1) {
            resolved = single_glow_displayed;
        } else {
            float chroma_magnitude = sqrtf(hue_cos_sum * hue_cos_sum + hue_sin_sum * hue_sin_sum);
            float coherence = intensity_weight_sum > 0 ? chroma_magnitude / intensity_weight_sum : 0;

            // When coherence is high enough, atan2 gives a well-defined
            // direction and chroma_magnitude is the right rotation driver.
            // When the chroma sum collapses (hues on opposite sides of the
            // wheel cancel), anchor to the brightest contributing glow's
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

            // Saturation stays 1 in the 2-glow path; the pulse dips are
            // brightness dips, not desaturation.
            float resolved_saturation = 1.0f;

            // Brightness pins to the brighter contributing glow so the two
            // out-of-phase pulses read as intermingled shimmer rather than
            // multiplying each other darker.
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
        this->displayed_colors[panel_index] = final_color.limit();
    }
}

Color GlowColorAlgorithm::get_color_for_panel(int panel_index) const {
    return this->displayed_colors[panel_index];
}

Color GlowColorAlgorithm::_pick_target_for_activation(int activating_sensor_index, int activating_panel_index) {
    // Adjacent = panels within 1 of each other (also covers the sibling
    // sensor on the same panel, distance 0). Anything farther is treated as
    // unrelated and picks a fresh random hue.
    const int neighbor_max_distance = 1;

    int closest_distance = neighbor_max_distance + 1;
    Color neighbor_color = this->config->wave_idle_color;
    bool found_neighbor = false;

    for (int other_sensor_index = 0; other_sensor_index < this->number_of_sensors; other_sensor_index++) {
        if (other_sensor_index == activating_sensor_index) continue;
        Glow* other = this->glows[other_sensor_index];
        if (!other->is_active()) continue;
        int other_panel = other->get_panel_index();
        int panel_distance = other_panel - activating_panel_index;
        if (panel_distance < 0) panel_distance = -panel_distance;
        if (panel_distance > neighbor_max_distance) continue;
        if (panel_distance < closest_distance) {
            closest_distance = panel_distance;
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

bool GlowColorAlgorithm::_is_high_interaction() const {
    int active_sensor_count = 0;
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        if (this->sensors[sensor_index].active) active_sensor_count++;
    }
    float fraction = static_cast<float>(active_sensor_count) / static_cast<float>(this->number_of_sensors);
    return fraction >= this->config->high_interaction_threshold_percent;
}

Color GlowColorAlgorithm::_rainbow_color_for_panel(int panel_index) const {
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

Color GlowColorAlgorithm::_pick_random_saturated_color() const {
    float candidate_hue = static_cast<float>(random(10000)) / 10000.0f;
    return hsv_to_rgb(candidate_hue, 1.0f, 1.0f);
}
