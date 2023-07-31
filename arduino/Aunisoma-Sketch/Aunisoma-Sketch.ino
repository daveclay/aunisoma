#include "SPI.h"
#include "wiring_private.h"  // pinPeripheral() function
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

#define NUMBER_OF_PANELS 20  // Number of LEDs in strip
#define SET_LIGHTS_SIZE_PER_PANEL 6 // number of chars to send the SET_LIGHTS message per panel

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
char TERMINATOR = '\r';
char responseBuffer[1024]; // should be 20 panels * however big messages are
char setLightsBuffer[(NUMBER_OF_PANELS * SET_LIGHTS_SIZE_PER_PANEL)];

bool panelsInitialized = false;

Sensor* sensors[NUMBER_OF_PANELS];

int number_of_panels = NUMBER_OF_PANELS;
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

int send_command(char cmd_byte, char params[]) {
    // Serial.print("Sending: '");
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
    Serial.println("Sending enumerate...");
    int bytesRead = send_command(ENUMERATE, NULL);
    if (bytesRead > 0) {
        // two bytes per panel
        int activePanels = bytesRead / 2;
        Serial.print("Initialized ");
        Serial.print(activePanels);
        Serial.print(" panels from ");
        Serial.print(bytesRead);
        Serial.println(" bytes: ");

        char buf[bytesRead];
        memcpy(buf, responseBuffer, bytesRead);
        Serial.println(responseBuffer);

        return true;
    } else {
        return false;
    }
}

void initializePanels() {
    digitalWrite(LED_BUILTIN, HIGH);

    while (!panelsInitialized && !send_enumerate()) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(300);
        digitalWrite(LED_BUILTIN, HIGH);
    }

    panelsInitialized = true;
}

bool sendColors(char* value) {
    int bytesRead = send_command(SET_LIGHTS, value);
    if (bytesRead > 0) {
        for (int i = 0; i < bytesRead; i++) {
            bool active = responseBuffer[i] == '1';
            sensors[i]->update(active);
        }
        return true;
    } else {
        return false;
    }
}

void setup(void) {
    Serial.begin(9600);
    Serial2.setTimeout(1000);
    Serial2.begin(230400);

    pinPeripheral(PIN_SERIAL3_RX, PIO_SERCOM);
    pinPeripheral(PIN_SERIAL3_TX, PIO_SERCOM);

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

    aunisoma = new Aunisoma(&config, maxAnimationGradient, gradients, 5, sensors);

    // Give some time for things to start up?
    delay(3000);
    initializePanels();
}

char panelColors[SET_LIGHTS_SIZE_PER_PANEL];

void loop(void) {
    aunisoma->event_loop();

    for (int i = 0; i < number_of_panels; i++) {
        Panel *panel = aunisoma->get_panel_at(i);
        Color color = panel->color;
        sprintf(panelColors,
                "%02x%02x%02x",
                gamma8[color.red],
                gamma8[color.green],
                gamma8[color.blue]);

        int startIndex = i * SET_LIGHTS_SIZE_PER_PANEL;
        for (int j = 0; j < 6; j++) {
            setLightsBuffer[startIndex + j] = panelColors[j];
        }
    }

    sendColors(setLightsBuffer);
}
