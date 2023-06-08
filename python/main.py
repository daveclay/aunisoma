from python.gradient import GradientValueMap
from python.interaction import InteractionConfig
from python.panel_context import PanelContext

number_of_panels = 20
min_reverberation_distance = 2
max_reverberation_distance = 6
reverb_delay_ticks = 20
min_trigger_panel_animation_loop_duration_ticks = 200
max_trigger_panel_animation_loop_duration_ticks = 500
max_interaction_threshold_percent = .5
max_interaction_duration_ticks = 1000
max_interaction_amount_of_reverberation = .1
max_interaction_value_multiplier = 3

interaction_config = InteractionConfig(
    min_reverberation_distance,
    max_reverberation_distance,
    reverb_delay_ticks,
    min_trigger_panel_animation_loop_duration_ticks,
    max_trigger_panel_animation_loop_duration_ticks,
    max_interaction_threshold_percent,
    max_interaction_duration_ticks,
    max_interaction_amount_of_reverberation,
    max_interaction_value_multiplier
)

alt_gradient = GradientValueMap()
alt_gradient.add_rgb_point(0.0,   0,   0,  60)
alt_gradient.add_rgb_point(.4,    0,   0, 255)
alt_gradient.add_rgb_point(1.0, 255,   0, 255)
alt_gradient.add_rgb_point(2.0, 255,   0, 255)

gradient = GradientValueMap()
gradient.add_rgb_point(0.0,  60,   0,   0)
gradient.add_rgb_point(.4,  255,   0,   0)
gradient.add_rgb_point(1.0, 255, 255,   0)
gradient.add_rgb_point(1.6,   0, 255, 255)
gradient.add_rgb_point(2.0, 255, 255,   0)
gradient.add_rgb_point(2.6, 255,   0,   0)
gradient.add_rgb_point(3,    60,   0,   0)
gradient.add_rgb_point(5,    60,   0,   0)

sensors = {}
for i in range(number_of_panels):
    sensors[i] = False

panel_context = PanelContext(
    number_of_panels,
    interaction_config,
    gradient,
    sensors
)
