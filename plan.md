# Plan: Migrate Timer/Cycle from ticks to milliseconds

## Goal

Express animation durations in ms instead of "loop iterations." Today a "tick"
is one `loop()` call, so animation speed drifts with how much work the loop is
doing (see comment at `src/Aunisoma-Sketch.cpp:272-273`: "300 ticks in 1m 12s
while doing knight rider. 240ms per tick. Faster when idle, though?"). Moving to
wall-clock time makes timing independent of loop variance.

## The core change: Clock becomes millis-based

`Clock` is the only place that needs to know about time. Everything above it
(Timer, Cycle, Pulse, Reverberation, Interpolation) just needs the math
re-expressed against an `elapsed_ms` accumulator instead of a `ticks` counter.

### New Clock shape

```cpp
class Clock {
public:
    bool running;
    unsigned long elapsed_ms;   // 0 when stopped, else millis since last start

    Clock();
    void start();
    void stop();       // reset elapsed_ms to "unstarted" sentinel
    void restart();    // stop + start; elapsed_ms goes to 0
    void update();     // refresh elapsed_ms from millis()
    bool isStopped() const;

    void shift_to(unsigned long ms);       // for Cycle/Interpolation time-warp

private:
    unsigned long start_ms;          // wall-clock at last (re)start, 0 if stopped
};
```

**Drop `pause()` and `isPaused()`.** Both are defined in `Clock.{h,cpp}` but
never called anywhere in the codebase (verified by grep). Today's pause
semantics — freeze ticks but preserve them for resume — would translate to
extra `accumulated_ms` bookkeeping under the millis model. Since nothing
needs it, just delete the methods rather than carry the complexity.

`restart()` stays — it's heavily used by Timer, Cycle, Reverberation, and
Interpolation.

Use `unsigned long` throughout — `millis()` returns `unsigned long` and rolls
over after ~49 days. The subtraction `millis() - start_ms` is rollover-safe in
unsigned arithmetic.

Keep `ticks` as a deprecated alias? No — there are only a handful of call
sites; rename to `elapsed_ms` everywhere in one pass.

## Cycle / Timer math changes

The position-within-cycle math at `src/Cycle.cpp:86-96` and
`src/Timer.cpp:57-58` is unit-agnostic — just rename `duration_ticks` →
`duration_ms` and `clock->ticks` → `clock->elapsed_ms`.

**The one subtle break** is completion detection at `src/Cycle.cpp:73` and
`src/Timer.cpp:49`:

```cpp
if (clock->ticks > 0 && clock->ticks % duration_ticks == 0)
```

This works today because ticks increment by exactly 1, so the modulo lands on
the boundary every cycle. With ms, elapsed jumps in chunks (e.g. 98ms → 117ms)
and skips the boundary entirely. Replace with a crossing check:

```cpp
unsigned long new_iteration = clock->elapsed_ms / duration_ms;
if (new_iteration > last_iteration_seen) {
    iterations += new_iteration - last_iteration_seen;
    last_iteration_seen = new_iteration;
    // ... handle one_shot / release_phase
}
```

Store `last_iteration_seen` as new state on Cycle and Timer.

## Cycle's time-warp on start() and release()

`src/Cycle.cpp:23-26` and `:46-55` mutate `clock->ticks` directly to mirror the
phase across the wave (UP_DOWN). In the ms model, "rewinding ticks by N"
becomes "shifting elapsed_ms by N" — same logic, different field. Wrap it in
`Clock::shift_to(new_elapsed_ms)` so Cycle doesn't have to know about the
internal start_ms/accumulated_ms bookkeeping.

## Interpolation's tick mutation

`src/Interpolation.cpp:25` reverses the timer by computing
`duration_ticks - clock->ticks`. Same translation: use `Clock::shift_to`.

## Reverberation reaches into ticks directly

`src/Reverberation.cpp:73,143` compares `delay_clock->ticks` to
`config->reverberation_panel_delay_ticks` (currently `3`). With ms, this 3ms
delay is too short to be perceptible. Two options:

- **(preferred)** Rename `reverberation_panel_delay_ticks` →
  `reverberation_panel_delay_ms` and rescale to ~30–60ms (using the
  240ms/tick figure: 3 ticks ≈ 720ms feels too long; aim for the original
  *intent* which was "a few loop iterations" ≈ 60–100ms).
- Keep Clock generic and let Reverberation track its own delay internally.

Go with the first — it's a straight rename + rescale and keeps Clock simple.

## Config rescaling

Using ~240ms/tick from the existing comment as a starting point. All these are
currently in ticks; the names should change to `_ms` and values rescale:

| Field | Now (ticks) | After (ms, approx) |
|---|---|---|
| `single_panel_pulse_duration` | 20–40 | 4800–9600 |
| `reverberation_panel_delay_ticks` | 3 | 720 |
| `default_gradient_delay_duration_range` | 1500–2000 | 360000–480000 (6–8 min) |
| `no_interaction_knight_rider_delay_range` | 6000–12000 | 1440000–2880000 (24–48 min) |
| `delay_for_gradient_transition_duration` | 4000 | 960000 (16 min) |
| `gradient_transition_animation_duration` | 100 | 24000 (24 s) |
| `watchdog_state_duration_limit_ticks` | 5000 | 1200000 (20 min) |
| `initial_trigger_panel_animation_loop_duration_ticks` | unknown | rescale by 240× |

**Don't touch:**
- `smoothing_fn_window_size` (10) — this is a sample count, not a duration.
- The `*_debounce_ms` fields — already in ms, already correct.

These conversions are rough — the loop time figure is variable and old. After
the migration is mechanically correct, do a live tuning pass on the values
that actually drive the visible animation (pulse duration first, then
reverberation delay, then the longer cooldowns).

## Step-by-step

1. **Rewrite `Clock`** to be millis-based with the new shape above. Add
   `shift_to(ms)` helper. Keep the public surface (`start/pause/stop/restart/
   update/isPaused/isStopped`) the same, rename `ticks` → `elapsed_ms`.

2. **Update `Timer`**:
   - Rename `duration_ticks` → `duration_ms` (field + ctor param + `restart(int)`).
   - Replace `% duration` completion check with the crossing check; add
     `last_iteration_seen` state.
   - The `current_value` formula at `Timer.cpp:57-58` is unchanged.

3. **Update `Cycle`**:
   - Same rename `duration_ticks` → `duration_ms`.
   - Same crossing-check replacement for the iteration boundary.
   - Replace direct `clock->ticks =` mutations in `start()` and `release()`
     with `clock->shift_to(...)`.
   - `isRising`, `isAtZeroPoint`, and the UP_DOWN piecewise math are
     unit-agnostic — just rename the field.

4. **Update `Interpolation`** at `Interpolation.cpp:25` to use
   `clock->shift_to(duration_ms - clock->elapsed_ms)`.

5. **Update `Reverberation`**:
   - Read `delay_clock->elapsed_ms` instead of `delay_clock->ticks`.
   - Rename `reverberation_panel_delay_ticks` → `reverberation_panel_delay_ms`
     in Config and Aunisoma-Sketch.

6. **Rescale Config values** in `src/Aunisoma-Sketch.cpp` setup. Rename the
   fields that change unit; leave debounce fields alone.

7. **Build both environments** (`pio run -e grandcentral_m4` and
   `pio run -e native`) and fix any compile errors from renames.

8. **Run the native mock**, capture `script.json`, replay in the HTML viewer.
   Visually compare to baseline (a `script.json` captured before the change).
   Animations should look the same — if anything, *more* consistent because
   they're no longer coupled to loop time.

9. **Tuning pass** on the actual hardware. Likely candidates to adjust:
   - `single_panel_pulse_duration` — the 240ms/tick assumption is rough.
   - `reverberation_panel_delay_ms` — visual wave spacing.
   - The long cooldowns can stay roughly where the conversion lands; they're
     not perceptually precise.

## Open questions / risks

- **`millis()` rollover at 49 days.** The hardware runs 11 hours/day max
  (`ACTIVE_RUNTIME_LIMIT_MS` at `Aunisoma-Sketch.cpp:320`), so rollover is
  not a practical concern. Use `unsigned long` math anyway — it's free.

- **Pure-rename vs. dual-unit.** This plan does a hard rename and rescale. An
  alternative is to keep `duration_ticks` as a tick-equivalent value and
  internally treat it as ms, but that's more confusing than just renaming.
  Hard rename is cleaner.

- **Tuning is unavoidable.** The 240ms/tick figure is from one observation
  while running knight-rider. Anything visual will need a tuning pass on real
  hardware after the mechanical migration lands.
