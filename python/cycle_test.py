import unittest
import cycle


class TestCycle(unittest.TestCase):

    def set_on_up_fn_value(self, value):
        self.on_up_fn_value = value

    def set_on_dnw_fn_value(self, value):
        self.on_down_fn_value = value

    def setUp(self):
        self.on_up_fn_value = None
        self.on_down_fn_value = None
        oneShot = False
        self.cycle = cycle.Cycle(
            100,
            lambda value: self.set_on_up_fn_value(value),
            lambda value: self.set_on_dnw_fn_value(value),
            oneShot
        )

    def test_cycle_initialized(self):
        self.assertEqual(0, self.cycle.iterations)

    def test_cycle_started(self):
        self.cycle.start()
        self.assertEqual(0, self.cycle.iterations)
        self.assertFalse(self.cycle.is_done())

    def test_cycle_next_after_start(self):
        self.cycle.start()
        self.cycle.next()
        self.assertEqual(0, self.cycle.iterations)
        self.assertFalse(self.cycle.is_done())
        self.assertEqual(0, self.on_up_fn_value)
        self.assertEqual(None, self.on_down_fn_value)

    def test_cycle_10_steps_after_start(self):
        self.cycle.start()
        for i in range(10):
            self.cycle.next()
        self.assertEqual(0, self.cycle.iterations)
        self.assertFalse(self.cycle.is_done())
        self.assertEqual(.18, self.on_up_fn_value)
        self.assertEqual(None, self.on_down_fn_value)

    def test_cycle_halfway_in_cycle(self):
        self.cycle.start()
        for i in range(50):
            self.cycle.next()
        self.assertEqual(0, self.cycle.iterations)
        self.assertFalse(self.cycle.is_done())
        self.assertEqual(.98, self.on_up_fn_value)
        self.assertEqual(None, self.on_down_fn_value)

    def test_cycle_after_halfway_in_cycle(self):
        self.cycle.start()
        for i in range(51):
            self.cycle.next()
        self.assertEqual(0, self.cycle.iterations)
        self.assertFalse(self.cycle.is_done())
        self.assertEqual(.98, self.on_up_fn_value)
        self.assertEqual(1, self.on_down_fn_value)

    def test_cycle_last_step_in_cycle(self):
        self.cycle.start()
        for i in range(100):
            self.cycle.next()

        self.cycle.next()
        self.assertEqual(0, self.cycle.iterations)
        self.assertFalse(self.cycle.is_done())

        # TODO: this is awkward where at 100 steps, the _values_ have reset to 0 but _iterations_ is still 0
        self.assertEqual(0, self.on_up_fn_value)
        self.assertEqual(.02, self.on_down_fn_value)

    def test_cycle_first_step_in_second_iteration_cycle(self):
        self.cycle.start()
        for i in range(102):
            self.cycle.next()
        self.assertEqual(1, self.cycle.iterations)
        self.assertFalse(self.cycle.is_done())
        self.assertEqual(0, self.on_up_fn_value)
        self.assertEqual(.02, self.on_down_fn_value)

