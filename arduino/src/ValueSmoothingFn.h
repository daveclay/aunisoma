//
// Created by Dave Clay on 6/3/25.
//

#ifndef PANELSMOOTHINGFN_H
#define PANELSMOOTHINGFN_H

class ValueSmoothingFn {
public:
    ValueSmoothingFn(int window_size);
    float get_smoothed_value(float new_value);
private:
    int window_size;
    int current_value_index;
    bool initializing;
    float values[200];
};
#endif //PANELSMOOTHINGFN_H
