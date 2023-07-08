//
// Created by David Clay on 7/7/23.
//

#ifndef C_AUNISOMA_ARDUINO_H
#define C_AUNISOMA_ARDUINO_H
#include <cmath>

int abs(int a);
int min(int a, int b);
int max(int a, int b);
int random(int min, int max);
int random(int max);

const int INPUT_PULLDOWN = 0;
const int HIGH = 1;

void pinMode(int pin, int mode);
int digitalRead(int pin);
void setPIRPinSensor(int panelIndex, int pinValue);
const int DOTSTAR_BGR = 1;
void delay(int);

class SerialThing {
public:
    void begin(int) const {
    }
};

const SerialThing Serial = SerialThing();


#endif //C_AUNISOMA_ARDUINO_H
