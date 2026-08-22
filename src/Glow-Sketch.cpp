#include <string.h>
#include <stdio.h>
#ifndef ARDUINO
#include <chrono>
#endif
#include "Arduino.h"
#include "Color.h"
#include "Config.h"
#include "GlowColorAlgorithm.h"
#include "Gradient.h"
#include "LoopTiming.h"
#include "NightSchedule.h"
#include "PanelLink.h"
#include "Range.h"
#include "Sensor.h"

#ifndef MOCK_INTERACTIONS
#define MOCK_INTERACTIONS 0
#endif

// Scripted playback (grandcentral_m4_glow_scripted): ignore the real PIR
// readings and replay lib/mock-arduino/script.txt — baked into the firmware
// as MockScriptData.h by scripts/embed_mock_script.py — on an endless loop,
// so the sculpture can be compared side by side with the desktop mock replay
// of the same script. The night schedule is bypassed so the build runs
// whenever it's flashed.
#ifndef SCRIPTED_INTERACTIONS
#define SCRIPTED_INTERACTIONS 0
#endif

#if SCRIPTED_INTERACTIONS
#include "MockScriptData.h"
// Matches MOCK_MS_PER_LOOP in lib/mock-arduino/Arduino.cpp: one script
// iteration is one mock loop(), which stands in for ~62 ms of hardware time.
static const long SCRIPT_MS_PER_ITERATION = 62;
// Quiet gap between repeats so each pass fades out fully before the next.
static const long SCRIPT_LOOP_TAIL_MS = 3000;
static const long SCRIPT_LOOP_PERIOD_MS =
    (long)MOCK_SCRIPT_LAST_ITERATION * SCRIPT_MS_PER_ITERATION + SCRIPT_LOOP_TAIL_MS;

static const char* scripted_pirs(long now_ms) {
    long script_iteration = (now_ms % SCRIPT_LOOP_PERIOD_MS) / SCRIPT_MS_PER_ITERATION;
    const char* latest = "00000000000000000000";
    for (int entry_index = 0; entry_index < MOCK_SCRIPT_ENTRY_COUNT; entry_index++) {
        if (MOCK_SCRIPT_ITERATIONS[entry_index] <= script_iteration) {
            latest = MOCK_SCRIPT_PIRS[entry_index];
        } else {
            break;
        }
    }
    return latest;
}
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
// Classic red sweep for the idle attract, same points the legacy sketch
// uses; the tail matches the idle color so far panels stay untouched.
static GradientValueMap knight_rider_gradient;

static int iterationCount = 0;
static int mockInteractionPeriod = 200;

static bool send_colors(char value[]) {
    if (!link.send_colors(value, pir_readings)) {
        return false;
    }
#if SCRIPTED_INTERACTIONS
    const char* script_pirs = scripted_pirs(millis());
#endif
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
#if SCRIPTED_INTERACTIONS
            char pir = script_pirs[panel_index];
#else
            char pir = pir_readings[panel_index];
#endif
            bool front_sensor_active = pir == '1' || pir == '3';
            bool back_sensor_active = pir == '2' || pir == '3';
            sensors[sensor_index].update(front_sensor_active);
            sensors[sensor_index + 1].update(back_sensor_active);
        }
    }
    return true;
}

// Nightly schedule: lights on at 20:00, off at 06:00 local time, read from
// the PCF8523 RTC on SDA/SCL. Without an RTC on the bus this falls back to
// the old behavior — run the window's 10 hours from power-on, then go
// dark. While dark, keep the master fed with zeroed colors every 30 s and
// otherwise do nothing, so the solar can charge the batteries with as
// little competition as possible from the LEDs.
static const int LIGHTS_ON_HOUR = 20;
static const int LIGHTS_ON_MINUTE = 0;
static const int LIGHTS_OFF_HOUR = 6;
static const int LIGHTS_OFF_MINUTE = 0;
static const long FALLBACK_RUNTIME_LIMIT_MS = 10L * 60L * 60L * 1000L;
static const long DARK_RECHECK_DELAY_MS = 30L * 1000L;
static NightSchedule night_schedule(LIGHTS_ON_HOUR, LIGHTS_ON_MINUTE,
                                    LIGHTS_OFF_HOUR, LIGHTS_OFF_MINUTE,
                                    FALLBACK_RUNTIME_LIMIT_MS);

void setup(void) {
    Serial.begin(9600);
    link.begin();
    night_schedule.begin();

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

    // Idle attract: after 10 minutes with nobody around, run the knight
    // rider sweep for 10 seconds (one full left-right-left trip every 2 s),
    // crossfading in and out over 1 s. Any interaction fades it out for the
    // normal glow animation.
    config.glow_knight_rider_idle_delay_ms = 10 * 60 * 1000;
    config.glow_knight_rider_run_duration_ms = 10 * 1000;
    config.glow_knight_rider_sweep_duration_ms = 2000;
    config.glow_knight_rider_fade_ms = 1000;

    // Sustained-crowd flicker: when at least 5 sensors (front and back
    // count separately) stay active for a random 1–2 minutes straight with
    // the sensor state still changing at least every
    // 15 s (people moving around, not a static arrangement), the flicker
    // ramps up over 10 s — one random panel flickers for 300–700 ms, eases
    // back to idle over 200 ms, another sparks, more and more panels
    // sparking at once as the ramp progresses — then every panel flickers
    // random colors for 5 s, and the whole thing crossfades out over 1 s.
    // Rather than snapping, each panel fades through its colors: every
    // 150–400 ms it crossfades from its current color to the next random
    // pick, so the flicker rolls instead of jittering.
    config.glow_flicker_min_active_sensors = 5;
    config.glow_flicker_trigger_min_delay_ms = 35 * 1000;
    config.glow_flicker_trigger_max_delay_ms = 90 * 1000;
    config.glow_flicker_change_timeout_ms = 15 * 1000;
    config.glow_flicker_ramp_duration_ms = 10 * 1000;
    config.glow_flicker_spark_min_duration_ms = 300;
    config.glow_flicker_spark_max_duration_ms = 700;
    config.glow_flicker_spark_fade_ms = 200;
    config.glow_flicker_run_duration_ms = 1500;
    config.glow_flicker_min_hold_ms = 150;
    config.glow_flicker_max_hold_ms = 400;
    config.glow_flicker_fade_ms = 1000;
    config.glow_flicker_hue_range = 0.12f;

    // Ripple: the N panels on each side follow the source with a
    // per-distance lag — a distance-1 neighbor starts brightening 100 ms
    // after the source and peaks 200 ms after the source's 200 ms peak
    // (i.e. at 400 ms), repeats the source's pulse dips 200 ms/panel behind
    // so every pulse travels outward, and trails the fade-out by the same
    // lags.
    config.glow_ripple_distance = 1;
    config.glow_ripple_start_delay_ms = 130;
    config.glow_ripple_peak_lag_ms = 500;

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

    knight_rider_gradient.add_rgb_point(0, 255, 25, 0);
    knight_rider_gradient.add_rgb_point(0.2, 255, 0, 0);
    knight_rider_gradient.add_rgb_point(1.2, 3, 0, 0);
    knight_rider_gradient.add_rgb_point(2, 3, 0, 0);

    glow_algorithm = new GlowColorAlgorithm(&config, sensors, NUMBER_OF_PANELS, NUMBER_OF_SENSORS,
                                            &knight_rider_gradient);

    link.map_panels_until_ok(panel_ids);
}

static const char HEX_DIGITS[] = "0123456789abcdef";

// Re-map panels periodically to heal any odd state a panel controller may
// get into. Wall-clock based, not iteration based, so the cadence doesn't
// change if the loop rate does — map_panels is a blocking round trip (up to
// the 1000 ms serial timeout) and stalls the animation while it runs.
static const long MAP_PANELS_INTERVAL_MS = 15L * 60L * 1000L;
static long next_map_panels_ms = MAP_PANELS_INTERVAL_MS;

void loop(void) {
    long start = millis();
#if !SCRIPTED_INTERACTIONS
    if (!night_schedule.is_active(start)) {
        send_colors(ZERO_COLORS);
        delay(DARK_RECHECK_DELAY_MS);
        return;
    }
#endif

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
