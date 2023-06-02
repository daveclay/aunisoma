class MockPanelValueSource:
    def __init__(self):
        self.panel_value = 0

    def get_value_for_panel(self, panel):
        return self.panel_value


class MockPanelContext:
    def __init__(self, gradient):
        self.gradient = gradient


class MockInteraction:
    def __init__(self,
                 source_panel,
                 distance_from_trigger_to_panel):
        self.source_panel = source_panel
        self.distance_from_trigger_to_panel = distance_from_trigger_to_panel
        self.currentReverberatingDistance = 0

    def get_distance_from_trigger_to_panel(self, panel):
        return self.distance_from_trigger_to_panel



class MockInteractionConfig:
    def __init__(self,
                 initial_trigger_panel_animation_loop_duration_ticks):
        self.initial_trigger_panel_animation_loop_duration_ticks = initial_trigger_panel_animation_loop_duration_ticks