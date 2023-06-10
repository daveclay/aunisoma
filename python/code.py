import time
import random
import board
import adafruit_dotstar as dotstar
import assembly

dots = dotstar.DotStar(board.SCK, board.MOSI, assembly.number_of_panels, brightness=1)


# MAIN LOOP
n_dots = len(dots)

while True:
    # Fill each dot with a random color
    for panel in assembly.panel_context.panels:
        # print(str(panel.to_dict()))
        color = panel.color
        dots[panel.index] = ( color.red, color.green, color.blue)
    
    time.sleep(0.25)
