//
// Created by David Clay on 6/16/23.
//

#include "Aunisoma.h"
#include "Maths.h"

Aunisoma::Aunisoma(Config* config,
                   GradientValueMap* maxAnimationGradient,
                   GradientValueMap* gradients,
                   int numberOfGradients,
                   Sensor* sensors) {
    this->config = config;
    this->gradients = gradients;
    this->numberOfGradients = numberOfGradients;
    this->sensors = sensors;
    this->current_gradient_index = 0;
    this->next_gradient_index = 0;
    this->numberOfPanels = config->number_of_panels;
    this->currentGradient = &this->gradients[this->current_gradient_index];
    this->maxGradientIndex = this->numberOfGradients - 1;
    this->_calculate_next_gradient_index();
    this->_create_panels();
    this->_create_interactions();
    this->transitionAnimation = new TransitionAnimation(500, this);
    this->maxInteractionAnimation = new MaxInteractionAnimation(config->number_of_panels,
                                                                this->panels,
                                                                maxAnimationGradient);
    this->transitioned_during_current_intermediate_state = false;
    this->ticks_since_last_transition = 0;
}

Panel* Aunisoma::get_panel_at(int index) {
    return this->panels[index];
}

void Aunisoma::_calculate_next_gradient_index() {
    if (this->current_gradient_index == this->maxGradientIndex) {
        this->next_gradient_index = 1; // don't go back to the non-max gradient until max is reset?
    } else {
        // TODO: randomize this? transition colors look weird if they're random, but might be more interesting for engagement.
        this->next_gradient_index = this->current_gradient_index + 1;
    }
}


void Aunisoma::_create_panels() {
    for (int i = 0; i < this->numberOfPanels; i++) {
        Color idle_color = this->currentGradient->getColorForValue(0);
        Panel* panel = new Panel(i, idle_color);
        this->panels[i] = panel;
    }
}

void Aunisoma::_create_interactions() {
    for (int i = 0; i < this->numberOfPanels; i++) {
        Panel* panel = this->panels[i];
        Interaction* interaction = new Interaction(panel, this->config, this);
        this->interactions_by_source_panel_index[panel->index] = interaction;
    }
}

void Aunisoma::read_sensors() {
    for (int i = 0; i < this->numberOfPanels; i++) {
        Sensor* sensor = &this->sensors[i];
        bool active = sensor->active;
        this->_handle_panel_sensor(i, active);
    }
}

void Aunisoma::_handle_panel_sensor(int panel_index, bool active) {
    Panel* panel = this->panels[panel_index];
    if (active != panel->active) {
        panel->active = active;
        if (active) {
            Interaction* interaction = this->interactions_by_source_panel_index[panel->index];
            if (!interaction->clock->running) {
                interaction->start();
            }
        }
    }
}

void Aunisoma::event_loop() {
    this->read_sensors();
    int active_panel_count = 0;

    // Zero-out the interaction panel values
    for (int i = 0; i < this->numberOfPanels; i++) {
        // TODO: combine these two outer loops?
        this->interaction_panel_values[i] = 0;
    }

    for (int i = 0; i < this->numberOfPanels; i++) {
        Interaction* interaction = this->interactions_by_source_panel_index[i];
        interaction->update();

        // now that the internal state has been updated, determine whether it is dead and stop it if necessary.
        if (interaction->isDead()) {
            // TODO: Should Interaction call stop() itself after update() is called, and then self can just ask?
            interaction->stop();
        } else {
            for (int j = 0; j < this->numberOfPanels; j++) {
                Panel* panel = this->panels[j];
                interaction_panel_values[panel->index] += interaction->get_value_for_panel(panel);
            }
        }

        if (interaction->sourcePanel->active) {
            active_panel_count += 1;
        }
    }

    float activePercent = (float) active_panel_count / (float) this->numberOfPanels;
    bool is_at_max_interactions = activePercent >= this->config->max_interaction_threshold_percent;
    if (is_at_max_interactions) {
        if (!this->maxInteractionAnimation->active) {
            this->maxInteractionAnimation->start();
        }
    } else {
        bool is_at_intermediate_interactions = activePercent >= this->config->intermediate_interaction_threshold_percent;
        if (is_at_intermediate_interactions) {
            if (!this->transitionAnimation->cycle->clock->running) {
                if (this->transitioned_during_current_intermediate_state) {
                    this->ticks_since_last_transition += 1;
                    if (this->ticks_since_last_transition > config->min_max_interaction_gradient_transition_duration
                        && maybe(config->odds_for_max_interaction_gradient_transition)) {
                        this->_start_transition();
                        this->ticks_since_last_transition = 0;
                    }
                } else {
                    this->_start_transition();
                    this->transitioned_during_current_intermediate_state = true;
                }
            }
        } else {
            // reset state for max tracking
            this->transitioned_during_current_intermediate_state = false;
            this->ticks_since_last_transition = 0;
            if (this->current_gradient_index != 0 && !this->transitionAnimation->active) {
                // reset back to original gradient
                this->next_gradient_index = 0;
                this->_start_transition();
            }
        }

        if (this->maxInteractionAnimation->active) {
            this->maxInteractionAnimation->startTransitionOut();
        }
    }

    if (this->maxInteractionAnimation->active) {
        // Note: only when the max interaction is active does this call update().
        // That logic covers transitioning in and steady-state running of the max animation.
        // Transitioning _out_ has a separate handler below.
        this->maxInteractionAnimation->update();
    } else if (this->transitionAnimation->active) {
        this->transitionAnimation->update();
    }
    this->_update_panels();
    if (!this->maxInteractionAnimation->hasTransitionedOut) {
        // Note: the max interaction is no longer active, so this is here to transition out of the max animation.
        this->maxInteractionAnimation->updateTransitionOut();
    }
}

void Aunisoma::_start_transition() {
    GradientValueMap *gradient = &this->gradients[this->next_gradient_index];
    this->transitionAnimation->start(gradient);
}

GradientValueMap* Aunisoma::getCurrentGradient() {
    return this->currentGradient;
}

void Aunisoma::switch_to_next_gradient() {
    this->current_gradient_index = this->next_gradient_index;
    this->_calculate_next_gradient_index();
    this->currentGradient = &this->gradients[this->current_gradient_index];
    this->ticks_since_last_transition = 1;
}

void Aunisoma::_update_panels() {
    for (int i = 0; i < this->numberOfPanels; i++) {
        Panel* panel = this->panels[i];
        this->_update_panel(panel, this->interaction_panel_values[panel->index]);
    }
}

void Aunisoma::_update_panel(Panel* panel, float total_panel_value) {
    panel->currentValue = total_panel_value;

    if (this->maxInteractionAnimation->active) {
        // TODO: use the total_panel_value for _something_?
        panel->color = this->maxInteractionAnimation->get_color(panel);
    } else {
        Color color;
        if (this->transitionAnimation->active) {
            // Transitioning to new gradient
            color = this->transitionAnimation->get_color(total_panel_value);
        } else {
            // regular animation
            color = this->currentGradient->getColorForValue(total_panel_value);
        }

        if (!this->maxInteractionAnimation->hasTransitionedOut) {
            Color fromColor = this->maxInteractionAnimation->panelColorByPanelIndex[panel->index];
            float transitionAmount = this->maxInteractionAnimation->get_transition_amount();
            color = fromColor.interpolate(color, transitionAmount);
        }

        panel->color = color;
    }
}

