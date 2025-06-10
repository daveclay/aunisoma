//
// Created by Dave Clay on 6/3/25.
//

#ifndef COLORMANAGER_H
#define COLORMANAGER_H
#include "Config.h"
#include "Debounce.h"
#include "DistributedPanelAnimation.h"
#include "Gradient.h"
#include "Interpolation.h"

enum ColorManagerState {
    LOW_INTERACTION_STATE,
    START_DEFAULT_GRADIENT_DELAY_STATE,
    TRANSITIONING_TO_DEFAULT_GRADIENT_STATE,
    TRANSITIONING_GRADIENT_STATE,
    TRANSITIONING_FROM_MID_TO_LOW_STATE,
    TRANSITIONING_FROM_MID_TO_HIGH_STATE,
    GRADIENT_SWAP_DELAY_STATE,
    HIGH_INTERACTION_STATE,
    TRANSITIONING_FROM_HIGH_TO_MID_STATE
};

/******************************************************************
 * ColorManager
 * manages the color gradients and transitions between them based
 * on interaction percentage.
 *****************************************************************/
class ColorManager {
public:
    ColorManager(GradientValueMap* gradients, int number_of_gradients, GradientValueMap* rainbow_gradient, Config* config);
    void update(float current_interaction_percent);
    Color get_color(int panel_index, float value) const;

private:
    Config* config;
    Debounce* high_interaction_debounce;
    Debounce* med_interaction_debounce;
    Timer* default_gradient_delay_timer;
    Timer* transition_delay_timer;
    Interpolation* transition_interpolation;
    DistributedPanelAnimation* distributed_panel_animation;
    GradientValueMap* gradients;
    int number_of_gradients;
    GradientValueMap* rainbow_gradient;
    int current_gradient_index;
    int next_gradient_index;
    GradientValueMap* current_gradient;
    ColorManagerState state;

    void _update_interaction_reading(float current_interaction_percent) const;
    void _switch_to_new_current_gradient();
    Color _get_transition_color(Color from_color, Color to_color) const;
    Color _get_rainbow_color_for_panel_index(int panel_index) const;
    void _update_clocks() const;
    void _update_state();
    void _set_state(ColorManagerState state);
    void _start_gradient_swap_transition();
    void _start_swap_to_default_gradient_transition();
    void _start_transition_to_low_interactivity() const;
    void _start_transition_to_high_interactivity() const;
    void _start_transition_from_high_to_mid_interactivity() const;
    bool _is_gradient_swap_delay_done() const;
    bool _is_transition_done() const;
    bool _is_low_interaction() const;
    bool _is_med_interaction() const;
    bool _is_high_interaction() const;
};

#endif //COLORMANAGER_H
