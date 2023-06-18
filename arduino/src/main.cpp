#include <iostream>
#include "Color.h"
#include "Config.h"
#include "Gradient.h"
#include "Sensor.h"
#include "Aunisoma.h"
#include <chrono>

void writeScriptLine(Aunisoma* aunisoma, int i, int count, int iterations) {
    std::cout << "\t{ \"iteration\": " << i << ",";
    std::cout << "\"panels\": [\n";
    for (int j = 0; j < aunisoma->numberOfPanels; j++) {
        Panel* panel = aunisoma->get_panel_at(j);
        Color color = panel->color;
        std::cout << "\t\t{ \"index\": " << panel->index << ", \"red\": " << color.red << ", \"green\": " << color.green << ", \"blue\": " << color.blue << "}";
        if (j < aunisoma->numberOfPanels - 1) {
            std::cout << ",";
        }
        std::cout << "\n";

    }
    std::cout << "],\n\t\"time\": " << count << "}";

    if (i < iterations - 1) {
        std::cout << ",";
    }

    std::cout << "\n";
}

void trigger(Aunisoma* aunisoma, int panelIndex, bool active) {
    int sensorIndex = panelIndex * 2;
    aunisoma->sensors[sensorIndex]->active = active;
}

int main() {
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
    std::cout << "[\n";

    Aunisoma* aunisoma = new Aunisoma(config, gradients, 3, sensors);
    int iterations = 6000;
    for (int i = 0; i < iterations; i++) {
        switch (i) {
            case 5:
                trigger(aunisoma, 3, true);
                break;
            case 1000:
                trigger(aunisoma, 3, false);
                trigger(aunisoma, 11, true);
                trigger(aunisoma, 12, true);
                trigger(aunisoma, 13, true);
                break;
            case 2000:
                trigger(aunisoma, 4, true);
                trigger(aunisoma, 5, true);
                trigger(aunisoma, 6, true);
                trigger(aunisoma, 7, true);
                trigger(aunisoma, 8, true);
                trigger(aunisoma, 9, true);
                trigger(aunisoma, 10, true);
                trigger(aunisoma, 14, true);
                trigger(aunisoma, 15, true);
                break;
            case 5000:
                trigger(aunisoma, 4, false);
                trigger(aunisoma, 5, false);
                trigger(aunisoma, 6, false);
                trigger(aunisoma, 7, false);
                trigger(aunisoma, 8, false);
                trigger(aunisoma, 9, false);
                trigger(aunisoma, 10, false);
                trigger(aunisoma, 11, false);
            case 5200:
                trigger(aunisoma, 12, false);
        }

        auto start = std::chrono::high_resolution_clock::now();

        aunisoma->event_loop();

        auto finish = std::chrono::high_resolution_clock::now();
        long long int count = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
        writeScriptLine(aunisoma, i, count, iterations);
    }

    std::cout << "]";

    return 0;
}