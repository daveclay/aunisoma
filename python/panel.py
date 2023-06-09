class Panel:
    def __init__(self,
                 index,
                 idle_color):
        self.interaction_active = False
        self.index = index
        self.idle_color = idle_color
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
        self.color = self.idle_color