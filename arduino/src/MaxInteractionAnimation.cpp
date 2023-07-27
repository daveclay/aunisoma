//
// Created by Dave Clay on 7/23/23.
//
#include "Arduino.h"
#include "MaxInteractionAnimation.h"
#include "Panel.h"

MaxInteractionAnimation::MaxInteractionAnimation(int numberOfPanels,
                                                 Panel** panels,
                                                 GradientValueMap *gradient) {
    this->numberOfPanels = numberOfPanels;
    this->cycle = new Cycle(100,
                           false,
                           CYCLE_TYPE_UP,
                           this);
    this->gradient = gradient;
    this->panels = panels;
    this->current_value = 0;
    this->active = 0;
    this->transitionTicks = 0;
    this->transitionDuration = 100;
    this->hasTransitionedOut = true;
};

void MaxInteractionAnimation::value(float value, CycleDirection direction) {
    this->current_value = value;
}

void MaxInteractionAnimation::start() {
        this->transitionTicks = -1;
        this->hasTransitionedOut = true;
        for (int i = 0; i < this->numberOfPanels; i++) {
            this->panelColorByPanelIndex[i] = this->panels[i]->color;
        }
        this->cycle->restart();
        this->active = true;
}

void MaxInteractionAnimation::update() {
    this->cycle->next();
    this->transitionTicks += 1;
}

int MaxInteractionAnimation::get_transition_amount() {
    return this->transitionTicks / this->transitionDuration;
}

void MaxInteractionAnimation::updateTransition() {
    this->transitionTicks += 1;
    this->hasTransitionedOut = this->transitionTicks >= this->transitionDuration;
}

Color MaxInteractionAnimation::get_color(Panel* panel) {
        float distanceRatio = ((float) panel->index / (float) this->numberOfPanels);
        // let value = Math.abs(this.current_value - distanceRatio);
        float value = this->current_value + distanceRatio;
        if (value > 1.0) {
            value = value - 1.0;
        }

        Color color = this->getColorForPanelAndValue(panel, value);

        this->panelColorByPanelIndex[panel->index] = color;
        return color;
}

Color MaxInteractionAnimation::getColorForPanelAndValue(Panel* panel, float value) {
    Color maxColor = this->gradient->getColorForValue(value);
    if (this->transitionTicks < this->transitionDuration) {
        Color fromColor = this->panelColorByPanelIndex[panel->index];
        return fromColor.interpolate(maxColor, this->get_transition_amount());
    } else {
        return maxColor;
    }
}
