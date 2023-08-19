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
    this->hasTransitionedIn = false;
    this->hasTransitionedOut = true;
};

void MaxInteractionAnimation::value(float value, CycleDirection direction) {
    this->current_value = value;
}

void MaxInteractionAnimation::start() {
        this->transitionTicks = -1;
        this->hasTransitionedOut = true;
        this->hasTransitionedIn = false;
        for (int i = 0; i < this->numberOfPanels; i++) {
            // TODO: while this keeps the current color at the _start_ of max animation, it may not be the color we use _after_ this max animation is done.
            // TODO: ideally, we just defer to what the algorithm is spitting out rather than a static color.
            this->panelColorByPanelIndex[i] = this->panels[i]->color;
        }
        this->cycle->restart();
        this->active = true;
}

void MaxInteractionAnimation::update() {
    this->cycle->next();
    if (!this->hasTransitionedIn) {
        this->transitionTicks += 1;
        if (this->transitionTicks > this->transitionDuration) {
            this->hasTransitionedIn = true;
            this->transitionTicks = -1;
        }
    }
}

float MaxInteractionAnimation::get_transition_amount() {
    return (float) this->transitionTicks / (float) this->transitionDuration;
}

void MaxInteractionAnimation::startTransitionOut() {
    this->active = false;
    // note that we need to transition in next time
    this->hasTransitionedIn = false;
    // indicate that the max interaction should transition out
    this->hasTransitionedOut = false;
    this->transitionTicks = 0;
}

void MaxInteractionAnimation::updateTransitionOut() {
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

        if (this->hasTransitionedIn) {
            this->panelColorByPanelIndex[panel->index] = color;
        }
        return color;
}

Color MaxInteractionAnimation::getColorForPanelAndValue(Panel* panel, float value) {
    Color maxColor = this->gradient->getColorForValue(value);
    if (!this->hasTransitionedIn) {
        Color fromColor = this->panelColorByPanelIndex[panel->index]; // TODO: this was the color before we started; not necessarily the color we're going to?
        float transitionAmount = this->get_transition_amount();
        return fromColor.interpolate(maxColor, transitionAmount);
    } else {
        return maxColor;
    }
}
