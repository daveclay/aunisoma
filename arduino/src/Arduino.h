//
// Created by David Clay on 7/7/23.
//

#ifndef C_AUNISOMA_ARDUINO_H
#define C_AUNISOMA_ARDUINO_H
#include <cmath>

const int PIN_SERIAL3_RX = 1;
const int PIN_SERIAL3_TX = 2;
const int PAD_SERIAL3_RX = 3;
const int PAD_SERIAL3_TX = 4;
const int PIO_SERCOM = 5;

const int sercom1 = 0;
const int LED_BUILTIN = 0;
const int OUTPUT = 0;

int abs(int a);
int min(int a, int b);
int max(int a, int b);
int random(int min, int max);
int random(int max);
void digitalWrite(int pin, int value);

long millis();

class Uart {
public:
    Uart();
    Uart(const int * sercom, int pinRx, int pinTx, int padRx, int padTx);
    void IrqHandler() const;
    void setTimeout(int time) const;
    void begin(int baud) const;
    void print(const char s) const;
    void print(const char * s) const;
    void println(const char * s) const;
    void flush() const;
    int readBytesUntil(char stop, char buffer[], int length) const;
};

const int INPUT_PULLDOWN = 0;
const int LOW = 0;
const int HIGH = 1;

void pinMode(int pin, int mode);
int digitalRead(int pin);
void setPIRPinSensor(int panelIndex, int pinValue);
const int DOTSTAR_BGR = 1;
void delay(int);

const Uart Serial = Uart();

#endif //C_AUNISOMA_ARDUINO_H
