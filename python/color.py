class Color:
    def __init__(self, red, green, blue):
        self.red = red
        self.green = green
        self.blue = blue
        self.min_red = 0
        self.min_green = 0
        self.min_blue = 0

    def set_min_red(self, min_red):
        self.min_red = min_red

    def set_min_green(self, min_green):
        self.min_green = min_green

    def set_min_blue(self, min_blue):
        self.min_blue = min_blue

    def set_min_color(self, color):
        self.set_min_red(color.red)
        self.set_min_green(color.green)
        self.set_min_blue(color.blue)

    def interpolate(self, color):
        red = (self.red + color.red) / 2
        green = (self.green + color.green) / 2
        blue = (self.blue + color.blue) / 2
        return Color(red, green, blue)

    def to_hex(self):
        return "#" + hex(self.red)[2:].rjust(2, '0') \
            + hex(self.green)[2:].rjust(2, '0') \
            + hex(self.blue)[2:].rjust(2, '0')

    def to_dict(self):
        return {
            "red": self.red,
            "green": self.green,
            "blue": self.blue,
            "hex": self.to_hex()
        }
