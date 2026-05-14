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
    void start();
    void pause();
    void stop();
    void restart();
    void update();
    bool isPaused();
    bool isStopped();
};


#endif //C_AUNISOMA_CLOCK_H
