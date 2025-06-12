//
// Created by Dave Clay on 6/3/25.
//

#include "ColorManager.h"

#include "Arduino.h"
#include "Config.h"
#include "Gradient.h"

ColorManager::ColorManager(GradientValueMap* gradients, int number_of_gradients, GradientValueMap* rainbow_gradient, Config* config) {
  this->config = config;
  this->gradients = gradients;
  this->number_of_gradients = number_of_gradients;
  this->rainbow_gradient = rainbow_gradient;
  // Debounces - don't flicker number of interactions, they have to stay high for a
  // few cycles. Note the values are in ms, not ticks.
  // TODO: move debounce time to config
  // going low should take longer
  this->low_interaction_debounce = new Debounce(10000);
  this->med_interaction_debounce = new Debounce(200);
  this->high_interaction_debounce = new Debounce(200);
  // clocks (need to be updated in update())
  this->default_gradient_delay_timer = new Timer(
    this->config->default_gradient_delay_duration_range->random_int_between()
  );
  this->transition_delay_timer = new Timer(
    this->config->delay_for_gradient_transition_duration
  );
  this->transition_interpolation = new Interpolation(config->gradient_transition_animation_duration);
  this->distributed_panel_animation = new DistributedPanelAnimation(config);
  // gradients
  this->current_gradient_index = 0;
  this->next_gradient_index = 0;
  this->current_gradient = &this->gradients[this->current_gradient_index];
  this->state = LOW_INTERACTION_STATE;
}

/**
 * Note: Think of two states: one is the global _current_ interactivity state,
 * the other is the state of this ColorManager. While the current interactivity
 * state fluctuates from tick to tick, the ColorManager interpolates colors
 * for longer-running interactivity states.
 */
void ColorManager::update(float current_interaction_percent) {
  this->_update_interaction_reading(current_interaction_percent);
  this->_update_clocks();
  this->_update_state();
}

void ColorManager::_update_interaction_reading(float current_interaction_percent) const {
  bool currently_low = current_interaction_percent < this->config->intermediate_interaction_threshold_percent;
  this->low_interaction_debounce->update(currently_low);

  bool currently_mid = current_interaction_percent > this->config->intermediate_interaction_threshold_percent;
  this->med_interaction_debounce->update(currently_mid);

  bool currently_high = current_interaction_percent > this->config->high_interaction_threshold_percent;
  this->high_interaction_debounce->update(currently_high);
}

void ColorManager::_switch_to_new_current_gradient() {
  this->current_gradient_index = this->next_gradient_index;
  this->current_gradient = &this->gradients[this->current_gradient_index];
}

Color ColorManager::get_color(int panel_index, float panel_value) const {
  Color from_color;
  Color to_color;
  switch (this->state) {
    case START_DEFAULT_GRADIENT_DELAY_STATE:
      // simply stays on the current gradient while the delay timer runs
    case TRANSITIONING_FROM_MID_TO_LOW_STATE:
      // Note this isn't really a transition, it's just leaving the current gradient as is,
      // clearing the transition interpolation
    case GRADIENT_SWAP_DELAY_STATE:
      // This state just sits on the current_gradient until we swap to the next
      // or transition away
    case LOW_INTERACTION_STATE:
      return this->current_gradient->getColorForValue(panel_value);
    case TRANSITIONING_GRADIENT_STATE:
      from_color = this->current_gradient->getColorForValue(panel_value);
      to_color = this->gradients[this->next_gradient_index].getColorForValue(panel_value);
      return this->_get_transition_color(from_color, to_color);
    case TRANSITIONING_FROM_MID_TO_HIGH_STATE:
      from_color = this->current_gradient->getColorForValue(panel_value);
      to_color = this->_get_rainbow_color_for_panel_index(panel_index);
      return this->_get_transition_color(from_color, to_color);
    case HIGH_INTERACTION_STATE:
      return this->_get_rainbow_color_for_panel_index(panel_index);
    case TRANSITIONING_FROM_HIGH_TO_MID_STATE:
      from_color = this->_get_rainbow_color_for_panel_index(panel_index);
      to_color = this->current_gradient->getColorForValue(panel_value);
      return this->_get_transition_color(from_color, to_color);
    case TRANSITIONING_TO_DEFAULT_GRADIENT_STATE:
      from_color = this->current_gradient->getColorForValue(panel_value);
      to_color = this->gradients[0].getColorForValue(panel_value);
      return this->_get_transition_color(from_color, to_color);
    default:
      return Color(1, 1, 1);
  }
}

Color ColorManager::_get_transition_color(Color from_color, Color to_color) const {
  float transition_value = this->transition_interpolation->get_value();
  return from_color.interpolate(to_color, transition_value);
}

Color ColorManager::_get_rainbow_color_for_panel_index(int panel_index) const {
  float rainbow_value = this->distributed_panel_animation->get_value_for_panel(panel_index);
  return this->rainbow_gradient->getColorForValue(rainbow_value);
}

void ColorManager::_update_clocks() const {
  if (this->default_gradient_delay_timer->is_running()) {
    this->default_gradient_delay_timer->update();
  }
  if (this->transition_interpolation->is_running()) {
    this->transition_interpolation->update();
  }
  if (this->transition_delay_timer->is_running()) {
    this->transition_delay_timer->update();
  }
  if (this->distributed_panel_animation->active) {
    this->distributed_panel_animation->update();
  }
}

void ColorManager::_update_state() {
  switch (this->state) {
    case LOW_INTERACTION_STATE:
      if (this->current_gradient_index != 0) {
        this->_set_state(START_DEFAULT_GRADIENT_DELAY_STATE);
      } else if (this->_is_med_interaction()) {
        this->_set_state(TRANSITIONING_GRADIENT_STATE);
      } else if (this->_is_high_interaction()) {
        this->_set_state(TRANSITIONING_FROM_MID_TO_HIGH_STATE);
      }
      break;
    case START_DEFAULT_GRADIENT_DELAY_STATE:
      if (this->_is_med_interaction()) {
        this->_set_state(TRANSITIONING_GRADIENT_STATE);
      } else if (this->_is_high_interaction()) {
        this->_set_state(TRANSITIONING_FROM_MID_TO_HIGH_STATE);
      } else if (this->default_gradient_delay_timer->is_done()) {
        this->_set_state(TRANSITIONING_TO_DEFAULT_GRADIENT_STATE);
      }
      break;
    case TRANSITIONING_TO_DEFAULT_GRADIENT_STATE:
      if (this->_is_transition_done()) {
        this->_switch_to_new_current_gradient();
        this->_set_state(LOW_INTERACTION_STATE);
      }
      break;
    case TRANSITIONING_GRADIENT_STATE:
      if (this->_is_transition_done()) {
        this->_switch_to_new_current_gradient();
        if (this->_is_high_interaction()) {
          this->_set_state(TRANSITIONING_FROM_MID_TO_HIGH_STATE);
        } else {
          // Note: this is how long it's going to stay on a single gradient while
          // the med interaction is maintained, meaning people have to be moving about.
          this->_set_state(GRADIENT_SWAP_DELAY_STATE);
        }
      }
      break;
    case GRADIENT_SWAP_DELAY_STATE:
      if (this->_is_low_interaction()) {
        this->_set_state(TRANSITIONING_FROM_MID_TO_LOW_STATE);
      } else if (this->_is_gradient_swap_delay_done()) {
        this->_set_state(TRANSITIONING_GRADIENT_STATE);
      }
      break;
    case TRANSITIONING_FROM_MID_TO_LOW_STATE:
      if (this->_is_transition_done()) {
        this->_set_state(LOW_INTERACTION_STATE);
      }
      break;
    case TRANSITIONING_FROM_MID_TO_HIGH_STATE:
      if (this->_is_transition_done()) {
        this->_set_state(HIGH_INTERACTION_STATE);
      }
    case HIGH_INTERACTION_STATE:
      if (!this->_is_high_interaction()) {
        this->_set_state(TRANSITIONING_FROM_HIGH_TO_MID_STATE);
      }
      break;
    case TRANSITIONING_FROM_HIGH_TO_MID_STATE:
      if (this->_is_transition_done()) {
        if (this->_is_high_interaction()) {
          this->_set_state(TRANSITIONING_FROM_MID_TO_HIGH_STATE);
        } else if (this->_is_med_interaction()) {
          this->_set_state(TRANSITIONING_GRADIENT_STATE);
        } else {
          this->_set_state(TRANSITIONING_FROM_MID_TO_LOW_STATE);
        }
      }
      break;
  }
}

void ColorManager::_set_state(ColorManagerState state) {
  this->state = state;
  switch(this->state) {
    case START_DEFAULT_GRADIENT_DELAY_STATE:
      this->default_gradient_delay_timer->restart(this->config->default_gradient_delay_duration_range->random_int_between());
      break;
    case TRANSITIONING_TO_DEFAULT_GRADIENT_STATE:
      this->_start_swap_to_default_gradient_transition();
      break;
    case TRANSITIONING_GRADIENT_STATE:
      this->_start_gradient_swap_transition();
      break;
    case GRADIENT_SWAP_DELAY_STATE:
      this->transition_delay_timer->restart(config->delay_for_gradient_transition_duration);
      break;
    case TRANSITIONING_FROM_MID_TO_LOW_STATE:
      this->_start_transition_to_low_interactivity();
      break;
    case TRANSITIONING_FROM_MID_TO_HIGH_STATE:
      this->_start_transition_to_high_interactivity();
      break;
    case TRANSITIONING_FROM_HIGH_TO_MID_STATE:
      this->_start_transition_from_high_to_mid_interactivity();
      break;
    case LOW_INTERACTION_STATE:
    case HIGH_INTERACTION_STATE:
      break;
  }
}

/**
 * Start the transition interpolation timer to swap gradients
 */
void ColorManager::_start_gradient_swap_transition() {
  this->transition_interpolation->start();
  do {
    this->next_gradient_index = random(1, this->number_of_gradients);
  } while (this->next_gradient_index == this->current_gradient_index);
}

void ColorManager::_start_swap_to_default_gradient_transition() {
  this->transition_interpolation->start();
  this->next_gradient_index = 0;
}

void ColorManager::_start_transition_to_low_interactivity() const {
  this->transition_interpolation->start();
}

void ColorManager::_start_transition_to_high_interactivity() const {
  this->transition_interpolation->start();
  this->distributed_panel_animation->start();
}

void ColorManager::_start_transition_from_high_to_mid_interactivity() const {
  this->transition_interpolation->start();
}

bool ColorManager::_is_gradient_swap_delay_done() const {
  return this->transition_delay_timer->is_done();
}

bool ColorManager::_is_transition_done() const {
  return this->transition_interpolation->is_done();
}

bool ColorManager::_is_low_interaction() const {
return this->low_interaction_debounce->reading;
}

bool ColorManager::_is_med_interaction() const {
  return this->med_interaction_debounce->reading;
}

bool ColorManager::_is_high_interaction() const {
  return this->high_interaction_debounce->reading;
}
