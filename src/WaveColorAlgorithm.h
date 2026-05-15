#ifndef C_AUNISOMA_WAVECOLORALGORITHM_H
#define C_AUNISOMA_WAVECOLORALGORITHM_H

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

    Color _pick_shared_or_new_target(int activating_sensor_index, int activating_origin_panel);
    Color _pick_random_target(Color exclude_color) const;
};

#endif //C_AUNISOMA_WAVECOLORALGORITHM_H
