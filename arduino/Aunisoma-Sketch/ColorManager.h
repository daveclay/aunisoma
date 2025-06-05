//
// Created by Dave Clay on 6/3/25.
//

#ifndef COLORMANAGER_H
#define COLORMANAGER_H
#include "Config.h"
#include "Debounce.h"
#include "Gradient.h"
#include "TransitionAnimation.h"


/******************************************************************
 * ColorManager
 * manages the color gradients and transitions between them based
 * on interaction percentage.
 *****************************************************************/
class ColorManager {
public:
      ColorManager(GradientValueMap* gradients, int number_of_gradients, Config* config);
      void update(float current_interaction_percent);
      Color get_color(float value);

private:
    Config* config;
    Debounce* intermediate_interaction_debounce;
    Cycle* intermediate_transition_delay_timer;
    TransitionAnimation* gradient_transition_animation;
    GradientValueMap* gradients;
    int number_of_gradients;
    int current_gradient_index;
    int next_gradient_index;
    GradientValueMap* current_gradient;
    int max_gradient_index;
    bool transition_active;
    float current_interaction_percent;

    void _update_intermediate_transition_state();
    void _start_gradient_transition();
    bool _is_intermediate_interaction();
    bool _is_max_interaction();
};

#endif //COLORMANAGER_H
