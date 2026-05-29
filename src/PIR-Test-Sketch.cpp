#include <stdio.h>
#include "Arduino.h"
#include "Color.h"
#include "PanelLink.h"

// PIR test sketch. Paints every panel red by default; turns a panel green
// when its front PIR is active, blue when its back PIR is active, teal when
// both. Raw PIR — no debouncing. Used to verify wiring and PANEL_IDS order
// independently of any color algorithm.

static char panel_ids[] = "281A221B162515111220181914171F131D211E23";

static PanelLink link;
// +1 for the trailing '\0' — Serial2.print(const char*) reads until null,
// so the buffer must be terminated or it will emit garbage past the 120
// hex bytes. Static storage zero-inits the terminator and write_panel
// never touches the last byte.
static char panel_color_hex[PanelLink::PANEL_COUNT * PanelLink::COLOR_HEX_LEN + 1];
static char pir_readings[PanelLink::PANEL_COUNT];

struct RGB {
    int red;
    int green;
    int blue;
};

static constexpr RGB IDLE_RED    = {  3,   0,   0};
static constexpr RGB FRONT_GREEN = {  0, 255,   0};
static constexpr RGB BACK_BLUE   = {  0,   0, 255};
static constexpr RGB BOTH_TEAL   = {  0, 255, 255};

static char single_panel_hex[PanelLink::COLOR_HEX_LEN + 1];

static void write_panel(int panel_index, RGB color) {
    snprintf(single_panel_hex,
             PanelLink::COLOR_HEX_LEN + 1,
             "%02x%02x%02x",
             gamma_lut[color.red],
             gamma_lut[color.green],
             gamma_lut[color.blue]);
    for (int char_index = 0; char_index < PanelLink::COLOR_HEX_LEN; char_index++) {
        panel_color_hex[(panel_index * PanelLink::COLOR_HEX_LEN) + char_index] = single_panel_hex[char_index];
    }
}

void setup(void) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(9600);
    link.begin();


    link.map_panels_until_ok(panel_ids);

    for (int panel_index = 0; panel_index < PanelLink::PANEL_COUNT; panel_index++) {
        pir_readings[panel_index] = '0';
    }
}

void loop(void) {
    for (int panel_index = 0; panel_index < PanelLink::PANEL_COUNT; panel_index++) {
        RGB next;
        switch (pir_readings[panel_index]) {
            case '1': next = FRONT_GREEN; break;
            case '2': next = BACK_BLUE;   break;
            case '3': next = BOTH_TEAL;   break;
            default:  next = IDLE_RED;    break;
        }
        write_panel(panel_index, next);
    }

    link.send_colors(panel_color_hex, pir_readings);
    delay(100);
}
