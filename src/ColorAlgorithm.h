#ifndef C_AUNISOMA_COLORALGORITHM_H
#define C_AUNISOMA_COLORALGORITHM_H

#include "Color.h"

class ColorAlgorithm {
public:
    virtual ~ColorAlgorithm() = default;
    virtual void update() = 0;
    virtual Color get_color_for_panel(int panel_index) const = 0;
};

#endif //C_AUNISOMA_COLORALGORITHM_H
