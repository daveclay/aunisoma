import time

Import("env")

# Bake the build host's current wall-clock time into the firmware so the
# RTC_FORCE_ADJUST path sets the PCF8523 to real time. __DATE__/__TIME__ are
# useless for this: they freeze at compile time of NightSchedule.cpp, and a
# cached object file carries that stale timestamp through any number of
# rebuilds. This define's value changes every build, which both delivers the
# current time and forces the recompile that makes it take effect.
#
# tm_gmtoff shifts the unix epoch (UTC) to local wall-clock time; the RTC
# stores local time, matching the schedule's local on/off hours.
local_epoch = int(time.time()) + time.localtime().tm_gmtoff
env.Append(CPPDEFINES=[
    "RTC_FORCE_ADJUST",
    ("RTC_FORCE_ADJUST_EPOCH", "%dUL" % local_epoch),
])
print("set_rtc_time: firmware will set the RTC to %s (local epoch %d)"
      % (time.strftime("%Y-%m-%d %H:%M:%S"), local_epoch))
