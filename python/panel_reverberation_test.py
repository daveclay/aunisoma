import unittest
import gradient
import panel
import panel_reverberation
from python.mock_test_utils import MockPanelContext, MockInteraction, MockInteractionConfig


class TestPanelReverberation(unittest.TestCase):

    def setUp(self):
        self.gradient = gradient.GradientValueMap()
        self.gradient.add_rgb_point(0, 10, 0, 0)
        self.gradient.add_rgb_point(1, 255, 0, 0)

        self.panel_context = MockPanelContext(self.gradient)
        self.source_panel = panel.Panel(0, self.panel_context)
        self.initial_trigger_panel_animation_loop_duration_ticks = 100
        self.distance_from_trigger_to_panel = 2
        self.interaction = MockInteraction(self.source_panel, self.distance_from_trigger_to_panel)
        self.interaction.currentReverberatingDistance = 4

        self.interaction_config = MockInteractionConfig(self.initial_trigger_panel_animation_loop_duration_ticks)

        self.panel = self.source_panel
        self.source_panel_reverberation = panel_reverberation.PanelReverberation(self.source_panel,
                                                                                 self.interaction,
                                                                                 self.interaction_config)

        self.another_panel = panel.Panel(1, self.panel_context)
        self.another_panel_reverberation = panel_reverberation.PanelReverberation(self.another_panel,
                                                                                  self.interaction,
                                                                                  self.interaction_config)

    def test_cycle_not_one_shot_for_source_panel(self):
        self.assertFalse(self.source_panel_reverberation.cycle.is_one_shot)

    def test_start_for_source_panel(self):
        self.source_panel_reverberation.start()
        self.assertEqual(1, self.source_panel_reverberation.scale)

    def test_cycle_one_shot_for_another_panel(self):
        self.assertTrue(self.another_panel_reverberation.cycle.is_one_shot)

    def test_start_for_another_panel(self):
        self.another_panel_reverberation.start()
        self.assertEqual(.5, self.another_panel_reverberation.scale)

    def test_update_before_halfway(self):
        self.another_panel_reverberation.start()
        self.source_panel.interaction_active = False
        self.another_panel_reverberation.cycle.clock.ticks = 29

        self.another_panel_reverberation.update()

        self.assertEqual(.3, self.another_panel_reverberation.current_value)
        self.assertEqual(70, self.another_panel_reverberation.cycle.clock.ticks)
        self.assertFalse(self.another_panel_reverberation.is_done())

    def test_update_at_halfway(self):
        self.another_panel_reverberation.start()
        self.another_panel_reverberation.cycle.clock.next()
        self.source_panel.interaction_active = False
        self.another_panel_reverberation.cycle.clock.ticks = 49

        self.another_panel_reverberation.update()

        self.assertEqual(.5, self.another_panel_reverberation.current_value)
        self.assertEqual(50, self.another_panel_reverberation.cycle.clock.ticks)
        self.assertFalse(self.another_panel_reverberation.is_done())

    def test_when_animation_loop_is_done(self):
        self.another_panel_reverberation.start()
        self.another_panel_reverberation.cycle.clock.next()
        self.source_panel.interaction_active = False
        self.another_panel_reverberation.cycle.clock.ticks = 100

        self.another_panel_reverberation.update()
        self.assertEqual(0, self.another_panel_reverberation.current_value)
        self.assertEqual(None, self.another_panel_reverberation.cycle.clock.ticks)
        self.assertTrue(self.another_panel_reverberation._is_animation_loop_done())

    def test_is_done(self):
        self.another_panel_reverberation.start()
        self.another_panel_reverberation.cycle.clock.next()
        self.source_panel.interaction_active = False
        self.another_panel_reverberation.cycle.clock.ticks = 100

        self.another_panel_reverberation.update()
        self.assertFalse(self.another_panel_reverberation.cycle.clock.running)
        self.assertTrue(self.another_panel_reverberation.is_done())
