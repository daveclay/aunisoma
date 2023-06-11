import time
import random
import board
import adafruit_dotstar as dotstar
import assembly
import scripts

dots = dotstar.DotStar(board.SCK, board.MOSI, assembly.number_of_panels, brightness=1, auto_write=False)


# MAIN LOOP
n_dots = len(dots)


while True:
    scripts.check_scripts()

    start = time.monotonic_ns()
    assembly.panel_context.event_loop()
    print((time.monotonic_ns() - start) / 1000000)

    for panel in assembly.panel_context.panels:
        color = panel.color
        dots[panel.index] = ( color.red, color.green, color.blue)
   
    dots.show()
    # time.sleep(0.0001)
