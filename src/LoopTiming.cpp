#include "LoopTiming.h"

#if MEASURE_LOOP_TIMING

#include "Arduino.h"

LoopTiming::LoopTiming(const char* const* segment_names, int segment_count, int window_size) {
    this->segment_names = segment_names;
    this->segment_count = segment_count;
    if (this->segment_count > MAX_SEGMENTS) this->segment_count = MAX_SEGMENTS;
    if (this->segment_count < 0) this->segment_count = 0;
    this->window_size = window_size < 1 ? 1 : window_size;

    this->period_stats.reset();
    for (int segment_index = 0; segment_index < MAX_SEGMENTS; segment_index++) {
        this->segment_stats[segment_index].reset();
    }
    this->previous_loop_start_us = 0;
    this->segment_start_us = 0;
    this->current_segment = 0;
    this->sample_count = 0;
}

void LoopTiming::begin_loop() {
    unsigned long loop_start_us = micros();
    if (this->previous_loop_start_us != 0) {
        this->period_stats.record(loop_start_us - this->previous_loop_start_us);
    }
    this->previous_loop_start_us = loop_start_us;
    this->current_segment = 0;
    this->segment_start_us = micros();
}

void LoopTiming::next_segment() {
    unsigned long now_us = micros();
    if (this->current_segment < this->segment_count) {
        this->segment_stats[this->current_segment].record(now_us - this->segment_start_us);
    }
    this->current_segment++;
    this->segment_start_us = now_us;
}

void LoopTiming::end_loop() {
    this->next_segment();

    this->sample_count++;
    if (this->sample_count < this->window_size) {
        return;
    }

    this->_print_stats("period", this->period_stats);
    for (int segment_index = 0; segment_index < this->segment_count; segment_index++) {
        this->_print_stats(this->segment_names[segment_index], this->segment_stats[segment_index]);
    }

    this->period_stats.reset();
    for (int segment_index = 0; segment_index < this->segment_count; segment_index++) {
        this->segment_stats[segment_index].reset();
    }
    this->sample_count = 0;
}

void LoopTiming::_print_stats(const char* label, const Stats& stats) const {
    Serial.print(label);
    Serial.print(" us min/avg/max: ");
    Serial.print(stats.min_us);
    Serial.print("/");
    Serial.print(stats.sum_us / static_cast<unsigned long>(this->window_size));
    Serial.print("/");
    Serial.println(stats.max_us);
}

void LoopTiming::Stats::record(unsigned long duration_us) {
    if (duration_us < this->min_us) this->min_us = duration_us;
    if (duration_us > this->max_us) this->max_us = duration_us;
    this->sum_us += duration_us;
}

void LoopTiming::Stats::reset() {
    this->min_us = 0xFFFFFFFFUL;
    this->max_us = 0;
    this->sum_us = 0;
}

#endif
