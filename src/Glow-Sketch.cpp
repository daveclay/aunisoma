#include <string.h>
#include <stdio.h>
#ifndef ARDUINO
#include <chrono>
#endif
#include "Arduino.h"
#include "Color.h"
#include "Config.h"
#include "GlowColorAlgorithm.h"
#include "LoopTiming.h"
#include "PanelLink.h"
#include "Range.h"
#include "Sensor.h"

#ifndef MOCK_INTERACTIONS
#define MOCK_INTERACTIONS 0
#endif

// Loop-timing instrumentation (see LoopTiming.h). Build with
// -DMEASURE_LOOP_TIMING=1 (the grandcentral_m4_glow_timing env) to print
// min/avg/max microseconds over USB serial every 100 loops: the loop period
// plus its three segments (algorithm update, color formatting, master
// round-trip). With the flag off these calls compile to nothing.
static const char* TIMING_SEGMENT_NAMES[] = {"update", "format", "send"};
static LoopTiming loop_timing(TIMING_SEGMENT_NAMES, 3);

// Glow-only sketch. Drives GlowColorAlgorithm directly: no wavefront
// propagation — each sensor pulses its own panel. Front and back sensors are
// independent interactions with their own colors; a panel active from both
// sides dances between the two sensors' colors on its own configurable
// period. Lives alongside Wave-Sketch.cpp as its own build target.

static char panel_ids[] = "281A221B162515111220181914171F131D211E23";

static char ZERO_COLORS[] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

#define NUMBER_OF_PANELS 20
#define NUMBER_OF_SENSORS 40
#define SIZE_OF_COLOR 6

static PanelLink link;
// +1 for the trailing '\0' — Serial2.print(const char*) reads until null,
// so the buffer must be terminated or it will emit garbage past the 120
// hex bytes.
static char panel_colors[(NUMBER_OF_PANELS * SIZE_OF_COLOR) + 1];
static char pir_readings[NUMBER_OF_PANELS];

static Sensor sensors[NUMBER_OF_SENSORS];
static Config config;
static GlowColorAlgorithm* glow_algorithm;

static int iterationCount = 0;
static int mockInteractionPeriod = 200;

static bool send_colors(char value[]) {
    if (!link.send_colors(value, pir_readings)) {
        return false;
    }
    for (int panel_index = 0; panel_index < NUMBER_OF_PANELS; panel_index++) {
        int sensor_index = panel_index * 2;
        if (MOCK_INTERACTIONS) {
            if (iterationCount % mockInteractionPeriod == 0) {
                bool mock_front_interactivity = random(0, 11) > 5;
                bool mock_back_interactivity = random(0, 11) > 5;
                sensors[sensor_index].update(mock_front_interactivity);
                sensors[sensor_index + 1].update(mock_back_interactivity);
            } else {
                sensors[sensor_index].update(sensors[sensor_index].last_reading);
                sensors[sensor_index + 1].update(sensors[sensor_index + 1].last_reading);
            }
        } else {
            char pir = pir_readings[panel_index];
            bool front_sensor_active = pir == '1' || pir == '3';
            bool back_sensor_active = pir == '2' || pir == '3';
            sensors[sensor_index].update(front_sensor_active);
            sensors[sensor_index + 1].update(back_sensor_active);
        }
    }
    return true;
}

void setup(void) {
    Serial.begin(9600);
    link.begin();

    pinMode(LED_BUILTIN, OUTPUT);

#ifdef ARDUINO
    unsigned long random_seed_value = static_cast<unsigned long>(micros());
    random_seed_value ^= static_cast<unsigned long>(analogRead(A0)) << 16;
    random_seed_value ^= static_cast<unsigned long>(analogRead(A1));
    randomSeed(random_seed_value);
#else
    auto wall_clock_ns = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    randomSeed(static_cast<unsigned long>(wall_clock_ns));
#endif

    config.number_of_panels = NUMBER_OF_PANELS;
    config.high_interaction_threshold_percent = 0.6f;

    // Glow parameters. Snap in fast on activation, linger on release.
    config.glow_fade_in_duration_ms = 200;
    config.glow_fade_out_duration_ms = 1000;
    // While active, brightness dips from 100% to 25% and back; each cycle
    // draws a fresh period from this range so no two sensors stay locked in
    // step for long.
    config.glow_pulse_min_period_ms = 3000;
    config.glow_pulse_max_period_ms = 4000;
    config.glow_pulse_min_value = 0.10f;

    // One person on each side of the same panel: that panel cycles between
    // the two sensors' colors, one full there-and-back trip per period. The
    // second side's color sits a third of the wheel from the first so the
    // dance reads clearly (adjacent-panel activations keep the subtle 1/20
    // chain-gradient shift).
    config.glow_dance_period_ms = 2500;
    config.glow_dance_partner_hue_offset = 1.0f / 3.0f;

    // Ripple: the 2 panels on each side follow the source with a
    // per-distance lag — a distance-1 neighbor starts brightening 100 ms
    // after the source and peaks 200 ms after the source's 200 ms peak
    // (i.e. at 400 ms), repeats the source's pulse dips 200 ms/panel behind
    // so every pulse travels outward, and trails the fade-out by the same
    // lags.
    config.glow_ripple_distance = 0;
    config.glow_ripple_start_delay_ms = 300;
    config.glow_ripple_peak_lag_ms = 900;

    // High-interaction rainbow override, carried over from the wave sketch.
    config.wave_rainbow_scroll_duration_ms = 2000;
    config.wave_rainbow_transition_duration_ms = 1200;
    config.wave_idle_color = Color(3, 0, 0);

    // Overlap blend for a panel active from both sides: rotate hue by
    // 30°/unit of chroma overshoot above 1.0, and trust the chroma sum's
    // direction unless coherence drops below 10% (e.g. red + cyan at equal
    // intensity, where atan2 is unstable).
    config.wave_overlap_hue_rotation_per_unit = 1.0f / 12.0f;
    config.wave_overlap_coherence_fallback_threshold = 0.1f;

    config.init();

    glow_algorithm = new GlowColorAlgorithm(&config, sensors, NUMBER_OF_PANELS, NUMBER_OF_SENSORS);

    link.map_panels_until_ok(panel_ids);
}

static const char HEX_DIGITS[] = "0123456789abcdef";

// Re-map panels periodically to heal any odd state a panel controller may
// get into. Wall-clock based, not iteration based, so the cadence doesn't
// change if the loop rate does — map_panels is a blocking round trip (up to
// the 1000 ms serial timeout) and stalls the animation while it runs.
static const long MAP_PANELS_INTERVAL_MS = 15L * 60L * 1000L;
static long next_map_panels_ms = MAP_PANELS_INTERVAL_MS;

// run for 11 hours, then zero out the panels and do nothing while
// waiting for me to wake up and come out and turn the power off.
// Note this isn't an accurate clock. It's just attempting to
// limit the amount of power it draws from the batteries in the
// morning, allowing the solar power to charge up the batteries
// with as little competition from the thing actively running LEDs.
static int ACTIVE_RUNTIME_LIMIT_MS = 11 * 60 * 60 * 1000;
static int WAIT_FOR_DAVE_TO_COME_SHUT_ME_OFF_DELAY = 15 * 60 * 1000;

void loop(void) {
    long start = millis();
    if (start > ACTIVE_RUNTIME_LIMIT_MS) {
        send_colors(ZERO_COLORS);
        delay(WAIT_FOR_DAVE_TO_COME_SHUT_ME_OFF_DELAY);
        return;
    }

    loop_timing.begin_loop();

    glow_algorithm->update();

    loop_timing.next_segment();

    for (int panel_index = 0; panel_index < NUMBER_OF_PANELS; panel_index++) {
        Color color = glow_algorithm->get_color_for_panel(panel_index).limit();

        int red_gamma = gamma_lut[color.red];
        int green_gamma = gamma_lut[color.green];
        int blue_gamma = gamma_lut[color.blue];
        char* hex_out = &panel_colors[panel_index * SIZE_OF_COLOR];
        hex_out[0] = HEX_DIGITS[red_gamma >> 4];
        hex_out[1] = HEX_DIGITS[red_gamma & 0xF];
        hex_out[2] = HEX_DIGITS[green_gamma >> 4];
        hex_out[3] = HEX_DIGITS[green_gamma & 0xF];
        hex_out[4] = HEX_DIGITS[blue_gamma >> 4];
        hex_out[5] = HEX_DIGITS[blue_gamma & 0xF];
    }

    loop_timing.next_segment();

    send_colors(panel_colors);

    loop_timing.end_loop();

    iterationCount++;

    if (start >= next_map_panels_ms) {
        link.map_panels(panel_ids);
        next_map_panels_ms = start + MAP_PANELS_INTERVAL_MS;
    }
}
