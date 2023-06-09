from python.cycle import Cycle


class TransitionAnimation:
    def __init__(self,
                 transition_duration_ticks):
        self.cycle = Cycle(transition_duration_ticks,
                           lambda value: self.on_up_cycle(value),
                           None,
                           False)
        self.current_value = 0
        self.target_gradient = None
        self.panel_values_by_panel_index = {}
        self.active = False

    def on_up_cycle(self, value):
        self.current_value = value

    def start(self,
              target_gradient):
        self.target_gradient = target_gradient
        self.cycle.start()
        self.active = True

    def update(self):
        if not self.active:
            return

        self.cycle.next()

        # TODO: calculate gradient? or color? color, right? oh, this has to be _per panel dawg_. Each panel has to transition from _its point_ to the new gradient.
        # get current color from current gradient at panel's current_value
        # get transition color from target gradient at panel's current_value
        # transition_ratio = how far along the transition duration we are ((transition tick / duration time)
        # interpolate current color to transition color using transition_ratio
        # that's the color?

        if self.current_value >= 1: # all panels move at the same rate, so this is OK
            # Done!
            self.cycle.stop()
            return

    def set_color_for_panel(self, panel, panel_value):
        current_gradient = panel.gradient
        from_color = current_gradient.get_color_for_value(panel_value)
        to_color = self.target_gradient.get_color_for_value(panel_value)

        color = from_color.interpolate(to_color, self.current_value)

        panel.current_value = panel_value
        panel.set_color(color)

