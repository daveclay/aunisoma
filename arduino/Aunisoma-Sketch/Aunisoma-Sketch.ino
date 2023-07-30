#include "Adafruit_DotStar.h"
#include "SPI.h"
#include "Clock.h"
#include "Cycle.h"
#include "Color.h"
#include "Config.h"
#include "Gradient.h"
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

int number_of_panels = NUMPIXELS;
Config config = Config();

GradientValueMap* maxAnimationGradient = new GradientValueMap();
GradientValueMap* initial_gradient = new GradientValueMap();
GradientValueMap* blue_gradient = new GradientValueMap();
GradientValueMap* green_gradient = new GradientValueMap();
GradientValueMap* purple_red_gradient = new GradientValueMap();
GradientValueMap* green_blue_gradient = new GradientValueMap();

GradientValueMap* gradients[5] = {
        initial_gradient,
        blue_gradient,
        purple_red_gradient,
        green_blue_gradient,
        green_gradient
};

Aunisoma* aunisoma;

void setup(void) {
    Serial.begin(9600);

    maxAnimationGradient->add_rgb_point(0.00,   255,   0,   0);
    maxAnimationGradient->add_rgb_point(0.02,   255, 127,   0);
    maxAnimationGradient->add_rgb_point(0.20,   255, 255,   0);
    maxAnimationGradient->add_rgb_point(0.29,   200, 255,   0);
    maxAnimationGradient->add_rgb_point(0.30,     0, 255,   0);
    maxAnimationGradient->add_rgb_point(0.31,     0, 255, 127);
    maxAnimationGradient->add_rgb_point(0.40,     0, 255, 200);
    maxAnimationGradient->add_rgb_point(0.45,     0, 255, 255);
    maxAnimationGradient->add_rgb_point(0.59,     0, 120, 255);
    maxAnimationGradient->add_rgb_point(0.60,     0,   0, 255);
    maxAnimationGradient->add_rgb_point(0.61,   160,   0, 255);
    maxAnimationGradient->add_rgb_point(0.80,   255,   0, 255);
    maxAnimationGradient->add_rgb_point(0.80,   255,   0, 255);
    maxAnimationGradient->add_rgb_point(0.89,   255,   0, 200);
    maxAnimationGradient->add_rgb_point(1.00,   255,   0,   0);

    initial_gradient->add_rgb_point(0.0, 3, 0, 0);
    initial_gradient->add_rgb_point(.4, 255, 0, 0);
    initial_gradient->add_rgb_point(1.0, 255, 255, 0);
    initial_gradient->add_rgb_point(1.6, 0, 255, 255);
    initial_gradient->add_rgb_point(3, 0, 255, 255);

    blue_gradient->add_rgb_point(0.0, 0, 0, 10);
    blue_gradient->add_rgb_point(.4, 0, 0, 255);
    blue_gradient->add_rgb_point(.8, 255, 0, 255);
    blue_gradient->add_rgb_point(2, 255, 255, 0);
    blue_gradient->add_rgb_point(3, 255, 255, 0);

    green_gradient->add_rgb_point(0.0, 0, 10, 0);
    green_gradient->add_rgb_point(.4, 0, 255, 0);
    green_gradient->add_rgb_point(.8, 255, 255, 0);
    green_gradient->add_rgb_point(1.5, 255, 0, 255);
    green_gradient->add_rgb_point(3, 255, 0, 255);

    purple_red_gradient->add_rgb_point(0,   1,    0,   1);
    purple_red_gradient->add_rgb_point(.4, 255,   0, 255);
    purple_red_gradient->add_rgb_point(.85,  255,   0,   0);
    purple_red_gradient->add_rgb_point(1.2,  255, 255,   0);
    purple_red_gradient->add_rgb_point(3,    0, 255,   0);

    green_blue_gradient->add_rgb_point(0,    0,   3,   0);
    green_blue_gradient->add_rgb_point(.3,   0, 255,   0);
    green_blue_gradient->add_rgb_point(1,    0, 255, 255);
    green_blue_gradient->add_rgb_point(2,    0,   0, 255);
    green_blue_gradient->add_rgb_point(3,  255,   0, 255);

    config.number_of_panels = number_of_panels;
    config.reverberation_distance_range = new Range(2, 5);
    // how long to wait to trigger a neighbor Panel to reverberate
    config.reverberation_panel_delay_ticks = 20;
    config.trigger_panel_animation_loop_duration_ticks_range = new Range(220, 300);
    config.max_interaction_threshold_percent = .85;
    config.intermediate_interaction_threshold_percent = .5;
    config.min_max_interaction_gradient_transition_duration = 5000;
    config.odds_for_max_interaction_gradient_transition = 90;

    config.init();

    for (int i = 0; i < 40; i++) {
        sensors[i] = new Sensor();
        pinMode(FIRST_INPUT_PIN + i, INPUT_PULLDOWN);
    }

    aunisoma = new Aunisoma(&config, maxAnimationGradient, gradients, 5, sensors);
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

    for (int i = 0; i < number_of_panels; i++) {
        Panel *panel = aunisoma->get_panel_at(i);
        Color color = panel->color;
        strip.setPixelColor(i, gamma8[color.red], gamma8[color.green], gamma8[color.blue]);
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
