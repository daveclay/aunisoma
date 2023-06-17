//
// Created by David Clay on 6/15/23.
//

#include "PanelReverberation.h"

PanelReverberation::PanelReverberation(Panel* panel,
                                       Panel* sourcePanel,
                                       int distanceFromTrigger,
                                       ReverberationDistanceSource* reverberationDistanceSource,
                                       Config* config) {
    this->panel = panel;
    this->sourcePanel = sourcePanel;
    this->config = config;
    this->reverberationDistanceSource = reverberationDistanceSource;
    this->isSourceInteraction = panel->index == this->sourcePanel->index;
    this->scale = 0;
    bool oneShot = !this->isSourceInteraction;
    this->cycle = new Cycle(config->initial_trigger_panel_animation_loop_duration_ticks,
                            oneShot,
                            CYCLE_TYPE_UP_AND_DOWN,
                            this);
    this->currentValue = 0;
    this->distanceFromTrigger = distanceFromTrigger;
}

void PanelReverberation::value(float value, CycleDirection direction) {
    this->setValue(value);
    if (direction == CYCLE_DIRECTION_UP) {
        if (!this->sourcePanel->active && !this->cycle->isAtZeroPoint()) {
            this->cycle->jumpToDownCycle();
        }
    }
}

void PanelReverberation::setValue(float value) {
    this->currentValue = value * this->scale;
}

void PanelReverberation::start() {
    this->scale = this->calculateScale();
    this->cycle->restart();
}

void PanelReverberation::stop() const {
    this->cycle->stop();
}

void PanelReverberation::update() const {
    this->cycle->next();
    if (this->isSourceInteraction && this->panel->active && this->isAnimationLoopDone()) {
        int newDurationTicks = this->config->getTriggerPanelAnimationLoopDurationTicks();
        this->cycle->restart(newDurationTicks);
    }
}

bool PanelReverberation::isDone() const {
    if (this->isSourceInteraction && this->panel->active) {
        return false;
    }

    if (!this->cycle->clock->running) {
        return true;
    }

    return this->isAnimationLoopDone();
}

float PanelReverberation::calculateScale() const {
    if (this->isSourceInteraction) {
        return 1;
    } else {
        int maxDistance = this->reverberationDistanceSource->getReverberationDistance();
        return (float)(maxDistance - this->distanceFromTrigger) / (float)maxDistance;
    }
}

bool PanelReverberation::isAnimationLoopDone() const {
    return this->cycle->isDone() || (this->cycle->iterations > 0 && this->cycle->isAtZeroPoint());
}
