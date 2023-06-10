import unittest
import gradient
import panel
from mock_test_utils import MockPanelValueSource, MockPanelContext


class TestPanel(unittest.TestCase):

    def setUp(self):
        self.index = 0

        self.gradient = gradient.GradientValueMap()
        self.gradient.add_rgb_point(0, 10, 0, 0)
        self.gradient.add_rgb_point(1, 255, 0, 0)

        self.panel_context = MockPanelContext(self.gradient)

        self.panel = panel.Panel(self.index, self.panel_context)

        self.panel_value_source_a = MockPanelValueSource()
        self.panel_value_source_b = MockPanelValueSource()
        self.panel_value_sources = [
            self.panel_value_source_a,
            self.panel_value_source_b
        ]

    def test_panel_idle_color(self):
        color = self.panel.color
        self.assertEqual(10, color.red)
        self.assertEqual(0, color.green)
        self.assertEqual(0, color.blue)

    def test_panel_color_at_zero(self):
        self.panel_value_source_a.panel_value = 0
        self.panel_value_source_b.panel_value = 0
        self.panel.update(self.panel_value_sources)
        color = self.panel.color
        self.assertEqual(10, color.red)
        self.assertEqual(0, color.green)
        self.assertEqual(0, color.blue)

    def test_panel_color_at_one(self):
        self.panel_value_source_a.panel_value = .5
        self.panel_value_source_b.panel_value = .5
        self.panel.update(self.panel_value_sources)
        color = self.panel.color
        self.assertEqual(255, color.red)
        self.assertEqual(0, color.green)
        self.assertEqual(0, color.blue)
