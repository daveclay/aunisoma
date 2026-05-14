# Apparent "jump" from #010001 to #010000 at tick 3104 → 3105

## Symptom (post-fix)

After the `interpolateValue` fix, the cross-fade from `purple_red_gradient`
back to `initial_gradient` looks like three discrete steps rather than a
smooth fade. Panel 0 across the entire 100-tick transition:

```
3054–3104: #010001   (50 ticks)
3105–3128: #010000   (24 ticks)
3129–3154: #020000   (26 ticks)
```

This is not a bug — the transition is mathematically continuous over all 100
ticks, but integer + gamma quantization at these brightness levels hide most
of the motion and concentrate all the visible change at three threshold
crossings.

## What's actually happening

Transition is `(1, 0, 1) → (3, 0, 0)` over 100 ticks, so `a` advances by
0.01 per tick from 3054. With the fixed `interpolateValue`:

- raw red  = `lround(1·(1−a) + 3·a) = lround(1 + 2a)`
- raw blue = `lround(1·(1−a))       = lround(1 − a)`

| `a` range     | raw red | raw blue | raw color | gamma output |
|---------------|---------|----------|-----------|--------------|
| [0, 0.25)     | 1       | 1        | (1,0,1)   | `#010001`    |
| [0.25, 0.5]   | 2       | 1        | (2,0,1)   | `#010001` *  |
| (0.5, 0.75)   | 2       | 0        | (2,0,0)   | `#010000`    |
| [0.75, 1]     | 3       | 0        | (3,0,0)   | `#020000`    |

\* `gamma_lut[1] == gamma_lut[2] == 1`, so the raw red step from 1→2 around
tick 3079 is invisible in the output.

### Tick-by-tick

- **3054**: state changes to `TRANSITIONING_TO_DEFAULT_GRADIENT_STATE`,
  `a = 0`, color = `#010001`.
- **3054–3104** (`a` 0.00 → 0.50): raw red ticks 1→2 around tick 3079, but
  gamma_lut collapses both to output 1. Blue still rounds to 1. Output stays
  `#010001` for 50 ticks.
- **3105** (`a ≈ 0.51`): blue raw flips to 0 (`lround(0.49) = 0`). Output
  becomes `#010000`.
- **3105–3128** (`a` 0.51 → 0.75): raw red still rounds to 2 → gamma still 1.
  Output stays `#010000` for ~24 ticks.
- **3129** (`a ≈ 0.75`): raw red flips to 3, `gamma_lut[3] = 2`. Output
  becomes `#020000` and holds for the remainder.

## Why it looks like a jump

At these brightness levels there are only 3 distinct gamma outputs the
result can ever land in (`#010001`, `#010000`, `#020000`). With raw color
endpoints ≤ 3 there isn't enough integer resolution to show a gradual fade
— every visible step is a quantization boundary, and `gamma_lut` collapsing
raw 1 and 2 to the same output makes the plateaus long and the visible
transitions abrupt.

## Options if a smoother fade is wanted

1. **Raise the idle endpoint values** in the gradients (e.g. `(8, 0, 8)`
   instead of `(1, 0, 1)`). More headroom = more visible steps.
2. **Use a less aggressive gamma curve** (γ ≈ 2.2 instead of 1.5). Fewer low
   raws collapse to the same output.
3. **Interpolate in gamma-corrected space**: apply `gamma_lut` before the
   fade, then interpolate the 8-bit outputs. Quantization happens once at the
   right end of the pipeline.

None of these are required — this is a "looks-stepped at idle brightness"
artifact, not a correctness bug.

---

# Gamma LUT collisions at low values

## Observation

The current `gamma_lut` in `src/Color.h` was generated as
`round((i/255)^1.5 · 255)`. At the low end this produces collisions where
consecutive inputs map to the same output:

```
i:  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
γ:  1  1  2  3  3  4  5  6  6  7  8  9 10 10 11
```

Collisions at 1=2, 4=5, 8=9, 13=14, 20=21, 27=28, … About one every 4–5
inputs at the bottom.

This compounds the integer-quantization issue in the previous section: the
raw red value can walk 1→2→3 smoothly, but the LUT collapses 1 and 2 to the
same output, so only two visible steps survive instead of three.

## Why it happens

Any γ > 1 applied uniformly *compresses* the low end — `(i/255)^γ` is near
zero and adjacent values round to the same integer. That's the opposite of
what we want: gamma is meant to compensate for the eye being *more* sensitive
to changes in dim values, but the 8-bit LUT throws that resolution away
exactly where the gradient idle values live.

## Options

1. **Identity at the bottom, gamma above a knee.** Piecewise LUT:

   ```
   gamma_lut[i] = i                                              for i ≤ T
   gamma_lut[i] = T + round(((i-T)/(255-T))^γ · (255-T))         for i > T
   ```

   With T ≈ 20–32 the bottom is collision-free (raw 1→1, 2→2, 3→3, …) and
   the top still gets a perceptual curve. Likely the best fix.

2. **Lower γ overall** (γ ≈ 1.1–1.2). Reduces collisions but cannot
   eliminate them at i=1,2 — no γ > 1 yields a strictly-increasing 8-bit LUT
   from i=1 upward.

3. **γ = 1 (drop gamma correction entirely).** Whether this looks worse at
   higher brightness depends on the LEDs (APA102/SK9822 fairly linear,
   WS2812 less so).

4. **Move quantization to the end of the pipeline**: interpolate in
   gamma-space (apply LUT first, then fade the 8-bit outputs). Orthogonal
   to fixing the LUT shape; addresses the same visible-fade problem from a
   different angle.

## Recommended next step

Generate a piecewise LUT (option 1) with T = 24 and γ = 1.5 above the knee.
Replace the table in `src/Color.h:10` and re-run `pio run -e native` to
verify the cross-fade now traverses more distinct outputs at idle.
