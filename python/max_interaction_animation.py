from python.cycle import Cycle


class MaxInteractionAnimation:
    def __init__(self, panel_context):
        self.panel_context = panel_context
        # TODO: move duration to config
        self.cycle = Cycle(
            500,
            lambda value: self.on_up_cycle(value),
            lambda value: self.on_down_cycle(value),
            False
        )
        self.current_value = 0
        self.panel_values_by_panel_index = {}

    def filter_interaction(self, interaction):
        return InteractionFilter(interaction)

    def is_running(self):
        if self.panel_context.is_at_max_interactions:
            return True
        else:
            non_zero_panel_values = list(filter(lambda panel_value: panel_value > 0, self.panel_values_by_panel_index))
        return len(non_zero_panel_values) > 0

    def update(self):
        if not self.cycle.clock.running:
            self.cycle.start()

        self.cycle.next()

    def on_up_cycle(self, value):
        self.current_value = value

    def on_down_cycle(self, value):
        self.current_value = value

    def get_value_for_panel(self, panel):
        if not self.panel_context.is_at_max_interactions:
            previous_panel_value = self.panel_values_by_panel_index[panel.index]
            if previous_panel_value <= 0.02:
                # Due to the math below, some panels never actually hit 0 value. This forces a 0 value if it's close,
                # which ensures the panels reset to 0 if they're inactive.
                self.panel_values_by_panel_index[panel.index] = 0
                return 0

        distance_ratio = panel.index / len(self.panel_context.panels)
        wrapped_value = abs(self.current_value - distance_ratio)
        value = 4 * wrapped_value
        self.panel_values_by_panel_index[panel.index] = value
        return value


class InteractionFilter:
    def __init__(self, interaction):
        self.interaction = interaction

    def get_value_for_panel(self, panel):
        originalValue = self.interaction.get_value_for_panel(panel)
        return originalValue * .4
