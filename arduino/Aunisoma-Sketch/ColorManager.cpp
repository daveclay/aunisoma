//
// Created by Dave Clay on 6/3/25.
//

#include "ColorManager.h"

#include "Config.h"
#include "Gradient.h"
#define ONE_SHOT true

ColorManager::ColorManager(GradientValueMap* gradients, int number_of_gradients, Config* config) {
    this->gradients = gradients;
    this->number_of_gradients = number_of_gradients;
    this->config = config;

    this->intermediate_interaction_debounce = new Debounce(10);
    this->intermediate_transition_delay_timer = new Cycle(
      this->config->delay_for_gradient_transition_duration, ONE_SHOT, UP_ONLY_CYCLE);
    this->gradient_transition_animation = new Interpolation(config->gradient_transition_animation_duration);
    this->current_gradient_index = 0;
    this->next_gradient_index = 0;
    this->current_gradient = &this->gradients[this->current_gradient_index];
    this->max_gradient_index = this->number_of_gradients - 1;
    this->transition_active = false;
}

void ColorManager::update(float current_interaction_percent) {
  this->current_interaction_percent = current_interaction_percent;
  this->_update_intermediate_transition_state();

  if (this->gradient_transition_animation->active) {
    // if the transition is active, update it's state/clock.
    this->gradient_transition_animation->update();

    // Once gradient_transition_animation->update() is called, it may result
    // in the gradient animation timer being "done". If we don't catch it here,
    // the resulting transition value will be 0 and this will return 100% of
    // the current_gradient, and 0% of the next_gradient, even though last
    // update() it was at 99%. Checking here ensures we catch that state.
    if (this->gradient_transition_animation->is_done()) {
      // If the transition animation is done, swap the gradients to the next
      // gradient and reset the transition->
      this->gradient_transition_animation->reset();
      // TODO: make this a random time - and make it _minutes_
      this->intermediate_transition_delay_timer->restart(config->delay_for_gradient_transition_duration);
      this->current_gradient_index = this->next_gradient_index;
      this->current_gradient = &this->gradients[this->current_gradient_index];
      this->transition_active = false;
      // TODO: ok, so the new problem is that this will constantly cycle through gradients because
      // there is no state where we say "don't run the timer and don't transition->"
      // Note: which is probably fine because we always want to shift through
      // gradients, just on a long, random timescale-> We don't have to, but that's
      // the best idea I've had so far->
    }
  }
}

Color ColorManager::get_color(float value) {
  if (this->transition_active) {
    return this->gradient_transition_animation->get_color(value);
  } else {
    return this->current_gradient->getColorForValue(value);
  }
}

void ColorManager::_update_intermediate_transition_state() {
  if (this->_is_intermediate_interaction()) {
    // If we've reached the intermediate interaction level
    if (this->intermediate_transition_delay_timer->isRunning()) {
      // if the delay cycle is running, update it's state
      this->intermediate_transition_delay_timer->update();
      if (this->intermediate_transition_delay_timer->isDone() && !this->transition_active) {
        // if the timer is done, and we're not transitioning, it is time to transition!
        this->_start_gradient_transition();
      }
    } else {
      // Otherwise we haven't started the cycle to trigger
      // intermediate transition, so start that cycle->
      this->intermediate_transition_delay_timer->start();
    }
  } else {
    // if the intermediate interactivity isn't reached, stop the timer
    this->intermediate_transition_delay_timer->stop();
  }
}

/**
 * Start the intermediate gradient transition
 */
void ColorManager::_start_gradient_transition() {
  this->transition_active = true;
  // Note: The intermediate_transition_delay_timer shoudl have stopped if isDone is true->
  // I think->
  this->next_gradient_index = this->current_gradient_index + 1;
  if (this->next_gradient_index == this->number_of_gradients) {
    // hit the max, loop around->
    this->next_gradient_index = 0;
  }

  GradientValueMap* next_gradient = &this->gradients[this->next_gradient_index];
  this->gradient_transition_animation->start(
    this->current_gradient,
    next_gradient
  );
}

bool ColorManager::_is_intermediate_interaction() {
  bool currently_high = this->current_interaction_percent > this->config->intermediate_interaction_threshold_percent;
  return this->intermediate_interaction_debounce->update(currently_high);
}

bool ColorManager::_is_max_interaction() {
  return this->current_interaction_percent > this->config->intermediate_interaction_threshold_percent;
}
