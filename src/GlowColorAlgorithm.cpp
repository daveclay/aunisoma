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

// HSV hue is not perceptually uniform: red→orange→yellow is packed into the
// first sixth of the wheel while the green and blue bands sprawl across half
// of it and read as one color each. Stepping or spreading raw hue evenly
// therefore makes greens/blues dominate the sculpture. These anchor tables
// define a piecewise-linear warp between a uniform "wheel position" (equal
// steps look like equal color changes) and HSV hue: warm hues get a wider
// share of the position axis, green/cyan/blue a narrower one.
static const int HUE_WARP_ANCHOR_COUNT = 8;
static const float HUE_WARP_POSITIONS[HUE_WARP_ANCHOR_COUNT] =
    { 0.00f, 0.27f, 0.38f, 0.42f, 0.48f, 0.60f, 0.70f, 1.00f };
static const float HUE_WARP_HUES[HUE_WARP_ANCHOR_COUNT] =
    { 0.000f, 0.083f, 0.167f, 0.333f, 0.500f, 0.667f, 0.792f, 1.000f };
//    red     orange  yellow  green   cyan    blue    violet  red

static float hue_from_wheel_position(float wheel_position) {
    wheel_position -= floorf(wheel_position);
    for (int segment = 1; segment < HUE_WARP_ANCHOR_COUNT; segment++) {
        if (wheel_position <= HUE_WARP_POSITIONS[segment]) {
            float segment_span = HUE_WARP_POSITIONS[segment] - HUE_WARP_POSITIONS[segment - 1];
            float segment_fraction = (wheel_position - HUE_WARP_POSITIONS[segment - 1]) / segment_span;
            return HUE_WARP_HUES[segment - 1]
                 + (HUE_WARP_HUES[segment] - HUE_WARP_HUES[segment - 1]) * segment_fraction;
        }
    }
    return 0.0f;
}

static float wheel_position_from_hue(float hue) {
    hue -= floorf(hue);
    for (int segment = 1; segment < HUE_WARP_ANCHOR_COUNT; segment++) {
        if (hue <= HUE_WARP_HUES[segment]) {
            float segment_span = HUE_WARP_HUES[segment] - HUE_WARP_HUES[segment - 1];
            float segment_fraction = (hue - HUE_WARP_HUES[segment - 1]) / segment_span;
            return HUE_WARP_POSITIONS[segment - 1]
                 + (HUE_WARP_POSITIONS[segment] - HUE_WARP_POSITIONS[segment - 1]) * segment_fraction;
        }
    }
    return 0.0f;
}

GlowColorAlgorithm::GlowColorAlgorithm(Config* config,
                                       Sensor* sensors,
                                       int number_of_panels,
                                       int number_of_sensors,
                                       GradientValueMap* knight_rider_gradient) {
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

    this->dance_clocks = new Clock[number_of_panels];
    this->dance_active = new bool[number_of_panels];
    this->dance_anchor_is_front = new bool[number_of_panels];
    for (int panel_index = 0; panel_index < number_of_panels; panel_index++) {
        this->dance_active[panel_index] = false;
        this->dance_anchor_is_front[panel_index] = true;
    }
    this->prev_glow_active = new bool[number_of_sensors];
    for (int sensor_index = 0; sensor_index < number_of_sensors; sensor_index++) {
        this->prev_glow_active[sensor_index] = false;
    }

    this->rainbow_target_active = false;
    this->rainbow_blend = 0.0f;
    this->blend_at_transition_start = 0.0f;

    this->knight_rider_animation = new KnightRiderAnimation(
        knight_rider_gradient, config->glow_knight_rider_sweep_duration_ms);
    this->knight_rider_target_active = false;
    this->knight_rider_blend = 0.0f;
    this->knight_rider_blend_at_transition_start = 0.0f;

    this->flicker_colors = new Color[number_of_panels];
    this->flicker_from_colors = new Color[number_of_panels];
    this->flicker_to_colors = new Color[number_of_panels];
    this->flicker_change_started_ms = new unsigned long[number_of_panels];
    this->flicker_next_change_ms = new unsigned long[number_of_panels];
    this->flicker_spark_active = new bool[number_of_panels];
    this->flicker_spark_end_ms = new unsigned long[number_of_panels];
    this->flicker_panel_strength = new float[number_of_panels];
    for (int panel_index = 0; panel_index < number_of_panels; panel_index++) {
        this->flicker_colors[panel_index] = config->wave_idle_color;
        this->flicker_from_colors[panel_index] = config->wave_idle_color;
        this->flicker_to_colors[panel_index] = config->wave_idle_color;
        this->flicker_change_started_ms[panel_index] = 0;
        this->flicker_next_change_ms[panel_index] = 0;
        this->flicker_spark_active[panel_index] = false;
        this->flicker_spark_end_ms[panel_index] = 0;
        this->flicker_panel_strength[panel_index] = 0.0f;
    }
    this->flicker_target_active = false;
    this->flicker_blend = 0.0f;
    this->flicker_blend_at_transition_start = 0.0f;
}

void GlowColorAlgorithm::update() {
    // Step 1: detect rising sensor edges. A fresh activation picks a target
    // color (shared with a nearby active glow, or freshly picked); a rising
    // edge during fall-off keeps the color and fades back in from wherever
    // the intensity currently is. Either way the pulse phase restarts on the
    // rising edge, which is what keeps a panel's front and back pulses
    // intermingled instead of in sync.
    bool any_sensor_changed = false;
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        bool active_now = this->sensors[sensor_index].active;
        bool was_active = this->prev_sensor_active[sensor_index];
        this->prev_sensor_active[sensor_index] = active_now;
        if (active_now != was_active) any_sensor_changed = true;

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

    // Step 3: two-sided dance bookkeeping. While both of a panel's own glows
    // are active (someone on each side), the panel's color cycles between
    // the two glows' targets. The clock starts when the second glow arrives,
    // anchored on the glow that was already running so the cycle departs
    // from the color the panel is currently showing.
    for (int panel_index = 0; panel_index < this->number_of_panels; panel_index++) {
        int front_sensor_index = panel_index * 2;
        int back_sensor_index = front_sensor_index + 1;
        bool front_glow_active = this->glows[front_sensor_index]->is_active();
        bool back_glow_active = this->glows[back_sensor_index]->is_active();
        bool dance_active_now = front_glow_active && back_glow_active;

        if (dance_active_now && !this->dance_active[panel_index]) {
            // Anchor on the glow that was running last tick; if both rose in
            // the same tick, the front arbitrarily leads.
            this->dance_anchor_is_front[panel_index] =
                this->prev_glow_active[front_sensor_index]
                || !this->prev_glow_active[back_sensor_index];
            this->dance_clocks[panel_index].restart();
        } else if (!dance_active_now && this->dance_active[panel_index]) {
            this->dance_clocks[panel_index].stop();
        }
        this->dance_active[panel_index] = dance_active_now;
        this->prev_glow_active[front_sensor_index] = front_glow_active;
        this->prev_glow_active[back_sensor_index] = back_glow_active;

        if (!this->dance_clocks[panel_index].isStopped()) {
            this->dance_clocks[panel_index].update();
        }
    }

    // Step 4: detect rainbow target changes and advance the crossfade blend.
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

    // Step 5: idle attract. The idle clock runs only while every glow is
    // idle; once it reaches the configured delay the knight rider sweep
    // crossfades in for its run duration, then fades back to idle. Any
    // interaction (a glow exists again) stops the countdown and fades the
    // sweep out in favor of the normal glow animation.
    bool any_glow_active = false;
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        if (this->glows[sensor_index]->is_active()) {
            any_glow_active = true;
            break;
        }
    }

    if (any_glow_active) {
        this->idle_clock.stop();
    } else {
        if (this->idle_clock.isStopped()) {
            this->idle_clock.restart();
        }
        this->idle_clock.update();
    }

    if (!this->knight_rider_target_active
        && !any_glow_active
        && static_cast<long>(this->idle_clock.elapsed_ms) >= this->config->glow_knight_rider_idle_delay_ms) {
        this->knight_rider_target_active = true;
        this->knight_rider_animation->restart();
        this->knight_rider_run_clock.restart();
        this->knight_rider_blend_at_transition_start = this->knight_rider_blend;
        this->knight_rider_transition_clock.restart();
        // The countdown to the next visit starts now, so continued idleness
        // brings the sweep back every glow_knight_rider_idle_delay_ms.
        this->idle_clock.restart();
    }

    if (this->knight_rider_target_active) {
        this->knight_rider_run_clock.update();
        bool run_expired = static_cast<long>(this->knight_rider_run_clock.elapsed_ms)
                        >= this->config->glow_knight_rider_run_duration_ms;
        if (any_glow_active || run_expired) {
            this->knight_rider_target_active = false;
            this->knight_rider_run_clock.stop();
            this->knight_rider_blend_at_transition_start = this->knight_rider_blend;
            this->knight_rider_transition_clock.restart();
        }
    }

    int knight_rider_fade_ms = this->config->glow_knight_rider_fade_ms;
    if (knight_rider_fade_ms < 1) knight_rider_fade_ms = 1;
    this->knight_rider_transition_clock.update();
    float knight_rider_fade_progress = static_cast<float>(this->knight_rider_transition_clock.elapsed_ms)
                                     / static_cast<float>(knight_rider_fade_ms);
    if (knight_rider_fade_progress > 1.0f) knight_rider_fade_progress = 1.0f;
    float knight_rider_blend_target = this->knight_rider_target_active ? 1.0f : 0.0f;
    this->knight_rider_blend = this->knight_rider_blend_at_transition_start
        + (knight_rider_blend_target - this->knight_rider_blend_at_transition_start) * knight_rider_fade_progress;

    // Keep sweeping while the attract is wanted or still fading out. (On the
    // trigger tick the blend is still 0, so gating on blend alone would stop
    // the freshly restarted cycle and freeze the sweep at the left end.)
    if (this->knight_rider_target_active || this->knight_rider_blend > 0.0f) {
        this->knight_rider_animation->update();
    } else if (this->knight_rider_animation->is_running()) {
        this->knight_rider_animation->stop();
    }

    // Step 6: sustained-crowd flicker. The countdown runs while enough
    // panels hold active glows AND the sensor state keeps changing (a crowd
    // moving around, not a static arrangement); either condition breaking
    // resets it. At glow_flicker_trigger_delay_ms the flicker ramps up in
    // sparks — one random panel flickers briefly, drops back to idle,
    // another sparks, spawning faster and faster across
    // glow_flicker_ramp_duration_ms — then every panel flickers together
    // for glow_flicker_run_duration_ms, each re-rolling its color on its
    // own random hold timer, before crossfading back to the normal
    // animation.
    if (any_sensor_changed) {
        this->sensor_change_clock.restart();
    } else if (!this->sensor_change_clock.isStopped()) {
        this->sensor_change_clock.update();
    }

    int active_panel_count = 0;
    for (int panel_index = 0; panel_index < this->number_of_panels; panel_index++) {
        if (this->glows[panel_index * 2]->is_active()
            || this->glows[panel_index * 2 + 1]->is_active()) {
            active_panel_count++;
        }
    }

    bool enough_panels_active = active_panel_count >= this->config->glow_flicker_min_active_panels;
    bool sensors_still_changing = !this->sensor_change_clock.isStopped()
        && static_cast<long>(this->sensor_change_clock.elapsed_ms)
           <= this->config->glow_flicker_change_timeout_ms;

    if (enough_panels_active && sensors_still_changing) {
        if (this->multi_panel_clock.isStopped()) {
            this->multi_panel_clock.restart();
        }
        this->multi_panel_clock.update();
    } else {
        this->multi_panel_clock.stop();
    }

    if (!this->flicker_target_active
        && this->config->glow_flicker_trigger_delay_ms > 0
        && !this->multi_panel_clock.isStopped()
        && static_cast<long>(this->multi_panel_clock.elapsed_ms)
           >= this->config->glow_flicker_trigger_delay_ms) {
        this->flicker_target_active = true;
        this->flicker_run_clock.restart();
        // No crossfade in: the ramp reveals the flicker spark by spark from
        // nothing, so the override runs at full strength immediately and
        // the per-panel spark states carry the build-up.
        this->flicker_blend = 1.0f;
        this->flicker_blend_at_transition_start = 1.0f;
        this->flicker_transition_clock.restart();
        for (int panel_index = 0; panel_index < this->number_of_panels; panel_index++) {
            // Every panel starts its color fade from idle, so whenever it
            // first shows the override it eases out of the idle color.
            this->flicker_colors[panel_index] = this->config->wave_idle_color;
            this->flicker_from_colors[panel_index] = this->config->wave_idle_color;
            this->flicker_to_colors[panel_index] = this->_pick_random_saturated_color();
            this->flicker_change_started_ms[panel_index] = 0;
            this->flicker_next_change_ms[panel_index] = 0;
            this->flicker_spark_active[panel_index] = false;
            this->flicker_spark_end_ms[panel_index] = 0;
        }
        // Continued crowd activity has to accumulate another full delay
        // before the flicker comes back.
        this->multi_panel_clock.restart();
    }

    long flicker_ramp_duration_ms = this->config->glow_flicker_ramp_duration_ms;
    if (flicker_ramp_duration_ms < 0) flicker_ramp_duration_ms = 0;

    if (this->flicker_target_active) {
        this->flicker_run_clock.update();
        if (static_cast<long>(this->flicker_run_clock.elapsed_ms)
            >= flicker_ramp_duration_ms + this->config->glow_flicker_run_duration_ms) {
            this->flicker_target_active = false;
            this->flicker_blend_at_transition_start = this->flicker_blend;
            this->flicker_transition_clock.restart();
        }
    }

    int flicker_fade_ms = this->config->glow_flicker_fade_ms;
    if (flicker_fade_ms < 1) flicker_fade_ms = 1;
    this->flicker_transition_clock.update();
    float flicker_fade_progress = static_cast<float>(this->flicker_transition_clock.elapsed_ms)
                                / static_cast<float>(flicker_fade_ms);
    if (flicker_fade_progress > 1.0f) flicker_fade_progress = 1.0f;
    float flicker_blend_target = this->flicker_target_active ? 1.0f : 0.0f;
    this->flicker_blend = this->flicker_blend_at_transition_start
        + (flicker_blend_target - this->flicker_blend_at_transition_start) * flicker_fade_progress;

    // Keep the spark scheduling and per-panel re-rolls running while the
    // flicker is wanted or still fading out, so the fade-out flickers all
    // the way to dark.
    if (this->flicker_target_active || this->flicker_blend > 0.0f) {
        this->flicker_run_clock.update();
        unsigned long flicker_elapsed_ms = this->flicker_run_clock.elapsed_ms;
        bool in_ramp = this->flicker_target_active
            && static_cast<long>(flicker_elapsed_ms) < flicker_ramp_duration_ms;

        int spark_fade_ms = this->config->glow_flicker_spark_fade_ms;
        if (spark_fade_ms < 1) spark_fade_ms = 1;

        if (in_ramp) {
            // A spark occupies its panel until it has fully eased back to
            // idle, glow_flicker_spark_fade_ms past its expiry.
            int active_spark_count = 0;
            int idle_panel_count = 0;
            for (int panel_index = 0; panel_index < this->number_of_panels; panel_index++) {
                if (this->flicker_spark_active[panel_index]
                    && flicker_elapsed_ms >= this->flicker_spark_end_ms[panel_index]
                                             + static_cast<unsigned long>(spark_fade_ms)) {
                    this->flicker_spark_active[panel_index] = false;
                }
                if (this->flicker_spark_active[panel_index]) active_spark_count++;
                else idle_panel_count++;
            }

            int spark_min_duration_ms = this->config->glow_flicker_spark_min_duration_ms;
            int spark_max_duration_ms = this->config->glow_flicker_spark_max_duration_ms;
            if (spark_min_duration_ms < 1) spark_min_duration_ms = 1;
            if (spark_max_duration_ms < spark_min_duration_ms) spark_max_duration_ms = spark_min_duration_ms;

            // Concurrent spark count grows with the square of ramp
            // progress: a lone hopping spark early, piling up late so the
            // ramp lands exactly on the everyone-flickers phase. An expired
            // spark below target is replaced on a different random panel,
            // which is what makes each spark visibly return to idle before
            // the next one lights.
            float ramp_progress = static_cast<float>(flicker_elapsed_ms)
                                / static_cast<float>(flicker_ramp_duration_ms);
            int target_spark_count = static_cast<int>(ceilf(
                ramp_progress * ramp_progress * static_cast<float>(this->number_of_panels)));
            if (target_spark_count < 1) target_spark_count = 1;
            if (target_spark_count > this->number_of_panels) target_spark_count = this->number_of_panels;

            while (active_spark_count < target_spark_count && idle_panel_count > 0) {
                int pick = static_cast<int>(random(idle_panel_count));
                for (int panel_index = 0; panel_index < this->number_of_panels; panel_index++) {
                    if (this->flicker_spark_active[panel_index]) continue;
                    if (pick == 0) {
                        this->flicker_spark_active[panel_index] = true;
                        this->flicker_spark_end_ms[panel_index] = flicker_elapsed_ms
                            + static_cast<unsigned long>(random(spark_min_duration_ms,
                                                                spark_max_duration_ms + 1));
                        // Restart the panel's color fade from idle so the
                        // spark eases in instead of snapping on.
                        this->flicker_colors[panel_index] = this->config->wave_idle_color;
                        this->flicker_next_change_ms[panel_index] = flicker_elapsed_ms;
                        break;
                    }
                    pick--;
                }
                active_spark_count++;
                idle_panel_count--;
            }
        }

        int min_hold_ms = this->config->glow_flicker_min_hold_ms;
        int max_hold_ms = this->config->glow_flicker_max_hold_ms;
        if (min_hold_ms < 1) min_hold_ms = 1;
        if (max_hold_ms < min_hold_ms) max_hold_ms = min_hold_ms;
        for (int panel_index = 0; panel_index < this->number_of_panels; panel_index++) {
            // Fade through the colors: each hold period crossfades from the
            // color the panel is currently showing to the next random pick,
            // so the flicker rolls instead of snapping.
            if (flicker_elapsed_ms >= this->flicker_next_change_ms[panel_index]) {
                this->flicker_from_colors[panel_index] = this->flicker_colors[panel_index];
                this->flicker_to_colors[panel_index] = this->_pick_random_saturated_color();
                this->flicker_change_started_ms[panel_index] = flicker_elapsed_ms;
                this->flicker_next_change_ms[panel_index] = flicker_elapsed_ms
                    + static_cast<unsigned long>(random(min_hold_ms, max_hold_ms + 1));
            }
            float hold_progress = static_cast<float>(flicker_elapsed_ms
                                                     - this->flicker_change_started_ms[panel_index])
                                / static_cast<float>(this->flicker_next_change_ms[panel_index]
                                                     - this->flicker_change_started_ms[panel_index]);
            if (hold_progress > 1.0f) hold_progress = 1.0f;
            this->flicker_colors[panel_index] = this->flicker_from_colors[panel_index]
                .interpolate(this->flicker_to_colors[panel_index], hold_progress);

            // Ramp: only sparking panels show the override, an expiring
            // spark easing back to idle over glow_flicker_spark_fade_ms.
            // Full phase and fade-out (in_ramp false): everyone at full
            // strength.
            float panel_strength;
            if (!in_ramp) {
                panel_strength = 1.0f;
            } else if (!this->flicker_spark_active[panel_index]) {
                panel_strength = 0.0f;
            } else if (flicker_elapsed_ms < this->flicker_spark_end_ms[panel_index]) {
                panel_strength = 1.0f;
            } else {
                panel_strength = 1.0f
                    - static_cast<float>(flicker_elapsed_ms - this->flicker_spark_end_ms[panel_index])
                      / static_cast<float>(spark_fade_ms);
                if (panel_strength < 0.0f) panel_strength = 0.0f;
            }
            this->flicker_panel_strength[panel_index] = panel_strength;
        }
    } else if (!this->flicker_run_clock.isStopped()) {
        this->flicker_run_clock.stop();
    }

    // Step 7: resolve per-panel color. A panel whose own two glows are both
    // active dances between their colors. Otherwise a panel can be touched
    // by its own glows and by the lagged neighbor envelopes of glows within
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

        // The dance takes the whole panel: neighbor ripple contributions are
        // ignored while both of its own sensors hold it, so the contributor
        // scan is skipped.
        bool dancing = this->dance_active[panel_index];

        for (int sensor_index = 0; !dancing && sensor_index < this->number_of_sensors; sensor_index++) {
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
        if (dancing) {
            resolved = this->_dance_color_for_panel(panel_index, idle_value);
        } else if (contributing_glow_count == 0) {
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

        // Crossfade in/out of the idle-attract knight rider sweep.
        if (this->knight_rider_blend > 0.0f) {
            Color knight_rider_color = this->knight_rider_animation->get_color_for_panel(panel_index);
            final_color = final_color.interpolate(knight_rider_color, this->knight_rider_blend);
        }

        // Sustained-crowd flicker: sparking panels only during the ramp,
        // every panel through the full phase and the fade-out.
        if (this->flicker_blend > 0.0f && this->flicker_panel_strength[panel_index] > 0.0f) {
            final_color = final_color.interpolate(this->flicker_colors[panel_index],
                this->flicker_blend * this->flicker_panel_strength[panel_index]);
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
    bool neighbor_is_to_the_left = false;

    for (int other_sensor_index = 0; other_sensor_index < this->number_of_sensors; other_sensor_index++) {
        if (other_sensor_index == activating_sensor_index) continue;
        Glow* other = this->glows[other_sensor_index];
        if (!other->is_active()) continue;
        int other_panel = other->get_panel_index();
        int signed_panel_distance = other_panel - activating_panel_index;
        int panel_distance = signed_panel_distance < 0 ? -signed_panel_distance : signed_panel_distance;
        if (panel_distance > neighbor_max_distance) continue;
        if (panel_distance < closest_distance) {
            closest_distance = panel_distance;
            neighbor_color = other->get_target_color();
            neighbor_is_to_the_left = signed_panel_distance < 0;
            found_neighbor = true;
        }
    }

    if (found_neighbor) {
        // Shift the neighbor's color by 1/20 of the wheel so a chain of
        // adjacent activations forms a smooth gradient instead of one
        // dominant color. The step happens in warped wheel-position space so
        // every step looks equally big — raw-hue steps crawl through the
        // wide green/blue bands and sprint through the warm ones. The step
        // direction follows the direction of spread: rightward walks step
        // forward around the wheel, leftward walks step backward, so one
        // person splitting into both directions diverges into two distinct
        // gradient trails instead of a mirrored copy. The same-panel partner
        // (distance 0) instead gets the larger dance offset — its color
        // exists to cycle against the other side's, so it needs visible
        // contrast.
        float wheel_position_offset;
        if (closest_distance == 0) {
            wheel_position_offset = this->config->glow_dance_partner_hue_offset;
        } else {
            float wheel_position_step = 1.0f / 20.0f;
            wheel_position_offset = neighbor_is_to_the_left
                                  ? wheel_position_step
                                  : -wheel_position_step;
        }
        Hsv neighbor_hsv = rgb_to_hsv(neighbor_color);
        float shifted_position = wheel_position_from_hue(neighbor_hsv.hue) + wheel_position_offset;
        shifted_position -= floorf(shifted_position);
        return hsv_to_rgb(hue_from_wheel_position(shifted_position), 1.0f, 1.0f);
    }
    return this->_pick_random_saturated_color();
}

// Cycle the panel between its two glows' hues along the shortest arc of the
// wheel. The raw cycle position (cosine, one full anchor → other → anchor
// trip per glow_dance_period_ms) is weighted by each glow's live intensity:
// a glow still fading in pulls weakly, so the dance eases in from the
// anchor's color, and a glow fading out lets go gradually, so the panel
// settles on the surviving color with no snap when the dance ends.
Color GlowColorAlgorithm::_dance_color_for_panel(int panel_index, float idle_value) const {
    int front_sensor_index = panel_index * 2;
    int back_sensor_index = front_sensor_index + 1;
    int anchor_sensor_index = this->dance_anchor_is_front[panel_index]
                            ? front_sensor_index : back_sensor_index;
    int other_sensor_index = this->dance_anchor_is_front[panel_index]
                           ? back_sensor_index : front_sensor_index;

    float anchor_intensity = this->glows[anchor_sensor_index]->get_intensity();
    float other_intensity = this->glows[other_sensor_index]->get_intensity();

    long period_ms = this->config->glow_dance_period_ms;
    if (period_ms < 1) period_ms = 1;
    long cycle_elapsed_ms = static_cast<long>(this->dance_clocks[panel_index].elapsed_ms) % period_ms;
    float cycle_phase = static_cast<float>(cycle_elapsed_ms) / static_cast<float>(period_ms);
    float raw_weight = 0.5f - 0.5f * cosf(cycle_phase * FULL_CIRCLE_RADIANS);

    float anchor_pull = (1.0f - raw_weight) * anchor_intensity;
    float other_pull = raw_weight * other_intensity;
    float pull_sum = anchor_pull + other_pull;
    float other_weight = pull_sum > 0.0f ? other_pull / pull_sum : 0.0f;

    float anchor_hue = this->glow_target_hue[anchor_sensor_index];
    float other_hue = this->glow_target_hue[other_sensor_index];
    float hue_delta = other_hue - anchor_hue;
    // Wrap to [-0.5, 0.5) so the interpolation takes the short way around.
    hue_delta -= floorf(hue_delta + 0.5f);
    float resolved_hue = anchor_hue + hue_delta * other_weight;
    resolved_hue -= floorf(resolved_hue);

    // Brightness pins to the brighter glow, same as the overlap rule, so the
    // two out-of-phase pulses shimmer instead of multiplying darker.
    float stronger_intensity = anchor_intensity > other_intensity
                             ? anchor_intensity : other_intensity;
    float resolved_value = idle_value * (1.0f - stronger_intensity) + stronger_intensity;

    return hsv_to_rgb(resolved_hue, 1.0f, resolved_value);
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
    float wheel_position = panel_fraction + scroll_offset;
    wheel_position = wheel_position - floorf(wheel_position);
    if (wheel_position < 0) wheel_position += 1.0f;
    return hsv_to_rgb(hue_from_wheel_position(wheel_position), 1.0f, 1.0f);
}

Color GlowColorAlgorithm::_pick_random_saturated_color() const {
    float wheel_position = static_cast<float>(random(10000)) / 10000.0f;
    return hsv_to_rgb(hue_from_wheel_position(wheel_position), 1.0f, 1.0f);
}
