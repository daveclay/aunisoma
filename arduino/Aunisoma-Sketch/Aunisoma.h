//
// Created by David Clay on 6/16/23.
//

#ifndef C_AUNISOMA_AUNISOMA_H
#define C_AUNISOMA_AUNISOMA_H


#include "ColorManager.h"
#include "Config.h"
#include "Gradient.h"
#include "Panel.h"
#include "Sensor.h"
#include "Reverberation.h"
#include "ValueSmoothingFn.h"

#define NUMBER_OF_PANELS 20
#define NUMBER_OF_SENSORS 40

class Aunisoma {
public:
    Aunisoma(Config* config,
             GradientValueMap* gradients,
             int number_of_gradients,
             GradientValueMap* rainbow_gradient,
             Sensor* sensors);
    Panel* get_panel_at(int);
    void update();
private:
    Config* config;
    Sensor* sensors;
    float current_interaction_percent;
    Panel* panels[NUMBER_OF_PANELS];
    Reverberation* reverberations[NUMBER_OF_PANELS];
    ValueSmoothingFn* panel_smoothing_functions[NUMBER_OF_PANELS];
    ColorManager* color_manager;

    void _create_panels();
    void _create_reverberations();
    void _create_panel_smoothing_functions();
    Color _calculate_color_for_value(int panel_index, float value) const;
    void _calculate_interaction_percent();
};


#endif //C_AUNISOMA_AUNISOMA_H
