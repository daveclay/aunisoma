//
// Created by David Clay on 7/7/23.
//
#include "algorithm"
#include "stdlib.h"
#include "Arduino.h"
#include <iostream>

char out[1024];
int outIndex = 0;
char in[1024];
char outPanelColors[6];
int panelColorLengthToRead = 6;
char currentCommand;
char lights[21] = "00000000000000000000";

int iteration = 0;

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
    int rand = std::rand();
    int res = min + (rand % (max - min + 1));
    return res;
}

int random(int max) {
    return std::rand() % max;
}

void delay(int) {
}

long millis() {
    return iteration;
}

void pinMode(int pin, int mode) {
}

void digitalWrite(int pin, int value) {
}

void writeScriptLine(char *colors, char *interactions) {
    std::cout << "\t{ \"iteration\": " << iteration << ", ";
    std::cout << "\"interactions\": \"" << interactions << "\", ";
    std::cout << "\"panels\": [\n";

    for (int i = 0; i < 120; i += panelColorLengthToRead) {
        int panelColorStartIndex = i; // 6 chars per panel, 2 chars per color
        char *start = colors + panelColorStartIndex;
        char *end = start + panelColorLengthToRead;
        std::copy(start, end, outPanelColors);

        int index = i / panelColorLengthToRead;
        char red[] = {outPanelColors[0], outPanelColors[1], '\0' };
        char green[] = {outPanelColors[2], outPanelColors[3], '\0' };
        char blue[] = {outPanelColors[4], outPanelColors[5], '\0' };

        std::cout << "\t\t{ \"index\": " << index << ", \"red\": \"" << red << "\", \"green\": \"" << green << "\", \"blue\": \"" << blue << "\"}";
        if (i < 114) {
            std::cout << ",";
        }
        std::cout << "\n";
    }

    std::cout << "\t]\n\t}";

    if (iteration < ITERATIONS - 1) {
        std::cout << ",";
    }

    std::cout << "\n";
}

Uart::Uart() {
}

Uart::Uart(const int * sercom, int pinRx, int pinTx, int padRx, int padTx) {
}

void Uart::IrqHandler() const {
}
void Uart::setTimeout(const int time) const {
}

void Uart::begin(const int baud) const {
}

void Uart::print(const char c) const {
    if (c == 'E') {
        currentCommand = c;
    } else if (c == 'L') {
        currentCommand = c;
    } else if (c == '\n') {
        if (currentCommand == 'L') {
            writeScriptLine(out, in);
            // int mod = iteration % 20;
            // if (mod == 0) {
            //     int numPanels = 1 + (std::rand() % 10); // 0 to 6
            //     for (int j = 0; j < numPanels; j++) {
            //         int panel = std::rand() % 20; // 0 through 19
            //         char active = ((std::rand() % 10) > 3) ? '1' : '0';
            //         lights[panel] = active;
            //     }
            // }
            // strcpy(in, lights);
            //
            if (iteration >= 290) {
                strcpy(in, "00000000000001100010");
            } else if (iteration > 2 && iteration < 290) {
                strcpy(in, "11011010000110101110");
            } else {
                strcpy(in, "00000000001000010000");
            }
            iteration++;
        } else if (currentCommand == 'M') {
            // 1F22201213191E111A1D21152425171B18281614
            // strcmp(in, "FAILED 1D2416");
            strcpy(in, "OK 00000000000000000000");
        }
    } else {
        out[outIndex] = c;
        outIndex++;
    }
}

void Uart::print(const char * s) const {
    strcpy(out, s);
}

void Uart::println(const char * s) const {
}

void Uart::flush() const {
}

int Uart::readBytesUntil(char stop, char buffer[], int length) const {
    if (currentCommand == 'M') {
        strcpy(buffer, in);
        return 40;
    } else if (currentCommand == 'L') {
        strcpy(buffer, in);
        return 20;
    } else {
        return 0;
    }
}