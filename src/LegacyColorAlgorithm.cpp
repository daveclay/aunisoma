#include "LegacyColorAlgorithm.h"

LegacyColorAlgorithm::LegacyColorAlgorithm(Config* config,
                                           GradientValueMap* gradients,
                                           int number_of_gradients,
                                           GradientValueMap* rainbow_gradient,
                                           GradientValueMap* knight_rider_gradient,
                                           Sensor* sensors,
                                           int number_of_panels,
                                           int number_of_sensors) {
    this->config = config;
    this->sensors = sensors;
    this->number_of_panels = number_of_panels;
    this->number_of_sensors = number_of_sensors;
    this->current_interaction_percent = 0;
    this->color_manager = new ColorManager(gradients, number_of_gradients, rainbow_gradient, knight_rider_gradient, config);

    this->reverberations = new Reverberation*[number_of_sensors];
    for (int panel_index = 0; panel_index < number_of_panels; panel_index++) {
        int sensor_index = panel_index * 2;
        this->reverberations[sensor_index] = new Reverberation(&sensors[sensor_index], config, panel_index);
        this->reverberations[sensor_index + 1] = new Reverberation(&sensors[sensor_index + 1], config, panel_index);
    }

    int window_size = config->smoothing_fn_window_size;
    this->panel_smoothing_functions = new ValueSmoothingFn*[number_of_panels];
    for (int panel_index = 0; panel_index < number_of_panels; panel_index++) {
        this->panel_smoothing_functions[panel_index] = new ValueSmoothingFn(window_size);
    }

    this->resolved_colors = new Color[number_of_panels];
}

void LegacyColorAlgorithm::update() {
    this->_calculate_interaction_percent();
    bool is_pong_interactivity = this->_is_pong_interactivity();

    this->color_manager->update(this->current_interaction_percent, is_pong_interactivity);

    for (int reverberation_index = 0; reverberation_index < this->number_of_sensors; reverberation_index++) {
        Reverberation* reverberation = this->reverberations[reverberation_index];
        reverberation->update();
    }

    for (int panel_index = 0; panel_index < this->number_of_panels; panel_index++) {
        float panel_value = 0;
        ValueSmoothingFn* panel_smoothing_fn = this->panel_smoothing_functions[panel_index];

        for (int reverberation_index = 0; reverberation_index < this->number_of_sensors; reverberation_index++) {
            Reverberation* reverberation = this->reverberations[reverberation_index];
            panel_value += reverberation->get_panel_value_for_panel_index(panel_index);
        }

        float smoothed_value = panel_smoothing_fn->get_smoothed_value(panel_value);
        this->resolved_colors[panel_index] = this->color_manager->get_color(panel_index, smoothed_value);
    }
}

Color LegacyColorAlgorithm::get_color_for_panel(int panel_index) const {
    return this->resolved_colors[panel_index];
}

bool LegacyColorAlgorithm::_is_pong_interactivity() const {
    for (int sensor_index = 2; sensor_index < this->number_of_panels - 2; sensor_index++) {
        if (this->sensors[sensor_index].active) {
            return false;
        }
    }

    Sensor first_front_sensor = this->sensors[0];
    Sensor first_back_sensor = this->sensors[1];
    Sensor last_front_sensor = this->sensors[this->number_of_sensors - 2];
    Sensor last_back_sensor = this->sensors[this->number_of_sensors - 1];

    bool first_panel_active = first_front_sensor.active || first_back_sensor.active;
    bool last_panel_active = last_front_sensor.active || last_back_sensor.active;

    return first_panel_active && last_panel_active;
}

void LegacyColorAlgorithm::_calculate_interaction_percent() {
    int active_sensor_count = 0;
    for (int sensor_index = 0; sensor_index < this->number_of_sensors; sensor_index++) {
        Sensor* sensor = &this->sensors[sensor_index];
        if (sensor->active) {
            active_sensor_count++;
        }
    }
    this->current_interaction_percent = static_cast<float>(active_sensor_count) / static_cast<float>(this->number_of_sensors);
}
