#include <iostream>
#include <chrono>
#include "Aunisoma-Sketch.h"
#include "Arduino.h"

int main() {

    setup();

    using namespace std::chrono;
    std::cout << "[\n";

    for (int i = 0; i < ITERATIONS; i++) {
        loop();
    }

    std::cout << "]";

    return 0;
}
