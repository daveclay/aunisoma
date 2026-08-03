#ifndef C_AUNISOMA_GLOWCOLORALGORITHM_H
#define C_AUNISOMA_GLOWCOLORALGORITHM_H

#include "Clock.h"
#include "ColorAlgorithm.h"
#include "Config.h"
#include "Glow.h"
#include "Sensor.h"

// Per-panel pulsing. Each sensor (front and back are independent
// interactions) drives its own Glow on its own panel; neighbors within
// glow_ripple_distance follow the source's fade envelope with a
// per-distance lag, so activation and release visibly propagate outward.
// Panels touched by more than one glow blend with the same chroma-vector
// overlap rule WaveColorAlgorithm uses. The high-interaction rainbow
// override is carried over unchanged.
class GlowColorAlgorithm : public ColorAlgorithm {
public:
    GlowColorAlgorithm(Config* config,
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
    Glow** glows;
    Color* displayed_colors;
    bool* prev_sensor_active;
    bool rainbow_target_active;
    float rainbow_blend;
    float blend_at_transition_start;
    Clock rainbow_clock;
    Clock transition_clock;

    Color _pick_target_for_activation(int activating_sensor_index, int activating_panel_index);
    Color _pick_random_saturated_color() const;
    bool _is_high_interaction() const;
    Color _rainbow_color_for_panel(int panel_index) const;
};

#endif //C_AUNISOMA_GLOWCOLORALGORITHM_H
