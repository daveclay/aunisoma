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

    def interpolate(self, color, amount):
        red = self._interpolate_value(self.red, color.red, amount)
        green = self._interpolate_value(self.green, color.green, amount)
        blue = self._interpolate_value(self.blue, color.blue, amount)
        return Color(red, green, blue)

    def _interpolate_value(self, value_a, value_b, amount):
        return int((value_a * (1 - amount)) + (value_b * amount))

    def to_hex(self):
        return "#" + self.to_hex_str(self.red) \
            + self.to_hex_str(self.green) \
            + self.to_hex_str(self.blue)

    def to_hex_str(self, value):
        hex_value = hex(value)[2:]
        return self.rjust(hex_value, '0', 2)

    def rjust(self, str_to_pad, ch, length):
        amount_to_pad = length - len(str_to_pad)
        if amount_to_pad > 0:
            return (ch * amount_to_pad) + str_to_pad
        else:
            return str_to_pad

    def to_dict(self):
        return {
            "red": self.red,
            "green": self.green,
            "blue": self.blue,
            "hex": self.to_hex()
        }
