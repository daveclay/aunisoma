from python.gradient import GradientValueMap
from python.interaction import InteractionConfig
from python.panel_context import PanelContext

NUMBER_OF_PANELS = 20
MIN_REVERBERATION_DISTANCE = 2
MAX_REVERBERATION_DISTANCE = 6
REVERB_DELAY_TICKS = 20
MIN_TRIGGER_PANEL_ANIMATION_LOOP_DURATION_TICKS = 200
MAX_TRIGGER_PANEL_ANIMATION_LOOP_DURATION_TICKS = 500
MAX_INTERACTION_THRESHOLD_PERCENT = .5

INTERACTION_CONFIG = InteractionConfig(
    MIN_REVERBERATION_DISTANCE,
    MAX_REVERBERATION_DISTANCE,
    REVERB_DELAY_TICKS,
    MIN_TRIGGER_PANEL_ANIMATION_LOOP_DURATION_TICKS,
    MAX_TRIGGER_PANEL_ANIMATION_LOOP_DURATION_TICKS,
    MAX_INTERACTION_THRESHOLD_PERCENT
)

gradient = GradientValueMap()
gradient.add_rgb_point(0, 60, 0, 0)
gradient.add_rgb_point(.4, 255, 0, 0)
gradient.add_rgb_point(1, 255, 255, 0)
gradient.add_rgb_point(1.2, 0, 255, 0)
gradient.add_rgb_point(1.5, 0, 255, 255)
gradient.add_rgb_point(3.6, 0, 0, 255)
gradient.add_rgb_point(4.5, 255, 0, 255)

PANEL_CONTEXT = PanelContext(
    NUMBER_OF_PANELS,
    INTERACTION_CONFIG,
    gradient
)
