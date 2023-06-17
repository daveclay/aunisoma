import time
import assembly
import scripts


class FuckPython:
    iterations = 0

def loop():
    scripts.check_scripts()
    start = time.monotonic_ns()
    assembly.panel_context.event_loop()
    duration = (time.monotonic_ns() - start)
    FuckPython.iterations += 1
    print(str(FuckPython.iterations).ljust(8, ' ') + str(duration) + "ns")

for i in range(100000):
    loop()
