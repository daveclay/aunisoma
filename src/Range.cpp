//
// Created by David Clay on 6/15/23.
//

#include "Range.h"
#include "Arduino.h"

Range::Range(int min, int max) {
    this->min = min;
    this->max = max;
}

int Range::random_int_between() {
  return random(this->min, this->max);
}
