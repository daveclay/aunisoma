//
// Created by David Clay on 6/16/23.
//

#include "Aunisoma.h"

Aunisoma::Aunisoma(Config* config,
                   GradientValueMap* gradients,
                   int number_of_gradients,
                   Sensor* sensors) {
    this->config = config;
    this->sensors = sensors;
    this->current_interaction_percent = 0;
    this->color_manager = new ColorManager(gradients, number_of_gradients, config);

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
      this->reverberations[i] = new Reverberation(&sensors[i * 2], config, i);
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
    this->color_manager->update(this->current_interaction_percent);

    for (int reverberation_index = 0; reverberation_index < NUMBER_OF_PANELS; reverberation_index++) {
        Reverberation* reverberation = this->reverberations[reverberation_index];
        reverberation->update();
    }

    for (int panel_index = 0; panel_index < NUMBER_OF_PANELS; panel_index++) {
        float panel_value = 0;
        ValueSmoothingFn* panel_smoothing_fn = this->panel_smoothing_functions[panel_index];

        for (int reverberation_index = 0; reverberation_index < NUMBER_OF_PANELS; reverberation_index++) {
            Reverberation* reverberation = this->reverberations[reverberation_index];
            panel_value += reverberation->get_panel_value_for_panel_index(panel_index);
        }

        float value = panel_smoothing_fn->get_smoothed_value(panel_value);
        Color color = this->_calculate_color_for_value(value);
        this->panels[panel_index]->color = color;
    }
}

Color Aunisoma::_calculate_color_for_value(float value) {
    return this->color_manager->get_color(value);
}

void Aunisoma::_calculate_interaction_percent() {
    int active_sensor_count = 0;
    for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
        Sensor* sensor = &this->sensors[i];
        if (sensor->active) {
            active_sensor_count++;
        }
    }
    this->current_interaction_percent = (float) active_sensor_count / (float) NUMBER_OF_SENSORS;
}