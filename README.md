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

Three firmware sketches share `src/` and the `PanelLink` serial-protocol module. `platformio.ini` defines one environment per sketch.

| Sketch   | Source                | PlatformIO env             | Purpose                                                                              |
| -------- | --------------------- | -------------------------- | ------------------------------------------------------------------------------------ |
| Legacy   | `Aunisoma-Sketch.cpp` | `grandcentral_m4`          | Original gradient/reverberation/knight-rider algorithm via the `Aunisoma` class.     |
| Wave     | `Wave-Sketch.cpp`     | `grandcentral_m4_wave`     | `WaveColorAlgorithm` driven directly — per-sensor color pulses that propagate.       |
| PIR test | `PIR-Test-Sketch.cpp` | `grandcentral_m4_pir_test` | Bench/install diagnostic. Red idle; green = front PIR, blue = back PIR, teal = both. |

The `build_src_filter` in `platformio.ini` picks which sketch file gets compiled, so only one `setup()`/`loop()` is linked at a time.

## Build & install

```bash
# Legacy algorithm firmware.
pio run -e grandcentral_m4
pio run -e grandcentral_m4 -t upload

# Wave algorithm firmware.
pio run -e grandcentral_m4_wave
pio run -e grandcentral_m4_wave -t upload

# PIR test firmware (paint red / green / blue / teal from raw PIR).
pio run -e grandcentral_m4_pir_test
pio run -e grandcentral_m4_pir_test -t upload

# Serial monitor (any sketch).
pio device monitor
```

Use the PIR test to verify each window's panel-id mapping in `PANEL_IDS` matches the physical install: trigger one sensor at a time and confirm the window in front of you is the one that changes color. Re-flash the production firmware (`pio run -e grandcentral_m4 -t upload`) when you're done.

## Run the desktop mock

The `native` envs compile a sketch against the mocks in `lib/mock-arduino/`. The binary calls `loop()` many times and writes a JSON script to stdout that the [mock HTML page](https://aftxr.com/aunisoma) replays. `native_wave` and `native_pir_test` are analogous targets for the other two sketches.

```bash
pio run -e native        # or -e native_wave / -e native_pir_test
.pio/build/native/program > script.json

pio run -e native_wave
.pio/build/native_wave/program > script.json
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
