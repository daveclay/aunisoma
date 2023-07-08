//
// Created by David Clay on 7/7/23.
//
#include "cmath"
#include "algorithm"
#include "stdlib.h"
#include <iostream>
#include "Arduino.h"
#include "Aunisoma.h"

int abs(int value) {
    return std::abs(value);
}

int min(int a, int b) {
    return std::min(a, b);
}

int max(int a, int b) {
    return std::max(a, b);
}

int random(int min, int max) {
    return min + (std::rand() % (max - min));
}

int random(int max) {
    return std::rand() % max;
}

void delay(int) {
}

class ArduinoStub {
public:
    int pins[40];
    ArduinoStub() {
        for (int i = 0; i < 40; i++) {
            pins[i] = 0;
        }
    }
};

ArduinoStub arduinoStub = ArduinoStub();


void pinMode(int pin, int mode) {
    // Arduino Stub
}

int digitalRead(int pin) {
    return arduinoStub.pins[pin];
}

void setPIRPinSensor(int panelIndex, int pinValue) {
    int sensorIndex = panelIndex * 2;
    arduinoStub.pins[sensorIndex] = pinValue;
}