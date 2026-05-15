//
// Created by David Clay on 6/16/23.
//

#ifndef C_AUNISOMA_AUNISOMA_H
#define C_AUNISOMA_AUNISOMA_H


#include "ColorAlgorithm.h"
#include "Config.h"
#include "Gradient.h"
#include "LegacyColorAlgorithm.h"
#include "Panel.h"
#include "Sensor.h"
#include "WaveColorAlgorithm.h"

#define NUMBER_OF_PANELS 20
#define NUMBER_OF_SENSORS 40

enum AunisomaAlgorithmKind {
    AUNISOMA_ALGORITHM_LEGACY = 0,
    AUNISOMA_ALGORITHM_WAVE = 1
};

class Aunisoma {
public:
    Aunisoma(Config* config,
             GradientValueMap* gradients,
             int number_of_gradients,
             GradientValueMap* rainbow_gradient,
             GradientValueMap* knight_rider_gradient,
             Sensor* sensors);
    Panel* get_panel_at(int);
    void update();
    void set_algorithm(int kind);
private:
    Config* config;
    Sensor* sensors;
    Panel* panels[NUMBER_OF_PANELS];
    LegacyColorAlgorithm* legacy_algorithm;
    WaveColorAlgorithm* wave_algorithm;
    ColorAlgorithm* current_algorithm;

    void _create_panels();
};


#endif //C_AUNISOMA_AUNISOMA_H
