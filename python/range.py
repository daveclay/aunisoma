class Range:
    def __init__(self, min_value, max_value):
        self.min_value = min_value
        self.max_value = max_value

    def random_int_between(self):
        return random_int_between(self.min_value, self.max_value)
