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

int iterations = 100000;
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
    return min + (std::rand() % (max - min));
}

int random(int max) {
    return std::rand() % max;
}

void delay(int) {
}

long millis() {
    return iteration;
}

void digitalWrite(int pin, int value) {
}

void writeScriptLine(char *colors) {
    std::cout << "\t{ \"iteration\": " << iteration << ",";
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

    if (iteration < iterations - 1) {
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
    } else if (c == '\r') {
        if (currentCommand == 'L') {
            writeScriptLine(out);
            if (iteration >= 3600) {
                strcpy(in, "10000000000000100001");
            } else if (iteration >= 2600) {
                strcpy(in, "11011011111111111111");
            } else if (iteration >= 1600) {
                strcpy(in, "00011010000000001110");
                /*
                int mod = iteration % 500;
                if (mod == 0) {
                    int numPanels = 1 + (std::rand() % 10); // 0 to 6
                    for (int j = 0; j < numPanels; j++) {
                        int panel = std::rand() % 20; // 0 through 19
                        char active = ((std::rand() % 10) > 3) ? '1' : '0';
                        lights[panel] = active;
                    }
                }
                strcpy(in, lights);
                 */
            } else if (iteration > 500 && iteration < 1600) {
                strcpy(in, "11011011111111111111");
            } else {
                strcpy(in, "00000000001000010000");
            }
            iteration++;
        } else if (currentCommand == 'E') {
            strcpy(in, "V1V1V1V1V1V1V1V1V1V1V1V1V1V1V1V1V1V1V1V1");
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
    if (currentCommand == 'E') {
        strcpy(buffer, in);
        return 40;
    } else if (currentCommand == 'L') {
        strcpy(buffer, in);
        return 20;
    } else {
        return 0;
    }
}