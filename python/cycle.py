from python.clock import Clock


class Cycle:
    def __init__(self,
                 duration_ticks,
                 on_up_fn,
                 on_down_fn,
                 is_one_shot):
        self.duration_ticks = duration_ticks
        self.on_up_fn = on_up_fn
        self.on_down_fn = on_down_fn
        self.is_one_shot = is_one_shot
        self.clock = Clock()
        self.iterations = 0

    def start(self):
        self.clock.start()

    def stop(self):
        self.clock.stop()

    def is_done(self):
        return self.is_one_shot and self.iterations > 0

    def next(self):
        if not self.clock.ticks is None and self.clock.ticks >= self.duration_ticks:
            self.iterations += 1
            if self.is_one_shot:
                self.stop()
                return
            else:
                # we're cycling; once self goes over the duration, we may as well reset back to 0
                self.clock.restart()

        # TODO: on the last step of the cycle, this will end up at a 0 point but _not_ rollover the next iteration
        self.clock.next()

        looping_elapsed_duration = self.clock.ticks % self.duration_ticks

        if self.on_down_fn is None:
            current_value = looping_elapsed_duration / self.duration_ticks
            self.on_up_fn(current_value)
        else:
            half_animation_loop_duration_ticks = self.duration_ticks / 2  # for up and down
            if looping_elapsed_duration < half_animation_loop_duration_ticks:
                # on our way up
                current_value = looping_elapsed_duration / half_animation_loop_duration_ticks
                self.on_up_fn(current_value)
            else:
                # on our way down
                current_value = (self.duration_ticks - looping_elapsed_duration) / half_animation_loop_duration_ticks
                self.on_down_fn(current_value)

    def jumpToDownCycle(self):
        if self.duration_ticks >= self.clock.ticks:
            # Don't "underun" the clock here: only jump down if it's meaningful to do so. This
            # shouldn't result in 0, otherwise we'll never hit a single iteration.
            # TODO: the problem with this logic is that it may never hit actual 0 value - just get close.
            self.clock.ticks = max(0, self.duration_ticks - self.clock.ticks)

    def restart(self, duration_ticks=None):
        if duration_ticks:
            self.duration_ticks = duration_ticks
        self.iterations = 0
        self.clock.restart()

    def is_at_zero_point(self):
        return self.clock.ticks is None or (self.clock.ticks % self.duration_ticks == 0)
