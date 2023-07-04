//
// Created by David Clay on 6/15/23.
//

#include "Interaction.h"
#include "Maths.h"
#include "Arduino.h"

Interaction::Interaction(Panel* sourcePanel, Config* config, PanelContext* panelContext) {
    this->sourcePanel = sourcePanel;
    this->config = config;
    this->panel_context = panelContext;
    this->clock = new Clock();
    this->source_panel_reverberation = new PanelReverberation(
            this->sourcePanel,
            this->sourcePanel,
            0,
            this,
            this->config);

    for (int i = 0; i < 20; i++) {
      this->panelReverberationsByPanelIndex[i] = NULL;
    }

    this->_build_panel_reverberations();
    this->panel_reverberations_still_active = false;
}

int Interaction::getReverberationDistance() {
    return this->currentReverberatingDistance;
}

int Interaction::getDistanceFromTrigger(Panel* panel) {
    return abs(this->sourcePanel->index - panel->index);
}

void Interaction::start() {
    if (this->clock->running) {
        return;
    }

    this->clock->start();
    this->_trigger_new_reverberation(true);
}

void Interaction::_trigger_new_reverberation(bool trigger_source_panel) {
    this->currentReverberatingDistance = this->config->get_reverberation_distance();
    this->numberOfEligiblePanelReverberations = 0;
    // Calculate which PanelReverberations to start. Because this is referencing them _by panel index_, the for
    // loop doesn't start at 0, but rather where we want the new reverberation to start, and goes till
    // source panel is 6, current distance is 2, start index = 6 - 2 = 4.
    int start = max(0,
                         this->sourcePanel->index - this->currentReverberatingDistance);
    // source panel is 6, current distance is 2, start index = 6 + 2 = 8.
    int end = min(this->config->number_of_panels - 1,
                       this->sourcePanel->index + this->currentReverberatingDistance); // don't go beyond the end
    // So panels reverberating would be 4, 5, 6, 7, 8. 2 panels on each side of Panel 6.
    // Picking eligible PanelReverberations from _all_ the PanelReverberations.
    for (int i = start; i < end; i++) {
        PanelReverberation* panelReverberation = this->panelReverberationsByPanelIndex[i];
        // TODO: we already calculated the distance based on the start and end indexes
        //if (this->currentReverberatingDistance >= panelReverberation->distanceFromTrigger) {
            this->eligible_panel_reverberations[this->numberOfEligiblePanelReverberations] = panelReverberation;
            this->numberOfEligiblePanelReverberations++;
            if (trigger_source_panel || !panelReverberation->isSourceInteraction) {
                panelReverberation->start();
            }
        //}
    }
}

void Interaction::stop() {
    if (!this->clock->running) {
        return;
    }
    this->clock->stop();
}

void Interaction::update() {
    if (!this->clock->running) {
        return;
    }
    this->clock->next();
    PanelReverberation* last_panel_reverberation_alive = nullptr;
    int numberOfAlivePanelReverberations = 0;
    // This is looping through _eligible_ panels, not _all_ panels.
    for (int i = 0; i < this->numberOfEligiblePanelReverberations; i++) {
        PanelReverberation* panelReverberation = this->eligible_panel_reverberations[i];
        if (!panelReverberation->isDone()) {
            this->_start_panel_reverberation(panelReverberation);
            if (panelReverberation->isDone()) {
                // maybe it's down now that it's been updated? TODO: is this necessary or will the next update() catch it?
                panelReverberation->stop();
            } else {
                numberOfAlivePanelReverberations++;
                last_panel_reverberation_alive = panelReverberation;
            }
        }
    }

    this->panel_reverberations_still_active = numberOfAlivePanelReverberations > 0;
    if (numberOfAlivePanelReverberations == 1 && last_panel_reverberation_alive->currentValue == 0 && this->sourcePanel->active && maybe(65)) {
        this->clock->restart();
        this->_trigger_new_reverberation(false);
    }
}

void Interaction::_start_panel_reverberation(PanelReverberation* panel_reverberation) const {
    int reverberation_panel_delay_ticks = this->config->reverberation_panel_delay_ticks;
    int delay_for_panel_to_start_ticks = panel_reverberation->distanceFromTrigger * reverberation_panel_delay_ticks;
    if (this->clock->ticks >= delay_for_panel_to_start_ticks) {
        if (!panel_reverberation->cycle->clock->running) {
            // TODO: sometimes the PanelReverberation has not had start() called
            panel_reverberation->start();
        }
        panel_reverberation->update();
    }
}

bool Interaction::isDead() {
    if (this->sourcePanel->active) {
        return false;
    }

    return !this->panel_reverberations_still_active;
}

float Interaction::get_value_for_panel(Panel* panel) {
    PanelReverberation* panelReverberation = this->panelReverberationsByPanelIndex[panel->index];
    if (panelReverberation == NULL) {
        return 0;
    } else {
        return panelReverberation->currentValue;
    }
}

void Interaction::_build_panel_reverberations() {
    // These track the indexes when adding left and right Panels
    this->panel_reverberations_start_index = 100; // arbitrary max value
    this->panel_reverberations_end_index = 0;

    // Add the source Panel
    this->addPanelReverberation(this->source_panel_reverberation);

    // iterate through _all_ potential possible Panels that may reverberate. This is potential more than will be
    // assigned to this Interaction, as the source Panel may be on the end and have none to the left (or right).
    int from_index = this->sourcePanel->index;
    for (int i = 0; i < this->config->max_reverberation_distance; i++) {
        int distance_from_index = i + 1;
        int left_panel_index = from_index - distance_from_index;
        int right_panel_index = from_index + distance_from_index;

        if (left_panel_index >= 0) {
            Panel* panel = this->panel_context->get_panel_at(left_panel_index);
            PanelReverberation *panelReverberation = this->createPanelReverberation(
                    panel,
                    distance_from_index);
            this->addPanelReverberation(panelReverberation);
        }

        if (right_panel_index <= this->config->number_of_panels - 1) {
            Panel* panel = this->panel_context->get_panel_at(right_panel_index);
            PanelReverberation *panelReverberation = this->createPanelReverberation(
                    panel,
                    distance_from_index);
            this->addPanelReverberation(panelReverberation);
        }
    }
}

void Interaction::addPanelReverberation(PanelReverberation* panelReverberation) {
    int panelIndex = panelReverberation->panel->index;
    // NOTE! The array isn't 0-based, but indexed by the Panel index (this makes lookups by Panel easier).
    this->panelReverberationsByPanelIndex[panelIndex] = panelReverberation;

    // This tracks which index to _start_ looking up PanelReverberations and where to _stop_, since it is not
    // 0 based.
    if (panelIndex > this->panel_reverberations_end_index) {
        this->panel_reverberations_end_index = panelIndex;
    }
    if (panelIndex < this->panel_reverberations_start_index) {
        this->panel_reverberations_start_index = panelIndex;
    }
}

PanelReverberation* Interaction::createPanelReverberation(Panel* panel, int distanceFromTrigger) {
    return new PanelReverberation(
            panel,
            this->sourcePanel,
            distanceFromTrigger,
            this,
            this->config);

}