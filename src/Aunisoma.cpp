//
// Created by David Clay on 6/16/23.
//

#include "Aunisoma.h"

Aunisoma::Aunisoma(Config* config,
                   GradientValueMap* gradients,
                   int number_of_gradients,
                   GradientValueMap* rainbow_gradient,
                   GradientValueMap* knight_rider_gradient,
                   Sensor* sensors) {
    this->config = config;
    this->sensors = sensors;

    this->_create_panels();

    this->legacy_algorithm = new LegacyColorAlgorithm(config,
                                                      gradients,
                                                      number_of_gradients,
                                                      rainbow_gradient,
                                                      knight_rider_gradient,
                                                      sensors,
                                                      NUMBER_OF_PANELS,
                                                      NUMBER_OF_SENSORS);
    this->wave_algorithm = new WaveColorAlgorithm(config, sensors, NUMBER_OF_PANELS, NUMBER_OF_SENSORS);

    // Default to the new wave algorithm; swap to legacy at runtime via set_algorithm(0).
    this->current_algorithm = this->wave_algorithm;
}

void Aunisoma::_create_panels() {
    for (int panel_index = 0; panel_index < NUMBER_OF_PANELS; panel_index++) {
        Panel* panel = new Panel(panel_index);
        this->panels[panel_index] = panel;
    }
}

Panel* Aunisoma::get_panel_at(int index) {
    return this->panels[index];
}

void Aunisoma::update() {
    this->current_algorithm->update();
    for (int panel_index = 0; panel_index < NUMBER_OF_PANELS; panel_index++) {
        this->panels[panel_index]->color = this->current_algorithm->get_color_for_panel(panel_index);
    }
}

void Aunisoma::set_algorithm(int kind) {
    if (kind == AUNISOMA_ALGORITHM_LEGACY) {
        this->current_algorithm = this->legacy_algorithm;
    } else {
        this->current_algorithm = this->wave_algorithm;
    }
}
