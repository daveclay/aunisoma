# Plan: Colorful wave overlap via chroma-vector summation with hue rotation on overshoot

## Goal

Replace the current intensity-weighted HSV averaging in `WaveColorAlgorithm`
(WaveColorAlgorithm.cpp:175-240) with a chroma-vector summation model so that
overlapping waves *combine* into new colors rather than just brightening or
gray-cancelling. When the combined chroma magnitude exceeds 1.0 (i.e.
saturation would otherwise clamp), the hue rotates around the color wheel by
an amount proportional to the overshoot. This gives the sculpture a richer
palette under dense interaction without changing how a single interaction
picks its target color.

## What stays the same

- `_pick_target_for_activation` (WaveColorAlgorithm.cpp:262-297): rising-edge
  detection, neighbor-color sharing, and the +1/20 hue shift for chained
  adjacents all stay. Each wave still has one well-defined target color for
  its lifetime.
- Per-wave envelope and spatial falloff in `Wave::get_intensity_for_panel`
  (Wave.cpp:45-66) stays.
- Single-wave path (one contributing wave) still lerps from `idle_color`
  toward the wave's target in RGB via
  `idle_color.interpolate(wave_target, wave_intensity)` so a lone wave fades
  in/out exactly as it does today.
- Rainbow override crossfade (WaveColorAlgorithm.cpp:137-161, 242-251) stays.
- The existing low-coherence gray fallback is **removed**. When hues
  cancel (e.g. red + cyan at equal intensity), instead of collapsing to
  gray we keep rotating around the wheel — see Step 3 below. The goal of
  this change is "more colorful, never gray," so falling back to neutral
  contradicts the spirit of the TODO.

## What changes — overlap blend in three steps

For each panel, iterate active waves and accumulate a chroma vector in the
unit disk where angle = hue * 2π and radius = wave_intensity. All target
colors from `_pick_target_for_activation` are saturation 1.0 and there is
no plan to change that, so the weight is just intensity — no per-wave
saturation factor.

### Step 1: Sum chroma vectors

For each contributing wave at this panel:

```
chroma_x_sum += cos(target_hue * 2π) * wave_intensity
chroma_y_sum += sin(target_hue * 2π) * wave_intensity
intensity_weight_sum += wave_intensity        // total chromatic "presence"
max_intensity = max(max_intensity, wave_intensity)
if (wave_intensity > strongest_intensity_so_far) {
    strongest_intensity_so_far = wave_intensity
    strongest_target_hue = target_hue   // stable reference for the
                                        // hue-cancellation case
}
```

Note the new `strongest_target_hue` accumulator — when the chroma sum
collapses (opposite hues cancelling), the chroma direction is unstable, so
we anchor the rotation to the single brightest contributing wave's target
hue instead.

### Step 2: Derive the rotation driver and base hue

```
chroma_magnitude = sqrt(chroma_x_sum² + chroma_y_sum²)
coherence = intensity_weight_sum > 0
          ? chroma_magnitude / intensity_weight_sum
          : 0

if (coherence >= coherence_fallback_threshold) {
    // Chroma direction is well-defined: use it.
    base_hue = atan2(chroma_y_sum, chroma_x_sum) / (2π)
    effective_magnitude = chroma_magnitude
} else {
    // Hues cancel — phase angle is numerically unstable. Anchor to the
    // brightest contributing wave's target hue and drive rotation off
    // total chromatic presence instead. Two opposite waves at full
    // intensity now produce a rotated saturated color rather than gray.
    base_hue = strongest_target_hue
    effective_magnitude = intensity_weight_sum
}
```

`coherence` keeps its current role as a numerical-stability check on
`atan2`, *not* as a "should we colorize" gate. Below the threshold we
still produce a saturated, rotating color.

### Step 3: Saturate, then rotate

```
float overshoot = max(0.0f, effective_magnitude - 1.0f)
resolved_hue = wrap_to_unit(
    base_hue + overshoot * wave_overlap_hue_rotation_per_unit
)
// Saturation is always 1.0 in the ≥2-wave path. Tying it to
// chroma_magnitude lets low-intensity overlap collapse to gray (two faint
// same-hue waves at the edge of their range produce a near-zero chroma
// sum), which contradicts "always colorful". Wave-edge fade is already
// handled by resolved_value via max_intensity.
resolved_saturation = 1.0f

// Value (brightness) keeps the current "max contributing intensity, plus
// a small idle floor so single-wave brightness matches" rule.
resolved_value = idle_value * (1.0 - max_intensity) + max_intensity
```

`effective_magnitude` drives rotation only — not saturation.

`wrap_to_unit(x)` is `x - floor(x)`, then `+1` if negative — the same pattern
already used in `_rainbow_color_for_panel` (WaveColorAlgorithm.cpp:316-318).

Note the symmetry: whether the magnitude comes from aligned chroma sum
(coherent) or from raw intensity total (cancelling), the rotation formula
is the same. Two reds at intensity 1.0 and red+cyan at intensity 1.0 both
end up with effective_magnitude = 2, saturation clamped to 1.0, hue
rotated by `wave_overlap_hue_rotation_per_unit`. The difference is which
base_hue they start from (aligned: the shared hue; cancelling: the
strongest wave's hue).

### Why this satisfies the goal

- One wave anywhere: magnitude ≤ 1, no rotation. Identical to today (the
  single-wave path doesn't even reach this branch).
- Two saturated waves of the *same* target hue at full intensity: magnitude
  ≈ 2, overshoot ≈ 1, hue rotates by `wave_overlap_hue_rotation_per_unit`.
  So two overlapping reds slide toward orange instead of staying red. This
  is the colorful behavior the TODO is asking for.
- Three same-hue waves: magnitude ≈ 3, overshoot ≈ 2, twice the rotation.
- Two waves of *adjacent* hues (red + orange): the chroma vectors mostly
  add, magnitude approaches but doesn't exceed 1 by much, small rotation —
  smooth gradient mixing.
- Two waves of *opposite* hues at full intensity (red + cyan): chroma
  vectors cancel → coherence drops below threshold → base_hue snaps to
  the brightest wave's target (say red), effective_magnitude =
  intensity_weight_sum = 2, overshoot = 1, hue rotates one step from
  red. Saturated rotated color instead of gray. Tie-breaking on
  equal-intensity falls to the first-seen wave (lowest sensor index),
  which is stable across ticks because sensor iteration order is fixed.
- Saturation never clamps "hard": at effective_magnitude = 1 the
  rotation rule kicks in continuously, so there is no visible
  discontinuity at the threshold.

## Why vectors (per the TODO's suggestion)

The TODO floats the idea of summing as vectors so values can exceed 255 and
then translate back. That is exactly what chroma-vector sum does — but in
the hue plane rather than in RGB. Pure RGB addition (sum-then-clamp) does
*not* solve the problem: two saturated red waves give (510, 0, 0) which
clamps right back to red. The 2D chroma plane has the property we want —
"more of the same color" has a well-defined direction (magnitude grows
along the hue axis), and "more than a unit's worth" can be naturally
re-interpreted as a rotation. RGB addition has no such direction; clamping
just throws information away.

## New config fields (Config.h, Wave-Sketch.cpp init block)

```cpp
// Fraction of the color wheel to rotate per unit of chroma-magnitude
// overshoot above 1.0. 1/12 = 30° per unit, so two same-hue saturated waves
// rotate hue by ~30°. Smaller values keep overlapping waves close to the
// dominant hue; larger values produce more dramatic color shifts.
float wave_overlap_hue_rotation_per_unit;

// Coherence ratio (chroma_magnitude / intensity_weight_sum) below which
// we treat the chroma direction as numerically unstable and anchor to
// the strongest contributing wave's hue instead. Not a "go gray" gate
// any more — both branches produce a saturated color, this just picks
// which base_hue is used.
float wave_overlap_coherence_fallback_threshold;
```

Suggested defaults in `Wave-Sketch.cpp`:

```cpp
config.wave_overlap_hue_rotation_per_unit = 1.0f / 12.0f;   // 30°/unit
config.wave_overlap_coherence_fallback_threshold = 0.1f;
```

Both are tuning knobs and expected to be adjusted on-hardware. The rotation
rate is the main creative dial: small (1/20–1/12) keeps the palette
recognizable, large (1/6–1/4) makes dense overlap visibly cycle through
the wheel.

## Files touched

- `src/Config.h`: add the two fields above to the wave parameters block
  alongside `wave_rainbow_*`.
- `src/Config.cpp`: add to the initializer list with sentinel 0.0f (no
  defaults; the sketch owns the values).
- `src/Wave-Sketch.cpp`: set the two new fields in the existing wave-config
  block (around line 100).
- `src/WaveColorAlgorithm.cpp`: rewrite the ≥2-wave branch
  (lines 209-240) per Steps 2 and 3. Add `strongest_intensity_so_far`
  and `strongest_target_hue` to the per-panel accumulator block. The
  `saturation_weighted_sum` accumulator and `mean_target_saturation`
  derived value go away — saturation now comes from
  `effective_magnitude` directly. The `coherence < 0.1f` branch that
  zeros out saturation is replaced by the strongest-hue-anchor logic.
- No header changes to `WaveColorAlgorithm.h` (the private helpers stay).

## Testing

Follow the CLAUDE.md end-to-end test routine (all six envs build, all
three native sketches run to 30000 iterations against `script.txt`, PIR
test correctness asserts pass unchanged).

Wave-specific manual check via the desktop mock:

1. Run `.pio/build/native_wave/program > /tmp/wave_overlap.json`.
2. With `MOCK_INTERACTIONS=1` or by editing `script.txt` to fire two
   adjacent sensors on the same panel chain at the same iteration, scrub
   the resulting JSON in the mock HTML viewer. Expected visual:
   - Two same-color overlapping waves should visibly shift hue at the
     overlap peak instead of just brightening.
   - A single isolated wave looks identical to before.
   - Three+ overlapping waves continue rotating proportionally.
3. Set `wave_overlap_hue_rotation_per_unit = 0.0f` and confirm the
   output collapses to (approximately) today's behavior — a regression
   guard that proves the new branch is only active on overshoot.

No unit tests in this repo today; the JSON-replay viewer is the oracle
for wave/legacy.

## Out of scope

- Changing `_pick_target_for_activation` (TODO explicitly preserves it).
- RGB-additive blending. Discussed above; doesn't achieve the goal.
- Touching the legacy algorithm. Different sketch, different code path.
