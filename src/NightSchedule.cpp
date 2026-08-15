#include "NightSchedule.h"

#ifdef ARDUINO
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
      lights_active(true),
      next_rtc_poll_ms(0) {
}

void NightSchedule::begin() {
#ifdef ARDUINO
    rtc_present = rtc.begin();
    if (rtc_present) {
        if (!rtc.initialized() || rtc.lostPower()) {
            // Never set (fresh board) or the coin cell died: seed from the
            // firmware's build timestamp so the schedule is roughly right
            // until the clock is set for real.
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
        // Clears the STOP bit; required after adjust() on a PCF8523 and
        // harmless otherwise.
        rtc.start();
    }
#endif
}

bool NightSchedule::is_active(long now_ms) {
#ifdef ARDUINO
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
