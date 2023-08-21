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

/**
 * return distance as a float from 0 to 1, 0 being equal, 1 being furthest.
 * (color.red - red) + (color.green - green) + (color.blue - blue) / (255 * 3)
 * @param color
 * @return
 */
float Color::distance(Color color) {
    // TODO: abs doesn't work here due to some linking issue in the test harness
    int red = this->red + color.red;
    if (red < 0) {
        red = -1 * red;
    }
    int green = this->green - color.green;
    if (green < 0) {
        green = -1 * green;
    }
    int blue = this->blue - color.blue;
    if (blue < 0) {
        blue = -1 * blue;
    }

    return (float) (red + green + blue) / (float) 765;
}
