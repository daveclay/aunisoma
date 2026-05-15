#ifndef C_AUNISOMA_LEGACYCOLORALGORITHM_H
#define C_AUNISOMA_LEGACYCOLORALGORITHM_H

#include "ColorAlgorithm.h"
#include "ColorManager.h"
#include "Config.h"
#include "Gradient.h"
#include "Reverberation.h"
#include "Sensor.h"
#include "ValueSmoothingFn.h"

class LegacyColorAlgorithm : public ColorAlgorithm {
public:
    LegacyColorAlgorithm(Config* config,
                         GradientValueMap* gradients,
                         int number_of_gradients,
                         GradientValueMap* rainbow_gradient,
                         GradientValueMap* knight_rider_gradient,
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
    Reverberation** reverberations;
    ValueSmoothingFn** panel_smoothing_functions;
    ColorManager* color_manager;
    Color* resolved_colors;

    bool _is_pong_interactivity() const;
    void _calculate_interaction_percent();
    float current_interaction_percent;
};

#endif //C_AUNISOMA_LEGACYCOLORALGORITHM_H
