import unittest
import gradient


class TestGradient(unittest.TestCase):

    def setUp(self):
        self.gradient = gradient.GradientValueMap()
        self.gradient.add_rgb_point(0.0,  60,   0,   0)
        self.gradient.add_rgb_point(.4,  255,   0,   0)
        self.gradient.add_rgb_point(1.0, 255, 255,   0)
        self.gradient.add_rgb_point(1.2,   0, 255,   0)
        self.gradient.add_rgb_point(1.5,   0, 255, 255)
        self.gradient.add_rgb_point(3.6,   0,   0, 255)
        self.gradient.add_rgb_point(4.5, 255,   0, 255)

    def test_value_at_zero(self):
        color = self.gradient.get_color_for_value(0)
        self.assertEqual(60, color.red)
        self.assertEqual(0, color.green)
        self.assertEqual(0, color.blue)

    def test_value_at_red_max(self):
        color = self.gradient.get_color_for_value(.4)
        self.assertEqual(255, color.red)
        self.assertEqual(0, color.green)
        self.assertEqual(0, color.blue)

    def test_value_at_upwards_towards_orange(self):
        color = self.gradient.get_color_for_value(.6)
        self.assertEqual(255, color.red)
        self.assertEqual(85, color.green)
        self.assertEqual(0, color.blue)

    def test_value_at_upwards_towards_orange_point_eight(self):
        color = self.gradient.get_color_for_value(.8)
        self.assertEqual(255, color.red)
        self.assertEqual(170, color.green)
        self.assertEqual(0, color.blue)

    def test_value_at_orange(self):
        color = self.gradient.get_color_for_value(1)
        self.assertEqual(255, color.red)
        self.assertEqual(255, color.green)
        self.assertEqual(0, color.blue)
