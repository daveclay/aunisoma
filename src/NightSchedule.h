#ifndef NIGHTSCHEDULE_H
#define NIGHTSCHEDULE_H

// Wall-clock on/off schedule for the sculpture, backed by an Adafruit
// PCF8523 RTC breakout on the main I2C port (SDA/SCL, address 0x68). The
// lights run from lights_on_hour to lights_off_hour (local time, 0-23; the
// window may span midnight) and stay dark the rest of the day so the solar
// panels can charge the batteries without the LEDs competing.
//
// If no RTC answers on I2C — not wired, or the breakout died mid-burn —
// is_active() falls back to the old behavior: run fallback_runtime_limit_ms
// from power-on, then stay dark until the next power cycle.
//
// Desktop builds have no I2C; they always use the fallback path.

class NightSchedule {
public:
    NightSchedule(int lights_on_hour, int lights_off_hour, long fallback_runtime_limit_ms);
    // Probe the RTC; seed it from the firmware build timestamp if it has
    // never been set or lost battery power.
    void begin();
    bool is_active(long now_ms);

private:
    void print_status_report();

    int lights_on_hour;
    int lights_off_hour;
    long fallback_runtime_limit_ms;
    bool rtc_present;
    bool seeded_from_build_time;
    bool lights_active;
    long next_rtc_poll_ms;
    // USB serial connection state from the previous is_active() call; the
    // RTC status report prints on each false→true edge, i.e. every time a
    // monitor attaches, so it is visible no matter when the port is opened.
    bool serial_was_connected;
};

#endif //NIGHTSCHEDULE_H
