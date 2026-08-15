#include <string.h>
#include <stdio.h>
#ifndef ARDUINO
#include <chrono>
#endif
#include "Arduino.h"
#include "Clock.h"
#include "NightSchedule.h"
#include "Cycle.h"
#include "Color.h"
#include "Config.h"
#include "Gradient.h"
#include "Panel.h"
#include "PanelLink.h"
#include "Reverberation.h"
#include "Sensor.h"
#include "Interpolation.h"
#include "Aunisoma.h"

#ifndef MOCK_INTERACTIONS
#define MOCK_INTERACTIONS 0
#endif

char panel_ids[] = "281A221B162515111220181914171F131D211E23";

char ZERO_COLORS[] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
char RAINBOW_COLORS[] = "FF0000FF4D00FF9900FFE600CCFF0080FF0033FF0000FF1A00FF6600FFB300FFFF00B2FF0066FF0019FF3300FF8000FFCC00FFFF00E5FF0099FF004C";

#define NUMBER_OF_PANELS 20
#define NUMBER_OF_SENSORS 40

#define SIZE_OF_COLOR 6  // number of chars to send the SET_LIGHTS message per panel

PanelLink link;
// +1 for the trailing '\0' — Serial2.print(const char*) reads until null,
// so the buffer must be terminated or it will emit garbage past the 120
// hex bytes. Static storage zero-inits the terminator and the fill loop
// below only writes indices 0..119.
char panel_colors[(NUMBER_OF_PANELS * SIZE_OF_COLOR) + 1];
char pir_readings[NUMBER_OF_PANELS];

Sensor sensors[NUMBER_OF_SENSORS];

Config config = Config();

GradientValueMap rainbow_gradient = GradientValueMap();
GradientValueMap knight_rider_gradient = GradientValueMap();
GradientValueMap initial_gradient = GradientValueMap();
GradientValueMap trans_gradient = GradientValueMap();
GradientValueMap blue_gradient = GradientValueMap();
GradientValueMap green_gradient = GradientValueMap();
GradientValueMap purple_red_gradient = GradientValueMap();
GradientValueMap green_blue_gradient = GradientValueMap();
GradientValueMap yellow_gradient = GradientValueMap();

GradientValueMap gradients[7] = {
  initial_gradient,
  trans_gradient,
  blue_gradient,
  green_blue_gradient,
  purple_red_gradient,
  green_gradient,
  yellow_gradient
};

Aunisoma* aunisoma;

int iterationCount = 0;
int mockInteractionPeriod = 200;

bool send_colors(char value[]) {
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
        // If we don't ping them with the previous value, they never reach the
        // debounce threshold. The debounce has to be called multiple times.
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

// Nightly schedule: lights on at 20:00, off at 07:00 local time, read from
// the PCF8523 RTC on SDA/SCL. Without an RTC on the bus this falls back to
// the old behavior — run 11 hours from power-on, then go dark. While dark,
// keep the master fed with zeroed colors every 30 s and otherwise do
// nothing, so the solar can charge the batteries with as little competition
// as possible from the LEDs.
static const int LIGHTS_ON_HOUR = 20;
static const int LIGHTS_OFF_HOUR = 7;
static const long FALLBACK_RUNTIME_LIMIT_MS = 11L * 60L * 60L * 1000L;
static const long DARK_RECHECK_DELAY_MS = 30L * 1000L;
static NightSchedule night_schedule(LIGHTS_ON_HOUR, LIGHTS_OFF_HOUR, FALLBACK_RUNTIME_LIMIT_MS);

void setup(void) {
  Serial.begin(9600);
  link.begin();
  night_schedule.begin();

  pinMode(LED_BUILTIN, OUTPUT);

  // Seed the RNG so the wave color picks vary between runs.
#ifdef ARDUINO
  // Combine a floating-pin analog read (electrical noise) with micros() so
  // even a board with unusually quiet analog pins still varies a bit.
  unsigned long random_seed_value = static_cast<unsigned long>(micros());
  random_seed_value ^= static_cast<unsigned long>(analogRead(A0)) << 16;
  random_seed_value ^= static_cast<unsigned long>(analogRead(A1));
  randomSeed(random_seed_value);
#else
  // Native: nanosecond-resolution clock so back-to-back runs differ.
  auto wall_clock_ns = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  randomSeed(static_cast<unsigned long>(wall_clock_ns));
#endif

  rainbow_gradient.add_rgb_point(0.00, 255, 0, 0);
  rainbow_gradient.add_rgb_point(0.14, 255, 255, 0);
  rainbow_gradient.add_rgb_point(0.25, 0, 255, 0);
  rainbow_gradient.add_rgb_point(0.35, 0, 255, 255);
  rainbow_gradient.add_rgb_point(0.50, 0, 0, 255);
  rainbow_gradient.add_rgb_point(0.80, 255, 0, 255);
  rainbow_gradient.add_rgb_point(1.00, 255, 0, 0);

  knight_rider_gradient.add_rgb_point(0, 255, 25, 0);
  knight_rider_gradient.add_rgb_point(.2, 255, 0, 0);
  knight_rider_gradient.add_rgb_point(1.2, 3, 0, 0);
  knight_rider_gradient.add_rgb_point(2, 3, 0, 0);

  initial_gradient.add_rgb_point(0.0, 3, 0, 0);
  initial_gradient.add_rgb_point(.7, 255, 0, 0);
  initial_gradient.add_rgb_point(2.3, 255, 255, 0);
  initial_gradient.add_rgb_point(3, 0, 255, 255);
  initial_gradient.add_rgb_point(5, 0, 100, 255);

  blue_gradient.add_rgb_point(0.0, 0, 0, 10);
  blue_gradient.add_rgb_point(.6, 0, 0, 255);
  blue_gradient.add_rgb_point(1.5, 255, 0, 255);
  blue_gradient.add_rgb_point(3, 255, 255, 0);
  blue_gradient.add_rgb_point(4, 255, 255, 0);

  green_gradient.add_rgb_point(0.0, 0, 10, 0);
  green_gradient.add_rgb_point(.6, 0, 255, 0);
  green_gradient.add_rgb_point(1.2, 255, 255, 0);
  green_gradient.add_rgb_point(3.2, 255, 0, 255);
  green_gradient.add_rgb_point(4, 255, 0, 255);

  purple_red_gradient.add_rgb_point(0, 1, 0, 1);
  purple_red_gradient.add_rgb_point(.7, 255, 0, 255);
  purple_red_gradient.add_rgb_point(1.3, 255, 0, 0);
  purple_red_gradient.add_rgb_point(3.3, 255, 255, 0);
  purple_red_gradient.add_rgb_point(4.2, 0, 255, 0);

  green_blue_gradient.add_rgb_point(0, 0, 3, 0);
  green_blue_gradient.add_rgb_point(.7, 0, 255, 0);
  green_blue_gradient.add_rgb_point(1.2, 0, 255, 255);
  green_blue_gradient.add_rgb_point(2.8, 0, 0, 255);
  green_blue_gradient.add_rgb_point(4, 255, 0, 255);

  trans_gradient.add_rgb_point(0, 3, 0, 1);
  trans_gradient.add_rgb_point(.6, 255, 0, 105);
  trans_gradient.add_rgb_point(2, 0, 145, 255);
  trans_gradient.add_rgb_point(4, 200, 255, 0);

  yellow_gradient.add_rgb_point(0, 3, 0, 1);
  yellow_gradient.add_rgb_point(.6, 255, 255, 0);
  yellow_gradient.add_rgb_point(1.8, 255, 0, 0);
  yellow_gradient.add_rgb_point(4, 0, 0, 255);
  yellow_gradient.add_rgb_point(5, 0, 255, 255);

  config.number_of_panels = NUMBER_OF_PANELS;

  config.reverberation_distance_range = new Range(1, 4);
  // The duration for pulses in a reverberation. Must be longer than
  // reverberation_panel_delay_ms or neighbors won't trigger before the
  // source panel finishes animating. Originally 20-40 ticks; baseline
  // loop period is ~2.4ms, so rescaled by ~2.4.
  config.single_panel_pulse_duration = new Range(48, 96);
  // How long to wait to trigger a neighbor Panel to reverberate. If this is longer
  // than the single panel pulse, they won't fire because neighbors are one-shots
  // triggered by the start of the source panel. Originally 3 ticks.
  config.reverberation_panel_delay_ms = 7;
  // How long to wait at no interaction before reverting back to the default
  // gradient color. Originally 1500-2000 ticks (~3.6-4.8 s).
  config.default_gradient_delay_duration_range = new Range(3600, 4800);
  // Long, highly variable - power consumption should be prioritized.
  // Originally 6000-12000 ticks (~14.4-28.8 s).
  config.no_interaction_knight_rider_delay_range = new Range(14400, 28800);
  config.high_interaction_threshold_percent = .25;
  config.intermediate_interaction_threshold_percent = .1;

  // How long to wait for a gradient transition while in the medium
  // interactivity state. Longer means people have to move for longer to get
  // it to switch color. Originally 4000 ticks (~9.6 s).
  config.delay_for_gradient_transition_duration = 9600;

  // smoothing amount for panel values. In the web mockup, 10 is a
  // little jumpy, 30 is smooth, 100 blurs so that it never goes
  // back to 0 even when the Reverberation is active (which I like).
  // NOTE: this is a sample count, NOT a duration.
  config.smoothing_fn_window_size = 10;
  // How long it takes to transition from one gradient to another.
  // Originally 100 ticks (~240 ms).
  config.gradient_transition_animation_duration = 240;
  // Reset to the default gradient is idle-to-idle (small raw color delta), so
  // a shorter duration avoids the visible-step plateau effect — quick fade
  // instead of long "stalled" plateaus.
  config.transition_to_default_gradient_duration = 4000;
  // How long a single state is allowed before resetting to a known state.
  // Originally 5000 ticks (~12 s). Currently unused; see _update_state_watchdog.
  config.watchdog_state_duration_limit_ms = 12000;

  // Debounce durations (ms) for interaction-level readings. Going low takes
  // longer than going high for knight-rider; no-interaction is biased to stay
  // true so we settle back to idle without flickering.
  config.knight_rider_interaction_debounce_ms = 1000;
  config.no_interaction_debounce_ms = 300;
  config.low_interaction_debounce_ms = 100;
  config.med_interaction_debounce_ms = 200;
  // TODO: it was kinda hard to keep it in the high state, maybe it should be
  // biased to stay high longer?
  config.high_interaction_debounce_ms = 200;

  config.init();

  aunisoma = new Aunisoma(&config, gradients, 7, &rainbow_gradient, &knight_rider_gradient, sensors);

  link.map_panels_until_ok(panel_ids);
}

// + 1 for \0 terminated, which snprintf wants
char current_panel_color[(SIZE_OF_COLOR + 1)];

void loop(void) {
  long start = millis();
  if (!night_schedule.is_active(start)) {
    send_colors(ZERO_COLORS);
    delay(DARK_RECHECK_DELAY_MS);
    return;
  }

  aunisoma->update();

  //panel_colors[0] = '\0';
  for (int i = 0; i < NUMBER_OF_PANELS; i++) {
    Panel* panel = aunisoma->get_panel_at(i);
    Color color = panel->color.limit();

    snprintf(current_panel_color,
             SIZE_OF_COLOR + 1,
             "%02x%02x%02x",
             gamma_lut[color.red],
             gamma_lut[color.green],
             gamma_lut[color.blue]);
    for (int j = 0; j < 7; j++) {
      panel_colors[(i * SIZE_OF_COLOR) + j] = current_panel_color[j];
    }
    // strcat(panel_colors, current_panel_color);
  }

  send_colors(panel_colors);

  iterationCount++;

  if (iterationCount == 4000) {
    link.map_panels(panel_ids);
    iterationCount = 0;
  }

  // Serial.print((micros() - start));
  // Serial.println("ns");
}
