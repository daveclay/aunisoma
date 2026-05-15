# Plan: new wave-based color interaction algorithm

## Goal

Add a new color algorithm. On sensor activation a panel picks a primary from
{R, G, B, C, M, Y} (excluding its current shown color) and a wave propagates
outward to the strip ends with distance-decaying intensity. Overlapping waves
yield color-wheel-opposite complements. Sensor deactivation fades the
wave's affected panels back to idle red (`#030000`) simultaneously.

The original `ColorManager` / `Reverberation` algorithm is preserved and
selectable at runtime. New algorithm is the default for now.

## Approach

Introduce a `ColorAlgorithm` abstraction. `Aunisoma::update()` becomes a thin
shim that delegates to whichever algorithm is currently selected. The
existing pipeline moves into `LegacyColorAlgorithm` with no behavior changes.
The new behavior goes into `WaveColorAlgorithm`.

```
Aunisoma
 ├─ Sensor[40]                       (unchanged)
 ├─ Panel[20]                        (unchanged)
 └─ ColorAlgorithm* current ──┬─ LegacyColorAlgorithm  (wraps existing code)
                              └─ WaveColorAlgorithm    (new)
```

Runtime switch mechanism is deferred — `Aunisoma::set_algorithm(int)` is
stubbed. A serial command from the master, a long-press gesture, or a
build flag can wire in later.

## Decisions (from spec discussion)

- **Per-panel pulse shape:** each panel does a full up-down animation —
  idle → attenuated target over `wave_rise_duration_ms` (600ms), then back
  to idle over `wave_fall_duration_ms` (600ms). Neighbors lag the source
  by `wave_panel_delay_ms` per panel of distance, with intensity scaled
  by linear distance falloff.
- **While sensor is active:** waves pulse continuously. When the current
  pulse completes its full up-down across the entire strip, a new pulse
  fires from the origin with a fresh target color.
- **Wave end on sensor inactive:** the in-flight pulse completes naturally
  (each panel finishes its up-down) and the wave then goes idle. No
  separate fade-out phase needed.
- **Target color pick:** uniform random from the 6 primaries, excluding
  whichever color the origin panel is currently displaying. Re-rolled on
  every new pulse.
- **Overlap rule:** color-wheel opposite of the additive sum of overlapping
  wave targets (R↔C, G↔M, B↔Y).
- **Idle color:** `#030000` (red, value 3) everywhere.
- **Existing algorithm:** keep as-is, switchable at runtime.

## New files

### `src/ColorAlgorithm.h`

Abstract base:

```cpp
class ColorAlgorithm {
public:
    virtual void update() = 0;
    virtual Color get_color_for_panel(int panel_index) const = 0;
    virtual ~ColorAlgorithm() = default;
};
```

### `src/LegacyColorAlgorithm.{h,cpp}`

Owns `ColorManager`, `Reverberation[40]`, `ValueSmoothingFn[20]`. `update()`
is the current body of `Aunisoma::update()` lifted verbatim. Zero behavior
change.

### `src/Wave.{h,cpp}`

One per sensor (40 total).

State:
- `Sensor* sensor`, `int origin_panel_index`
- `int max_distance` = `max(origin_panel_index, NUMBER_OF_PANELS - 1 - origin_panel_index)`
- `Color current_pulse_color`
- `enum {IDLE, PROPAGATING} state`
- `Clock pulse_clock` — ms since the current pulse started

`update()`:
- On inactive→active sensor edge: pick `current_pulse_color` (caller passes
  "exclude" color), reset `pulse_clock`, enter PROPAGATING.
- In PROPAGATING when the current pulse fully completes (see below):
  - if sensor still active → fire next pulse (re-pick color, reset
    `pulse_clock`).
  - if sensor now inactive → enter IDLE.

**Pulse complete** condition: `pulse_clock >= max_distance * panel_delay_ms + rise_duration_ms + fall_duration_ms` — the farthest panel has finished both its rise and its fall back to idle. This guarantees clean separation between consecutive pulses on the same wave.

`float get_intensity_for_panel(int panel_index)`:
- `distance = |panel_index - origin_panel_index|`
- `falloff = (max_distance - distance) / max_distance` (linear; tweakable)
- `panel_delay = distance * panel_delay_ms`
- `elapsed_in_panel = pulse_clock - panel_delay`
- if `elapsed_in_panel <= 0`: not yet started → intensity = 0
- if `elapsed_in_panel < rise_duration_ms`: rising → intensity = falloff × (elapsed_in_panel / rise_duration_ms)
- if `elapsed_in_panel < rise_duration_ms + fall_duration_ms`: falling → intensity = falloff × (1 − (elapsed_in_panel − rise_duration_ms) / fall_duration_ms)
- else: pulse for this panel is over → intensity = 0

`Color get_color_for_panel(int panel_index)` returns `current_pulse_color`
(no crossfade — each panel naturally returns to idle between pulses, so
the previous pulse's color is gone by the time the next pulse begins).

### `src/WaveColorAlgorithm.{h,cpp}`

Owns `Wave[40]` + per-panel display state.

Per-panel state:
- `Color displayed` (the most recent resolved output for that panel)

`update()`:
1. `waves[wave_index].update()` for each wave (handles pulse re-firing internally).
2. For each panel: collect waves where `intensity > epsilon`. Each wave
   already exposes `(color, intensity)` per panel — including its own
   previous→current crossfade. Resolve panel color:
   - **0 waves** → idle red.
   - **1 wave** → `lerp(idle, wave.color_at(panel_index), wave.intensity_at(panel_index))`.
   - **≥2 waves** → `lerp(idle, complement(wave_colors_at(panel_index)), max(intensities))`.
3. Write straight to `displayed[panel_index]`. The per-pulse crossfade lives
   in the wave (so a new pulse smoothly retargets each panel as its leading
   edge passes); no separate per-panel from/target/clock state is needed.

`get_color_for_panel(panel_index)` returns `displayed[panel_index]`.

## Color picking (random, excluding current shown)

Six primaries as constants:
```
R = (255,   0,   0)
G = (  0, 255,   0)
B = (  0,   0, 255)
C = (  0, 255, 255)
M = (255,   0, 255)
Y = (255, 255,   0)
```

When a wave starts, `WaveColorAlgorithm` passes the origin panel's currently
displayed color to `Wave::start(Color exclude)`. Classify `exclude` into the
nearest primary (by max-channel pattern), drop it from the candidate set,
pick uniformly from the remaining 5.

## Overlap rule (color-wheel opposite of A+B)

Component-wise add the overlapping wave target colors, classify the sum into
the nearest primary, return its wheel-opposite via static table:

```
R↔C   G↔M   B↔Y
```

For ≥3 overlapping waves, sum them all then take the same opposite. Small
precomputed 6×6 table covers the 2-wave case; degenerate cases (e.g. both
waves are R) fall through to "the obvious complement of that single color."

## Modified files

### `src/Aunisoma.{h,cpp}`

- Drop direct ownership of `ColorManager` / `Reverberation[]` /
  `ValueSmoothingFn[]`.
- Hold `ColorAlgorithm* current_algorithm` plus pointers to both concrete
  algorithms (so we can swap without reallocating).
- `update()` becomes:

```cpp
current_algorithm->update();
for (int panel_index = 0; panel_index < NUMBER_OF_PANELS; panel_index++)
    panels[panel_index]->color = current_algorithm->get_color_for_panel(panel_index);
```

- Add `void set_algorithm(int kind)` stub (0 = legacy, 1 = wave). Default
  to wave.

### `src/Config.{h,cpp}`

Add new wave-specific config:
- `int wave_rise_duration_ms` (default 600 — idle → attenuated target)
- `int wave_fall_duration_ms` (default 600 — attenuated target → idle)
- `int wave_panel_delay_ms` (default 150 — per panel of distance from origin)
- `int wave_inter_pulse_delay_ms` (default 0 — gap between consecutive pulses)
- `Color wave_idle_color` (default `(3, 0, 0)`)

### `src/Aunisoma-Sketch.cpp`

Pass new config values. Existing gradient setup remains untouched (legacy
algorithm still needs it).

## Verification

- `pio run -e native`, capture stdout to `script.json`, replay in mock HTML.
- Walk through:
  1. Idle holds at `#030000`.
  2. Single sensor activate → source rises to picked primary over 600ms,
     then falls back to idle over 600ms; neighbors cascade outward with
     the same up-down envelope delayed by `wave_panel_delay_ms` per panel
     of distance, with intensity scaled by linear distance falloff.
  3. The wave visibly travels across the strip with panels lighting up and
     going dark as the pulse sweeps past.
  4. Sensor stays active → after the current pulse fully completes, a
     new pulse with a different color starts from origin and sweeps
     outward again. Repeats until the sensor goes inactive.
  5. Sensor deactivate → in-flight pulse completes naturally; no separate
     fade-out phase.
  6. Two sensors at opposite ends → waves propagate inward, overlap band
     shows wheel-opposite color; each side keeps re-pulsing independently.
  7. Same sensor toggled twice → second activation picks a different
     target than the first.
- Confirm legacy algorithm path still works when `set_algorithm(0)` is
  invoked (a temporary `#define` at top of `setup()` is sufficient for the
  smoke test).

## Open question

Inter-pulse gap: should the next pulse fire the instant the previous one
finishes (continuous), or wait a short configurable delay first? A small
gap (~500ms) makes individual color changes more readable; zero gap keeps
the strip in constant motion.

Default plan: zero gap (continuous). Add a `wave_inter_pulse_delay_ms`
config knob defaulting to 0 so we can tune later without code changes.
