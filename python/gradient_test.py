import unittest
import gradient


class TestGradient(unittest.TestCase):

    def setUp(self):
        self.gradient = gradient.GradientValueMap()
        self.gradient.add_rgb_point(0, 0, 0, 0)
        self.gradient.add_rgb_point(1, 255, 0, 0)
        self.gradient.add_rgb_point(2, 0, 255, 0)
        self.gradient.add_rgb_point(3, 0, 0, 255)

    def test_value_at_zero(self):
        color = self.gradient.get_color_for_value(0)
        self.assertEqual(0, color.red)
        self.assertEqual(0, color.green)
        self.assertEqual(0, color.blue)

    def test_value_at_red_max(self):
        color = self.gradient.get_color_for_value(1)
        self.assertEqual(255, color.red)
        self.assertEqual(0, color.green)
        self.assertEqual(0, color.blue)

    def test_value_at_green_max(self):
        color = self.gradient.get_color_for_value(2)
        self.assertEqual(0, color.red)
        self.assertEqual(255, color.green)
        self.assertEqual(0, color.blue)

    def test_value_at_blue_max(self):
        color = self.gradient.get_color_for_value(3)
        self.assertEqual(0, color.red)
        self.assertEqual(0, color.green)
        self.assertEqual(255, color.blue)

    def test_value_at_halfway_red_green(self):
        color = self.gradient.get_color_for_value(1.5)
        self.assertEqual(127, color.red)
        self.assertEqual(128, color.green)
        self.assertEqual(0, color.blue)

    def test_value_at_halfway_green_blue(self):
        color = self.gradient.get_color_for_value(2.5)
        self.assertEqual(0, color.red)
        self.assertEqual(127, color.green)
        self.assertEqual(128, color.blue)
