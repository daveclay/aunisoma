//
// Created by David Clay on 7/7/23.
//
#include "algorithm"
#include "stdlib.h"
#include <chrono>
#include "Arduino.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

char out[1024];
int outIndex = 0;
char in[1024];
char outPanelColors[6];
int panelColorLengthToRead = 6;
char currentCommand;
char lights[21] = "00000000000000000000";

int iteration = 0;

struct ScriptEntry {
    int iteration;
    std::string pirs;
};

static std::vector<ScriptEntry> mockScript;
static bool mockScriptLoaded = false;

static void loadMockScript() {
    mockScriptLoaded = true;
    const char *path = std::getenv("AUNISOMA_MOCK_SCRIPT");
    if (path == nullptr) {
        path = "lib/mock-arduino/script.txt";
    }
    std::ifstream file(path);
    if (!file) {
        std::cerr << "mock-arduino: could not open script file '" << path
                  << "' (set AUNISOMA_MOCK_SCRIPT to override)\n";
        return;
    }
    std::string line;
    int lineNo = 0;
    while (std::getline(file, line)) {
        lineNo++;
        auto hash = line.find('#');
        if (hash != std::string::npos) line.erase(hash);
        size_t start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r");
        line = line.substr(start, end - start + 1);
        size_t sep = line.find_first_of(" \t");
        if (sep == std::string::npos) {
            std::cerr << "mock-arduino: " << path << ":" << lineNo
                      << ": expected '<iteration> <pirs>'\n";
            continue;
        }
        int iter = std::atoi(line.substr(0, sep).c_str());
        size_t pirStart = line.find_first_not_of(" \t", sep);
        std::string pirs = line.substr(pirStart);
        if ((int)pirs.size() != NUMBER_OF_PANELS) {
            std::cerr << "mock-arduino: " << path << ":" << lineNo
                      << ": expected " << NUMBER_OF_PANELS
                      << " PIR digits, got " << pirs.size() << "\n";
            continue;
        }
        mockScript.push_back({iter, pirs});
    }
    std::sort(mockScript.begin(), mockScript.end(),
              [](const ScriptEntry &a, const ScriptEntry &b) {
                  return a.iteration < b.iteration;
              });
}

static const char *currentMockPirs() {
    if (!mockScriptLoaded) loadMockScript();
    static const std::string allZero(NUMBER_OF_PANELS, '0');
    const std::string *latest = &allZero;
    for (const auto &entry : mockScript) {
        if (entry.iteration <= iteration) {
            latest = &entry.pirs;
        } else {
            break;
        }
    }
    return latest->c_str();
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

long micros() {
    return iteration * 1000;
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
    } else {
        int  i= 0;
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
    } else if (c == 'M') {
        currentCommand = 'M';
    } else if (c == '\n') {
        if (currentCommand == 'L') {
            writeScriptLine(out, in);
            std::snprintf(in, sizeof(in), "OK %s", currentMockPirs());
            iteration++;
        } else if (currentCommand == 'M') {
            // 1F22201213191E111A1D21152425171B18281614
            // strcmp(in, "FAILED 1D2416");
            strcpy(in, "OK 22332223323333231222");
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
        return 35;
    } else {
        return 0;
    }
}