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

NightSchedule::NightSchedule(int lights_on_hour, int lights_off_hour, long fallback_runtime_limit_ms)
    : lights_on_hour(lights_on_hour),
      lights_off_hour(lights_off_hour),
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
        // One-shot clock-recovery build: unconditionally set the RTC to this
        // firmware's build timestamp. Flash once with this flag when the
        // clock is wrong, then reflash without it — otherwise every reboot
        // drags the clock back to this build's time.
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
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
            Serial.println("NightSchedule: PCF8523 was unset or lost power; seeded from firmware build timestamp");
        }
        DateTime now = rtc.now();
        char timestamp[24];
        std::snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
                      static_cast<int>(now.year()), static_cast<int>(now.month()),
                      static_cast<int>(now.day()), static_cast<int>(now.hour()),
                      static_cast<int>(now.minute()), static_cast<int>(now.second()));
        Serial.print("NightSchedule: PCF8523 time ");
        Serial.print(timestamp);
        Serial.print("; lights on at ");
        Serial.print(lights_on_hour);
        Serial.print(":00, off at ");
        Serial.print(lights_off_hour);
        Serial.println(":00");
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
            int hour_of_day = rtc.now().hour();
            if (lights_on_hour > lights_off_hour) {
                // Window spans midnight, e.g. on at 20:00, off at 07:00.
                lights_active = hour_of_day >= lights_on_hour || hour_of_day < lights_off_hour;
            } else {
                lights_active = hour_of_day >= lights_on_hour && hour_of_day < lights_off_hour;
            }
        }
        return lights_active;
    }
#endif
    return now_ms < fallback_runtime_limit_ms;
}
