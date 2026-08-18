#include "NightSchedule.h"

#ifdef ARDUINO
#include <cstdio>
#include <RTClib.h>

static RTC_PCF8523 rtc;
#endif

// Reading the RTC is an I2C round trip; once every 30 seconds is plenty for
// an hour-granularity schedule and keeps it out of the animation loop's
// timing budget.
static const long RTC_POLL_INTERVAL_MS = 30L * 1000L;

NightSchedule::NightSchedule(int lights_on_hour, int lights_on_minute,
                             int lights_off_hour, int lights_off_minute,
                             long fallback_runtime_limit_ms)
    : lights_on_minute_of_day(lights_on_hour * 60 + lights_on_minute),
      lights_off_minute_of_day(lights_off_hour * 60 + lights_off_minute),
      fallback_runtime_limit_ms(fallback_runtime_limit_ms),
      rtc_present(false),
      seeded_from_build_time(false),
      lights_active(true),
      next_rtc_poll_ms(0),
      serial_was_connected(false) {
}

void NightSchedule::begin() {
#ifdef ARDUINO
    rtc_present = rtc.begin();
    if (rtc_present) {
#ifdef RTC_FORCE_ADJUST
#ifndef RTC_FORCE_ADJUST_EPOCH
#error "RTC_FORCE_ADJUST needs RTC_FORCE_ADJUST_EPOCH from scripts/set_rtc_time.py; build the *_settime env instead of passing the flag by hand"
#endif
        // One-shot clock-recovery build: unconditionally set the RTC to the
        // build host's wall-clock time, injected as a local-time epoch by
        // scripts/set_rtc_time.py. Not __DATE__/__TIME__ — those freeze at
        // compile time of this file, so a cached object serves a stale
        // timestamp. Flash the *_settime env once, then reflash the normal
        // env — otherwise every reboot drags the clock back to this build's
        // time.
        rtc.adjust(DateTime(static_cast<uint32_t>(RTC_FORCE_ADJUST_EPOCH)));
        seeded_from_build_time = true;
#else
        if (!rtc.initialized() || rtc.lostPower()) {
            // Never set (fresh board) or the coin cell died: seed from the
            // firmware's build timestamp so the schedule is roughly right
            // until the clock is set for real.
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
            seeded_from_build_time = true;
        }
#endif
        // Clears the STOP bit; required after adjust() on a PCF8523 and
        // harmless otherwise.
        rtc.start();
    }
#endif
}

#ifdef ARDUINO
void NightSchedule::print_status_report() {
    if (!rtc_present) {
        Serial.print("NightSchedule: no PCF8523 answering on I2C; falling back to ");
        Serial.print(fallback_runtime_limit_ms / (60L * 60L * 1000L));
        Serial.println("h runtime limit from power-on");
    } else {
        if (seeded_from_build_time) {
#ifdef RTC_FORCE_ADJUST
            Serial.println("NightSchedule: settime build; clock force-set to the build host's time — reflash the normal firmware");
#else
            Serial.println("NightSchedule: PCF8523 was unset or lost power; seeded from firmware build timestamp");
#endif
        }
        DateTime now = rtc.now();
        char timestamp[32];
        std::snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
                      static_cast<int>(now.year()), static_cast<int>(now.month()),
                      static_cast<int>(now.day()), static_cast<int>(now.hour()),
                      static_cast<int>(now.minute()), static_cast<int>(now.second()));
        char schedule[48];
        std::snprintf(schedule, sizeof(schedule), "; lights on at %02d:%02d, off at %02d:%02d",
                      lights_on_minute_of_day / 60, lights_on_minute_of_day % 60,
                      lights_off_minute_of_day / 60, lights_off_minute_of_day % 60);
        Serial.print("NightSchedule: PCF8523 time ");
        Serial.print(timestamp);
        Serial.println(schedule);
    }
}
#endif

bool NightSchedule::is_active(long now_ms) {
#ifdef ARDUINO
    // Print the RTC status each time a monitor attaches. USB enumeration
    // races setup(), so a boot-time print is usually gone before pio
    // monitor can open the port; the connection edge is the reliable hook.
    // Serial.dtr() rather than bool(Serial): the latter hides a delay(10)
    // in the core, which this loop's timing budget can't afford every pass.
    bool serial_connected_now = Serial.dtr();
    if (serial_connected_now && !serial_was_connected) {
        print_status_report();
    }
    serial_was_connected = serial_connected_now;

    if (rtc_present) {
        if (now_ms >= next_rtc_poll_ms) {
            next_rtc_poll_ms = now_ms + RTC_POLL_INTERVAL_MS;
            DateTime now = rtc.now();
            int minute_of_day = now.hour() * 60 + now.minute();
            if (lights_on_minute_of_day > lights_off_minute_of_day) {
                // Window spans midnight, e.g. on at 20:30, off at 06:00.
                lights_active = minute_of_day >= lights_on_minute_of_day ||
                                minute_of_day < lights_off_minute_of_day;
            } else {
                lights_active = minute_of_day >= lights_on_minute_of_day &&
                                minute_of_day < lights_off_minute_of_day;
            }
        }
        return lights_active;
    }
#endif
    return now_ms < fallback_runtime_limit_ms;
}
