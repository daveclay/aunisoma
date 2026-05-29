# Aunisoma

Large interactive sculpture for Burning Man — 20 windows over 70 feet. Each window has an RGB LED strip (0–255 per channel) and two PIR motion sensors (front + back).

This project contains the algorithm that computes each window's color from the PIR status of all windows. It runs on an **Adafruit Grand Central M4** (SAMD51) and talks over serial to a separate "master" controller running the aunisoma firmware (not in this repo), which broadcasts colors to per-window "panel" controllers and collects their PIR readings by timeslot.

PIR reply values: `0` = no motion, `1` = front, `2` = back, `3` = both.

Main loop: read sensor values from master → compute color per window → send colors back to master.

## Layout

Single source tree, built several ways via PlatformIO (`platformio.ini`):

- `src/` — shared source. Three sketches each provide their own `setup()`/`loop()` pair; the `build_src_filter` in `platformio.ini` selects exactly one per env:
  - `Aunisoma-Sketch.cpp` — production sketch driving the legacy gradient/reverberation/knight-rider algorithm via the `Aunisoma` class.
  - `Wave-Sketch.cpp` — production sketch driving `WaveColorAlgorithm` directly (no `Aunisoma` wrapper).
  - `PIR-Test-Sketch.cpp` — wiring test that paints raw PIR state per panel.
- `main.cpp` — desktop driver; excluded from every hardware build so the Arduino framework's `main()` is used there.
- `lib/mock-arduino/` — desktop-only mocks (`Arduino.{h,cpp}`, `SPI.h`, `Adafruit_DotStar.h`, `wiring_private.h`). Gated to `platforms: native` in its `library.json`, so it is not built on hardware.

## Building

Hardware:
- `pio run -e grandcentral_m4` — legacy algorithm firmware.
- `pio run -e grandcentral_m4_wave` — wave algorithm firmware.
- `pio run -e grandcentral_m4_pir_test` — PIR wiring test firmware.

Desktop mock (run the resulting binary and redirect stdout to a JSON file, e.g. `script.json`, that the mock HTML page replays):
- `pio run -e native` — legacy algorithm.
- `pio run -e native_wave` — wave algorithm.
- `pio run -e native_pir_test` — PIR test.

`MOCK_INTERACTIONS` is set per environment via `build_flags` and defaults to 0 in every env; flip it on at build time when you want synthetic PIR activity instead of real sensor reads.
