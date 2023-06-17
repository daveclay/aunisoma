class Maths {
  public:
    static int interpolateValue(int value_a, int value_b, float amount) {
        float value_a_amount = value_a * (1.0 - amount);
        float value_b_amount = value_b * amount;

        return (int) value_a_amount + value_b_amount;
    }
};

class Color {
    int red;
    int green;
    int blue;

public:
    Color(int red, int green, int blue) {
      this->red = red;
      this->green = green;
      this->blue = blue;
    }

    Color interpolate(Color *color, float amount) {
        int red = Maths::interpolateValue(this->red, color->red, amount);
        int green = Maths::interpolateValue(this->green, color->green, amount);
        int blue = Maths::interpolateValue(this->blue, color->blue, amount);
        return Color(red, green, blue);
    }
};

class GradientValuePoint {
  public:
    float value;
    Color *color;
    GradientValuePoint(float value, Color *color) {
      this->value = value;
      this->color = color;
    }
};

class SingleGradientValueMap {
  private:
    int index = 0;
    int size = 0;

  public:
    GradientValuePoint *gradientValuePoints[10];

    void addPoint(float value, Color *color) {
        this->gradientValuePoints[this->index] = new GradientValuePoint(value, color);
        this->index += 1;
        this-> size = sizeof(this->gradientValuePoints);
    }

    Color get_color_at_value(float value) {
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

    void get_gradient_value_points_for_value(float value, GradientValuePoint value_points[]) {
        GradientValuePoint matching_points_for_value[2];
        GradientValuePoint lower_point;
        for (int i = 0; i < this->size; i++) {
            GradientValuePoint point = this->gradient_value_points[i];
            if (value >= point->value) {
                lower_point = point
            } else {
                if (lower_point) {
                    matching_points_for_value[0] = lower_point;
                    matching_points_for_value[1] = point;
                } else {
                    matching_points_for_value[0] = point;
                    matching_points_for_value[1] = point;
                }
                break
            }
        }

        if matching_points_for_value:
            return matching_points_for_value

        # otherwise, use the max two points
        return [
            self.gradient_value_points[len(self.gradient_value_points) - 2],
            self.gradient_value_points[len(self.gradient_value_points) - 1]
        ]
    }
};



void setup() {

}

void loop() {
  Color first(60, 0, 0);
  Color second(20, 255, 255);

  Color interpolatedColor = first.interpolate(&second, .25);

}