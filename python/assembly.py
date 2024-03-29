from gradient import GradientValueMapBuilder
from interaction import InteractionConfig
from panel_context import PanelContext

number_of_panels = 20
min_reverberation_distance = 2
max_reverberation_distance = 6
reverb_delay_ticks = 20
min_trigger_panel_animation_loop_duration_ticks = 120
max_trigger_panel_animation_loop_duration_ticks = 200
max_interaction_threshold_percent = .5
max_interaction_duration_ticks = 300
max_interaction_amount_of_reverberation = 0 # this tends to flicker the max animation if set > 0
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

initial_gradient = GradientValueMapBuilder()
initial_gradient.add_rgb_point(0.0,  10,   0,   0)
initial_gradient.add_rgb_point(.4,  255,   0,   0)
initial_gradient.add_rgb_point(1.0, 255, 255,   0)
initial_gradient.add_rgb_point(1.6,   0, 255, 255)
initial_gradient.add_rgb_point(3,     0, 255, 255)

blue_gradient = GradientValueMapBuilder()
blue_gradient.add_rgb_point(0.0,   0,   0,  10)
blue_gradient.add_rgb_point(.4,    0,   0, 255)
blue_gradient.add_rgb_point(1.0, 255,   0, 255)
blue_gradient.add_rgb_point(2,   255, 255,   0)
blue_gradient.add_rgb_point(3,   255, 255,   0)

green_gradient = GradientValueMapBuilder()
green_gradient.add_rgb_point(0.0,   0,  10,   0)
green_gradient.add_rgb_point(.4,    0, 255,   0)
green_gradient.add_rgb_point(1.0, 255, 255,   0)
green_gradient.add_rgb_point(2,   255,   0, 255)
green_gradient.add_rgb_point(3,   255,   0, 255)

gradients = [
    initial_gradient.build(),
    blue_gradient.build(),
    green_gradient.build()
]

sensors = {}
for i in range(number_of_panels):
    sensors[i] = False

panel_context = PanelContext(
    number_of_panels,
    interaction_config,
    gradients,
    sensors
)
