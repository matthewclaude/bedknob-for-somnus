# Host tests

Small, dependency-free tests that compile against the firmware headers with a
plain host compiler — no ESP-IDF, no board.

## `test_dial_rel.c`

Pins the relative-temperature scale invariants in `components/dial_state/dial_state.h`
(the `dial_rel_from_dc` / `dial_rel_to_dc` / `dial_rel_step` helpers, all in
tenths of °C — the canonical unit since the 2026-08-30 units fix, over the
uniform 1.0°C/level −15…+15 scale from the 2026-09-01 relative-scale fix,
`docs/SPEC-somnus-relative-scale.md`): level round-trips, one-detent-per-level
stepping, the rails, and that every carrier stays inside the Somnus API's
accepted range. Run it before touching that scale.

```sh
cc -I components/dial_state -Wall -o /tmp/test_dial_rel test/test_dial_rel.c -lm && /tmp/test_dial_rel
```

Expected output: `all relative-scale table assertions passed`.

The spliced `+` glyph in `dial_font_num_88` (relative `+N` levels) is verified
visually instead — the simulator's `dial-relative` scenario renders `+2`.
