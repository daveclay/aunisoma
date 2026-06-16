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

// Battery-life test mode. Ignores real PIR readings and instead simulates
// people walking by on a path in front of the windows: while occupied, one or
// more groups each cover a contiguous run of 1-10 windows (front sensors only),
// then the path clears, giving ~50% occupancy averaged over a ~5-minute window.
// Used to estimate average LED current draw over a session, not to validate
// the animation. See update_battery_mock_sensors().
#ifndef MOCK_BATTERY_TEST
#define MOCK_BATTERY_TEST 0
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

// Battery-life mock state. Mimics people walking past on the path in front of
// the windows rather than wiggling individual sensors: while the path is
// occupied, one or more groups each cover a contiguous run of 1-10 windows
// (front sensors only); when it clears, everything goes idle. Occupied and
// clear spells are drawn from the same range, so the path is occupied ~50% of
// the time averaged over a ~5-minute window. Because several groups can share
// the path at once, the active-window count is not capped at 10.
static bool mock_panel_active[NUMBER_OF_PANELS];
static long mock_phase_until_ms = 0;
static bool mock_path_occupied = false;

// Spells run tens-of-seconds to a couple of minutes, so any single minute may
// be all-on or all-off but a 5-minute window averages to ~50% occupancy. Equal
// occupied/clear ranges give the 50% duty cycle.
static const long MOCK_PHASE_MIN_MS = 15000;   // 15 s
static const long MOCK_PHASE_MAX_MS = 120000;  // 2 min

// Up to this many independent groups may be on the path during an occupied
// spell, letting more than 10 windows be active at once.
static const int MOCK_MAX_GROUPS = 4;

static void update_battery_mock_sensors() {
    long now = millis();
    if (now >= mock_phase_until_ms) {
        mock_path_occupied = !mock_path_occupied;
        mock_phase_until_ms = now + random(MOCK_PHASE_MIN_MS, MOCK_PHASE_MAX_MS + 1);
        for (int panel_index = 0; panel_index < NUMBER_OF_PANELS; panel_index++) {
            mock_panel_active[panel_index] = false;
        }
        if (mock_path_occupied) {
            int number_of_groups = random(1, MOCK_MAX_GROUPS + 1);  // 1-4 groups
            for (int group = 0; group < number_of_groups; group++) {
                int group_size = random(1, 11);  // 1-10 windows per group
                int group_start = random(0, NUMBER_OF_PANELS - group_size + 1);
                for (int offset = 0; offset < group_size; offset++) {
                    int target_panel = group_start + offset;
                    if (target_panel < NUMBER_OF_PANELS) {
                        mock_panel_active[target_panel] = true;
                    }
                }
            }
        }
    }
    // Re-assert every iteration so the sensor debounce keeps tracking; back
    // side is never triggered (people pass on one side only).
    for (int panel_index = 0; panel_index < NUMBER_OF_PANELS; panel_index++) {
        int sensor_index = panel_index * 2;
        sensors[sensor_index].update(mock_panel_active[panel_index]);
        sensors[sensor_index + 1].update(false);
    }
}

static bool send_colors(char value[]) {
    if (!link.send_colors(value, pir_readings)) {
        return false;
    }
    if (MOCK_BATTERY_TEST) {
        update_battery_mock_sensors();
        return true;
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
    config.wave_rise_duration_ms = 350;
    config.wave_fall_duration_ms = 1800;
    config.wave_panel_delay_ms = 200;
    // Zero gap = the next pulse fires the instant the current one finishes
    // its full up-down across the strip.
    config.wave_inter_pulse_delay_ms = 0;
    // Wave reaches up to N panels from its source; exponential falloff to
    // zero at that distance. Extend it to get smoother wave propagation
    config.wave_max_propagation_distance = 8;
    // Decay constant for the per-panel exponential intensity falloff
    // exp(-decay*distance). 0.25 gives ~0.78 at distance 1, ~0.37 at
    // distance 4, ~0.14 at distance 8, ~0.08 at distance 10 — enough
    // intensity headroom at the far edge to produce a smooth fade through
    // 8-bit channel quantization instead of 1-bit on/off blips. Raise for a
    // sharper drop-off near the source; a more linear (lower) value tends to
    // make a higher-value color wavefront rather than a softer fade-in to
    // target color.
    config.wave_spatial_decay_constant = 0.45f;
    // When the active-sensor fraction crosses high_interaction_threshold_percent
    // the wave algorithm switches to a scrolling rainbow. This is the time for
    // one full revolution.
    config.wave_rainbow_scroll_duration_ms = 2000;
    // Crossfade duration between wave-blended output and the rainbow override
    // (each direction).
    config.wave_rainbow_transition_duration_ms = 1200;
    config.wave_idle_color = Color(3, 0, 0);

    // Overlap blend: rotate hue by 30°/unit of chroma overshoot above 1.0,
    // and trust the chroma sum's direction unless coherence drops below 10%
    // (e.g. red + cyan at equal intensity, where atan2 is unstable).
    config.wave_overlap_hue_rotation_per_unit = 1.0f / 12.0f;
    config.wave_overlap_coherence_fallback_threshold = 0.1f;

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
