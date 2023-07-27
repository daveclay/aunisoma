//
// Created by Dave Clay on 7/23/23.
//

#ifndef C_AUNISOMA_MAXINTERACTIONANIMATION_H
#define C_AUNISOMA_MAXINTERACTIONANIMATION_H


#include "Gradient.h"
#include "Cycle.h"
#include "Panel.h"

class MaxInteractionAnimation: public CycleHandler {
public:
    MaxInteractionAnimation(int numberOfPanels, Panel** panels, GradientValueMap* gradient);
    bool active;
    int transitionTicks;
    bool hasTransitionedOut;
    Color panelColorByPanelIndex[20];

    void value(float value, CycleDirection direction);
    void start();
    void update();
    int get_transition_amount();
    Color get_color(Panel* panel);
    void updateTransition();

private:
    int numberOfPanels;
    int transitionDuration;
    Cycle* cycle;
    float current_value;
    GradientValueMap* gradient;
    Panel** panels;

    Color getColorForPanelAndValue(Panel* panel, float value);
};


#endif //C_AUNISOMA_MAXINTERACTIONANIMATION_H
