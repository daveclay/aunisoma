//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_PANELREVERBERATION_H
#define C_AUNISOMA_PANELREVERBERATION_H

#include "Cycle.h"
#include "Panel.h"
#include "Config.h"
#include "ReverberationDistanceSource.h"

class PanelReverberation: public CycleHandler {
public:
    PanelReverberation(Panel* panel,
                       Panel* sourcePanel,
                       int disanceFromTrigger,
                       ReverberationDistanceSource* reverberationDistanceSource,
                       Config* config);

    Panel* panel;
    Panel* sourcePanel;
    Config* config;
    ReverberationDistanceSource* reverberationDistanceSource;
    bool isSourceInteraction;
    float scale;
    Cycle* cycle;
    float currentValue;
    int distanceFromTrigger;

    void value(float value, CycleDirection direction);
    void setValue(float value);
    void start();
    void stop() const;
    void update() const;
    bool isDone() const;

private:
    float calculateScale() const;
    bool isAnimationLoopDone() const;
};


#endif //C_AUNISOMA_PANELREVERBERATION_H
