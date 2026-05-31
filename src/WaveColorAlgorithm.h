#ifndef C_AUNISOMA_WAVECOLORALGORITHM_H
#define C_AUNISOMA_WAVECOLORALGORITHM_H

#include "Clock.h"
#include "ColorAlgorithm.h"
#include "Config.h"
#include "Sensor.h"
#include "Wave.h"

class WaveColorAlgorithm : public ColorAlgorithm {
public:
    WaveColorAlgorithm(Config* config,
                       Sensor* sensors,
                       int number_of_panels,
                       int number_of_sensors);
    void update() override;
    Color get_color_for_panel(int panel_index) const override;

private:
    Config* config;
    Sensor* sensors;
    int number_of_panels;
    int number_of_sensors;
    Wave** waves;
    Color* displayed_colors;
    bool* prev_sensor_active;
    bool rainbow_target_active;
    float rainbow_blend;
    float blend_at_transition_start;
    Clock rainbow_clock;
    Clock transition_clock;

    Color _pick_target_for_activation(int activating_sensor_index, int activating_origin_panel);
    Color _pick_random_saturated_color() const;
    bool _is_high_interaction() const;
    Color _rainbow_color_for_panel(int panel_index) const;
};

#endif //C_AUNISOMA_WAVECOLORALGORITHM_H
