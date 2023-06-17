//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_CLOCK_H
#define C_AUNISOMA_CLOCK_H


class Clock {
public:
    bool running;
    int ticks;

    Clock();
    void stop();
    void start();
    void restart();
    void next();
};


#endif //C_AUNISOMA_CLOCK_H
