# SPEC — Delete SCR_SIDEPICK (1.0.3-beta.1)

Status: spec only, written 2026-09-23 against `547231f` (`somnus-v1.0.2`,
`PROJECT_VER` 1.0.2, tree clean). No firmware source changes in this
document's commit. The owner decided on 2026-09-05 to delete the side picker
rather than fix it; this spec covers how to delete it safely, not whether.
Owner decisions recorded 2026-09-23 (section 9); ready for the code change.

Scope rule in force: no new features, one change per beta, spec on disk
before code, hardware test before tag. The one change in 1.0.3-beta.1 is
**deleting SCR_SIDEPICK**. Moving the `"relmode"` seed (§1) is part of that
change: without it the deletion causes a regression.

All `file:line` references are at `547231f`. Firmware paths are relative to
`firmware/dial-idf/` unless they start with `simulator/` or `docs/`.

A note on words: the relative-vs-absolute preference is the Settings row
**Scale** (`scr_settings.c:474`, values "Relative"/"Absolute" at `:565`),
stored in NVS as `"ui"/"relmode"`. "Adjust mode" in this firmware means
something else: the Schedule/Hold screen `SCR_ADJUST_MODE`. This spec says
"Scale" throughout.

---

## 1. The relmode question (the one everything else depends on)

### 1.1 Every writer and reader of `"zone"` and `"relmode"`

```
$ git grep -n -E "\"zone\"|\"relmode\"|dial_state_set_ui_zone\(" -- firmware/ simulator/
firmware/dial-idf/components/dial_state/dial_state.c:169:    // restore_prefs when it finds a pre-existing "zone" key but no "relmode".
firmware/dial-idf/components/dial_state/dial_state.c:194:    bool have_zone    = nvs_get_u8(h, "zone", &zone) == ESP_OK && zone < ZONE_COUNT;
firmware/dial-idf/components/dial_state/dial_state.c:198:    bool have_rel     = nvs_get_u8(h, "relmode", &relmode) == ESP_OK;
firmware/dial-idf/components/dial_state/dial_state.c:257:        // The "zone" key's mere existence means some earlier session already
firmware/dial-idf/components/dial_state/dial_state.c:272:    // this device was set up before this release (has a "zone" key but no
firmware/dial-idf/components/dial_state/dial_state.c:273:    // "relmode"), keep it on ABSOLUTE — an unattended OTA must not change what
firmware/dial-idf/components/dial_state/dial_state.c:407:void dial_state_set_ui_zone(zone_idx_t zone)
firmware/dial-idf/components/dial_state/dial_state.c:421:            nvs_set_u8(h, "zone", (uint8_t)zone);
firmware/dial-idf/components/dial_state/dial_state.c:454:    // "zone" key so it never reaches here). Persisting "relmode" now is what
firmware/dial-idf/components/dial_state/dial_state.c:456:    // never opens Settings — otherwise its second boot would see a "zone" key
firmware/dial-idf/components/dial_state/dial_state.c:457:    // (written by the side pick) but no "relmode" and wrongly apply the
firmware/dial-idf/components/dial_state/dial_state.c:463:        if (nvs_get_u8(h, "relmode", &existing) != ESP_OK) {
firmware/dial-idf/components/dial_state/dial_state.c:464:            nvs_set_u8(h, "relmode", rel ? 1 : 0);
firmware/dial-idf/components/dial_state/dial_state.c:501:        nvs_set_u8(h, "relmode", rel_mode ? 1 : 0);
firmware/dial-idf/components/dial_state/dial_state.h:493:    // SCR_SIDEPICK, or (upgrade path) NVS already had a "zone" key from
firmware/dial-idf/components/dial_state/dial_state.h:863:void dial_state_set_ui_zone(zone_idx_t zone);
firmware/dial-idf/components/dial_state/dial_state.h:869:// that pick a side also call dial_state_set_ui_zone() to persist it.
firmware/dial-idf/components/dial_state/dial_state.h:879:// "ui"/"relmode".
firmware/dial-idf/components/dial_ui/scr_dial.c:1493:            dial_state_set_ui_zone(ZONE_A);
firmware/dial-idf/components/dial_ui/scr_dial.c:1506:        dial_state_set_ui_zone(ZONE_B);
firmware/dial-idf/components/dial_ui/scr_sidepick.c:41:    dial_state_set_ui_zone(z);        // persists "ui"/"zone" to NVS
simulator/sim_state.c:84:void dial_state_set_ui_zone(zone_idx_t zone)
```

(`simulator/sim_state.c:84` is the host stand-in. It has no NVS.)

**Writers of `"zone"`.** There is exactly one: `dial_state_set_ui_zone`
(`dial_state.c:407-424`). It writes **only when the value changes**
(`changed = (s_state.ui_zone != zone)` at `:410`, and the write is guarded
by `if (changed)` at `:418`). It has three callers:

| Caller | Condition |
|---|---|
| `scr_dial.c:1493` (swipe left from Dial(B) to Dial(A)) | `dual && s_zone == ZONE_B` (`:1489`) |
| `scr_dial.c:1506` (swipe right from Dial(A) to Dial(B)) | `dual && s_zone == ZONE_A` (`:1505`) |
| `scr_sidepick.c:41` (`pick()`) | only reachable when `dial_state_is_dual` (`main.c:429`) |

Every caller is dual-only. The Settings screen does not call it: the
"My side" row is already gone (`scr_settings.c:447-449`). `main.c:502-503`
changes `ui_zone` when the stored side isn't present, but only in RAM, inside
a store mutator, and never touches NVS.

**Writers of `"relmode"`.** There are two:
- `dial_state_set_rel_mode` (`dial_state.c:488-505`) writes it every time
  (the Settings → Scale tap, `scr_settings.c:224`).
- `dial_state_set_side_picked` (`dial_state.c:444-469`) writes it only
  **if it is absent**, seeding the current RAM `rel_mode`. Its only caller is
  `scr_sidepick.c:42`.

**Readers.** Only `dial_state_restore_prefs` reads these keys:
- `:194` `have_zone` and `:198` `have_rel`.
- `:255-261` When `"zone"` exists, set `ui_zone` from it and set
  `side_picked = true` because the key exists.
- `:276-277` is the upgrade-to-absolute rule:
  `if (have_rel) rel_mode = relmode; else if (have_zone) rel_mode = false;`
- `:248-252` returns early when *no* key exists at all. This leaves init's
  defaults in place.

**Defaults.** `dial_state_init` zeroes the struct (`dial_state.c:125`), so
`ui_zone = ZONE_A` (`dial_state.h:45`, `ZONE_A = 0`). It then seeds
`rel_mode = true` (`dial_state.c:170`). The Bed Mode default is One Bed
(`DIAL_PAD_DEFAULT_SINGLE_ZONE true`, `dial_state.h:373`).

**Ordering.** `restore_prefs` runs at `main.c:1587`, and the worker task is
created later at `main.c:1628`. `have_state` (and so `SCR_DIAL`) cannot
exist before `restore_prefs` has run, so no swipe can reach
`set_ui_zone` before the stored prefs have loaded.

### 1.2 The cases

**(a) Fresh single-zone device, today (1.0.2).** A factory reset
(`main.c:852-854`, `nvs_flash_erase()`) leaves neither key. The device
defaults to One Bed, so `dial_state_is_dual` is false. None of the three
`set_ui_zone` callers can run, so `"zone"` is **never written**. Every boot
then takes the early return or the `have_rel == have_zone == false` path,
and `rel_mode` stays at init's `true`. The relative default survives reboots
because nothing is ever persisted. Is `"zone"` ever written without
`"relmode"`? **Not while the device stays One Bed.** It can happen in the
latent case below.

**(b) Fresh dual-mode device, after deletion.** With the picker gone, only
the on-face swipes write `"zone"` (`scr_dial.c:1493/1506`). Settings does
not write it. The first swipe from Dial(A) to Dial(B) writes `"zone"=1`, and
nothing writes `"relmode"`. On the next boot, `have_zone && !have_rel`
reaches `dial_state.c:277`, and **`rel_mode` flips to false (Absolute)**.
The user never touched Scale, yet the big number changes meaning after a
reboot. **This is a regression, and deletion must not ship without the fix
in §1.3.**

(If the user never swipes and stays on Dial(A), `set_ui_zone` never writes,
because `ZONE_A` is already the RAM value and `changed` is false. That
device stays Relative. The regression needs at least one swipe, and every
dual-mode user swipes.)

**(c) Upgraded device already carrying both keys.** `have_rel` wins at
`:276`, and `"zone"` restores the side. A swipe still writes `"zone"` just
as it does in 1.0.2, and the new seed does nothing because `"relmode"`
already exists. **No change.**

**Latent case, present in 1.0.2 today.** Take a device that is *not* fresh
(it has Wi-Fi credentials, so `fresh_device` is false per `main.c:1585`),
has never had Scale tapped, and is switched from One Bed to Dual Sides in
Settings. That device never sees the picker, because the picker is gated on
`fresh_device`. Its first swipe to Dial(B) writes `"zone"` without
`"relmode"`, so the next boot flips it to Absolute. This is the exact
failure in (b), reachable today by any device whose Bed Mode was changed
after its first session. Relatedly, the picker already skips the relmode
seed on any fresh device that reboots before its first `PH_READY`, because
`fresh_device` is per boot. The picker was never a complete guard.

### 1.3 Recommendation: seed `"relmode"` at the only write of `"zone"`

Move the "write `"relmode"` if absent" logic from `dial_state_set_side_picked`
into `dial_state_set_ui_zone`, in the same `if (changed)` NVS block, and
write it **before** `"zone"`:

```c
void dial_state_set_ui_zone(zone_idx_t zone)
{
    xSemaphoreTake(s_mux, portMAX_DELAY);
    bool changed = (s_state.ui_zone != zone);
    s_state.ui_zone = zone;
    bool rel = s_state.rel_mode;
    xSemaphoreGive(s_mux);
    ...
    if (changed) {
        nvs_handle_t h;
        if (nvs_open(NVS_NS, NVS_READWRITE, &h) == ESP_OK) {
            // Seed "relmode" before the first "zone" (moved here from the
            // deleted side picker): restore_prefs reads "zone" without
            // "relmode" as a pre-relative-scale upgrade and forces Absolute.
            uint8_t existing;
            if (nvs_get_u8(h, "relmode", &existing) != ESP_OK)
                nvs_set_u8(h, "relmode", rel ? 1 : 0);
            nvs_set_u8(h, "zone", (uint8_t)zone);
            nvs_commit(h);
            nvs_close(h);
        }
    }
}
```

Why here:
- `set_ui_zone` is the **only** writer of `"zone"` (§1.1). Seeding at this
  point guarantees the invariant restore_prefs relies on: **`"zone"` never
  exists without `"relmode"` unless it was written by a pre-relative-scale
  build.**
- Writing `"relmode"` before `"zone"` means a power loss between the two
  `nvs_set_u8` calls leaves `"relmode"` without `"zone"`. Restore handles
  that correctly (`have_rel` wins). The reverse order would recreate the bug
  for one boot.
- The value seeded is the RAM `rel_mode`. On a device with neither key that
  is init's `true`. On a genuine legacy device (`"zone"` present, no
  `"relmode"`), restore_prefs has already set RAM to `false` at `:277`
  before any swipe is possible (ordering note in §1.1). The seed then writes
  `0`, which pins exactly what that device already shows, so its behaviour
  is unchanged.

Alternatives considered and rejected:
- *Seed at boot when neither key exists.* `restore_prefs` opens NVS
  read-only (`:186`) and early-returns on an empty namespace. That would add
  a write path to boot, which is a bigger change than moving one existing
  guarded write.
- *Delete the upgrade-to-absolute rule.* That changes behaviour for legacy
  devices. It is a separate decision and out of scope.

What the recommendation changes:
- **(a)** No change. A One Bed device never calls `set_ui_zone`.
- **(b)** This is the case it exists for. The fresh dual device stays
  Relative across reboots after swiping.
- **(c)** No change. `"relmode"` exists, so the seed is a no-op.
- **Latent case:** fixed as a side effect. The same line closes it, and it
  cannot be separated from the move. See Open question 1.

---

## 2. Every reader and writer of `side_picked`, `fresh_device`, `SCR_SIDEPICK`

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

The host test directory `firmware/dial-idf/test/` has one test,
`test_dial_rel.c`, and it has no hits: it tests the relative-scale carriers,
not prefs or navigation.

| Hit | Fate |
|---|---|
| `dial_state.c:257-261` (restore: `side_picked = true` on `"zone"` existing) | **Change**: keep `ui_zone = zone` (`:256`), delete the comment and assignment at `:257-260` |
| `dial_state.c:444-469` `dial_state_set_side_picked` | **Delete** the whole function. Its relmode seed moves to `set_ui_zone` (§1.3) |
| `dial_state.c:169` init comment ("restore_prefs when it finds a pre-existing zone key…") | Keep. Still true |
| `dial_state.h:481-487` `fresh_device` + comment | **Keep** the field. Reword `:484` "Gates SCR_WELCOME/SCR_SIDEPICK" → "Gates SCR_WELCOME", and drop "never picked a side" |
| `dial_state.h:492-496` `side_picked` + comment | **Delete** |
| `dial_state.h:868-870` `dial_state_set_side_picked` decl + comment | **Delete** |
| `dial_state.h:408-413` `zone_present` comment ("…the page dots, the side picker…") | **Change**: drop "the side picker" |
| `dial_state.h:860-863` `dial_state_set_ui_zone` comment | **Change**: add one sentence saying it also seeds `"relmode"` when absent (§1.3) |
| `dial_ui/CMakeLists.txt:5` `"scr_sidepick.c"` | **Delete** the entry |
| `scr_settings.c:447-449` "No My side row…" comment | **Change**: it names a screen that no longer exists. Reword to say the side is set by swiping on the dial, with no Settings row, and don't name SCR_SIDEPICK |
| `scr_sidepick.c` (whole file) | **Delete** (`git rm`) |
| `scr_welcome.c:3` (`fresh_device`) | Keep. SCR_WELCOME stays |
| `ui_router.c:87` `case SCR_SIDEPICK:` in `screen_blocks_sleep` | **Delete** the case line |
| `ui_router.h:29` enum value | **Delete** (ordinal analysis in §4) |
| `ui_screens.c:16` register call | **Delete** |
| `ui_screens_internal.h:21` extern | **Delete** |
| `main.c:213` SCR_WELCOME gate | Keep |
| `main.c:424-431` nav_policy comment + SCR_SIDEPICK branch | **Delete**. `:432-433` (`*arg = ui_zone; return … SCR_DIAL`) stays as is |
| `main.c:559`, `:587`, `:1579-1586` (`fresh_device` mutator, comment, commit) | Keep. `:1582` "(it keeps its tokens, side, and prefs)" is still accurate, since "side" is the `"zone"` key |
| `simulator/CMakeLists.txt:85` | **Delete** the line |
| `simulator/main.c:254` `st->side_picked = true;` | **Delete** (the field is gone, so it would not compile) |
| `simulator/main.c:325-331` `scenario_sidepick` | **Delete** |
| `simulator/main.c:1162` call | **Delete** |
| `simulator/sim_state.c:8` header list | **Change**: drop `set_side_picked,` from the list |
| `simulator/sim_state.c:96-100` stub | **Delete** |

**`side_picked` and `dial_state_set_side_picked` disappear entirely** from
firmware and simulator. After the change, the same `git grep` should hit
only the `fresh_device` lines kept above: `dial_state.h` field and comment,
`scr_welcome.c:3`, and `main.c:213/559/587/1586`.

---

## 3. What a fresh dual-mode device shows after first link

1. The nav_policy branch at `main.c:429-431` is gone, so at `PH_READY` it
   falls through to `main.c:432-433`: `*arg = st->ui_zone`, then
   `SCR_DIAL` (or the standby face if the power level is already STANDBY).
2. `ui_zone` is **`ZONE_A`**. It comes from `dial_state_init`'s
   `memset(&s_state, 0, …)` (`dial_state.c:125`) with `ZONE_A = 0`
   (`dial_state.h:45`). A factory-reset device has no `"zone"` key, so
   `restore_prefs` doesn't override it. `main.c:502-503` only corrects
   `ui_zone` when the zone isn't present, and in Dual Sides both are
   present (`main.c:715-717` seeds `present[ZONE_B] = !single_zone`).
3. `ZONE_A` is the **right** side of the bed (`scr_dial.c:1468-1470`: "zone_b
   is the LEFT side of the bed and zone_a the RIGHT"). The face label reads
   RIGHT SIDE.
4. The side swipe is reachable from there. In `scr_dial.c` `on_gesture`, a
   RIGHT swipe with `dual && s_zone == ZONE_A` (`:1505`) calls
   `dial_state_set_ui_zone(ZONE_B)` and goes to Dial(B) (`:1506-1507`). A
   LEFT swipe from Dial(B) comes back (`:1489-1494`). A LEFT swipe from
   Dial(A) goes to the menu (`:1498`).
5. The face opens on the right side, and a user on the left side swipes
   once. That swipe persists the side (`"zone"`) and, with §1.3, the Scale.
   Wake-to-last-side (design-spec §8.4) then applies on every later boot.

---

## 4. Full deletion list

1. `components/dial_ui/scr_sidepick.c`: `git rm`.
2. `components/dial_ui/CMakeLists.txt:5`: remove `"scr_sidepick.c"`, and
   keep the rest of the line (`"scr_standby.c" "scr_welcome.c"`).
3. `components/dial_ui/ui_router.h:29`: remove `SCR_SIDEPICK,`.
4. `components/dial_ui/ui_router.c:87`: remove the `case SCR_SIDEPICK:`
   line from `screen_blocks_sleep`.
5. `components/dial_ui/ui_screens.c:16`: remove the register call.
6. `components/dial_ui/ui_screens_internal.h:21`: remove the extern.
7. `main/main.c:424-431`: remove the comment block and the `if` that
   returns `SCR_SIDEPICK`.
8. `components/dial_state/dial_state.c`: remove `:257-260` in restore_prefs
   (keep `:256`). Remove `dial_state_set_side_picked` (`:444-469`). Add the
   relmode seed to `dial_state_set_ui_zone` (§1.3).
9. `components/dial_state/dial_state.h`: remove `side_picked` (`:492-496`)
   and the setter declaration (`:868-870`). Reword the `fresh_device`
   comment (`:484-486`), the `zone_present` comment (`:411`) and the
   `set_ui_zone` comment (`:860-862`).
10. `components/dial_ui/scr_settings.c:447-449`: reword the comment (see §2).
11. Simulator: `simulator/CMakeLists.txt:85`, `simulator/main.c:254`,
    `:325-331`, `:1162`, `simulator/sim_state.c:8`, `:96-100` (§2, §5).

**Enum ordinals.** At `547231f`, `SCR_SIDEPICK` = 10. Removing it
renumbers `SCR_SETTINGS` … `SCR_UPDATE_PROMPT` (11-24 → 10-23), and
`SCR_COUNT` goes from 25 to 24. Every place the numbering could matter:
- `ui_router.c:12` `s_screens[SCR_COUNT]` is indexed by the enum and filled
  at runtime by `ui_screens_register_all`. It is rebuilt with the new
  numbering, so it is safe.
- `ui_router.c:103`/`:35` `configASSERT(id < SCR_COUNT …)` is safe.
- `scr_menu.c:69/82` packs a `screen_id_t` into LVGL `user_data` at runtime
  only, so it is safe.
- No NVS key stores a screen id. Every `nvs_set_*` in `components/` and
  `main/` is a pref, Wi-Fi, haptics-calibration or timezone key.
- No screen ordinal crosses an OTA or a reboot. The `SCR_TIMEZONE`,
  `SCR_ADJUST_MODE` and `SCR_UPDATE_PROMPT` args are zone/origin packs, not
  screen ids.
- The one numeric log, `ui_router.c:214` `"router up, screen %d"`, prints
  `SCR_CONNECTING` = 0, which is unchanged.
- No script under `tools/` parses screen numbers.

Conclusion: **nothing depends on the ordinals**, and the renumbering is
safe.

---

## 5. Simulator

`simulator/main.c:325-331` `scenario_sidepick()` renders `SCR_SIDEPICK` and
writes `docs/screens/sidepick.png`, called from `:1162`. No other scene
reaches the screen. `apply_baseline()` sets `side_picked = true` (`:254`)
only so that nothing routes there. The simulator does not link `main.c`,
so there is no nav_policy.

- `docs/screens/sidepick.png` is **deleted** (`git rm`) in the code commit.
  The simulator will no longer write it, and a stale PNG would suggest the
  screen still exists. The simulator goes from 49 PNGs to 48.
- Rebuild and run `dial_sim` to confirm that it compiles without the file
  and that **no other PNG changes**. Only one scenario is removed, and no
  scenario depends on another's leftover state through `side_picked`.
  Existing practice (back up and restore `docs/screens/` around a run)
  applies. The expected `git status` for `docs/screens/` is the one
  deletion.
- Historical reports that quote `wrote sidepick …` output stay as written.

---

## 6. Docs that describe the side picker as live

Reword these in the code commit, each by hand, reading the sentence around
the reference. Do not use search-and-replace.

| File:line | Now says | Change |
|---|---|---|
| `firmware/dial-idf/README.md:197-198` | "In Dual Sides mode a fresh dial also asks 'Which side of the bed?' once." | Replace with: in Dual Sides the dial opens on the right side and one swipe shows the left; it remembers the last side shown. |
| `docs/ARCHITECTURE.md:183-186` | "Two setup gates fire only at `PH_READY`: the timezone picker … and the **side pick** on a fresh Dual Sides dial." | Change to one gate (timezone), and drop the side-pick clause. |
| `docs/ARCHITECTURE.md:191` | Screen list includes "side pick" | Remove "side pick" from the list. |

Checked and left alone:
- `firmware/dial-idf/docs/design-spec.md` §8.4 (`:186`), "`zone_a`/`zone_b`
  mapped at setup so swiping right reveals the person who actually sleeps
  to your right". This describes the fixed left/right mapping, which does
  not change. It does not describe the picker (the picker chose a default
  side, not the mapping). The wake-to-last-side clause also stays true.
- `docs/SPEC-connect-phases.md:408-413, :534` and
  `docs/SPEC-pad-discovery.md:626-632` mention `fresh_device` only. That
  field stays and so do these docs.
- `docs/PLAN-screen-layout-fixes.md:30, :95` is an executed plan (layout fix
  A4). Leave it, like the reports.
- All `docs/REPORT-*.md` and `docs/REVIEW-*.md` (including
  `REVIEW-2026-09-02.md:328-330`, "ui/zone … sidepick") are historical and
  stay as written.
- The top-level `README.md:253` "which side was whose" is about the cloud,
  not the picker.
- `CHANGELOG.md` and `CHANGELOG-orion.md`: existing sections are never
  edited.

---

## 7. Bench test plan (hardware test before tag)

**Setup.** Flash the 1.0.3-beta.1 candidate. Capture serial with a raw `cat`
of `/dev/cu.usbmodem*` at 115200 into a new file under `bench-logs/`, one
file per boot. Never use `idf.py monitor`, which resets the chip. Restart
the capture after each reboot, and wake the dial before starting a short
capture.

**How to read the Scale.** Open Settings and look at the **Scale** row. It
shows "Relative" or "Absolute" (`scr_settings.c:565`). Reading it does not
write anything; only tapping it does (`:224`). **Never tap the Scale row in
these tests.** The whole point is a device whose Scale was never set by
hand. The face also shows it: relative numbers vs °F.

**Rules for touching the pad.** The pad stays single-zone; only the dial is
set to Dual Sides (section 9, Q3). In the dual-mode steps (T2, T4), touch
the glass only: change sides by **swiping**, and wake the dial with a
single fingertip tap, which writes nothing. **No knob turns or presses** on
either face: a turn changes the setpoint and a press toggles pad power, and
either would send a side1 write while the dial is set to Dual Sides. Do not
tap the power disc either. At the end, set Bed Mode back to One Bed. Diff
the pad's first and last state for the session.

**Reboot.** Every reboot in this plan is a press of the board's **RST**
button (section 9, Q2). NVS survives any reset, so a power cycle with the
cell disconnected would prove nothing more. On the battery SKU, unplugging
USB alone does not reboot it (it keeps running on the cell).

**Order of the session.** There is exactly one dial, and it is the owner's
nightly dial, so the steps run in this order:
1. Wire-flash the candidate with `idf.py flash`. That writes only the
   bootloader, partition table, OTA data and app, so the dial's settings
   are kept until T1's factory reset.
2. Run T1, T1-r, T4, T4-r, T2, T2-a, T2-r and T2-z, then Close.
3. Set the dial up again for nightly use (Wi-Fi, timezone, pad), since the
   factory resets wiped these.
4. T3 runs only after 1.0.3-beta.1 is published, as with 1.0.2-beta.1's OTA
   bench. First put the dial back on 1.0.2 with its settings kept: wire-flash
   a build of the tag `somnus-v1.0.2` with `idf.py flash` (never the browser
   flasher, which always erases settings). Confirm About shows 1.0.2. Note
   the side and the Scale. Then turn Beta builds On and tap Check for
   updates.

| # | Steps | Observe on the dial | Serial needed? |
|---|---|---|---|
| T1 | Settings → Factory reset. Walk onboarding (welcome, Wi-Fi, pad). Bed Mode stays One Bed. | No side picker. Lands on the dial face labelled BOTH SIDES. Scale = Relative. | Yes: `sb_face: no key -> default` proves a genuinely empty `"ui"` namespace. Also confirm the router-up line and no panic. |
| T1-r1, T1-r2 | Press RST, wait for the face, check Scale. Twice. | Scale = Relative both times. | Yes: two boot banners, to prove two real reboots happened. |
| T4 (latent case) | Same device, now non-fresh: Settings → Bed Mode → Dual Sides. Back to the dial. Swipe right to LEFT SIDE. From here on, glass only: swipes, and single-fingertip taps to wake; no knob turns or presses. | No side picker (not fresh). LEFT SIDE face. | Yes: `zone mode set to dual (Dual Sides)`. There must be no `POST` line containing `"side1"`. |
| T4-r1, T4-r2 | Press RST twice, waiting for the face each time. Wake with a fingertip tap only; no knob. | Wakes to LEFT SIDE. **Scale = Relative** both times. (On 1.0.2 this reads Absolute after the first reboot. That is the latent bug.) | Yes: boot banners, and no `"side1"` POST. |
| T2 | Factory reset again. Walk onboarding. **In this first session**, after the pad links: Settings → Bed Mode → Dual Sides, back to the dial. From here on, glass only: swipes, and single-fingertip taps to wake; no knob turns or presses. | **No side picker.** The face opens on **RIGHT SIDE**. A right swipe shows LEFT SIDE, and a left swipe returns. | Yes: `sb_face: no key -> default`, dual zone-mode line, no `"side1"` POST. |
| T2-a | Leave the dial on LEFT SIDE (so `"zone"` was written). | | |
| T2-r1, T2-r2 | Press RST twice, waiting for the face each time. Wake with a fingertip tap only; no knob. | Wakes to LEFT SIDE. **Scale = Relative** both times. This is the regression check for §1.2(b). | Yes: boot banners, no `"side1"` POST. |
| T2-z | Swipe back to RIGHT SIDE, press RST once. No knob. | Wakes to RIGHT SIDE, Scale = Relative. | Yes: boot banner. |
| T3 (upgrade) | After publication, with the dial back on 1.0.2 as set out in step 4 of the order above: OTA to the beta. | No side picker. Same side and same Scale as before the update. | Optional: the OTA and boot lines. |
| Close | Set the one dial, Bedknob #1, back to Bed Mode → One Bed. | BOTH SIDES label. | Diff the pad's state at the start and end of the session. |

**Note (2026-09-30, after the bench).** The T1 and T2 rows expect
`sb_face: no key -> default` after a factory reset. That line cannot
appear. After an NVS erase the `"ui"` namespace does not exist, so
`dial_state_restore_prefs` returns at its `nvs_open(NVS_NS, NVS_READONLY,
&h)` check (`dial_state.c:186`) before it logs anything. The proof of an
empty namespace is the `factory reset requested — erasing NVS` line
followed by a boot with no `sb_face` line at all.

T1 and T2 both reboot twice, as the task requires. T4 reuses the T1 device
because a device that was factory-reset and has never had Scale tapped is
exactly the state that exposes the latent path.

The Scale checks are visual. The serial capture is there to prove each
reboot really happened, that restore_prefs saw an empty namespace (the
`sb_face` line at `dial_state.c:245-246`), that the Dual Sides setting
reached `dial_somnus` (`dial_somnus.c:23`), and that no side1 write went
out (`dial_somnus.c:239` logs each POST body).

---

## 8. Draft CHANGELOG wording for 1.0.3-beta.1

This draft lives here only. It goes into `CHANGELOG.md` as a **new**
section in the commit that bumps `PROJECT_VER`. Existing sections are not
touched.

```
## 1.0.3-beta.1 — YYYY-MM-DD (beta)

A new dial in Dual Sides mode no longer stops to ask "Which side of the
bed?" after it first reaches the pad. It opens on the right side, as it
already did for anyone who never saw that question, and one swipe shows
the left. The dial still remembers the last side you looked at.

Also fixed: switching an existing dial from One Bed to Dual Sides and then
swiping to the other side could change Scale from Relative to Absolute on
the next restart, even though you never changed it. Scale now stays where
you left it.

Internal: SCR_SIDEPICK and the side_picked flag are removed; the "relmode"
key is now seeded alongside the first "zone" write in
dial_state_set_ui_zone instead of by the side picker.
```

(The owner kept the latent fix in scope (section 9, Q1), so the "Also
fixed" paragraph stays. §1.3 would be required for the deletion itself
either way.)

---

## 9. Owner decisions (2026-09-23)

1. **The latent-case fix rides along.** §1.3 is required to delete the
   picker without a regression. The same line also fixes a 1.0.2 bug in
   which a non-fresh dial switched to Dual Sides flips to Absolute after a
   swipe and a reboot. The two cannot be separated, since it is one
   guarded write at the only writer of `"zone"`. Is it acceptable under
   "one change per beta" to ship this as part of the deletion and mention it
   in the release notes (§8)? If not, the only way to exclude it is to
   gate the seed on `fresh_device`, which would keep a known bug on
   purpose. This spec recommends shipping the fix.

   **Decided:** yes. The fix for the latent 1.0.2 bug ships as part of the
   deletion. It is one guarded write at the only writer of `"zone"` and
   cannot be separated from the deletion. Gating the seed on
   `fresh_device` to exclude it would keep a known bug on purpose, and is
   rejected. The 1.0.3-beta.1 CHANGELOG section names both the deletion
   and the fix.
2. **Reboot method on the bench dial.** The reset button, or a full power
   cycle with the cell disconnected? Unplugging USB alone does not reboot
   the battery SKU.

   **Decided:** the RST button. NVS survives any reset, so a power cycle
   with the cell disconnected proves nothing more. Every "reboot" in
   section 7 means a press of RST.
3. **Dual-mode steps against a single-zone pad.** T2 and T4 set the *dial*
   to Dual Sides while the pad stays single-zone, and allow swipes only.
   Nothing in those steps sends a write, and the serial capture proves no
   `"side1"` POST. Is that acceptable, or would the owner rather switch the
   pad to dual in the Somnus app for the session? That would change the
   bed's state and need restoring afterwards.

   **Decided:** acceptable. The pad stays in single-zone mode. Only the
   dial is set to Dual Sides for T2 and T4. During those steps, wake the
   dial with a single fingertip tap on the glass only. No knob turns or
   presses: a turn changes the setpoint and a press toggles pad power. The
   serial capture must show no side1 POST.
