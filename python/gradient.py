from color import Color


class GradientValuePoint:
    def __init__(self, value, color_value):
        self.value = value
        self.color_value = color_value


class SingleGradientValueMap:
    def __init__(self):
        self.gradient_value_points = []

    def get_color_at_value(self, value):
        points = self.get_gradient_value_points_for_value(value)

        # ratio = zero-based value / range
        # ratio = val - min / max - min
        min_value = points[0].value
        max_value = points[1].value
        ratio = (value - float(min_value)) / (max_value - min_value)

        # ints
        min_color = points[0].color_value
        max_color = points[1].color_value

        # int
        color_range = max_color - min_color
        # double
        color_value = color_range * ratio

        # int
        color = min_color + round(color_value)

        return min(255, max(0, color))

    def get_gradient_value_points_for_value(self, value):
        matching_points_for_value = None
        lower_point = None
        for i in range(len(self.gradient_value_points)):
            point = self.gradient_value_points[i]
            if value >= point.value:
                lower_point = point
            else:
                if lower_point is None:
                    matching_points_for_value = [point, point]
                else:
                    matching_points_for_value = [lower_point, point]
                break

        if matching_points_for_value:
            return matching_points_for_value

        # otherwise, use the max two points
        return [
            self.gradient_value_points[len(self.gradient_value_points) - 2],
            self.gradient_value_points[len(self.gradient_value_points) - 1]
        ]

    def addPoint(self, value, color):
        point = GradientValuePoint(value, color)
        self.gradient_value_points.append(point)

    def setGradientValuePoints(self, points):
        self.gradient_value_points[:] = points


class GradientValueMap:
    def __init__(self):
        self.red_map = SingleGradientValueMap()
        self.green_map = SingleGradientValueMap()
        self.blue_map = SingleGradientValueMap()

    def add_color_point(self, value, color):
        self.add_rgb_point(value, color.red, color.green, color.blue)

    def add_rgb_point(self, value, red, green, blue):
        self.add_red_point(value, red)
        self.add_green_point(value, green)
        self.add_blue_point(value, blue)

    def add_red_point(self, value, color):
        self.red_map.addPoint(value, color)

    def add_green_point(self, value, color):
        self.green_map.addPoint(value, color)

    def add_blue_point(self, value, color):
        self.blue_map.addPoint(value, color)

    def get_color_for_value(self, value):
        red = self.red_map.get_color_at_value(value)
        green = self.green_map.get_color_at_value(value)
        blue = self.blue_map.get_color_at_value(value)
        return Color(red, green, blue)

