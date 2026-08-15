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
    int lights_on_hour;
    int lights_off_hour;
    long fallback_runtime_limit_ms;
    bool rtc_present;
    bool lights_active;
    long next_rtc_poll_ms;
};

#endif //NIGHTSCHEDULE_H
