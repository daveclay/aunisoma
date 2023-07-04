//
// Created by David Clay on 6/15/23.
//

#include "Panel.h"
#include "Color.h"

Panel::Panel(int index, Color idleColor): idleColor(0, 0, 0), color(0, 0, 0) {
    this->index = index;
    this->idleColor = idleColor;
    this->color = idleColor;
    this->active = false;
    this->currentValue = 0;
}

void Panel::reset() {
    this->color = this->idleColor;
    this->currentValue = 0;
}
