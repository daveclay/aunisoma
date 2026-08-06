# Aunisoma

Large interactive sculpture for Burning Man — 20 windows over 70 feet. Each window has an RGB LED strip (0–255 per channel) and two PIR motion sensors (front + back).

This project contains the algorithm that computes each window's color from the PIR status of all windows. It runs on an **Adafruit Grand Central M4** (SAMD51) and talks over serial to a separate "master" controller running the aunisoma firmware (not in this repo), which broadcasts colors to per-window "panel" controllers and collects their PIR readings by timeslot.

PIR reply values: `0` = no motion, `1` = front, `2` = back, `3` = both.

Main loop: read sensor values from master → compute color per window → send colors back to master.

## Layout

Single source tree, built several ways via PlatformIO (`platformio.ini`):

- `src/` — shared source. Each sketch provides its own `setup()`/`loop()` pair; the `build_src_filter` in `platformio.ini` selects exactly one per env:
  - `Aunisoma-Sketch.cpp` — production sketch driving the legacy gradient/reverberation/knight-rider algorithm via the `Aunisoma` class.
  - `Wave-Sketch.cpp` — production sketch driving `WaveColorAlgorithm` directly (no `Aunisoma` wrapper).
  - `Glow-Sketch.cpp` — production sketch driving `GlowColorAlgorithm`: each sensor (front and back are independent interactions with their own colors) pulses its own panel's brightness 100%→25%→100% on a random 800–1200 ms period, 200 ms fade-in, 3000 ms fall-off. Neighbors within `glow_ripple_distance` (default 2) follow the source with a per-distance lag (`glow_ripple_start_delay_ms` to start, `glow_ripple_peak_lag_ms` at the peak, same lags trailing the fade-out) and repeat the source's pulse dips delayed by the same per-panel lag, so every pulse travels outward for as long as the sensor stays active. A panel whose front and back sensors are both active dances between the two glows' colors, one full cycle per `glow_dance_period_ms`.
  - `PIR-Test-Sketch.cpp` — wiring test that paints raw PIR state per panel.
- `main.cpp` — desktop driver; excluded from every hardware build so the Arduino framework's `main()` is used there.
- `lib/mock-arduino/` — desktop-only mocks (`Arduino.{h,cpp}`, `SPI.h`, `Adafruit_DotStar.h`, `wiring_private.h`). Gated to `platforms: native` in its `library.json`, so it is not built on hardware.

## Building

Hardware:
- `pio run -e grandcentral_m4` — legacy algorithm firmware.
- `pio run -e grandcentral_m4_wave` — wave algorithm firmware.
- `pio run -e grandcentral_m4_glow` — glow algorithm firmware.
- `pio run -e grandcentral_m4_pir_test` — PIR wiring test firmware.

Desktop mock (run the resulting binary and redirect stdout to a JSON file, e.g. `script.json`, that the mock HTML page replays):
- `pio run -e native` — legacy algorithm.
- `pio run -e native_wave` — wave algorithm.
- `pio run -e native_glow` — glow algorithm.
- `pio run -e native_pir_test` — PIR test.

Every native env also has a `mock` target (`pio run -e native_glow -t mock`)
that builds, runs the binary against `lib/mock-arduino/script.txt` (the
gitignored scratch script), and writes `script.json` in one step.

`MOCK_INTERACTIONS` is set per environment via `build_flags` and defaults to 0 in every env; flip it on at build time when you want synthetic PIR activity instead of real sensor reads.

## Testing

When asked to test the project, run this end-to-end every time. The PIR-test
JSON output is the project's correctness oracle — it has a fixed PIR→color
mapping, so any divergence is a real bug. Legacy, wave, and glow are emergent
algorithms and only get smoke-tested for crash/output sanity.

1. Build every env:

```bash
pio run -e grandcentral_m4 -e grandcentral_m4_wave -e grandcentral_m4_glow \
        -e grandcentral_m4_pir_test \
        -e native -e native_wave -e native_glow -e native_pir_test
```

2. Run each native binary against `lib/mock-arduino/test-validation-script.txt`
   (the pinned test input — never edit it; `lib/mock-arduino/script.txt` is the
   gitignored default for freeform experiments). Capture stdout and stderr per
   sketch:

```bash
for env in native native_wave native_glow native_pir_test; do
    AUNISOMA_MOCK_SCRIPT=lib/mock-arduino/test-validation-script.txt \
        .pio/build/$env/program > /tmp/aunisoma_$env.json 2> /tmp/aunisoma_$env.err
    echo "$env: exit=$? size=$(wc -c < /tmp/aunisoma_$env.json) iters=$(grep -c '\"iteration\":' /tmp/aunisoma_$env.json) stderr_lines=$(wc -l < /tmp/aunisoma_$env.err)"
done
```

Smoke-test gate (all four sketches): exit 0, 30000 iterations, empty stderr,
file ends with `]\n\t}\n]` (`tail -c 6 ... | xxd` → `5d 0a 09 7d 0a 5d`).

3. PIR test correctness check. `test-validation-script.txt` flips panel 0 to
   `1` (front) at iter 10, panel 2 to `2` (back) at iter 980, and panel 0 to
   `3` (both) at iter 920. The sketch reads the mock's PIR string one loop after it's
   returned, so the color appears at iter+1:

```bash
jq -c '.[11].panels[0], .[981].panels[2], .[921].panels[0]' /tmp/aunisoma_native_pir_test.json
```

Expected:

```
{"index":0,"red":"00","green":"ff","blue":"00"}   # front  → green
{"index":2,"red":"00","green":"00","blue":"ff"}   # back   → blue
{"index":0,"red":"00","green":"ff","blue":"ff"}   # both   → teal
```

Any idle panel at the same iterations must be `{"red":"03","green":"00","blue":"00"}`
(idle is RGB `(3,0,0)` and `gamma_lut[3] = 3`). If the PIR test diverges from
these expected values, stop and investigate before reporting the build as
working.
