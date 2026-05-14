# Black-flash during gradient transition

## Symptom

At tick ~3055 the panels go to `#000000` for ~33 ticks before settling on the
default gradient idle color (`#010000`) at tick 3088. The previous color
(`#010001`, the `purple_red_gradient` idle) drops straight to black rather than
fading.

## Root cause

`interpolateValue` in `src/Maths.h:11` truncates each component to `int`
**before** adding them:

```cpp
return static_cast<int>(value_a_amount) + static_cast<int>(value_b_amount);
```

When both `value_a` and `value_b` are small (≤ ~3), each per-side product is
less than 1 for most of the transition, so each `int(...)` truncates to 0 and
the sum stays at 0 until `value_b * amount` finally clears 1.

### Trace at the failing tick range

`TRANSITIONING_TO_DEFAULT_GRADIENT_STATE` starts at tick 3054 with
`transition_value = 0` (renders identically to the prior state, hence the
"reset hasn't happened yet" appearance). The transition is
`(1, 0, 1) → (3, 0, 0)` over 24000ms / 240ms-per-loop = 100 ticks.

For red, `int(1 * (1-a)) + int(3 * a)`:

| amount | term A         | term B         | result |
|--------|----------------|----------------|--------|
| 0.00   | `int(1.00) = 1`| `int(0.00) = 0`| **1**  |
| 0.01   | `int(0.99) = 0`| `int(0.03) = 0`| **0**  |
| 0.33   | `int(0.67) = 0`| `int(0.99) = 0`| **0**  |
| 0.34   | `int(0.66) = 0`| `int(1.02) = 1`| **1**  |

For blue, `int(1 * (1-a)) + int(0 * a)` = 0 for any `a > 0`.

So red and blue both collapse to 0 from `a ≈ 0+` until `a ≥ 1/3`, which at a
100-tick duration is ~33 ticks. Matches the observed 3055–3087 black window
and the recovery to `#010000` (red=1, blue=0) at 3088.

The same bug fires anywhere two low-RGB gradients cross-fade — e.g. the brief
black at ticks 320–327 in the same script.

## Fix

Sum the floats first, then round once at the end. In `src/Maths.h`:

```cpp
static int interpolateValue(int value_a, int value_b, float amount) {
    float value_a_amount = static_cast<float>(value_a) * (1.0f - amount);
    float value_b_amount = static_cast<float>(value_b) * amount;
    return static_cast<int>(std::lround(value_a_amount + value_b_amount));
}
```

With the fix, red at `a = 0.01` becomes `lround(0.99 + 0.03) = 1`, and the
transition fades smoothly `#010001 → #010000 → #020000` instead of dropping to
black.

## Verification

Re-run `pio run -e native` and inspect ticks 3054–3090 (and 318–328) in the
output JSON. No `#000000` rows should appear during the cross-fade.
