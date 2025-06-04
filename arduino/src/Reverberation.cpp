//
// Created by David Clay on 6/15/23.
//

#include "Arduino.h"
#include "Reverberation.h"

Reverberation::Reverberation(Sensor* sensor,
                             Config* config,
                             int panel_index) {
    this->sensor = sensor;
    this->config = config;
    this->panel_index = panel_index;
    this->first_panel_index = 0;
    this->last_panel_index = 0;
    this->active = false;
    this->delay_complete = false;
    this->max_distance = config->reverberation_distance_range->max;
    this->delay = config->reverberation_panel_delay_ticks;
    this->delay_clock = new Clock();

    this->_calculate_new_distance();

    // number _per side_.
    int durationTicks = config->getTriggerPanelAnimationLoopDurationTicks();
    for (int i = 0; i < this->max_distance; i++) {
        float amplitude = ((float)this->max_distance - (float)i) / (float)this->max_distance;
        // TODO: if the duration is the same, the lower amplitude colors shift slower. With the gamma map,
        //  that means those slow color changes are perceptible as "jank". Might have to fix the gamma to cycle
        //  through darker colors more quickly than the low-mids?
        //  I'd say the same cycle duration isn't as nice either - the further cycles through the colors slower,
        //  while the front panel cycles really quickly. It does pulse, but it's not as natural.
        // TODO: I think the delay should be a similar value or calc to the duration and amplitude to make a more natural flow.
        //  it looks janky when the far panel animates really quickly after a short delay.
        bool one_shot = i != 0; // one shot if this isn't index 0
        this->pulses[i] = new Pulse(amplitude, durationTicks, one_shot);
    }
}

void Reverberation::update() {
    // This method is tricky:
    // The order in which we call methods and read values on the associated
    // objects is important. Generally speaking, we have to manage the
    // active state of our clocks for this Reverberation. Then we have to
    // update the delay clock. Only then we can update the Pulses, and
    // only after we update the Pulses can we read their values and use
    // them.
    //
    // If the order is changed, or we change how states are updated, then
    // we end up with bugs where ticks don't reset, clocks are stopped,
    // and values aren't updated.

    // First, determine if this Reverberation should be started or
    // stopped based on the current sensor reading. This manages the
    // delay clock start/stop state, which is critical to ensure
    // the "distant" Pulses wait a certain number of ticks before
    // generating their Cycle value, creating the wave-like animation->
    if (this->active && !this->sensor->active) {
        // interaction was active, and now is not->
        this->stop();
    } else if (!this->active && this->sensor->active) {
        // interaction was not active, and now is
        this->start();
    }

    // Now, if the Reverberation is in an active state, and the
    // delay clock is running, update the delay clock's ticks->
    if (this->active && this->delay_clock->running) {
        this->delay_clock->update();
    }

    // Now that the delay clock is updated, we can use it to
    // update the Pulses->
    for (int i = 0; i < this->max_distance; i++) {
        Pulse* pulse = this->pulses[i];
        // calculate the delay for this Pulse
        int pulseDelay = this->delay * i;
        // check to see if the delay has comleted or the delay
        // clock has passed that number of ticks
        bool waited_long_enough = this->delay_complete || this->delay_clock->ticks >= pulseDelay;

        // and calculate whether the Pulse should be "active" or not->
        // This allows the Pulse to determine what it's own "active" state
        // means rather than have the Reverberation read and interpret
        // the Pulse state->
        bool pulse_active = this->active && i < this->distance && waited_long_enough;
        pulse->update(pulse_active);
    }

    // Only after we update the Pulses can we ask if the first
    // Pulse (corresponding to the origin interaction panel)
    // is at its zero point-> If so, recalculate new Pulse
    // distance and re-start the Pulses->
    if (this->active) {
        if (this->pulses[0]->isAtZeroPoint()) {
            this->_restart_pulses();
        }
    }

    // Lastly, check to see if all the Pulse delays have passed-> If
    // so, we can stop the delay clock and flag that the delay has
    // been completed, letting the Pulses run free->
    bool delay_over = this->_is_delay_complete();
    if (delay_over) {
        this->delay_complete = true;
        this->delay_clock->stop();
    }
}

void Reverberation::start() {
    // Start the Reverberation means putting it into a state where
    // the delay clock starts running on the next update() call
    // and Pulses wait for their delay time to start their Cycle.
    this->active = true;
    this->delay_clock->restart();
    this->delay_complete = false;
}

void Reverberation::stop() {
    this->active = false;
    this->delay_complete = false;
}

float Reverberation::get_panel_value_for_panel_index(int panel_index) {
    // Returns a value for the given panel index, based on
    // this Reverberation's Pulse values for their corresponding
    // Cycles.
    if (panel_index < this->first_panel_index || panel_index > this->last_panel_index) {
        return 0;
    }

    // given panel index of 0, and this panel index of 3, and distance of 2:
    // ? = 3 - 0 - outside of range!
    // 2 = 3 - 1
    // 1 = 3 - 2
    // 0 = 3 - 3
    // 1 = 3 - 4
    int pulse_index = abs(this->panel_index - panel_index);
    return this->pulses[pulse_index]->current_value;
}

void Reverberation::_restart_pulses() {
    // at this point the interaction just started or has looped.
    this->_calculate_new_distance();
    this->delay_clock->restart();
    this->delay_complete = false;
}

bool Reverberation::_is_delay_complete() {
    return this->delay_clock->ticks > this->delay * this->distance;
}

void Reverberation::_calculate_new_distance() {
    this->distance = this->config->get_reverberation_distance();
    this->first_panel_index = max(0, this->panel_index - this->distance + 1);
    this->last_panel_index = min(NUMBER_OF_PANELS - 1, this->panel_index + this->distance - 1);
}
