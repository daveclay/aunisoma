//
// Created by Dave Clay on 6/9/25.
//

#ifndef DISTRIBUTEDPANELANIMATION_H
#define DISTRIBUTEDPANELANIMATION_H

#include "Config.h"
#include "Cycle.h"

class DistributedPanelAnimation {
public:
    bool active;
    DistributedPanelAnimation(Config* config);
    void start();
    void update() const;
    void stop();
    float get_value_for_panel(int panel_index) const;

private:
    Cycle* cycle;
    float panel_distribution_ratio;
};

#endif //DISTRIBUTEDPANELANIMATION_H
