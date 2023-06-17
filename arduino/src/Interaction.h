//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_INTERACTION_H
#define C_AUNISOMA_INTERACTION_H


#include <cmath>
#include "Panel.h"
#include "Config.h"
#include "Clock.h"
#include "PanelReverberation.h"
#include "PanelContext.h"
#include "ReverberationDistanceSource.h"

class Interaction: public ReverberationDistanceSource {
public:
    Panel* sourcePanel;
    Config* config;
    PanelContext* panel_context;
    Clock* clock;
    PanelReverberation* panelReverberationsByPanelIndex[20];
    PanelReverberation* eligible_panel_reverberations[20];

    Interaction(Panel* sourcePanel, Config* config, PanelContext* panelContext);
    int getDistanceFromTrigger(Panel* panel);
    int getReverberationDistance();
    void start();
    void update();
    float get_value_for_panel(Panel* panel);
    void stop();
    bool isDead();

private:
    // Total number that _could_ be reverberating at any given time
    int number_of_panel_reverberations;
    // the _current_ number of Panels that are reverberating to one side
    int currentReverberatingDistance;
    // the index of the first Panel to the left that is reverberating
    int panel_reverberations_start_index;
    // the index of the last Panel to the _right_ that is reverberatin
    int panel_reverberations_end_index;
    int numberOfEligiblePanelReverberations;
    bool panel_reverberations_still_active;
    PanelReverberation* source_panel_reverberation;
    void _build_panel_reverberations();
    void addPanelReverberation(PanelReverberation* panelReverberation);
    PanelReverberation* createPanelReverberation(Panel* panel, int distanceFromTrigger);
    void _trigger_new_reverberation(bool trigger_source_panel);
    void _start_panel_reverberation(PanelReverberation* panel_reverberation) const;
};

#endif //C_AUNISOMA_INTERACTION_H
