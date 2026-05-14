//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_CLOCK_H
#define C_AUNISOMA_CLOCK_H


class Clock {
public:
    bool running;
    unsigned long elapsed_ms;

    Clock();
    void start();
    void stop();
    void restart();
    void update();
    bool isStopped() const;

    // Shift elapsed_ms to the given value while running. Used by Cycle and
    // Interpolation to mirror the phase of a wave (rewind/fast-forward).
    void shift_to(unsigned long new_elapsed_ms);

private:
    unsigned long start_ms;
    bool started;
};


#endif //C_AUNISOMA_CLOCK_H
