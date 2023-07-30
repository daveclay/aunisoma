// https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
// https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/muxing-it-up
// SAMD51 variants.h: https://github.com/adafruit/ArduinoCore-samd/blob/master/variants/grand_central_m4/variant.h
// SAMD51 datasheet: https://cdn.sparkfun.com/assets/0/0/5/9/2/60001507C.pdf?_gl=1*1edca6q*_ga*NzcwOTIyNDUwLjE2OTAwNjE5MTk.*_ga_T369JS7J9N*MTY5MDA2MTkxOC4xLjEuMTY5MDA2MjIxNS4zNS4wLjA.
// SCK is pin 52
// MOSI is pin 51

#include <Arduino.h>         // required before wiring_private.h
#include "wiring_private.h"  // pinPeripheral() function

// for Serial2 pins: https://github.com/adafruit/ArduinoCore-samd/blob/master/variants/grand_central_m4/variant.h#L155C9-L155C23
// Also read for SAMD51: https://cdn-learn.adafruit.com/downloads/pdf/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports.pdf
// RX pin, TX pin, RX PAD, TX PAD
// PIN_SERIAL3_RX is pin 17, PIN_SERIAL3_TX is pin 16
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
char ENUMERATE = 'E';
char SET_STATUS = 'S';
char SET_LIGHTS = 'L';
char rcvBuffer[256];  // should be 20 panels * however big messages are

void send_command(char cmd_byte, String params) {
  // Serial.print("Sending: '");
  // Serial.print(cmd_byte);
  // Serial.print(params);
  // Serial.println("'");

  Serial2.print(cmd_byte + params + '\r');
  Serial2.flush();

  int bytesRead = Serial2.readBytesUntil('\r', rcvBuffer, 256);  // ('\r');
  if (bytesRead > 0) {
    // Serial.print("Received: '");
    // Serial.print(rcvBuffer);
    // Serial.println("'");
  }
}

bool send_enumerate() {
  send_command(ENUMERATE, "");
  String response = String(rcvBuffer);
  return response == "V1V1";
}

void send_set_lights(String value) {
  send_command(SET_LIGHTS, value);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.setTimeout(1000);

  Serial2.begin(230400);
  pinPeripheral(PIN_SERIAL3_RX, PIO_SERCOM);
  pinPeripheral(PIN_SERIAL3_TX, PIO_SERCOM);

  pinMode(LED_BUILTIN, OUTPUT);
  delay(3000);
}

int i = 0;
bool enumerated = false;

void loop() {
  memset(rcvBuffer, 0, sizeof(rcvBuffer));
  digitalWrite(LED_BUILTIN, HIGH);

  while (!enumerated && !send_enumerate()) {
    memset(rcvBuffer, 0, sizeof(rcvBuffer));
    digitalWrite(LED_BUILTIN, LOW);
    delay(300);
    digitalWrite(LED_BUILTIN, HIGH);
  }

  enumerated = true;
  delay(10);
  digitalWrite(LED_BUILTIN, LOW);
  
  send_set_lights("030000030000");
  if (rcvBuffer[1] == '1') {
    send_set_lights("FF00303000FF");
    Serial.println("on");
  } else {

  }
}

