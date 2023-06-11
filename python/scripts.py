import time
import assembly


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


def check_scripts():
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
