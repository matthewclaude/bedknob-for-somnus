# Handoff — Sep 23 2026, end of session

## State of the tree

Branch `main`, HEAD `fe5b139`, **not pushed**: `somnus/main` is still
behind by the four commits since `547231f` (`5b23998`, `a05e47a`,
`e14e671`, `fe5b139`). `PROJECT_VER` is still `1.0.2` on line 21 of
`firmware/dial-idf/CMakeLists.txt`. Latest tag is `somnus-v1.0.2` on
`5512537`, published as stable; `releases/latest` is 1.0.2 and the beta
channel still serves 1.0.2-beta.1.

Uncommitted, left for you: `docs/REPORT-sidepick-code.md` (new), this
file, and their two lines in `docs/REPORTS.md`. Nothing else is dirty.

## What this session did

Implemented `docs/SPEC-sidepick-deletion.md`, the one change for
1.0.3-beta.1, in two commits:

| SHA | What |
|---|---|
| `e14e671` | Spec touch-up: §8 records that the latent Scale fix stays in (§9 Q1); §7 gets an "Order of the session" paragraph for the single nightly dial, and the T3 row points at its step 4. |
| `fe5b139` | The change: SCR_SIDEPICK, `scr_sidepick.c`, `side_picked` and `dial_state_set_side_picked` are gone; `dial_state_set_ui_zone` seeds `"relmode"` (if absent) before writing `"zone"`; the simulator scenario and `docs/screens/sidepick.png` are gone; README and ARCHITECTURE reworded. |

Host test passed, `idf.py build` clean (no compiler warnings),
`somnus-dial.bin` 1,611,440 bytes (880 smaller than 1.0.2's asset),
identity strings correct, simulator renders 48 screens. Full evidence is in
`docs/REPORT-sidepick-code.md`. Nothing was flashed.

## Open

- **Bench test, spec §7 (T1, T4, T2, Close)** on the nightly dial, in the
  order §7 now gives. Wire-flash with `idf.py flash`; the factory resets in
  T1/T2 wipe Wi-Fi, timezone and pad, so set the dial up again afterwards.
- Then the release commit: PROJECT_VER `1.0.3-beta.1`, a new CHANGELOG
  section from spec §8's draft, tag, push.
- T3 (OTA from 1.0.2) only after 1.0.3-beta.1 is published.
- Six simulator PNGs (`about*`, `update*`) are committed showing v1.0.0;
  every sim run shows them modified. They were restored, not committed.
  Regenerate them in the version-bump commit.
- Push: four commits on `main` are local only.
