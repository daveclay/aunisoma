//
// Created by David Clay on 6/16/23.
//

#ifndef C_AUNISOMA_AUNISOMA_H
#define C_AUNISOMA_AUNISOMA_H


#include "Gradient.h"
#include "Panel.h"
#include "Interaction.h"
#include "TransitionAnimation.h"
#include "Sensor.h"
#include "MaxInteractionAnimation.h"

class Aunisoma: public PanelContext, public TransitionAnimationCallback {
public:
    Aunisoma(Config* config,
             GradientValueMap* maxAnimationGradient,
             GradientValueMap* gradients,
             int numberOfGradients,
             Sensor* sensors);

    int numberOfPanels;
    Sensor* sensors;

    Panel* get_panel_at(int);
    void event_loop();
    void switch_to_next_gradient();
    GradientValueMap* getCurrentGradient();

private:
    Config* config;
    GradientValueMap* gradients;
    int numberOfGradients;
    GradientValueMap* currentGradient;
    Panel* panels[20];
    Interaction* interactions_by_source_panel_index[20];
    TransitionAnimation* transitionAnimation;
    MaxInteractionAnimation* maxInteractionAnimation;
    int current_gradient_index;
    int next_gradient_index;
    int maxGradientIndex;
    bool transitioned_during_current_intermediate_state;
    int ticks_since_last_transition;
    float interaction_panel_values[20];

    void _calculate_next_gradient_index();
    void _create_panels();
    void _create_interactions();
    void read_sensors();
    void _handle_panel_sensor(int panel_index, bool active);
    void _start_transition();
    void _update_panels();
    void _update_panel(Panel* panel, float total_panel_value);
};


#endif //C_AUNISOMA_AUNISOMA_H
