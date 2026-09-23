SPEC WRITTEN — docs/SPEC-sidepick-deletion.md committed as 5b23998 (spec only, no firmware changes); report not committed.

# REPORT — SPEC-sidepick-deletion (1.0.3-beta.1 plan)

Date: 2026-09-23. Base: 547231f (somnus-v1.0.2).

## Gate (raw output)

```
$ sed -n 21p firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.2")
[exit 0]
$ git --no-optional-locks status --short --untracked-files=no
[exit 0]
$ git describe --tags --abbrev=0
somnus-v1.0.2
[exit 0]
$ test ! -e docs/SPEC-sidepick-deletion.md
[exit 0]
```

All four checks pass. The status check printed nothing, so the tree was clean.

## The relmode question (spec §1)

- `"zone"` has exactly one writer, `dial_state_set_ui_zone`
  (`dial_state.c:421`), and it writes only when the value changes
  (`:410`, `:418`). Its callers are the two dual-only swipes
  (`scr_dial.c:1493`, `:1506`) and `scr_sidepick.c:41`.
- `"relmode"` has two writers: `dial_state_set_rel_mode` (`:501`, the
  Settings → Scale tap) and the seed-if-absent in
  `dial_state_set_side_picked` (`:463-464`).
- The upgrade-to-absolute rule is `dial_state.c:276-277`:
  `if (have_rel) … else if (have_zone) rel_mode = false;`.
- (a) A fresh One Bed device never writes `"zone"`, so Relative survives
  every reboot on init's default (`:170`).
- (b) Once the picker is deleted, a fresh Dual Sides device's first swipe to
  the left side writes `"zone"` with no `"relmode"`. The next boot flips it
  to Absolute. This is a regression.
- (c) A device with both keys: no change.
- Latent case in 1.0.2: a non-fresh device switched to Dual Sides fails the
  same way (b) does, because the picker is gated on `fresh_device`
  (`main.c:430`).
- Recommendation: move the seed-if-absent into `dial_state_set_ui_zone`,
  written before `"zone"` in the same NVS block. This fixes (b) and the
  latent case, and leaves (a) and (c) unchanged. Whether shipping the
  latent fix fits "one change per beta" is Open question 1 in the spec.

## git grep (raw)

```
$ git grep -n -E "side_picked|fresh_device|SCR_SIDEPICK|scr_sidepick|sidepick" -- firmware/ simulator/
firmware/dial-idf/components/dial_state/dial_state.c:259:        // side_picked existed, so those devices never re-run SCR_SIDEPICK.
firmware/dial-idf/components/dial_state/dial_state.c:260:        s_state.side_picked = true;
firmware/dial-idf/components/dial_state/dial_state.c:444:void dial_state_set_side_picked(void)
firmware/dial-idf/components/dial_state/dial_state.c:447:    s_state.side_picked = true;
firmware/dial-idf/components/dial_state/dial_state.c:452:    // Onboarding runs only on a fresh device (nav_policy gates SCR_SIDEPICK on
firmware/dial-idf/components/dial_state/dial_state.c:453:    // !side_picked, and an upgrade restores side_picked from the existing
firmware/dial-idf/components/dial_state/dial_state.h:484:    // dial_net_bringup runs the portal). Gates SCR_WELCOME/SCR_SIDEPICK so an
firmware/dial-idf/components/dial_state/dial_state.h:487:    bool fresh_device;
firmware/dial-idf/components/dial_state/dial_state.h:493:    // SCR_SIDEPICK, or (upgrade path) NVS already had a "zone" key from
firmware/dial-idf/components/dial_state/dial_state.h:496:    bool side_picked;
firmware/dial-idf/components/dial_state/dial_state.h:868:// Mark that a default side is known (see app_state_t.side_picked). Callers
firmware/dial-idf/components/dial_state/dial_state.h:870:void dial_state_set_side_picked(void);
firmware/dial-idf/components/dial_ui/CMakeLists.txt:5:         "scr_standby.c" "scr_welcome.c" "scr_sidepick.c"
firmware/dial-idf/components/dial_ui/scr_settings.c:447:    // No "My side" row: it only re-ran SCR_SIDEPICK, which sets the very same
firmware/dial-idf/components/dial_ui/scr_sidepick.c:2: * SCR_SIDEPICK — "Which side of the bed?" Shown once, right after first
firmware/dial-idf/components/dial_ui/scr_sidepick.c:39:    // policy follows ui_zone/side_picked, so an uncommitted pick would be
firmware/dial-idf/components/dial_ui/scr_sidepick.c:42:    dial_state_set_side_picked();     // stop nav_policy pinning this screen
firmware/dial-idf/components/dial_ui/scr_sidepick.c:138:const ui_screen_t scr_sidepick = {
firmware/dial-idf/components/dial_ui/scr_welcome.c:3: * Wi-Fi credentials at boot; dial_state's fresh_device flag, set once in
firmware/dial-idf/components/dial_ui/ui_router.c:87:    case SCR_SIDEPICK:       // first-run side choice
firmware/dial-idf/components/dial_ui/ui_router.h:29:    SCR_SIDEPICK,         // "which side of the bed?" (M4, reused from Settings)
firmware/dial-idf/components/dial_ui/ui_screens.c:16:    ui_router_register(SCR_SIDEPICK, &scr_sidepick);
firmware/dial-idf/components/dial_ui/ui_screens_internal.h:21:extern const ui_screen_t scr_sidepick;
firmware/dial-idf/main/main.c:213:    if (st->fresh_device && !st->welcomed &&
firmware/dial-idf/main/main.c:425:            // the dial (SCR_SIDEPICK). Nothing to pick on a single-zone topper,
firmware/dial-idf/main/main.c:430:                ((st->fresh_device && !st->side_picked) || cur == SCR_SIDEPICK))
firmware/dial-idf/main/main.c:431:                return SCR_SIDEPICK;
firmware/dial-idf/main/main.c:559:static void mut_fresh_device(app_state_t *st, void *arg) { st->fresh_device = *(bool *)arg; }
firmware/dial-idf/main/main.c:587:// fresh_device (set here) / welcomed (cleared by scr_welcome.c).
firmware/dial-idf/main/main.c:1586:    dial_state_commit(mut_fresh_device, &fresh);
simulator/CMakeLists.txt:85:  ${DIAL_UI_DIR}/scr_sidepick.c
simulator/main.c:254:    st->side_picked = true;
simulator/main.c:325:static void scenario_sidepick(void)
simulator/main.c:328:    ui_router_go(SCR_SIDEPICK, NULL, LV_SCR_LOAD_ANIM_NONE);
simulator/main.c:330:    snapshot("sidepick");
simulator/main.c:1162:    scenario_sidepick();
simulator/sim_state.c:8: * set_ui_temp, set_zone_on, set_ui_zone, set_welcomed, set_side_picked,
simulator/sim_state.c:96:void dial_state_set_side_picked(void)
simulator/sim_state.c:98:    s_state.side_picked = true;
```

The spec also quotes the `"zone"`/`"relmode"` grep in full (§1.1).

## Commit

```
$ git diff --stat HEAD~1 HEAD
 docs/SPEC-sidepick-deletion.md | 513 +++++++++++++++++++++++++++++++++++++++++
 1 file changed, 513 insertions(+)
```

Commit: `5b23998` "docs: SPEC-sidepick-deletion - 1.0.3-beta.1 plan (spec only, no code)". Not pushed. This report is not committed; it
goes into the next docs commit with a line in docs/REPORTS.md.

## Deviations from this task

- **Grep scope widened.** The task asked for the grep across `firmware/`
  "including the simulator and tests". The simulator lives at the top-level
  `simulator/`, not under `firmware/`, so the grep also covers
  `simulator/`. The tests (`firmware/dial-idf/test/`) are inside the
  original scope and have no hits.
- **Wording: Scale, not adjust mode.** The task says "adjust mode" for the
  relative/absolute setting. In this firmware that setting is the Settings
  row "Scale", and "Adjust mode" (`SCR_ADJUST_MODE`) is Schedule/Hold. The
  spec says "Scale" and has a note explaining why.
- **Recommendation reaches further than the deletion.** The spec
  recommends a change that also fixes the latent 1.0.2 case. It is flagged
  as Open question 1 rather than decided.

## Not verifiable without hardware

- None of spec §7 has been run: the Scale shown after two reboots in each of
  T1, T2 and T4, and T3's OTA upgrade.
- Whether the bench dial reboots on a USB unplug. The battery SKU likely
  doesn't; see Open question 2 in the spec.
- That a Dual Sides dial against the single-zone pad sends no side1 write
  during swipe-only steps. The code says so (no write path on a swipe, and
  the POST log is at `dial_somnus.c:239`), but it has not been observed on
  hardware.
- The simulator's behaviour with `scenario_sidepick` removed (48 PNGs, the
  others unchanged). This is reasoned from the code; nothing was built,
  because this was a spec-only task.
