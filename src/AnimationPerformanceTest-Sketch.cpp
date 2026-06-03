#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Arduino.h"
#include "Color.h"
#include "PanelLink.h"

// Animation performance test. Pulses every panel in lockstep from idle
// (3,0,0) up to peak (255,0,0) and back, on a continuous sine wave with no
// per-loop delay. Every panel gets the identical color every frame.
//
// Purpose: separate the two sources of visual flicker.
//   - If the arduino loop is the bottleneck, frames are coarse but all 20
//     panels still light up identically each frame — the pulse looks chunky
//     but stays geometrically aligned across the strip.
//   - If the Serial2 link to the master (or the master->panel fan-out) is
//     the bottleneck, panels visibly drift apart, tear, or stutter against
//     each other even though this sketch sends them the same value.

static char panel_ids[] = "281A221B162515111220181914171F131D211E23";

static PanelLink link;
// +1 for the trailing '\0' — Serial2.print(const char*) reads until null,
// so the buffer must be terminated or it will emit garbage past the 120
// hex bytes.
static char panel_color_hex[PanelLink::PANEL_COUNT * PanelLink::COLOR_HEX_LEN + 1];
static char pir_readings[PanelLink::PANEL_COUNT];

static constexpr int IDLE_RED_VALUE = 3;
static constexpr int PEAK_RED_VALUE = 255;
static constexpr unsigned long PULSE_PERIOD_MS = 2000;

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
    unsigned long now_ms = millis();
    float phase = (float)(now_ms % PULSE_PERIOD_MS) / (float)PULSE_PERIOD_MS;
    // 0 → 1 → 0 over one period via cosine; smooth enough that any
    // frame-to-frame stutter shows up as visible flicker rather than as a
    // discontinuity baked into the waveform itself.
    float wave = 0.5f - 0.5f * cosf(phase * 2.0f * (float)M_PI);
    int red_value = IDLE_RED_VALUE + (int)((PEAK_RED_VALUE - IDLE_RED_VALUE) * wave + 0.5f);

    char single_panel_hex[PanelLink::COLOR_HEX_LEN + 1];
    snprintf(single_panel_hex,
             PanelLink::COLOR_HEX_LEN + 1,
             "%02x%02x%02x",
             gamma_lut[red_value],
             0,
             0);

    for (int panel_index = 0; panel_index < PanelLink::PANEL_COUNT; panel_index++) {
        memcpy(&panel_color_hex[panel_index * PanelLink::COLOR_HEX_LEN],
               single_panel_hex,
               PanelLink::COLOR_HEX_LEN);
    }

    link.send_colors(panel_color_hex, pir_readings);
}
