from cycle import Cycle


class TransitionAnimation:
    def __init__(self,
                 transition_duration_ticks,
                 panel_context):
        self.panel_context = panel_context
        self.cycle = Cycle(transition_duration_ticks,
                           lambda value: self.on_up_cycle(value),
                           None,
                           False)
        self.current_value = 0
        self.target_gradient = None
        self.active = False

    def on_up_cycle(self, value):
        self.current_value = value

    def start(self,
              target_gradient):
        self.target_gradient = target_gradient
        self.cycle.restart()
        self.active = True

    def update(self):
        if not self.active:
            return

        self.cycle.next()

        if self.current_value >= .97:
            self.panel_context.switch_to_next_gradient()
            # Done!
            self.cycle.stop()
            self.active = False
            return

    def get_color(self, panel_value):
        current_gradient = self.panel_context.current_gradient
        from_color = current_gradient.get_color_for_value(panel_value)
        to_color = self.target_gradient.get_color_for_value(panel_value)

        return from_color.interpolate(to_color, self.current_value)

