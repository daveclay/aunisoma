#include <iostream>
#include <chrono>
#include "Aunisoma-Sketch.h"
#include "Arduino.h"

int main() {

    setup();

    using namespace std::chrono;
    std::cout << "[\n";

    int iterations = 100000;

    for (int i = 0; i < iterations; i++) {
        loop();
    }

    std::cout << "]";

    return 0;
}