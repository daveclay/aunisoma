#include <string.h>
#include <stdio.h>
#ifndef ARDUINO
#include <chrono>
#endif
#include "Arduino.h"
#include "Color.h"
#include "Config.h"
#include "PanelLink.h"
#include "Range.h"
#include "Sensor.h"
#include "WaveColorAlgorithm.h"

#ifndef MOCK_INTERACTIONS
#define MOCK_INTERACTIONS 0
#endif

// Wave-only sketch. Drives WaveColorAlgorithm directly — no Aunisoma wrapper,
// no legacy gradient/knight-rider state. Lives alongside Aunisoma-Sketch.cpp
// as a second build target so each algorithm has its own entry point.

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
static WaveColorAlgorithm* wave_algorithm;

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
    config.high_interaction_threshold_percent = .25;

    // Wave algorithm parameters. Each panel rises to its attenuated target
    // over wave_rise_duration_ms then falls back to idle over
    // wave_fall_duration_ms; neighbors lag by wave_panel_delay_ms per panel
    // of distance from the source.
    config.wave_rise_duration_ms = 200;
    config.wave_fall_duration_ms = 2000;
    config.wave_panel_delay_ms = 200;
    // Zero gap = the next pulse fires the instant the current one finishes
    // its full up-down across the strip.
    config.wave_inter_pulse_delay_ms = 0;
    // Wave reaches up to N panels from its source; exponential falloff to
    // zero at that distance. Extend it to get smoother wave propagation
    config.wave_max_propagation_distance = 14;
    // Decay constant k for the per-panel exponential intensity falloff
    // exp(-k*distance). k=0.2 gives ~0.82 at distance 1, ~0.45 at distance 4,
    // ~0.20 at distance 8 — visible reach across the whole propagation
    // range. Raise for a sharper drop-off near the source. A more linear (lower)
    // value tends to make a higher-value color wavefront rather than a softer
    // fade-in to target color. Increasing wave distance helps larger values
    config.wave_spatial_decay_constant = 0.8f;
    // When the active-sensor fraction crosses high_interaction_threshold_percent
    // the wave algorithm switches to a scrolling rainbow. This is the time for
    // one full revolution.
    config.wave_rainbow_scroll_duration_ms = 2000;
    // Crossfade duration between wave-blended output and the rainbow override
    // (each direction).
    config.wave_rainbow_transition_duration_ms = 1200;
    config.wave_idle_color = Color(3, 0, 0);

    config.init();

    wave_algorithm = new WaveColorAlgorithm(&config, sensors, NUMBER_OF_PANELS, NUMBER_OF_SENSORS);

    link.map_panels_until_ok(panel_ids);
}

// +1 for \0-terminated, which snprintf wants
static char current_panel_color[SIZE_OF_COLOR + 1];

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

    wave_algorithm->update();

    for (int panel_index = 0; panel_index < NUMBER_OF_PANELS; panel_index++) {
        Color color = wave_algorithm->get_color_for_panel(panel_index).limit();

        snprintf(current_panel_color,
                 SIZE_OF_COLOR + 1,
                 "%02x%02x%02x",
                 gamma_lut[color.red],
                 gamma_lut[color.green],
                 gamma_lut[color.blue]);
        for (int char_index = 0; char_index < SIZE_OF_COLOR; char_index++) {
            panel_colors[(panel_index * SIZE_OF_COLOR) + char_index] = current_panel_color[char_index];
        }
    }

    send_colors(panel_colors);

    iterationCount++;

    if (iterationCount == 4000) {
        link.map_panels(panel_ids);
        iterationCount = 0;
    }
}
