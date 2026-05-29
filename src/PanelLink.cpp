#include "SPI.h"
#include "Arduino.h"
#include "wiring_private.h"
#include "PanelLink.h"

static Uart Serial2(&sercom1, PIN_SERIAL3_RX, PIN_SERIAL3_TX, PAD_SERIAL3_RX, PAD_SERIAL3_TX);

extern "C" {
void SERCOM1_0_Handler() { Serial2.IrqHandler(); }
void SERCOM1_1_Handler() { Serial2.IrqHandler(); }
void SERCOM1_2_Handler() { Serial2.IrqHandler(); }
void SERCOM1_3_Handler() { Serial2.IrqHandler(); }
}

static constexpr char ENUMERATE = 'E';
static constexpr char SET_LIGHTS = 'L';
static constexpr char MAP_PANELS = 'M';
static constexpr char TERMINATOR = '\n';

static char responseBuffer[512];

static int send_command(char cmd_byte, const char* params) {
    Serial2.print(cmd_byte);
    if (params) {
        Serial2.print(params);
    }
    Serial2.print(TERMINATOR);
    Serial2.flush();

    int bytesRead = Serial2.readBytesUntil(TERMINATOR, responseBuffer, sizeof(responseBuffer));
    responseBuffer[bytesRead] = '\0';
    return bytesRead;
}

void PanelLink::begin() {
    Serial2.setTimeout(1000);
    Serial2.begin(230400);
    pinPeripheral(PIN_SERIAL3_RX, PIO_SERCOM);
    pinPeripheral(PIN_SERIAL3_TX, PIO_SERCOM);
}

bool PanelLink::enumerate() {
    int bytesRead = send_command(ENUMERATE, nullptr);
    if (bytesRead <= 0) {
        return false;
    }
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
}

bool PanelLink::map_panels(const char* panel_ids) {
    int bytesRead = send_command(MAP_PANELS, panel_ids);
    if (bytesRead > 1) {
        Serial.println(responseBuffer);
        if (responseBuffer[0] == 'O' && responseBuffer[1] == 'K') {
            return true;
        }
    }
    return false;
}

void PanelLink::map_panels_until_ok(const char* panel_ids) {
    for (int i = 0; i < 100; i++) {
        if (map_panels(panel_ids)) {
            return;
        }
        delay(100);
    }
}

bool PanelLink::send_colors(const char* color_hex, char pir_out[PANEL_COUNT]) {
    int bytesRead = send_command(SET_LIGHTS, color_hex);
    if (bytesRead <= 0) {
        return false;
    }
    // Response is "OK " then PANEL_COUNT PIR bytes. The (3 + PANEL_COUNT)
    // cap mirrors the original sketch — observed bytesRead can exceed the
    // 23-byte payload, and we must not read past it into pir_out.
    int limit = min(3 + PANEL_COUNT, bytesRead);
    for (int panel_index = 0; panel_index < PANEL_COUNT; panel_index++) {
        int response_index = 3 + panel_index;
        if (response_index < limit) {
            pir_out[panel_index] = responseBuffer[response_index];
        } else {
            pir_out[panel_index] = '0';
        }
    }
    return true;
}
