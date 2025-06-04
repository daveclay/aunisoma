//
// Created by Dave Clay on 6/3/25.
//

#ifndef DEBOUNCE_H
#define DEBOUNCE_H

class Debounce {
public:
    Debounce(int debounce_ticks);
    bool update(bool new_reading);
private:
    bool reading;
    long last_debounce_time;
    bool previous_reading;
    int debounce_ticks;
};
#endif //DEBOUNCE_H
