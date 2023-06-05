class Panel:
    def __init__(self, index, panel_context):
        self.panel_context = panel_context
        self.interaction_active = False
        self.index = index
        self.idle_color = panel_context.gradient.get_color_for_value(0)
        self.color = self.idle_color

        self.set_color(self.idle_color)

    def to_dict(self):
        return {
            'index': self.index,
            'interactionActive': self.interaction_active,
            'idle_color': self.idle_color.to_dict(),
            'color': self.color.to_dict()
        }

    def set_active(self, active):
        self.interaction_active = active

    def reset(self):
        self.set_color(self.idle_color)

    def update(self, panel_value_sources):
        self.set_color(self.next_color(panel_value_sources))

    def set_color(self, color):
        self.color = color

    def next_color(self, panel_value_sources):
        value_sums = 0.0
        for panel_value_source in panel_value_sources:
            value_sums += panel_value_source.get_value_for_panel(self)

        return self.panel_context.gradient.get_color_for_value(value_sums)
