//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_GRADIENT_H
#define C_AUNISOMA_GRADIENT_H


#include "Color.h"

class GradientValuePoint {
public:
    float value;
    int color_value;
    GradientValuePoint(float value, int color_value);
};

class SingleGradientValueMap {
public:
    SingleGradientValueMap();
    GradientValuePoint* gradientValuePoints[10]; // TODO: eh, only need 10 or so. I'm sure I'll remember to bump this when I want more. Enjoy, future me!

    void addPoint(float value, int color_value);
    int get_color_value_at_value(float value);

private:
    int size;
    void get_gradient_value_points_for_value(float value, GradientValuePoint* value_points[]);

};

class GradientValueMap {
public:
    SingleGradientValueMap* redMap;
    SingleGradientValueMap* greenMap;
    SingleGradientValueMap* blueMap;

    GradientValueMap();

    void add_rgb_point(float value, int red, int green, int blue);

    Color getColorForValue(float value);
};


#endif //C_AUNISOMA_GRADIENT_H
