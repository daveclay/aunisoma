from python.interaction import InteractionContext
from python.max_interaction_animation import MaxInteractionAnimation
from python.panel import Panel


class PanelContext:
    def __init__(self,
                 number_of_panels,
                 interaction_config,
                 gradient,
                 sensors):
        self.number_of_panels = number_of_panels
        self.interaction_config = interaction_config
        self.interaction_context = InteractionContext(interaction_config, self)
        self.gradient = gradient
        self.sensors = sensors
        self.panels = []
        self.interactions_by_source_panel_index = {}
        self._create_panels()
        self._create_interactions()
        self.max_interaction_animation = MaxInteractionAnimation(self, self.interaction_config)
        self.is_at_max_interactions = False
        self.amps = 0
        self.panel_interaction_collector = PanelInteractionCollector()
        self.running = False

    def _create_panels(self):
        for index in range(self.number_of_panels):
            self.panels.append(Panel(index, self))

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

                    self.running = True # TODO: Not strictly necessary unless we're breaking the loop to save power

    def event_loop(self):
        self.read_sensors()

        self.is_at_max_interactions = self._calculate_at_max_interaction()

        # TODO: tell it to stop? Maybe? Or will we lose its value before we render?
        # TODO: do we need a setup, calculate, render, teardown lifecycle steps? It's confusing to know when update is called vs when the color is set.

        self.panel_interaction_collector.reset()
        activeInteractions = []

        for interaction in self.interactions:
            # update the internal state of the Interaction
            # This must be first - the Interactions' state must be updated to determine whether it is _dead_ or not.
            interaction.update()
            # now that the internal state has been updated, determine whether it is dead and stop it if necessary.
            if interaction.is_dead():
                # TODO: Should Interaction call stop() itself after update() is called, and then self can just ask?
                interaction.stop()
            else:
                activeInteractions.append(interaction)
                for panelReverberation in interaction.active_panel_reverberations:
                    panel = panelReverberation.panel
                    interaction = panelReverberation.interaction

                    if self.is_at_max_interactions:
                        # TODO: filterValueSource? only max does self?
                        interaction = self.max_interaction_animation.filter_interaction(interaction)

                    self.panel_interaction_collector.add_panel_and_value_source(panel, interaction)

        self.max_interaction_animation.update()

        if self.max_interaction_animation.is_running():
            # all Panels, both active and inactive, participate in the max animation
            for panel in self.panels:
                self.panel_interaction_collector.add_panel_and_value_source(panel, self.max_interaction_animation)

        for panelAndValueSource in self.panel_interaction_collector.panel_and_value_sources_by_panel_index.values():
            panel = panelAndValueSource['panel']
            panel.update(panelAndValueSource['panelValueSources'])

        if len(activeInteractions) == 0:
            self.running = False

    def _calculate_at_max_interaction(self):
        activePanels = list(filter(lambda panel: panel.interaction_active, self.panels))
        activePercent = len(activePanels) / len(self.panels)
        return activePercent >= self.interaction_config.max_interaction_threshold_percent


class PanelInteractionCollector:
    def __init__(self):
        self.panel_and_value_sources_by_panel_index = {}

    def add_panel_and_value_source(self, panel, value_source):
        value_sources_involving_panel = self._get_value_sources_for_panel(panel)
        value_sources_involving_panel.append(value_source)

    def _get_value_sources_for_panel(self, panel):
        if not self.panel_and_value_sources_by_panel_index.get(panel.index):
            self.panel_and_value_sources_by_panel_index[panel.index] = {
                'panel': panel,
                'panelValueSources': []
            }

        return self.panel_and_value_sources_by_panel_index[panel.index]['panelValueSources']

    def reset(self):
        self.panel_and_value_sources_by_panel_index.clear()

