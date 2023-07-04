//
// Created by David Clay on 6/15/23.
//

#include <cmath>
#include <algorithm>
#include "Gradient.h"

GradientValuePoint::GradientValuePoint(float value, int color_value) {
    this->value = value;
    this->color_value = color_value;
}

SingleGradientValueMap::SingleGradientValueMap() {
    this->size = 0;
}

void SingleGradientValueMap::addPoint(float value, int color_value) {
    this->gradientValuePoints[this->size] = new GradientValuePoint(value, color_value);
    this->size += 1;
}

int SingleGradientValueMap::get_color_value_at_value(float value) {
    GradientValuePoint* points[2] = { NULL, NULL };
    this->get_gradient_value_points_for_value(value, points);

    float min_value = points[0]->value;
    float max_value = points[1]->value;
    float ratio = (value - min_value) / (max_value - min_value);

    int min_color = points[0]->color_value;
    int max_color = points[1]->color_value;

    int color_range = max_color - min_color;
    float ranged_color_value = (float)color_range * ratio;

    int color_value = min_color + (int)round(ranged_color_value);

    return std::min(255, std::max(0, color_value));
}

void SingleGradientValueMap::get_gradient_value_points_for_value(float value,
                                                                 GradientValuePoint* value_points[]) {
    GradientValuePoint* lower_point = NULL;
    for (int i = 0; i < this->size; i++) {
        GradientValuePoint* point = this->gradientValuePoints[i];
        if (value >= point->value) {
            lower_point = point;
        } else {
            if (lower_point) {
                value_points[0] = lower_point;
                value_points[1] = point;
            } else {
                value_points[0] = point;
                value_points[1] = point;
            }
            break;
        }
    }

    if (value_points[0] == NULL) {
        //otherwise, use the last (max) two points
        value_points[0] = this->gradientValuePoints[this->size - 2];
        value_points[1] = this->gradientValuePoints[this->size - 1];
    }
}

GradientValueMap::GradientValueMap() {
    this->redMap = new SingleGradientValueMap();
    this->greenMap = new SingleGradientValueMap();
    this->blueMap = new SingleGradientValueMap();
}

void GradientValueMap::add_rgb_point(float value, int red, int green, int blue) {
    this->redMap->addPoint(value, red);
    this->greenMap->addPoint(value, green);
    this->blueMap->addPoint(value, blue);
}

Color GradientValueMap::getColorForValue(float value) {
    int red = this->redMap->get_color_value_at_value(value);
    int green = this->greenMap->get_color_value_at_value(value);
    int blue = this->blueMap->get_color_value_at_value(value);
    return Color(red, green, blue);
}
