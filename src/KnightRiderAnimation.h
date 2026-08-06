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
  // sweep_duration_ms is one full left-right-left trip; the default keeps
  // the legacy tick-rescaled value the Aunisoma sketch was tuned around.
  KnightRiderAnimation(GradientValueMap* knight_rider_gradient, int sweep_duration_ms = 101);

  void start() const;
  // Begin a fresh sweep from the left end (start() resumes mid-phase).
  void restart() const;
  void stop() const;
  bool is_running() const;
  void update() const;
  Color get_color_for_panel(int panel_index);

private:
  GradientValueMap* knight_rider_gradient;
  Cycle* knight_rider_cycle;
  int leading_panel_index;
};

#endif //KNIGHTRIDER_H
