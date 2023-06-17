//
// Created by David Clay on 6/15/23.
//

#include <cstdlib>
#include "Range.h"

Range::Range(int min, int max) {
    this->min = min;
    this->max = max;
    this->range = this->max - this->min;
}

int Range::random_int_between() {
    return this->min + (std::rand() % (this->range));
}
