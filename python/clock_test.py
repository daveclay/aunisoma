import unittest
import clock


class TestClock(unittest.TestCase):
    def setUp(self):
        self.clock = clock.Clock()

    def test_clock(self):
        with self.subTest("when initialized"):
            self.assertFalse(self.clock.running)
            self.assertEqual(self.clock.start_tick, None)
            self.assertEqual(self.clock.ticks, None)

        with self.subTest("when started"):
            self.clock.start()
            self.assertTrue(self.clock.running)
            self.assertEqual(self.clock.start_tick, None)
            self.assertEqual(self.clock.ticks, None)

        with self.subTest("when next is called"):
            self.clock.next()
            self.assertTrue(self.clock.running)
            self.assertEqual(self.clock.start_tick, 0)
            self.assertEqual(self.clock.ticks, 0)

        with self.subTest("when running 100 iterations"):
            for i in range(100):
                self.clock.next()
            self.assertTrue(self.clock.running)
            self.assertEqual(self.clock.start_tick, 0)
            self.assertEqual(self.clock.ticks, 100)

        with self.subTest("when stopped"):
            self.clock.stop()
            self.assertFalse(self.clock.running)
            self.assertEqual(self.clock.start_tick, None)
            self.assertEqual(self.clock.ticks, None)

        with self.subTest("when restarted"):
            self.clock.restart()
            self.assertTrue(self.clock.running)
            self.assertEqual(self.clock.start_tick, None)
            self.assertEqual(self.clock.ticks, None)

        with self.subTest("when next is called after restart"):
            self.clock.next()
            self.assertTrue(self.clock.running)
            self.assertEqual(self.clock.start_tick, 0)
            self.assertEqual(self.clock.ticks, 0)


if __name__ == '__main__':
    unittest.main()
