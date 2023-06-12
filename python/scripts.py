import time
import assembly


def interact_with_panel(index):
    assembly.sensors[index] = True


scripts = [
    {'at': 1, 'panel': 0, 'for': 14},
    {'at': 1, 'panel': 4, 'for': 15},
    {'at': 1, 'panel': 8, 'for': 18},
    {'at': 1, 'panel': 9, 'for': 12},
    {'at': 1, 'panel': 3, 'for': 16},
    {'at': 1, 'panel': 11, 'for': 17},
    {'at': 1, 'panel': 12, 'for': 17},
    {'at': 1, 'panel': 13, 'for': 8},
    {'at': 1, 'panel': 1, 'for': 19},
    {'at': 1, 'panel': 2, 'for': 18},
    {'at': 1, 'panel': 3, 'for': 15},
    {'at': 1, 'panel': 5, 'for': 18},
    {'at': 1, 'panel': 6, 'for': 16},
]

start_time = time.time()


def check_scripts():
    current_time = time.time() + 1
    time_passed = (current_time - start_time) % 23
    for script in scripts:
        at = script['at']
        for_time = script['for']
        panel_index = script['panel']
        if time_passed > (at + for_time):
            assembly.sensors[panel_index] = False
        elif time_passed > at:
            assembly.sensors[panel_index] = True
