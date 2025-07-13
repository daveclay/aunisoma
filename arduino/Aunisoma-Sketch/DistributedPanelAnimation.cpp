//
// Created by Dave Clay on 6/9/25.
//

#include "DistributedPanelAnimation.h"

DistributedPanelAnimation::DistributedPanelAnimation(Config* config) {
    this->active = false;
    // This Cycle is for how fast each panel iterates over the gradient
    // TODO: duration_ticks to Config
    this->cycle = new Cycle(30, false, UP_ONLY_CYCLE);
    this->panel_distribution_ratio = (float) 1 / (float) config->number_of_panels;
}

void DistributedPanelAnimation::start() {
    this->active = true;
    this->cycle->restart();
}

void DistributedPanelAnimation::update() const {
    this->cycle->update();
}

void DistributedPanelAnimation::stop() {
    this->cycle->stop();
    this->active = false;
}

float DistributedPanelAnimation::get_value_for_panel(int panel_index) const {
    float value = this->cycle->current_value + ((float) panel_index * this->panel_distribution_ratio);
    if (value > 1) {
        return value - 1;
    }

    return value;
}
