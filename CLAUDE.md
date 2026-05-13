# Aunisoma

Large interactive sculpture for Burning Man — 20 windows over 70 feet. Each window has an RGB LED strip (0–255 per channel) and two PIR motion sensors (front + back).

This project contains the algorithm that computes each window's color from the PIR status of all windows. It runs on an **Adafruit Grand Central M4** (SAMD51) and talks over serial to a separate "master" controller running the aunisoma firmware (not in this repo), which broadcasts colors to per-window "panel" controllers and collects their PIR readings by timeslot.

PIR reply values: `0` = no motion, `1` = front, `2` = back, `3` = both.

Main loop: read sensor values from master → compute color per window → send colors back to master.

## Layout

Single source tree under `arduino/`, built two ways via PlatformIO (`arduino/platformio.ini`):

- `arduino/src/` — shared source. `Aunisoma-Sketch.cpp` holds `setup()`/`loop()`; `main.cpp` is the desktop driver (excluded from the hardware build).
- `arduino/lib/mock-arduino/` — desktop-only mocks (`Arduino.{h,cpp}`, `SPI.h`, `Adafruit_DotStar.h`, `wiring_private.h`). Gated to `platforms: native` in its `library.json`, so it is not built on hardware.

## Building

- `pio run -e grandcentral_m4` — firmware build.
- `pio run -e native` — desktop mock build. Run the resulting binary and redirect stdout to a JSON file (e.g. `script.json`) that the mock HTML page replays.

`MOCK_INTERACTIONS` is set per environment via `build_flags` (1 for native, 0 for hardware).
