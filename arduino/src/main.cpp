#include <iostream>
#include <chrono>
#include "Arduino.h"
#include "Aunisoma.h"
#include "Aunisoma-Sketch.h"

void writeScriptLine(Aunisoma* aunisoma, int i, int duration, int iterations) {
    std::cout << "\t{ \"iteration\": " << i << ",";
    std::cout << "\"panels\": [\n";
    for (int j = 0; j < aunisoma->numberOfPanels; j++) {
        Panel* panel = aunisoma->get_panel_at(j);
        Color color = panel->color;
        std::cout << "\t\t{ \"index\": " << panel->index << ", \"red\": " << color.red << ", \"green\": " << color.green << ", \"blue\": " << color.blue << "}";
        if (j < aunisoma->numberOfPanels - 1) {
            std::cout << ",";
        }
        std::cout << "\n";

    }
    std::cout << "\t],\n\t\"time\": " << duration << "}";

    if (i < iterations - 1) {
        std::cout << ",";
    }

    std::cout << "\n";
}

int main() {

    setup();

    using namespace std::chrono;
    std::cout << "[\n";

    int iterations = 100000;
    // int i = 0;
    // while (true) {
    for (int i = 0; i < iterations; i++) {
        int mod = i % 700;
        if (mod == 0) {
            int numPanels = 1 + (std::rand() % 6); // 0 to 6
            for (int j = 0; j < numPanels; j++) {
                int panel = std::rand() % 20; // 0 through 19
                int active = (std::rand() % 10) > 4; // 50%
                setPIRPinSensor(panel, active);
            }
        }

        /*
        if (i % 100000 == 0) {
            std::cout << i << " iterations completed\n";
        }
        */
        auto start = std::chrono::high_resolution_clock::now();

        loop();

        auto finish = std::chrono::high_resolution_clock::now();
        long long int count = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
        writeScriptLine(aunisoma, i, count, iterations);
        // i += 1;
    }

    std::cout << "]";

    return 0;
}