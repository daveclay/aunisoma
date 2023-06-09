class Panel:
    def __init__(self,
                 index,
                 gradient):
        self.interaction_active = False
        self.index = index
        self.gradient = gradient
        self.idle_color = self.gradient.get_color_for_value(0)
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

    def update(self, value):
        self.current_value = value
        if value is None:
            print("oh no.")
        color = self.gradient.get_color_for_value(value)
        self.set_color(color)

    def set_color(self, color):
        self.color = color