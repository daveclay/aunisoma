//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_COLOR_H
#define C_AUNISOMA_COLOR_H


class Color {
public:
    int red;
    int green;
    int blue;

    Color(int red, int green, int blue);
    Color interpolate(Color color, float amount) const;
};

#endif //C_AUNISOMA_COLOR_H
