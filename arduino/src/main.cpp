#include <iostream>
#include "Cycle.h"
#include "Color.h"
#include "Range.h"
#include "Config.h"
#include "Gradient.h"
#include "Sensor.h"
#include "Aunisoma.h"
#include <chrono>


class Thing: public CycleHandler {
public:
    void value(float value, CycleDirection direction) {
        std::cout << direction << " with value of " << value  << std::endl;
    }
};

int main() {
    std::cout << "Hello, World!" << std::endl;

    Color first(60, 0, 0);
    Color second(20, 255, 255);

    Color interpolatedColor = first.interpolate(second, .25);

    std::cout << interpolatedColor.red << ", " << interpolatedColor.green << ", " << interpolatedColor.blue << std::endl;

    Thing* thing = new Thing();
    Cycle* cycle = new Cycle(100, false, CYCLE_TYPE_UP_AND_DOWN, thing);

    cycle->start();
    for (int i = 0; i < 500; i++) {
        cycle->next();
        if (i == 325) {
            std::cout << "Jumping to down cycle!" << std::endl;
            cycle->jumpToDownCycle();
        }
    }

    Range* range = new Range(10, 50);
    for (int i = 0; i < 20000; i++) {
        int val = range->random_int_between();
        if (val < 0 || val > 50) {
            std::cout << range->random_int_between() << std::endl;
        }
    }

    // Make const so we can assign arrays because screw c++
    int number_of_panels = 20;
    // number of panels to the left and right
    int min_reverberation_distance = 2;
    int max_reverberation_distance = 5;
    // how long to wait to trigger a neighbor Panel to reverberate
    int reverb_delay_ticks = 20;
    int min_trigger_panel_animation_loop_duration_ticks = 120;
    int max_trigger_panel_animation_loop_duration_ticks = 200;
    float max_interaction_threshold_percent = .5;
    int max_interaction_duration_ticks = 300;
    float max_interaction_amount_of_reverberation = 0; // this tends to flicker the max animation if set > 0
    float max_interaction_value_multiplier = 3;

    Config* config = new Config(
            number_of_panels,
            min_reverberation_distance,
            max_reverberation_distance,
            reverb_delay_ticks,
            min_trigger_panel_animation_loop_duration_ticks,
            max_trigger_panel_animation_loop_duration_ticks,
            max_interaction_threshold_percent,
            max_interaction_duration_ticks,
            max_interaction_amount_of_reverberation,
            max_interaction_value_multiplier
    );


    GradientValueMap* initial_gradient = new GradientValueMap();
    initial_gradient->add_rgb_point(0.0,  10,   0,   0);
    initial_gradient->add_rgb_point(.4,  255,   0,   0);
    initial_gradient->add_rgb_point(1.0, 255, 255,   0);
    initial_gradient->add_rgb_point(1.6,   0, 255, 255);
    initial_gradient->add_rgb_point(3,     0, 255, 255);

    GradientValueMap* blue_gradient = new GradientValueMap();
    blue_gradient->add_rgb_point(0.0,   0,   0,  10);
    blue_gradient->add_rgb_point(.4,    0,   0, 255);
    blue_gradient->add_rgb_point(1.0, 255,   0, 255);
    blue_gradient->add_rgb_point(2,   255, 255,   0);
    blue_gradient->add_rgb_point(3,   255, 255,   0);

    GradientValueMap* green_gradient = new GradientValueMap();
    green_gradient->add_rgb_point(0.0,   0,  10,   0);
    green_gradient->add_rgb_point(.4,    0, 255,   0);
    green_gradient->add_rgb_point(1.0, 255, 255,   0);
    green_gradient->add_rgb_point(2,   255,   0, 255);
    green_gradient->add_rgb_point(3,   255,   0, 255);

    GradientValueMap* gradients[3] = {
            initial_gradient,
            blue_gradient,
            green_gradient
    };

    Sensor* sensors[40] = {};
    for (int i = 0; i < 40; i++) {
        sensors[i] = new Sensor();
    }

    using namespace std::chrono;
    Aunisoma* aunisoma = new Aunisoma(config, gradients, 3, sensors);
    for (int i = 0; i < 100000; i++) {
        switch (i) {
            case 10:
                aunisoma->sensors[3]->active = true;
                break;
            case 1000:
                aunisoma->sensors[3]->active = false;
                break;
            case 1010:
                aunisoma->sensors[1]->active = true;
                aunisoma->sensors[2]->active = true;
                aunisoma->sensors[3]->active = true;
                break;
            case 5000:
                aunisoma->sensors[4]->active = true;
                aunisoma->sensors[5]->active = true;
                aunisoma->sensors[6]->active = true;
                aunisoma->sensors[7]->active = true;
                aunisoma->sensors[8]->active = true;
                aunisoma->sensors[9]->active = true;
                aunisoma->sensors[10]->active = true;
                aunisoma->sensors[11]->active = true;
                aunisoma->sensors[12]->active = true;
                break;
        }

        auto start = std::chrono::high_resolution_clock::now();

        aunisoma->event_loop();

        auto finish = std::chrono::high_resolution_clock::now();
        long long int count = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();

        std::cout << i << ": ";
        for (int i = 0; i < aunisoma->numberOfPanels; i++) {
            Panel* panel = aunisoma->get_panel_at(i);
            Color color = panel->color;
            if (color.red != 10) {
                std::cout << "panel " << panel->index << ": " << color.red << "," << color.green << "," << color.blue << "|";
            }
        }
        std::cout << " took " << count << "ns\n";
    }

    return 0;
}