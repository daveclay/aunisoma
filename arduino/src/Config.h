//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_CONFIG_H
#define C_AUNISOMA_CONFIG_H


#include "Range.h"

class Config {
public:
    int number_of_panels;
    int initial_trigger_panel_animation_loop_duration_ticks;
    Range* reverberation_distance_range;
    int max_reverberation_distance;
    int reverberation_panel_delay_ticks;
    Range* trigger_panel_animation_loop_duration_ticks_range;
    float max_interaction_threshold_percent;
    float intermediate_interaction_threshold_percent;
    int min_max_interaction_gradient_transition_duration;
    int odds_for_interaction_gradient_transition;

    Config();

    void init();
    int getTriggerPanelAnimationLoopDurationTicks() const;
    int get_reverberation_distance() const;
};


#endif //C_AUNISOMA_CONFIG_H
