# Plan: panel-wiring test sketch

## Goal

Add a separate "test" sketch that exercises only the panel-link protocol so
we can verify wiring and PIR reporting on hardware without any color
algorithm running. Behavior:

- Every panel starts at red (idle).
- Front PIR active → that panel goes green.
- Back PIR active → that panel goes blue.
- Both PIRs active → that panel goes teal.

Re-use the existing serial protocol code (enumerate, map_panels, set_lights)
without copy-paste.

## Strategy

The cleanest split: extract the panel-link protocol out of
`Aunisoma-Sketch.cpp` into its own translation unit, then write a small
`Test-Sketch.cpp` that uses it. A second PlatformIO environment picks which
sketch file gets compiled.

```
src/
 ├─ PanelLink.{h,cpp}        (new — owns Serial2 + protocol)
 ├─ Aunisoma-Sketch.cpp      (modified — calls PanelLink)
 ├─ Test-Sketch.cpp          (new — calls PanelLink, no algorithm)
 └─ main.cpp                 (unchanged desktop driver)
```

The alternative — duplicating the protocol helpers inside `Test-Sketch.cpp`
— is rejected: ~80 lines of Serial2 setup + framing that would silently
drift. The extraction is small and self-contained.

## New files

### `src/PanelLink.h`

Owns `Serial2`, the SERCOM1 IRQ handlers, response buffer, and protocol
constants. Exposes:

```cpp
class PanelLink {
public:
    static constexpr int NUMBER_OF_PANELS = 20;
    static constexpr int SIZE_OF_COLOR = 6;

    void begin();                                  // Serial2 setup + pinPeripheral
    bool enumerate();                              // 'E' command
    bool map_panels(const char* panel_ids);        // 'M' command
    void map_panels_until_ok(const char* panel_ids); // current initializePanels loop

    // Send 20-panel color hex string. After the master responds with
    // "OK " + 20 PIR bytes, copies the 20 PIR bytes ('0'/'1'/'2'/'3')
    // into pir_out. Returns true on success.
    bool send_colors(const char* color_hex, char pir_out[NUMBER_OF_PANELS]);
};
```

### `src/PanelLink.cpp`

- Move `Uart Serial2(...)` declaration and the four `SERCOM1_x_Handler`
  shims here. These must stay at file scope to be picked up by the
  Arduino IRQ vector table — wrapping them in the class is not an option.
- Move `responseBuffer`, command-byte constants (`ENUMERATE`, `SET_STATUS`,
  `SET_LIGHTS`, `MAP_PANELS`, `TERMINATOR`), `send_command`, the
  enumerate/map helpers, and the protocol bits of `send_colors`.
- `send_colors` returns the raw 20 PIR bytes; sensor debounce + the
  `MOCK_INTERACTIONS` path move *out* of PanelLink and stay in the
  production sketch (see below). PanelLink is protocol-only.

### `src/Test-Sketch.cpp`

Stand-alone `setup()` / `loop()`. No `Aunisoma`, no `Config`, no `Sensor[]`,
no gradients.

```cpp
PanelLink link;
char panel_color_hex[PanelLink::NUMBER_OF_PANELS * PanelLink::SIZE_OF_COLOR];
char pir_readings[PanelLink::NUMBER_OF_PANELS];

constexpr char PANEL_IDS[] = "281A221B162515111220181914171F131D211E23";

// Test palette (raw 0-255, gamma applied on write).
constexpr Color IDLE_RED   { 3,   0,   0};
constexpr Color FRONT_GREEN{ 0, 255,   0};
constexpr Color BACK_BLUE  { 0,   0, 255};
constexpr Color BOTH_TEAL  { 0, 255, 255};

void setup() {
    Serial.begin(9600);
    link.begin();
    link.map_panels_until_ok(PANEL_IDS);
    // Prime every panel to red so the very first send_colors paints idle.
    write_all_panels(IDLE_RED);
}

void loop() {
    link.send_colors(panel_color_hex, pir_readings);
    for (int panel_index = 0; panel_index < PanelLink::NUMBER_OF_PANELS; panel_index++) {
        Color next;
        switch (pir_readings[panel_index]) {
            case '1': next = FRONT_GREEN; break;
            case '2': next = BACK_BLUE;   break;
            case '3': next = BOTH_TEAL;   break;
            default:  next = IDLE_RED;    break;
        }
        write_panel_hex(panel_index, next);
    }
}
```

`write_panel_hex` reuses the same `snprintf("%02x%02x%02x", gamma_lut[...])`
pattern from `Aunisoma-Sketch.cpp` so the on-wire format is identical to
production. No debouncing — raw PIR. Per `feedback_assume_sensor_legit`,
this matches what we want: active means real interaction.

The 11-hour shutdown guard from production is **not** included — the test
sketch is run interactively at the workbench, not left running on solar.

## Modified files

### `src/Aunisoma-Sketch.cpp`

- Delete `Uart Serial2`, the four SERCOM handlers, `responseBuffer`, the
  command-byte constants, `send_command`, `send_enumerate`, `map_panels`,
  `initializePanels`, and the protocol slice of `send_colors`.
- Add `PanelLink link;` at file scope.
- `setup()` calls `link.begin()` and `link.map_panels_until_ok(panel_ids)`
  in place of the current Serial2/pinPeripheral block + `initializePanels()`.
- `loop()` builds the color hex string as today, then calls
  `link.send_colors(panel_colors, pir_readings)` and walks `pir_readings`
  to drive `sensors[].update(...)`. The `MOCK_INTERACTIONS` branch stays
  here (it gates the *sensor* update, not the wire protocol).

Net: production behavior unchanged; only the protocol plumbing moves.

### `platformio.ini`

Add a third environment that swaps which sketch file is built:

```ini
[env:grandcentral_m4_test]
extends = env:grandcentral_m4
build_src_filter = +<*> -<main.cpp> -<Aunisoma-Sketch.cpp>
```

And update the existing production env's filter so the new test sketch
isn't compiled into firmware:

```ini
[env:grandcentral_m4]
...
build_src_filter = +<*> -<main.cpp> -<Test-Sketch.cpp>
```

Native env stays as-is (it builds everything but `main.cpp` provides the
entry point — having two `setup()`/`loop()` pairs would break the link).
To keep native compiling, also exclude `Test-Sketch.cpp` from native:

```ini
[env:native]
...
build_src_filter = +<*> -<Test-Sketch.cpp>
```

## Open question

`PanelLink` currently has no constructor parameters because the protocol
is fixed. If we ever need to retarget at a different baud or pin set,
those become constructor args. Not worth parameterising up front.

## Build / run

- `pio run -e grandcentral_m4` — production firmware (behavior unchanged).
- `pio run -e grandcentral_m4_test -t upload` — flash the wiring test.
- `pio run -e native` — desktop mock build (unchanged).

## Verification

1. `pio run -e grandcentral_m4` succeeds with no new warnings; behavior
   on hardware is the same as before the refactor (smoke test: panels
   light up with current wave algorithm).
2. `pio run -e grandcentral_m4_test -t upload`. Power on with no one
   in front of any window: every panel solid red.
3. Wave a hand in front of one window's front sensor only: that panel
   turns green; releases back to red after the front sensor clears.
4. Same for the back sensor: that panel turns blue.
5. Trigger both sensors on the same panel: panel turns teal.
6. Trigger neighbouring panels independently: each panel reflects only
   its own pair of sensors (no cross-talk → wiring map is correct).
7. Physically walk past each window in order, triggering one sensor
   at a time, and confirm the window that lights up is the one you're
   standing in front of. If `PANEL_IDS` is out of order relative to
   the install, the wrong window will react — which is the wiring bug
   this sketch exists to catch.
