from cycle import Cycle


class PanelReverberation:
    def __init__(self, panel, interaction, interaction_config):
        self.panel = panel
        self.interaction = interaction
        self.interaction_config = interaction_config
        self.scale = 0.0
        oneShot = not self.is_source_interaction()
        self.cycle = Cycle(self.interaction_config.initial_trigger_panel_animation_loop_duration_ticks,
                           lambda value: self.on_up_cycle(value), lambda value: self.on_down_cycle(value), oneShot)
        self.current_value = 0.0
        self.distance_from_trigger = self.interaction.get_distance_from_trigger_to_panel(self.panel)

    def to_dict(self):
        return {
            "panel": self.panel.to_dict(),
            "interactionSourcePanel": self.interaction.source_panel.to_dict(),
            "distanceFromTrigger": self.distance_from_trigger,
            "scale": self.scale,
            "currentValue": self.current_value
        }

    def _calculate_scale(self):
        if self.panel == self.interaction.source_panel:
            return 1
        else:
            maxDistance = self.interaction.current_reverberating_distance
            return (maxDistance - self.distance_from_trigger) / maxDistance

    def is_source_interaction(self):
        return self.panel.index == self.interaction.source_panel.index

    def start(self):
        self.scale = self._calculate_scale()
        self.cycle.restart()
        # console.log("Started PanelReverberation", self)

    def stop(self):
        self.cycle.stop()
        # console.log("Stopped PanelReverberation", self)

    def on_up_cycle(self, value):
        # on our way up
        self._set_value(value)
        if not self.interaction.source_panel.interaction_active and not self.cycle.is_at_zero_point():
            # Fade out from where self animation loop is at by jumping the cycle to the down-side.
            self.cycle.jumpToDownCycle()

    def on_down_cycle(self, value):
        # on our way down
        self._set_value(value)

    def _set_value(self, value):
        self.current_value = value * self.scale

    def update(self):
        self.cycle.next()
        if self.is_source_interaction() and self.panel.interaction_active and self._is_animation_loop_done():
            self.cycle.restart(self.interaction_config.get_trigger_panel_animation_loop_duration_ticks())

    def _is_animation_loop_done(self):
        return self.cycle.is_done() or (self.cycle.iterations > 0 and self.cycle.is_at_zero_point())

    def is_done(self):
        if self.is_source_interaction() and self.panel.interaction_active:
            return False

        if not self.cycle.clock.running:
            return True

        # Otherwise, self is waiting for the animation loop to be done.
        return self._is_animation_loop_done()
