//
// Created by David Clay on 6/15/23.
//

#include "Arduino.h"

static int interpolateValue(int value_a, int value_b, float amount) {
    float value_a_amount = ((float)value_a) * (1.0f - amount);
    float value_b_amount = ((float)value_b) * amount;

    return (int) value_a_amount + (int) value_b_amount;
}

static bool maybe(int percentage) {
    return random(101) > percentage;
}
