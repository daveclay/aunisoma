import random

from clock import Clock
from panel_reverberation import PanelReverberation
from range import Range


class Interaction:
    def __init__(self, source_panel, interaction_config, panel_context):
        self.source_panel = source_panel
        self.interaction_config = interaction_config
        self.panel_context = panel_context

        self.clock = Clock()
        self._build_panel_reverberations()
        self.is_at_zero_point = False
        self.active_panel_reverberations = []
        self.panel_reverberations_still_active = False

    def to_dict(self):
        return {
            "sourcePanel": self.source_panel.to_dict(),
            "clock": self.clock.to_dict(),
            "activePanelReverberations": map(
                lambda panel_reverberation: panel_reverberation.to_dict(),
                self.active_panel_reverberations
            )
        }

    def _build_panel_reverberations(self):
        self.source_panel_reverberation = PanelReverberation(self.source_panel, self, self.interaction_config)
        self.panel_reverberations_by_panel_index = {self.source_panel.index: self.source_panel_reverberation}
        for panel in self._collect_all_neighbor_panels():
            self.panel_reverberations_by_panel_index[panel.index] = \
                PanelReverberation(panel, self, self.interaction_config)

        self.panel_reverberations = list(self.panel_reverberations_by_panel_index.values())

    def _collect_all_neighbor_panels(self):
        from_index = self.source_panel.index
        reverberating_panels = []
        for i in range(self.interaction_config.max_reverberation_distance - 1):
            distance_from_index = i + 1
            left_panel_index = from_index - distance_from_index
            right_panel_index = from_index + distance_from_index

            if left_panel_index >= 0:
                reverberating_panels.append(self.panel_context.get_panel_at(left_panel_index))

            if right_panel_index <= len(self.panel_context.panels) - 1:
                reverberating_panels.append(self.panel_context.get_panel_at(right_panel_index))

        return reverberating_panels

    def start(self):
        if self.clock.running:
            raise Exception("Interaction is already started!")

        self.clock.start()
        # print("Starting interaction", self.to_dict())
        self._trigger_new_reverberation(True)
        # print("Started interaction", self.to_dict())

    def _trigger_new_reverberation(self, trigger_source_panel):
        self.current_reverberating_distance = self.interaction_config.get_reverberation_distance()
        # TODO: hrm, slow - two loops, but only when interactions start?
        self.eligible_panel_reverberations = [panel_reverberation for panel_reverberation in self.panel_reverberations
                                              if
                                              self.current_reverberating_distance >= panel_reverberation.distance_from_trigger]

        # print("triggering", self.current_reverberating_distance, "reverberating panels")
        for panel_reverberation in self.eligible_panel_reverberations:
            if trigger_source_panel or not panel_reverberation.is_source_interaction:
                panel_reverberation.start()

    def stop(self):
        if not self.clock.running:
            return

        self.clock.stop()
        for panel_reverberation in self.active_panel_reverberations:
            panel_reverberation.stop()
        # print("Stopped Interaction", self.to_dict())

    def update(self):
        if not self.clock.running:
            return

        self.clock.next()
        self.active_panel_reverberations[:] = self._calculate_active_panel_reverberations()

        self.panel_reverberations_still_active = False

        for panel_reverberation in self.active_panel_reverberations:
            reverberation_panel_delay_ticks = self.interaction_config.reverberation_panel_delay_ticks
            delay_for_panel_to_start_ticks = panel_reverberation.distance_from_trigger * reverberation_panel_delay_ticks
            if self.clock.ticks >= delay_for_panel_to_start_ticks:
                if not panel_reverberation.cycle.clock.running:
                    # TODO: sometimes the PanelReverberation has not had start() called
                    panel_reverberation.start()
                panel_reverberation.update()

            if panel_reverberation.is_done():
                panel_reverberation.stop()
            else:
                self.panel_reverberations_still_active = True

        self.is_at_zero_point = self._calculate_is_at_zero_point()

        self._maybe_revive_panel_reverberations()

    def _calculate_is_at_zero_point(self):
        last = self._find_last_remaining_alive_source_panel_reverberation()
        return last is not None and last.current_value == 0

    def _calculate_active_panel_reverberations(self):
        return [panel_reverberation for panel_reverberation
                in self.eligible_panel_reverberations
                if not panel_reverberation.is_done()]

    def _find_last_remaining_alive_source_panel_reverberation(self):
        alive = [panel_reverberation for panel_reverberation in self.panel_reverberations if
                 not panel_reverberation.is_done()]
        if len(alive) == 1:
            return alive[0]
        else:
            return None

    def _maybe_revive_panel_reverberations(self):
        if self.is_at_zero_point and self.source_panel.interaction_active and random.random() > 0.65:
            self.clock.restart()
            self._trigger_new_reverberation(False)

    def is_dead(self):
        if self.source_panel.interaction_active:
            return False

        if not self.active_panel_reverberations:
            return True

        return not self.panel_reverberations_still_active

    def get_distance_from_trigger_to_panel(self, panel):
        return abs(self.source_panel.index - panel.index)

    def get_value_for_panel(self, panel):
        panel_reverberation = self.panel_reverberations_by_panel_index.get(panel.index)
        if panel_reverberation is None:
            return 0
        else:
            return panel_reverberation.current_value


class InteractionContext:
    def __init__(self, interaction_config, panel_context):
        self.interaction_config = interaction_config
        self.panel_context = panel_context

    def create_interaction(self, panel):
        return Interaction(panel, self.interaction_config, self.panel_context)


class InteractionConfig:
    def __init__(self,
                 min_reverberation_distance,
                 max_reverberation_distance,
                 reverberation_panel_delay_ticks,
                 min_trigger_panel_animation_loop_duration_ticks,
                 max_trigger_panel_animation_loop_duration_ticks,
                 max_interaction_threshold_percent,
                 max_interaction_duration_ticks,
                 max_interaction_amount_of_reverberation,
                 max_interaction_value_multiplier):
        self.max_reverberation_distance = max_reverberation_distance
        self.reverberation_distance_range = Range(min_reverberation_distance, max_reverberation_distance)
        self.reverberation_panel_delay_ticks = reverberation_panel_delay_ticks
        self.trigger_panel_animation_loop_duration_ticks_range = Range(
            min_trigger_panel_animation_loop_duration_ticks, max_trigger_panel_animation_loop_duration_ticks
        )
        self.initial_trigger_panel_animation_loop_duration_ticks = \
            (max_trigger_panel_animation_loop_duration_ticks + min_trigger_panel_animation_loop_duration_ticks) / 2
        self.max_interaction_threshold_percent = max_interaction_threshold_percent
        self.max_interaction_duration_ticks = max_interaction_duration_ticks
        self.max_interaction_amount_of_reverberation = max_interaction_amount_of_reverberation
        self.max_interaction_value_multiplier = max_interaction_value_multiplier

    def get_reverberation_distance(self):
        return self.reverberation_distance_range.random_int_between()

    def get_trigger_panel_animation_loop_duration_ticks(self):
        return self.trigger_panel_animation_loop_duration_ticks_range.random_int_between()

    def to_dict(self):
        return {
            'maxReverberationDistance': self.max_reverberation_distance,
            'reverberationPanelDelayTicks': self.reverberation_panel_delay_ticks,
            'maxInteractionThresholdPercent': self.max_interaction_threshold_percent
        }
