//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_PANEL_H
#define C_AUNISOMA_PANEL_H


#include "Color.h"

class Panel {
public:

    int index;
    Color idleColor;
    Color color;
    float currentValue;
    bool active;

    Panel(int index, Color idleColor);
    void reset();
};


#endif //C_AUNISOMA_PANEL_H
