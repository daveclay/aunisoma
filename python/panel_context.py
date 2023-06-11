import random

from interaction import InteractionContext
from max_interaction_animation import MaxInteractionAnimation
from panel import Panel
from transition_animation import TransitionAnimation


class PanelContext:
    def __init__(self,
                 number_of_panels,
                 interaction_config,
                 gradients,
                 sensors):
        self.number_of_panels = number_of_panels
        self.interaction_config = interaction_config
        self.interaction_context = InteractionContext(interaction_config, self)
        self.gradients = gradients
        self.current_gradient_index = 0
        self.current_gradient = gradients[self.current_gradient_index]
        self.max_gradient_index = len(self.gradients) - 1
        self._calculate_next_gradient_index()
        self.sensors = sensors
        self.panels = []
        self.number_of_panels = number_of_panels
        self.interactions_by_source_panel_index = {}
        self._create_panels()
        self._create_interactions()
        self.max_interaction_animation = MaxInteractionAnimation(self, self.interaction_config)
        self.transition_animation = TransitionAnimation(500, self)
        self.is_at_max_interactions = False
        self.transitioned_during_this_max = False
        self.ticks_since_last_transition = 0
        self.amps = 0
        self.panel_interaction_collector = PanelInteractionCollector(self.panels, self.interactions)

    def _calculate_next_gradient_index(self):
        if self.current_gradient_index == self.max_gradient_index:
            self.next_gradient_index = 0
        else:
            self.next_gradient_index = self.current_gradient_index + 1

    def _create_panels(self):
        for index in range(self.number_of_panels):
            idle_color = self.current_gradient.get_color_for_value(0)
            panel = Panel(index, idle_color)
            self.panels.append(panel)

    def _create_interactions(self):
        self.interactions = list(map(
            self._create_interaction,
            self.panels
        ))

    def _create_interaction(self, panel):
        interaction = self.interaction_context.create_interaction(panel)
        self.interactions_by_source_panel_index[panel.index] = interaction
        return interaction

    def get_panel_at(self, index):
        return self.panels[index]

    def read_sensors(self):
        for panel_index in self.sensors:
            is_panel_active = self.sensors[panel_index]
            self._handle_panel_sensor(panel_index, is_panel_active)

    def _handle_panel_sensor(self, panel_index, is_panel_active):
        panel = self.panels[panel_index]
        if is_panel_active != panel.interaction_active:
            panel.set_active(is_panel_active)
            if is_panel_active:
                interaction = self.interactions_by_source_panel_index[panel.index]
                if not interaction.clock.running:
                    interaction.start()

    def event_loop(self):
        self.read_sensors()

        self.is_at_max_interactions = self._calculate_at_max_interaction()

        # TODO: if this has been at zero for a while and not at gradient index 0, go back to initial gradient.

        for interaction in self.interactions:
            # update the internal state of the Interaction
            # This must be first - the Interactions' state must be updated to determine whether it is _dead_ or not.
            interaction.update()
            # now that the internal state has been updated, determine whether it is dead and stop it if necessary.
            if interaction.is_dead():
                # TODO: Should Interaction call stop() itself after update() is called, and then self can just ask?
                interaction.stop()

        if self.is_at_max_interactions:
            if not self.transition_animation.cycle.clock.running:
                if self.transitioned_during_this_max:
                    self.ticks_since_last_transition += 1
                    # If we transitioned once already and we're _still_ at max - need some state to know if
                    # TODO: also, restart this after some delay if we're still at max interactions
                    if self.ticks_since_last_transition > 5000 and random.random() > .999:
                        self._start_transition()
                        self.ticks_since_last_transition = 0
                else:
                    self._start_transition()
                    self.transitioned_during_this_max = True
        else:
            # reset state for max tracking
            self.transitioned_during_this_max = False
            self.ticks_since_last_transition = 0
            if self.current_gradient_index != 0 and not self.transition_animation.active and random.random() > .999:
                # reset back to original gradient.
                self.next_gradient_index = 0
                self._start_transition()

        if self.transition_animation.active:
            self.transition_animation.update()
            self._update_panels_for_transition()
        else:
            self._update_panels_for_interactions()

    def _start_transition(self):
        self.transition_animation.start(self.gradients[self.next_gradient_index])

    def switch_to_next_gradient(self):
        self.current_gradient_index = self.next_gradient_index
        self._calculate_next_gradient_index()
        self.current_gradient = self.gradients[self.current_gradient_index]
        self.ticks_since_last_transition = 1

    # Updates _all_ panels, not just ones with interactions and reverberations
    def _update_panels_for_transition(self):
        for panel in self.panels:
            panel_value_sources = self.panel_interaction_collector.panel_value_sources_for_panel(panel)
            if panel_value_sources is None:
                panel_value_sources = []
            self._update_panel(panel, panel_value_sources)

    # Updates panels involved in interactions and reverberations
    def _update_panels_for_interactions(self):
        for panel_and_value_sources in self.panel_interaction_collector.all_panel_and_value_sources():
            panel = panel_and_value_sources['panel']
            panel_value_sources = panel_and_value_sources['panelValueSources']
            self._update_panel(panel, panel_value_sources)

    def _update_panel(self, panel, panel_value_sources):
        total_panel_value = 0.0
        for panel_value_source in panel_value_sources:
            total_panel_value += panel_value_source.get_value_for_panel(panel)

        panel.current_value = total_panel_value
        if self.transition_animation.active:
            # Transitioning to new gradient
            panel.color = self.transition_animation.get_color(total_panel_value)
        else:
            # regular animation
            panel.color = self.current_gradient.get_color_for_value(total_panel_value)

    def _calculate_at_max_interaction(self):
        activePanels = list(filter(lambda panel: panel.interaction_active, self.panels))
        activePercent = len(activePanels) / self.number_of_panels
        return activePercent >= self.interaction_config.max_interaction_threshold_percent


class PanelInteractionCollector:
    def __init__(self, panels, interactions):
        self.panel_and_value_sources_by_panel_index = {}
        for panel in panels:
            self.panel_and_value_sources_by_panel_index[panel.index] = {
                'panel': panel,
                'panelValueSources': interactions
            }

    def add_panel_and_value_source(self, panel, value_source):
        value_sources = self.panel_and_value_sources_by_panel_index[panel.index]['panelValueSources']
        value_sources.append(value_source)

    def panel_value_sources_for_panel(self, panel):
        panel_and_value_source = self.panel_and_value_sources_by_panel_index.get(panel.index)
        if not panel_and_value_source is None:
            return panel_and_value_source['panelValueSources']
        else:
            return None

    def all_panel_and_value_sources(self):
        return self.panel_and_value_sources_by_panel_index.values()
