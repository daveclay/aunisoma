//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_REVERBERATION_H
#define C_AUNISOMA_REVERBERATION_H

#include "Cycle.h"
#include "Config.h"
#include "Pulse.h"
#include "Sensor.h"

class Reverberation {
public:
    Reverberation(Sensor* sensor, Config* config, int panel_index);

    Sensor* sensor;
    Config* config;
    int panel_index;
    int first_panel_index;
    int last_panel_index;
    bool active;
    int max_distance;
    int delay;
    bool delay_complete;
    int distance;
    Clock* delay_clock;
    Pulse* pulses[5];

    void update();
    void start();
    void stop();
    float get_panel_value_for_panel_index(int panel_index);

private:
    void _restart_pulses();
    bool _is_delay_complete();
    void _calculate_new_distance();
};


#endif //C_AUNISOMA_REVERBERATION_H
