#include <Adafruit_DotStar.h>
#include <SPI.h>
#include "Clock.h"
#include "Cycle.h"
#include "Color.h"
#include "Config.h"
#include "Gradient.h"
#include "Interaction.h"
#include "Panel.h"
#include "PanelContext.h"
#include "PanelReverberation.h"
#include "Sensor.h"
#include "TransitionAnimation.h"
#include "Aunisoma.h"

#define NUMPIXELS 20  // Number of LEDs in strip
#define FIRST_INPUT_PIN 4 // 14 through 54 are PIR pins

Adafruit_DotStar strip(NUMPIXELS, DOTSTAR_BGR);

Sensor* sensors[40] = {};

int number_of_panels = 20;
// number of panels to the left and right
int min_reverberation_distance = 2;
int max_reverberation_distance = 5;
// how long to wait to trigger a neighbor Panel to reverberate
int reverb_delay_ticks = 20;
int min_trigger_panel_animation_loop_duration_ticks = 120;
int max_trigger_panel_animation_loop_duration_ticks = 200;
float max_interaction_threshold_percent = .5;
int max_interaction_duration_ticks = 300;
float max_interaction_amount_of_reverberation = 0;  // this tends to flicker the max animation if set > 0
float max_interaction_value_multiplier = 3;

Config* config = new Config(
  number_of_panels,
  min_reverberation_distance,
  max_reverberation_distance,
  reverb_delay_ticks,
  min_trigger_panel_animation_loop_duration_ticks,
  max_trigger_panel_animation_loop_duration_ticks,
  max_interaction_threshold_percent,
  max_interaction_duration_ticks,
  max_interaction_amount_of_reverberation,
  max_interaction_value_multiplier);

GradientValueMap* initial_gradient = new GradientValueMap();
GradientValueMap* blue_gradient = new GradientValueMap();
GradientValueMap* green_gradient = new GradientValueMap();

GradientValueMap* gradients[3] = {
  initial_gradient,
  blue_gradient,
  green_gradient
};

Aunisoma* aunisoma;

void setup(void) {
  Serial.begin(9600);

  initial_gradient->add_rgb_point(0.0, 3, 0, 0);
  initial_gradient->add_rgb_point(.4, 255, 0, 0);
  initial_gradient->add_rgb_point(1.0, 255, 255, 0);
  initial_gradient->add_rgb_point(1.6, 0, 255, 255);
  initial_gradient->add_rgb_point(3, 0, 255, 255);

  blue_gradient->add_rgb_point(0.0, 0, 0, 10);
  blue_gradient->add_rgb_point(.4, 0, 0, 255);
  blue_gradient->add_rgb_point(1.0, 255, 0, 255);
  blue_gradient->add_rgb_point(2, 255, 255, 0);
  blue_gradient->add_rgb_point(3, 255, 255, 0);

  green_gradient->add_rgb_point(0.0, 0, 10, 0);
  green_gradient->add_rgb_point(.4, 0, 255, 0);
  green_gradient->add_rgb_point(1.0, 255, 255, 0);
  green_gradient->add_rgb_point(2, 255, 0, 255);
  green_gradient->add_rgb_point(3, 255, 0, 255);

  for (int i = 0; i < 40; i++) {
    sensors[i] = new Sensor();
    pinMode(FIRST_INPUT_PIN + i, INPUT_PULLDOWN);
  }

  pinMode(51, INPUT_PULLDOWN);

  aunisoma = new Aunisoma(config, gradients, 3, sensors);

  strip.begin();
}

void loop(void) {
  for (int i = 0; i < 40; i++) {
    int sensorPin = FIRST_INPUT_PIN + i;
    int panelActive = digitalRead(sensorPin) == HIGH;
    sensors[i]->active = panelActive;
/*
    if (panelActive) {
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(" reading pin ");
    Serial.print(sensorPin);
    Serial.println(" HIGH ");
    }
    */
  }

  aunisoma->event_loop();
  Serial.print("count: ");
  Serial.print(aunisoma->active_panel_count);
  Serial.print(" %: ");
  Serial.println(aunisoma->activePercent);

  for (int i = 0; i < number_of_panels; i++) {
    Panel* panel = aunisoma->get_panel_at(i);
    Color color = panel->color;
    strip.setPixelColor(i, color.red, color.green, color.blue);
/*
    Serial.print(i);
    Serial.print("\t");
    Serial.print(color.red);
    Serial.print("\t");
    Serial.print(color.green);
    Serial.print("\t");
    Serial.println(color.blue);
    */
  }

  strip.show();
  delay(4);
}