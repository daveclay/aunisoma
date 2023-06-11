import time
import random
import board
import adafruit_dotstar as dotstar
import assembly

dots = dotstar.DotStar(board.SCK, board.MOSI, assembly.number_of_panels, brightness=1, auto_write=False)


# MAIN LOOP
n_dots = len(dots)


def interact_with_panel(index):
    assembly.sensors[index] = True


scripts = [
    {'at': 2, 'panel': 0, 'for': 4},
    {'at': 4, 'panel': 4, 'for': 5},
    {'at': 5, 'panel': 8, 'for': 8},
    {'at': 8, 'panel': 9, 'for': 2},
    {'at': 12, 'panel': 3, 'for': 6},
    {'at': 14, 'panel': 11, 'for': 7},
    {'at': 19, 'panel': 12, 'for': 7},
    {'at': 22, 'panel': 13, 'for': 8},
    {'at': 30, 'panel': 1, 'for': 19},
    {'at': 31, 'panel': 2, 'for': 18},
    {'at': 32, 'panel': 3, 'for': 15},
    {'at': 37, 'panel': 5, 'for': 18},
    {'at': 38, 'panel': 6, 'for': 16},
    {'at': 39, 'panel': 7, 'for': 15},
    {'at': 39, 'panel': 8, 'for': 16},
    {'at': 40, 'panel': 9, 'for': 19},
    {'at': 41, 'panel': 10, 'for': 10},
    {'at': 42, 'panel': 11, 'for': 6},
]


start_time = time.time()


while True:
    current_time = time.time() + 1
    time_passed = (current_time - start_time) % 90
    for script in scripts:
        at = script['at']
        for_time = script['for']
        panel_index = script['panel']
        if time_passed > (at + for_time):
            assembly.sensors[panel_index] = False
        elif time_passed > at:
            assembly.sensors[panel_index] = True

    start = time.monotonic_ns()
    assembly.panel_context.event_loop()
    print((time.monotonic_ns() - start) / 1000000)

    for panel in assembly.panel_context.panels:
        color = panel.color
        dots[panel.index] = ( color.red, color.green, color.blue)
   
    dots.show()
    # time.sleep(0.0001)
