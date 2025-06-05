//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_CONFIG_H
#define C_AUNISOMA_CONFIG_H


#include "Range.h"

class Config {
public:
    int initial_trigger_panel_animation_loop_duration_ticks;
    Range* reverberation_distance_range;
    int max_reverberation_distance;
    int reverberation_panel_delay_ticks;
    Range* trigger_panel_animation_loop_duration_ticks_range;
    float max_interaction_threshold_percent;
    float intermediate_interaction_threshold_percent;
    int min_max_interaction_gradient_transition_duration;
    int odds_for_max_interaction_gradient_transition;

    int delay_for_gradient_transition_duration;
    int gradient_transition_animation_duration;
    int smoothing_fn_window_size;

    // This is here to make it available to the C++ files, since importing Aunisoma-Sketch isn't good.
    int number_of_panels;

    Config();

    void init();
    int getTriggerPanelAnimationLoopDurationTicks() const;
    int get_reverberation_distance() const;
};


#endif //C_AUNISOMA_CONFIG_H
