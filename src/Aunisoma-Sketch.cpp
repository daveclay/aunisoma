#include "SPI.h"
#include <string.h>
#include <stdio.h>
#ifndef ARDUINO
#include <chrono>
#endif
#include "Arduino.h"         // required before wiring_private.h
#include "wiring_private.h"  // pinPeripheral() function
#include "Clock.h"
#include "Cycle.h"
#include "Color.h"
#include "Config.h"
#include "Gradient.h"
#include "Panel.h"
#include "Reverberation.h"
#include "Sensor.h"
#include "Interpolation.h"
#include "Aunisoma.h"

#ifndef MOCK_INTERACTIONS
#define MOCK_INTERACTIONS 0
#endif

char panel_ids[] = "20131B1A2215191F17182512141D1E2328112116";

char ZERO_COLORS[] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
char RAINBOW_COLORS[] = "FF0000FF4D00FF9900FFE600CCFF0080FF0033FF0000FF1A00FF6600FFB300FFFF00B2FF0066FF0019FF3300FF8000FFCC00FFFF00E5FF0099FF004C";

#define NUMBER_OF_PANELS 20
#define NUMBER_OF_SENSORS 40

#define SIZE_OF_COLOR 6  // number of chars to send the SET_LIGHTS message per panel

Uart Serial2(&sercom1, PIN_SERIAL3_RX, PIN_SERIAL3_TX, PAD_SERIAL3_RX, PAD_SERIAL3_TX);
void SERCOM1_0_Handler() {
  Serial2.IrqHandler();
}
void SERCOM1_1_Handler() {
  Serial2.IrqHandler();
}
void SERCOM1_2_Handler() {
  Serial2.IrqHandler();
}
void SERCOM1_3_Handler() {
  Serial2.IrqHandler();
}

// Panel Board Protocol:
char ENUMERATE = 'E';
char SET_STATUS = 'S';
char SET_LIGHTS = 'L';
char MAP_PANELS = 'M';
char TERMINATOR = '\n';
char responseBuffer[512];  // should be 20 panels * however big messages are
char panel_colors[(NUMBER_OF_PANELS * SIZE_OF_COLOR)];

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

int send_command(char cmd_byte, char params[]) {
  // Serial.print("Sending ");
  // Serial.print(" bytes: '");
  // Serial.print(cmd_byte);
  // Serial.print(params);
  // Serial.println("'");

  Serial2.print(cmd_byte);
  if (params) {
    Serial2.print(params);
  }
  Serial2.print(TERMINATOR);
  Serial2.flush();

  int bytesRead = Serial2.readBytesUntil(TERMINATOR, responseBuffer, sizeof(responseBuffer));
  responseBuffer[bytesRead] = '\0';  // Null-terminate the string
  return bytesRead;
}

bool send_enumerate() {
  // Serial.println("Sending enumerate...");
  int bytesRead = send_command(ENUMERATE, NULL);
  if (bytesRead > 0) {
    // two bytes per panel
    int activePanels = bytesRead / 2;
    Serial.print("Initialized ");
    Serial.print(activePanels);
    Serial.print(" panels from ");
    Serial.print(bytesRead);
    Serial.print(" bytes: ");
    Serial.println(responseBuffer);
    for (int i = 0; i < bytesRead; i += 2) {
      if (responseBuffer[i] != 'V') {
        return false;
      }
    }

    return true;
  } else {
    return false;
  }
}

bool map_panels() {
  int bytesRead = send_command(MAP_PANELS, panel_ids);
  if (bytesRead > 1) {
    Serial.println(responseBuffer);
    if (responseBuffer[0] == 'O' && responseBuffer[1] == 'K') {
      return true;
    }
  }
  return false;
}


void initializePanels() {
  for (int i = 0; i < 100; i++) {
    if (map_panels()) {
      return;
    } else {
      delay(100);
    }
  }
}

int iterationCount = 0;
int mockInteractionPeriod = 200;

bool send_colors(char value[]) {
  int bytesRead = send_command(SET_LIGHTS, value);
  if (bytesRead > 0) {
    // Serial.println(responseBuffer);
    // 3 - skip "OK " and get to the PIRs
    // Note: this `min(23, )` business is because I was getting a `bytesRead` value of `35`
    // even though Serial.println(responseBuffer) returned the normal 23 length string,
    // and that then writes beyond the sensor array lengths
    // if (iterationCount % mockInteractionPeriod == 0) {
    //   std::cout << "WTF " << iterationCount << ": ";
    // }
    for (int i = 3; i < min(23, bytesRead); i++) {
      int panel_index = i - 3;
      int sensor_index = panel_index * 2;

      if (MOCK_INTERACTIONS) {
        if (iterationCount % mockInteractionPeriod == 0) {
          bool mock_front_interactivity = random(0, 11) > 5;
          bool mock_back_interactivity = random(0, 11) > 5;
          // std::cout << sensor_index << ": " << (mock_interactivity ? "1" : "0") << ", ";
          sensors[sensor_index].update(mock_front_interactivity);
          sensors[sensor_index + 1].update(mock_back_interactivity);
        } else {
          // If we don't ping them with the previous value, they never reach the
          // debounce threshold. The debounce has to be called multiple times.
          sensors[sensor_index].update(sensors[sensor_index].last_reading);
          sensors[sensor_index + 1].update(sensors[sensor_index + 1].last_reading);
        }
      } else {
        bool front_sensor_active = responseBuffer[i] == '1' || responseBuffer[i] == '3';
        bool back_sensor_active = responseBuffer[i] == '2' || responseBuffer[i] == '3';
        sensors[sensor_index].update(front_sensor_active);
        sensors[sensor_index + 1].update(back_sensor_active);
      }
    }
    // if (iterationCount % mockInteractionPeriod == 0) {
    //   std::cout << std::endl;
    // }
    return true;
  } else {
    return false;
  }
}

void setup(void) {
  Serial.begin(9600);
  Serial2.setTimeout(1000);
  Serial2.begin(230400);

  pinMode(LED_BUILTIN, OUTPUT);

  pinPeripheral(PIN_SERIAL3_RX, PIO_SERCOM);
  pinPeripheral(PIN_SERIAL3_TX, PIO_SERCOM);

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
  // loop period was ~240ms, so rescaled by ~240.
  config.single_panel_pulse_duration = new Range(4800, 9600);
  // How long to wait to trigger a neighbor Panel to reverberate. If this is longer
  // than the single panel pulse, they won't fire because neighbors are one-shots
  // triggered by the start of the source panel. Originally 3 ticks.
  config.reverberation_panel_delay_ms = 720;
  // How long to wait at no interaction before reverting back to the default
  // gradient color. Originally 1500-2000 ticks (~6-8 min).
  config.default_gradient_delay_duration_range = new Range(360000, 480000);
  // Long, highly variable - power consumption should be prioritized.
  // Originally 6000-12000 ticks (~24-48 min).
  config.no_interaction_knight_rider_delay_range = new Range(1440000, 2880000);
  config.high_interaction_threshold_percent = .25;
  config.intermediate_interaction_threshold_percent = .1;

  // How long to wait for a gradient transition while in the medium
  // interactivity state. Longer means people have to move for longer to get
  // it to switch color. Originally 4000 ticks (~16 min).
  config.delay_for_gradient_transition_duration = 960000;

  // smoothing amount for panel values. In the web mockup, 10 is a
  // little jumpy, 30 is smooth, 100 blurs so that it never goes
  // back to 0 even when the Reverberation is active (which I like).
  // NOTE: this is a sample count, NOT a duration.
  config.smoothing_fn_window_size = 10;
  // How long it takes to transition from one gradient to another.
  // Originally 100 ticks (~24s).
  config.gradient_transition_animation_duration = 24000;
  // Reset to the default gradient is idle-to-idle (small raw color delta), so
  // a shorter duration avoids the visible-step plateau effect — quick fade
  // instead of long "stalled" plateaus.
  config.transition_to_default_gradient_duration = 4000;
  // How long a single state is allowed before resetting to a known state.
  // Originally 5000 ticks (~20 min). Currently unused; see _update_state_watchdog.
  config.watchdog_state_duration_limit_ms = 1200000;

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

  // Wave algorithm parameters. Each panel rises to its attenuated target
  // over 600ms then falls back to idle over 600ms; neighbors lag by
  // wave_panel_delay_ms per panel of distance from the source.
  config.wave_rise_duration_ms = 5000;
  config.wave_fall_duration_ms = 5000;
  config.wave_panel_delay_ms = 500;
  // Zero gap = the next pulse fires the instant the current one finishes
  // its full up-down across the strip.
  config.wave_inter_pulse_delay_ms = 0;
  // Wave reaches up to 8 panels from its source; linear falloff to zero at
  // that distance.
  config.wave_max_propagation_distance = 8;
  config.wave_idle_color = Color(3, 0, 0);

  config.init();

  aunisoma = new Aunisoma(&config, gradients, 7, &rainbow_gradient, &knight_rider_gradient, sensors);

  initializePanels();
}

// + 1 for \0 terminated, which snprintf wants
char current_panel_color[(SIZE_OF_COLOR + 1)];

// run for 11 hours, then zero out the panels and do nothing while
// waiting for me to wake up and come out and turn the power off.
// Note this isn't an accurate clock. It's just attempting to
// limit the amount of power it draws from the batteries in the
// morning, allowing the solar power to charge up the batteries
// with as little competition from the thing actively running LEDs.
int ACTIVE_RUNTIME_LIMIT_MS = 11 * 60 * 60 * 1000;
int WAIT_FOR_DAVE_TO_COME_SHUT_ME_OFF_DELAY = 15 * 60 * 1000;

void loop(void) {
  long start = millis();
  if (start > ACTIVE_RUNTIME_LIMIT_MS) {
    send_colors(ZERO_COLORS);
    delay(WAIT_FOR_DAVE_TO_COME_SHUT_ME_OFF_DELAY);
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
    map_panels();
    iterationCount = 0;
  }

  // Serial.print((micros() - start));
  // Serial.println("ns");
}
