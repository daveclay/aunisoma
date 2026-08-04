#ifndef C_AUNISOMA_LOOPTIMING_H
#define C_AUNISOMA_LOOPTIMING_H

// Loop-timing instrumentation, shareable by any sketch. Build with
// -DMEASURE_LOOP_TIMING=1 (e.g. the grandcentral_m4_glow_timing env) to
// print min/avg/max microseconds over USB serial every window_size loops:
// the loop period plus each named segment. The period's min/max spread is
// the jitter that turns ms-based animation into visible jank.
//
// Usage — call unconditionally; with the flag off every method is an inline
// no-op and the compiler removes the calls entirely:
//
//   static const char* TIMING_SEGMENT_NAMES[] = {"update", "format", "send"};
//   static LoopTiming loop_timing(TIMING_SEGMENT_NAMES, 3);
//
//   void loop() {
//       loop_timing.begin_loop();
//       algorithm->update();
//       loop_timing.next_segment();   // closes "update", opens "format"
//       format_colors();
//       loop_timing.next_segment();   // closes "format", opens "send"
//       send_colors(panel_colors);
//       loop_timing.end_loop();       // closes "send", prints every window
//   }

#ifndef MEASURE_LOOP_TIMING
#define MEASURE_LOOP_TIMING 0
#endif

#if MEASURE_LOOP_TIMING

class LoopTiming {
public:
    static const int MAX_SEGMENTS = 8;

    LoopTiming(const char* const* segment_names, int segment_count, int window_size = 100);

    // Record the loop-to-loop period and open the first segment.
    void begin_loop();

    // Close the current segment and open the next.
    void next_segment();

    // Close the last segment; print and reset stats once per window.
    void end_loop();

private:
    struct Stats {
        unsigned long min_us;
        unsigned long max_us;
        unsigned long sum_us;

        void record(unsigned long duration_us);
        void reset();
    };

    const char* const* segment_names;
    int segment_count;
    int window_size;

    Stats period_stats;
    Stats segment_stats[MAX_SEGMENTS];
    unsigned long previous_loop_start_us;
    unsigned long segment_start_us;
    int current_segment;
    int sample_count;

    void _print_stats(const char* label, const Stats& stats) const;
};

#else

// Instrumentation disabled: every call inlines to nothing.
class LoopTiming {
public:
    static const int MAX_SEGMENTS = 8;
    LoopTiming(const char* const*, int, int = 100) {}
    void begin_loop() {}
    void next_segment() {}
    void end_loop() {}
};

#endif

#endif //C_AUNISOMA_LOOPTIMING_H
