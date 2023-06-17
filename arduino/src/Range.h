//
// Created by David Clay on 6/15/23.
//

#ifndef C_AUNISOMA_RANGE_H
#define C_AUNISOMA_RANGE_H


class Range {
public:
    int min;
    int max;

    Range(int min, int max);
    int random_int_between();

private:
    int range;
};


#endif //C_AUNISOMA_RANGE_H
