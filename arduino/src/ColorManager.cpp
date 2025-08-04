//
// Created by Dave Clay on 6/3/25.
//

#include "ColorManager.h"

#include "Arduino.h"
#include "Config.h"
#include "Gradient.h"
#include "KnightRiderAnimation.h"

ColorManager::ColorManager(GradientValueMap* gradients, int number_of_gradients, GradientValueMap* rainbow_gradient, GradientValueMap* knight_rider_gradient, Config* config) {
  this->config = config;
  this->gradients = gradients;
  this->number_of_gradients = number_of_gradients;
  this->rainbow_gradient = rainbow_gradient;
  // Debounces - don't flicker number of interactions, they have to stay high for a
  // few cycles. Note the values are in ms, not ticks.
  // TODO: move debounce time to config
  // going low should take longer
  this->knight_rider_interaction_debounce = new Debounce(300, false);
  this->no_interaction_debounce = new Debounce(300, true);
  this->low_interaction_debounce = new Debounce(100, false);
  this->med_interaction_debounce = new Debounce(200, false);
  // TODO: it was kinda hard to keep it in the high state, maybe it should be
  // biased to stay high longer?
  this->high_interaction_debounce = new Debounce(200, false);
  // delay for how long to wait before switching back to the default no-interactivity gradient
  this->default_gradient_delay_timer = new Timer(
    this->config->default_gradient_delay_duration_range->random_int_between()
  );
  this->transition_delay_timer = new Timer(
    this->config->delay_for_gradient_transition_duration
  );
  this->transition_interpolation = new Interpolation(config->gradient_transition_animation_duration);
  this->rainbow_panel_animation = new DistributedPanelAnimation(config);
  this->knight_rider_animation = new KnightRiderAnimation(knight_rider_gradient);
  // gradients
  this->current_gradient_index = 0;
  this->next_gradient_index = 0;
  this->current_gradient = &this->gradients[this->current_gradient_index];
  this->state = NO_INTERACTION_STATE;
}

/**
 * Note: Think of two states: one is the global _current_ interactivity state,
 * the other is the state of this ColorManager. While the current interactivity
 * state fluctuates from tick to tick, the ColorManager interpolates colors
 * for longer-running interactivity states.
 */
void ColorManager::update(float current_interaction_percent, bool is_pong_interactivity) {
  this->_update_interaction_reading(current_interaction_percent, is_pong_interactivity);
  this->_update_clocks();
  this->_update_state();
}

void ColorManager::_update_interaction_reading(float current_interaction_percent, bool is_pong_interactivity) const {
  this->knight_rider_interaction_debounce->update(is_pong_interactivity);

  bool currently_zero = current_interaction_percent < .01;
  bool currently_low = !currently_zero && current_interaction_percent < this->config->intermediate_interaction_threshold_percent;
  bool currently_high = current_interaction_percent > this->config->high_interaction_threshold_percent;
  bool currently_mid = !currently_high && current_interaction_percent > this->config->intermediate_interaction_threshold_percent;

  this->no_interaction_debounce->update(currently_zero);
  this->low_interaction_debounce->update(currently_low);
  this->med_interaction_debounce->update(currently_mid);
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
    case NO_INTERACTION_STATE:
    case DEFAULT_GRADIENT_DELAY_STATE:
      // simply stays on the current gradient while the delay timer runs
    case TRANSITIONING_FROM_MID_TO_LOW_STATE:
      // Note this isn't a color transition, it's a state transition using
      // the current gradient.
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
    case KNIGHT_RIDER_INTERACTION_STATE:
      return this->_get_knight_rider_color_for_panel_index(panel_index);
    default:
      return Color(1, 1, 1);
  }
}

Color ColorManager::_get_transition_color(Color from_color, Color to_color) const {
  float transition_value = this->transition_interpolation->get_value();
  return from_color.interpolate(to_color, transition_value);
}

Color ColorManager::_get_rainbow_color_for_panel_index(int panel_index) const {
  float rainbow_value = this->rainbow_panel_animation->get_value_for_panel(panel_index);
  return this->rainbow_gradient->getColorForValue(rainbow_value);
}

Color ColorManager::_get_knight_rider_color_for_panel_index(int panel_index) const {
  return this->knight_rider_animation->get_color_for_panel(panel_index);
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
  if (this->rainbow_panel_animation->active) {
    this->rainbow_panel_animation->update();
  }
  if (this->knight_rider_animation->is_running()) {
    this->knight_rider_animation->update();
  }
}

void ColorManager::_update_state() {
  switch (this->state) {
    case NO_INTERACTION_STATE:
      if (!this->_is_no_interaction()) {
        this->_set_state(LOW_INTERACTION_STATE);
      } else if (this->current_gradient_index != 0) {
        this->_set_state(DEFAULT_GRADIENT_DELAY_STATE);
      }
      break;
    case LOW_INTERACTION_STATE:
      if (this->_is_pong_interaction()) {
        this->_set_state(KNIGHT_RIDER_INTERACTION_STATE);
      } else if (this->_is_no_interaction()) {
        this->_set_state(NO_INTERACTION_STATE);
      } else if (this->_is_med_interaction() || this->_is_high_interaction()) {
        this->_set_state(TRANSITIONING_GRADIENT_STATE);
      }
      break;
    case DEFAULT_GRADIENT_DELAY_STATE:
      if (!this->_is_no_interaction()) {
        // for now, just jump to low, which will handle other interactivity states
        // Since this comes from a no interactivity state, if a big group rolls up,
        // this will cycle them through a bit before hitting pride mode, which I
        // this is a better aesthetic.
        this->_set_state(LOW_INTERACTION_STATE);
      } else if (this->default_gradient_delay_timer->is_done()) {
        this->_set_state(TRANSITIONING_TO_DEFAULT_GRADIENT_STATE);
      }
      break;
    case TRANSITIONING_TO_DEFAULT_GRADIENT_STATE:
      if (this->_is_transition_done()) {
        this->_switch_to_new_current_gradient();
        if (this->_is_no_interaction()) {
          this->_set_state(NO_INTERACTION_STATE);
        } else {
          this->_set_state(LOW_INTERACTION_STATE);
        }
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
      if (this->_is_no_interaction()) {
        this->_set_state(TRANSITIONING_FROM_MID_TO_LOW_STATE);
      } else if (this->_is_low_interaction()) {
        this->_set_state(TRANSITIONING_FROM_MID_TO_LOW_STATE);
      } else if (this->_is_high_interaction()) {
        this->_set_state(TRANSITIONING_FROM_MID_TO_HIGH_STATE);
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
    case KNIGHT_RIDER_INTERACTION_STATE:
      if (!this->_is_pong_interaction()) {
        this->_set_state(LOW_INTERACTION_STATE);
      }
      break;
  }
}

void ColorManager::_set_state(ColorManagerState state) {
  this->state = state;
  switch(this->state) {
    case DEFAULT_GRADIENT_DELAY_STATE:
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
    case KNIGHT_RIDER_INTERACTION_STATE:
      this->_start_knight_rider_state();
      break;
    case NO_INTERACTION_STATE:
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
    this->next_gradient_index = random(1, this->number_of_gradients - 1);
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
  this->rainbow_panel_animation->start();
}

void ColorManager::_start_knight_rider_state() const {
  this->knight_rider_animation->start();
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

bool ColorManager::_is_pong_interaction() const {
  return this->knight_rider_interaction_debounce->reading;
}

bool ColorManager::_is_no_interaction() const {
  return this->no_interaction_debounce->reading;
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
