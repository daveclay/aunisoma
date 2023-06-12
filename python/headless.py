import time
import assembly
import scripts


class FuckPython:
    iterations = 0

def loop():
    scripts.check_scripts()
    start = time.monotonic_ns()
    assembly.panel_context.event_loop()
    s = (time.monotonic_ns() - start) / 1000000
    FuckPython.iterations += 1
    print(str(FuckPython.iterations).ljust(8, ' ') + str(s))

for i in range(10000):
    loop()
