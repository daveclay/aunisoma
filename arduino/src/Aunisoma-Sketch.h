#include "SPI.h"
#include <string.h>
#include <stdio.h>
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

char panel_ids[] = "22181F20121113191E1A211D152417251B281614";
// char panel_ids[] = "0E";

char ZERO_COLORS[] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
char FAILED_MAPPING_COLORS[] = "C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000C80000";
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
GradientValueMap initial_gradient = GradientValueMap();
GradientValueMap trans_gradient = GradientValueMap();
GradientValueMap blue_gradient = GradientValueMap();
GradientValueMap green_gradient = GradientValueMap();
GradientValueMap purple_red_gradient = GradientValueMap();
GradientValueMap green_blue_gradient = GradientValueMap();

GradientValueMap gradients[6] = {
  initial_gradient,
  trans_gradient,
  blue_gradient,
  green_blue_gradient,
  purple_red_gradient,
  green_gradient
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

  return Serial2.readBytesUntil(TERMINATOR, responseBuffer, sizeof(responseBuffer));
}

bool send_enumerate() {
  // Serial.println("Sending enumerate...");
  int bytesRead = send_command(ENUMERATE, NULL);
  if (bytesRead > 0) {
    // two bytes per panel
    int activePanels = bytesRead / 2;
    // Serial.print("Initialized ");
    // Serial.print(activePanels);
    // Serial.print(" panels from ");
    // Serial.print(bytesRead);
    // Serial.print(" bytes: ");
    // Serial.println(responseBuffer);
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

bool failed_mapping_flag = false;

bool map_panels() {
  int bytesRead = send_command(MAP_PANELS, panel_ids);
  if (bytesRead > 0) {
    //Serial.println(responseBuffer);
    int success = strcmp(responseBuffer, "OK");
    if (success < 0) {
      failed_mapping_flag = true;
    }
    return true;
  }
  return false;
}

void initializePanels() {
  digitalWrite(LED_BUILTIN, HIGH);
  while (!map_panels()) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  digitalWrite(LED_BUILTIN, LOW);
}

bool send_colors(char value[]) {
  int bytesRead = send_command(SET_LIGHTS, value);
  if (bytesRead > 0) {
    // Serial.println(responseBuffer);
    // 3 - skip "OK " and get to the PIRs
    // Note: this `min(23, )` business is because I was getting a `bytesRead` value of `35`
    // even though Serial.println(responseBuffer) returned the normal 23 length string,
    // and that then writes beyond the sensor array lengths
    for (int i = 3; i < min(23, bytesRead); i++) {
      bool front_sensor_active = responseBuffer[i] == '1' || responseBuffer[i] == '3';
      bool back_sensor_active = responseBuffer[i] == '2' || responseBuffer[i] == '3';
      int panel_index = i - 3;
      int sensor_index = panel_index * 2;
      sensors[sensor_index].update(front_sensor_active);
      sensors[sensor_index + 1].update(back_sensor_active);
    }
    return true;
  } else {
    return false;
  }
}

void setup(void) {
//  Serial.begin(9600);
  Serial2.setTimeout(1000);
  Serial2.begin(230400);

  pinMode(LED_BUILTIN, OUTPUT);

  pinPeripheral(PIN_SERIAL3_RX, PIO_SERCOM);
  pinPeripheral(PIN_SERIAL3_TX, PIO_SERCOM);

  rainbow_gradient.add_rgb_point(0.00, 255, 0, 0);
  rainbow_gradient.add_rgb_point(0.14, 255, 255, 0);
  rainbow_gradient.add_rgb_point(0.25, 0, 255, 0);
  rainbow_gradient.add_rgb_point(0.35, 0, 255, 255);
  rainbow_gradient.add_rgb_point(0.50, 0, 0, 255);
  rainbow_gradient.add_rgb_point(0.80, 255, 0, 255);
  rainbow_gradient.add_rgb_point(1.00, 255, 0, 0);

  initial_gradient.add_rgb_point(0.0, 10, 0, 0);
  initial_gradient.add_rgb_point(.4, 255, 0, 0);
  initial_gradient.add_rgb_point(1.8, 255, 255, 0);
  initial_gradient.add_rgb_point(2.2, 0, 255, 255);
  initial_gradient.add_rgb_point(3, 0, 100, 255);

  blue_gradient.add_rgb_point(0.0, 0, 0, 10);
  blue_gradient.add_rgb_point(.5, 0, 0, 255);
  blue_gradient.add_rgb_point(1.2, 255, 0, 255);
  blue_gradient.add_rgb_point(2.5, 255, 255, 0);
  blue_gradient.add_rgb_point(3, 255, 255, 0);

  green_gradient.add_rgb_point(0.0, 0, 10, 0);
  green_gradient.add_rgb_point(.5, 0, 255, 0);
  green_gradient.add_rgb_point(1.2, 255, 255, 0);
  green_gradient.add_rgb_point(2.5, 255, 0, 255);
  green_gradient.add_rgb_point(3, 255, 0, 255);

  purple_red_gradient.add_rgb_point(0, 5, 0, 5);
  purple_red_gradient.add_rgb_point(.6, 255, 0, 255);
  purple_red_gradient.add_rgb_point(1.2, 255, 0, 0);
  purple_red_gradient.add_rgb_point(2.5, 255, 255, 0);
  purple_red_gradient.add_rgb_point(3, 0, 255, 0);

  green_blue_gradient.add_rgb_point(0, 0, 10, 0);
  green_blue_gradient.add_rgb_point(.5, 0, 255, 0);
  green_blue_gradient.add_rgb_point(1.2, 0, 255, 255);
  green_blue_gradient.add_rgb_point(2, 0, 0, 255);
  green_blue_gradient.add_rgb_point(3, 255, 0, 255);

  trans_gradient.add_rgb_point(0, 3, 0, 1);
  trans_gradient.add_rgb_point(.7, 255, 0, 105);
  trans_gradient.add_rgb_point(2, 0, 155, 255);

  config.number_of_panels = NUMBER_OF_PANELS;

  config.reverberation_distance_range = new Range(3, 5);
  // how long to wait to trigger a neighbor Panel to reverberate
  config.reverberation_panel_delay_ticks = 20;
  config.trigger_panel_animation_loop_duration_ticks_range = new Range(20, 50);
  // TODO make longer (several minutes)
  config.default_gradient_delay_duration_range = new Range(100, 300);
  config.high_interaction_threshold_percent = .45;
  config.intermediate_interaction_threshold_percent = .2;

  // how long to wait for a gradient transition
  // tODO: make this random and longer
  config.delay_for_gradient_transition_duration = 500;

  // smoothing amount for panel values. In the web mockup, 10 is a
  // little jumpy, 30 is smooth, 100 blurs so that it never goes
  // back to 0 even when the Reverberation is active (which I like)
  config.smoothing_fn_window_size = 10;
  // How long it takes to transition from one gradient to another
  config.gradient_transition_animation_duration = 200;

  config.init();

  aunisoma = new Aunisoma(&config, gradients, 6, &rainbow_gradient, sensors);

  initializePanels();
}

// + 1 for \0 terminated, which snprintf wants
char current_panel_color[(SIZE_OF_COLOR + 1)];

int iterationCount = 0;

void loop(void) {
  long start = micros();
  if (failed_mapping_flag) {
    if (iterationCount % 50 < 40) {
      send_colors(FAILED_MAPPING_COLORS);
    } else {
      send_colors(ZERO_COLORS);
    }
    if (iterationCount > 5000) {
      // clear it, let it run with whatever panels did respond.
      failed_mapping_flag = false;
    }
  } else {
    aunisoma->update();

    //panel_colors[0] = '\0';
    for (int i = 0; i < NUMBER_OF_PANELS; i++) {
      Panel* panel = aunisoma->get_panel_at(i);
      Color color = panel->color;
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
  }

  iterationCount++;

  if (iterationCount == 60000) {
    //initializePanels();
    iterationCount = 0;
  }

  // Serial.print((micros() - start));
  // Serial.println("ns");
}
