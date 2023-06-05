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
max_interaction_duration_ticks = 500
max_interaction_gradient_value_multiplier = .2 # spreads values across the gradient

interaction_config = InteractionConfig(
    min_reverberation_distance,
    max_reverberation_distance,
    reverb_delay_ticks,
    min_trigger_panel_animation_loop_duration_ticks,
    max_trigger_panel_animation_loop_duration_ticks,
    max_interaction_threshold_percent,
    max_interaction_duration_ticks,
    max_interaction_gradient_value_multiplier
)

gradient = GradientValueMap()
gradient.add_rgb_point(0.0,  60,   0,   0)
gradient.add_rgb_point(.4,  255,   0,   0)
gradient.add_rgb_point(1.0, 255, 255,   0)
gradient.add_rgb_point(1.2,   0, 255,   0)
gradient.add_rgb_point(1.5,   0, 128, 255)
gradient.add_rgb_point(1.7,   0,   0, 255)
gradient.add_rgb_point(2.1, 128,   0, 255)

sensors = {}
for i in range(number_of_panels):
    sensors[i] = False

panel_context = PanelContext(
    number_of_panels,
    interaction_config,
    gradient,
    sensors
)
