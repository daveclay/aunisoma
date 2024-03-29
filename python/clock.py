class Clock:
    def __init__(self):
        self.running = False
        self.start_tick = None
        self.ticks = None

    def to_dict(self):
        return {
            "running": self.running,
            "start_tick": self.start_tick,
            "ticks": self.ticks
        }

    def start(self):
        self.running = True

    def stop(self):
        self.running = False
        self.start_tick = None
        self.ticks = None

    def next(self):
        if not self.running:
            # Don't confuse people by automatically running; next() can only be called after start()
            return # race condition

        if self.start_tick is None:
            self.start_tick = 0

        if self.ticks is None:
            self.ticks = 0
        else:
            self.ticks += 1

    def restart(self):
        self.running = True
        self.start_tick = None
        self.ticks = None
