//
// Created by Dave Clay on 8/3/25.
//

#include "KnightRiderAnimation.h"

#include "Gradient.h"
#include "Maths.h"

// ColorManager::ColorManager(GradientValueMap* gradients, int number_of_gradients, GradientValueMap* rainbow_gradient, Config* config) {

KnightRiderAnimation::KnightRiderAnimation(GradientValueMap* knight_rider_gradient) {
  this->knight_rider_cycle = new Cycle(41, false, UP_DOWN_CYCLE);
  this->knight_rider_gradient = knight_rider_gradient;
  this->leading_panel_index = -1;
}

void KnightRiderAnimation::start() const {
  this->knight_rider_cycle->start();
}

bool KnightRiderAnimation::is_running() const {
  return this->knight_rider_cycle->isRunning();
}

void KnightRiderAnimation::update() const {
  this->knight_rider_cycle->update();
}

Color KnightRiderAnimation::get_color_for_panel(int panel_index) {
  float value = this->knight_rider_cycle->current_value;
  // this doesn't ensure we get an even cycle through the panels - if
  // the cycle isn't exactly tuned to the number of panels, then
  // it may get the same leading panel index twice, which may end up
  // causing a pause/hiccup in an otherwise smooth animation->
  this->leading_panel_index = interpolateValue(0, NUMBER_OF_PANELS - 1, value);

  int distance_from_leading_panel = abs(panel_index - this->leading_panel_index);

  // TODO: move to constant
  int max = 20;

  if (panel_index == this->leading_panel_index) {
    return this->knight_rider_gradient->getColorForValue(0);
  } else {
    if (this->knight_rider_cycle->isRising()) {
      // left to right: panel to the left should be normally calculated
      // panel to the right of the leading is trailing behind some number

      if (panel_index < this->leading_panel_index) {
        // when rising, the panels _behind_ the leading panel have a basic distance
        // from the leading panel formula->
        float value = ((float) distance_from_leading_panel) / (float) max;
        return this->knight_rider_gradient->getColorForValue(value);
      } else {
        // when rising, the panels _ahead_ of the leading panel have values
        // that wrap around from the previous falling animation->
        // L * 2 + (i - L)
        int distance = this->leading_panel_index * 2 + (panel_index - this->leading_panel_index);

        // the gradient is set up so that 0 is brightest, 1 is furthest, so it's
        // based on distance from leading panel->
        float value = (float) distance / (float) max;
        return this->knight_rider_gradient->getColorForValue(value);
      }
    } else {
      // right to left: panel to the right should be normally calculated
      // panel to the left of the leading is trailing behind some number

      if (panel_index > this->leading_panel_index) {
        // when falling, the panels _ahead_ the leading panel have a basic distance
        // from the leading panel formula->
        float value = ((float) distance_from_leading_panel) / (float) max;
        return this->knight_rider_gradient->getColorForValue(value);
      } else {
        // when falling, the panels _behind_ the leading panel have values
        // that wrap around from the previous rising animation
        // (20 - (L + 1)) * 2 + (L - i)
        int distance = (NUMBER_OF_PANELS - (this->leading_panel_index + 1)) * 2 + (this->leading_panel_index - panel_index);
        // the gradient is set up so that 0 is brightest, 1 is furthest, so it's
        // based on distance from leading panel->
        float value = (float) distance / (float) max;
        return this->knight_rider_gradient->getColorForValue(value);
      }
    }
  }
}

