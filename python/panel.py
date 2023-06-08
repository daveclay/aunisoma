class Panel:
    def __init__(self, index, panel_context):
        self.panel_context = panel_context
        self.interaction_active = False
        self.index = index
        self.gradient = panel_context.gradient
        self.idle_color = panel_context.gradient.get_color_for_value(0)
        self.current_value = 0
        self.color = self.idle_color
        self.reset()

    def to_dict(self):
        return {
            'index': self.index,
            'interactionActive': self.interaction_active,
            'idle_color': self.idle_color.to_dict(),
            'currentValue': self.current_value,
            'color': self.color.to_dict()
        }

    def set_active(self, active):
        self.interaction_active = active

    def reset(self):
        self.current_value = 0
        self.set_color(self.idle_color)

    def update(self, panel_value_sources):
        value = self._get_value(panel_value_sources)
        self.current_value = value
        color = self.gradient.get_color_for_value(value)
        self.set_color(color)

    def set_color(self, color):
        self.color = color

    def _get_value(self, panel_value_sources):
        value_sums = 0.0
        for panel_value_source in panel_value_sources:
            value_sums += panel_value_source.get_value_for_panel(self)
        return value_sums