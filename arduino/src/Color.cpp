//
// Created by David Clay on 6/15/23.
//

#include "Color.h"
#include "Maths.h"

Color::Color(int red, int green, int blue) {
    this->red = red;
    this->green = green;
    this->blue = blue;
}

Color Color::interpolate(Color color, float amount) const {
    int red = interpolateValue(this->red, color.red, amount);
    int green = interpolateValue(this->green, color.green, amount);
    int blue = interpolateValue(this->blue, color.blue, amount);
    return Color(red, green, blue);
}