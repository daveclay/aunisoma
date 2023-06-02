class Panel:
    def __init__(self, index, panel_context):
        self.panel_context = panel_context
        self.interaction_active = False
        self.index = index
        self.idle_color = panel_context.gradient.get_color_for_value(0)
        self.color = self.idle_color

        self.set_color(self.idle_color)

    def start_interaction(self):
        self.interaction_active = True

    def stop_interaction(self):
        self.interaction_active = False

    def reset(self):
        self.set_color(self.idle_color)

    def update(self, panel_value_sources):
        self.set_color(self.next_color(panel_value_sources))

    def set_color(self, color):
        self.color = color
        # TODO: Separate UI from logic?
        # self.panel_ui_element.style.backgroundColor = self.color.to_hex_string()
        # self.panel_context.add_power_draw_amps(color.get_power_draw_amps())

    def next_color(self, panel_value_sources):
        value_sums = 0
        for panel_value_source in panel_value_sources:
            panel_value = panel_value_source.get_value_for_panel(self)
            value_sums += panel_value

        return self.panel_context.gradient.get_color_for_value(value_sums)
