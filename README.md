# Aunisoma
[Burning Man 2023 interactive sculpture](https://www.daveclay.com/burning-man-2023)
## Mockup Webpage
[Mockup](https://aftxr.com/aunisoma)

## ColorManager State Flow Diagram
![Aunisoma ColorManager Flow Diagram](https://github.com/user-attachments/assets/d62a4196-0e61-4de8-b65d-50e73889ca3e)

## Setup (macOS)

Install [PlatformIO](https://platformio.org/) via [uv](https://docs.astral.sh/uv/) — it provides one build tool for both the hardware firmware and the desktop mock.

```bash
brew install uv          # skip if you already use uv for the firmware tools below
uv tool install platformio
```

That gives you `pio` on your PATH. The Adafruit Grand Central M4 (SAMD51) toolchain is downloaded on first build.

## Sketches

Five firmware sketches share `src/` and the `PanelLink` serial-protocol module. `platformio.ini` defines one environment per sketch.

| Sketch         | Source                              | PlatformIO env              | Purpose                                                                                                          |
| -------------- | ----------------------------------- | --------------------------- | ---------------------------------------------------------------------------------------------------------------- |
| Legacy         | `Aunisoma-Sketch.cpp`               | `grandcentral_m4`           | Original gradient/reverberation/knight-rider algorithm via the `Aunisoma` class.                                 |
| Wave           | `Wave-Sketch.cpp`                   | `grandcentral_m4_wave`      | `WaveColorAlgorithm` driven directly — per-sensor color pulses that propagate.                                   |
| Glow           | `Glow-Sketch.cpp`                   | `grandcentral_m4_glow`      | `GlowColorAlgorithm` — each sensor (front and back are independent interactions with their own colors) pulses its own panel's brightness 100%→25%→100% on a random 800–1200 ms period; 200 ms fade-in, 3000 ms fall-off. A panel active from both sides intermingles the two pulses. Neighbors within 2 panels (distance and timing configurable) follow the source with a per-distance lag — brightening staggered behind it, repeating its pulse dips delayed so every pulse travels outward while the sensor stays active, and trailing its fade-out. |
| PIR test       | `PIR-Test-Sketch.cpp`               | `grandcentral_m4_pir_test`  | Bench/install diagnostic. Red idle; green = front PIR, blue = back PIR, teal = both.                             |
| Animation test | `AnimationPerformanceTest-Sketch.cpp` | `grandcentral_m4_anim_test` | Pulses every panel in lockstep from `(3,0,0)` to `(255,0,0)` to isolate loop latency vs. Serial2/master latency. |

The `build_src_filter` in `platformio.ini` picks which sketch file gets compiled, so only one `setup()`/`loop()` is linked at a time.

### Battery-life test variant

`grandcentral_m4_wave_battery` builds the Wave sketch with `-DMOCK_BATTERY_TEST=1`. It ignores real PIR readings and simulates **people walking by on a path in front of the windows**: while the path is occupied, one or more groups (up to 4) each cover a contiguous run of 1–10 windows (front sensors only, back never), so more than 10 windows can be active at once; when it clears, everything goes idle. Occupied and clear spells run from 15 seconds to 2 minutes and share the same range, so the path is occupied ~50% of the time averaged over a ~5-minute window (any single minute may be all-on or all-off). The wave algorithm then drives the LEDs from that occupancy, so the measured current is the real propagating load — not a fixed per-LED duty. Use it to estimate average draw for battery sizing; it's a power-draw test, not an animation test. `native_wave_battery` is the desktop counterpart.

## Build & install

```bash
# Legacy algorithm firmware.
pio run -e grandcentral_m4
pio run -e grandcentral_m4 -t upload

# Wave algorithm firmware.
pio run -e grandcentral_m4_wave
pio run -e grandcentral_m4_wave -t upload

# Glow algorithm firmware (per-panel pulsing, lagged neighbor ripple).
pio run -e grandcentral_m4_glow
pio run -e grandcentral_m4_glow -t upload

# Glow + loop-timing stats: prints "period/update/format/send us min/avg/max"
# over USB serial every 100 loops. Use with `pio device monitor` to measure
# the real loop period and where it goes (algorithm vs master round-trip).
pio run -e grandcentral_m4_glow_timing -t upload

# PIR test firmware (paint red / green / blue / teal from raw PIR).
pio run -e grandcentral_m4_pir_test
pio run -e grandcentral_m4_pir_test -t upload

# Animation performance test firmware (synced red pulse across all panels).
pio run -e grandcentral_m4_anim_test
pio run -e grandcentral_m4_anim_test -t upload

# Wave battery-life test firmware (mock front-side ~50% activity for power-draw testing).
pio run -e grandcentral_m4_wave_battery
pio run -e grandcentral_m4_wave_battery -t upload

# Serial monitor (any sketch).
pio device monitor
```

Use the PIR test to verify each window's panel-id mapping in `PANEL_IDS` matches the physical install: trigger one sensor at a time and confirm the window in front of you is the one that changes color. Re-flash the production firmware (`pio run -e grandcentral_m4 -t upload`) when you're done.

Use the animation performance test to isolate flicker. Every panel is sent the identical red value on every frame, so if the strip stays uniform but the pulse looks coarse, the bottleneck is the arduino loop; if panels visibly drift or tear against each other, the bottleneck lives in the Serial2 link, the master, or the master→panel fan-out.

## Run the desktop mock

The `native` envs compile a sketch against the mocks in `lib/mock-arduino/`. The binary calls `loop()` many times and writes a JSON script to stdout that the [mock HTML page](https://aftxr.com/aunisoma) replays. `native_wave`, `native_glow`, `native_pir_test`, `native_anim_test`, and `native_wave_battery` are analogous targets for the other sketches and variants.

Every native env has a `mock` target that builds the binary, runs it against `lib/mock-arduino/script.txt` (your editable scratch script — the pinned `test-validation-script.txt` is only for the test procedure), and writes `script.json` in one step:

```bash
pio run -e native_glow -t mock   # or -e native / -e native_wave / -e native_pir_test / -e native_anim_test
```

Or run the binary by hand:

```bash
pio run -e native        # or -e native_wave / -e native_glow / -e native_pir_test / -e native_anim_test
.pio/build/native/program > script.json
```

Then open `script.json` in the mock page.

# TODO
## Code
* idle should animate "pulse" occasionally
* Is 5 seconds too short for real-life human interaction? should be more like 30 seconds?
* bring back `MaxAnimation` to limit power draw
* Split thresholds into "low interactivity", "med interactivity" (color shifts), and "max interactivity" (`MaxAnimation`)
* should color transition for fewer than max interactions?


# Firmware

https://github.com/wrs/aunisma-rs

```bash
uv run python -m serial.tools.miniterm -e hwgrep://Aunisoma --eol LF
uv run ./test.py
```
