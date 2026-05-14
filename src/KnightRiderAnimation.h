//
// Created by Dave Clay on 8/3/25.
//

#ifndef KNIGHTRIDER_H
#define KNIGHTRIDER_H
#include "Color.h"
#include "Cycle.h"
#include "Gradient.h"

/******************************************************************
 * Knight Rider
 * Stop riding my ass, Michael
 *****************************************************************/
class KnightRiderAnimation {
public:
  KnightRiderAnimation(GradientValueMap* knight_rider_gradient);

  void start() const;
  bool is_running() const;
  void update() const;
  Color get_color_for_panel(int panel_index);

private:
  GradientValueMap* knight_rider_gradient;
  Cycle* knight_rider_cycle;
  int leading_panel_index;
};

#endif //KNIGHTRIDER_H
