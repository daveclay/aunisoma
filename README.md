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

## Build

The Arduino source lives in `arduino/`. Both targets share `arduino/src/`; `arduino/platformio.ini` defines the two environments.

```bash
cd arduino

# Firmware for the Adafruit Grand Central M4 (SAMD51).
pio run -e grandcentral_m4

# Upload to a connected board.
pio run -e grandcentral_m4 -t upload

# Serial monitor.
pio device monitor
```

## Run the desktop mock

The `native` env compiles the same source against the mocks in `arduino/lib/mock-arduino/`. The binary calls `loop()` many times and writes a JSON script to stdout that the [mock HTML page](https://aftxr.com/aunisoma) replays.

```bash
cd arduino
pio run -e native
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
