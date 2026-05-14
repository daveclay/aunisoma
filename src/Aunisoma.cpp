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
    this->current_interaction_percent = 0;
    this->color_manager = new ColorManager(gradients, number_of_gradients, rainbow_gradient, knight_rider_gradient, config);

    this->_create_panels();
    this->_create_reverberations();
    this->_create_panel_smoothing_functions();
}

void Aunisoma::_create_panels() {
    for (int i = 0; i < NUMBER_OF_PANELS; i++) {
        Panel* panel = new Panel(i);
        this->panels[i] = panel;
    }
}

void Aunisoma::_create_reverberations() {
    for (int i = 0; i < NUMBER_OF_PANELS; i++) {
      int sensor_index = i * 2;
      this->reverberations[sensor_index] = new Reverberation(&sensors[sensor_index], config, i);
      this->reverberations[sensor_index + 1] = new Reverberation(&sensors[sensor_index + 1], config, i);
    }
}

void Aunisoma::_create_panel_smoothing_functions() {
    int window_size = this->config->smoothing_fn_window_size;
    for (int i = 0; i < NUMBER_OF_PANELS; i++) {
        this->panel_smoothing_functions[i] = new ValueSmoothingFn(window_size);
    }
}

Panel* Aunisoma::get_panel_at(int index) {
    return this->panels[index];
}

void Aunisoma::update() {
    this->_calculate_interaction_percent();
    bool is_pong_interactivity = this->_is_pong_interactivity();

    this->color_manager->update(this->current_interaction_percent, is_pong_interactivity);

    for (int reverberation_index = 0; reverberation_index < NUMBER_OF_SENSORS; reverberation_index++) {
        Reverberation* reverberation = this->reverberations[reverberation_index];
        reverberation->update();
    }

    for (int panel_index = 0; panel_index < NUMBER_OF_PANELS; panel_index++) {
        float panel_value = 0;
        ValueSmoothingFn* panel_smoothing_fn = this->panel_smoothing_functions[panel_index];

        for (int reverberation_index = 0; reverberation_index < NUMBER_OF_SENSORS; reverberation_index++) {
            Reverberation* reverberation = this->reverberations[reverberation_index];
            panel_value += reverberation->get_panel_value_for_panel_index(panel_index);
        }

        float value = panel_smoothing_fn->get_smoothed_value(panel_value);
        Color color = this->_calculate_color_for_value(panel_index, value);
        this->panels[panel_index]->color = color;
    }
}

bool Aunisoma::_is_pong_interactivity() {
    // see if all panels between the first and last are _inactive_.
    for (int i = 2; i < NUMBER_OF_PANELS - 2; i++) {
        if (this->sensors[i].active) {
            return false;
        }
    }

    Sensor firstFrontSensor = this->sensors[0];
    Sensor firstBackSensor = this->sensors[1];
    Sensor lastFrontSensor = this->sensors[NUMBER_OF_SENSORS - 2];
    Sensor lastBackSensor = this->sensors[NUMBER_OF_SENSORS - 1];

    bool firstPanelActive = firstFrontSensor.active || firstBackSensor.active;
    bool lastPanelActive = lastFrontSensor.active || lastBackSensor.active;

    return firstPanelActive && lastPanelActive;
}

Color Aunisoma::_calculate_color_for_value(int panel_index, float value) const {
    return this->color_manager->get_color(panel_index, value);
}

void Aunisoma::_calculate_interaction_percent() {
    int active_sensor_count = 0;
    for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
        Sensor* sensor = &this->sensors[i];
        if (sensor->active) {
            active_sensor_count++;
        }
    }
    this->current_interaction_percent = static_cast<float>(active_sensor_count) / static_cast<float>(NUMBER_OF_SENSORS);
}