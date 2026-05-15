#ifndef C_AUNISOMA_WAVE_H
#define C_AUNISOMA_WAVE_H

#include "Clock.h"
#include "Color.h"
#include "Config.h"
#include "Sensor.h"

enum WaveState {
    WAVE_IDLE,
    WAVE_PROPAGATING
};

class Wave {
public:
    Wave(Sensor* sensor, Config* config, int origin_panel_index);

    // Called by WaveColorAlgorithm on the sensor's rising edge with a target
    // color it has already chosen (either shared with a nearby active wave
    // or picked fresh). The wave reuses this color for every pulse while
    // the sensor stays active.
    void start_activation(Color target);

    // Advance the current pulse's clock, re-pulse if the cycle completes and
    // the sensor is still active, otherwise transition to IDLE.
    void update_pulse_state();

    // Per-panel contribution at the current instant.
    //   - color: the activation's target color (constant while active).
    //   - intensity: distance falloff (linear, zero beyond
    //     config->wave_max_propagation_distance) scaled by the panel's
    //     up-down envelope position within the current pulse.
    Color get_color_for_panel(int panel_index) const;
    float get_intensity_for_panel(int panel_index) const;

    bool is_active() const;
    int get_origin_panel_index() const;
    Color get_target_color() const;

private:
    Sensor* sensor;
    Config* config;
    int origin_panel_index;
    int max_distance;

    WaveState state;

    Color target_color;
    Clock pulse_clock;

    void _start_new_pulse();
    int _pulse_total_duration_ms() const;
    float _panel_envelope_value(int panel_index) const;
};

#endif //C_AUNISOMA_WAVE_H
