# Spec: User-settable night window

Status: **BUILT — shipped as `0.1.5-beta.1` / `0.1.5-beta.2` on the beta
channel, verified on hardware 2026-09-03** (palette, brightness rows, Update
row, the Off rule, and the night flip forced by moving the timezone to
Tokyo). Written 2026-09-03 as a proposal held behind the no-new-features
rule; the v1 list closed the same day with `somnus-v0.1.4` and the owner
released this as the first feature after it. Shipping it on the beta channel
found and fixed a real OTA bug (§10). **Revision 3** is what was built; the
change list is at the end. Graduates to stable `0.1.5` when the owner says
so. One post-build correction: on-screen strings use ASCII hyphens, not en
dashes — the compiled fonts carry no dash glyph (`3db55ad`).

> **Note for an on-disk reader:** `V1-scope.md`, `START-HERE.md`,
> `HARDWARE-bringup-log.md`, `LICENSING.md` and
> `somnus-dial-project-summary.md` are **not in this repo** — they live only
> in the Claude Project. Everything this spec needs is restated here; do not
> go looking for them on disk.

## 1. What exists today

Night is one hardcoded line, `main/main.c` (~1182), inside `worker_task`'s
steady-state loop:

```c
struct tm lt;
if (dial_time_now(&lt)) {
    int now_min = lt.tm_hour * 60 + lt.tm_min;
    ...
    bool night = (lt.tm_hour >= 21 || lt.tm_hour < 7);
    dial_power_set_night(night);
    if (night != s_ui_night) {
        s_ui_night = night;
        dial_palette_set_night(night);
        dial_state_commit(mut_bump, NULL);
    }
```

21:00–07:00, unconditional. The comment above it is honest about why:
Somnus's local API has no sleep-schedule endpoint to derive a real window
from, unlike Orion's `get_sleep_schedules`, so the port replaced a derived
window with a fixed one.

**One flag, computed in one place, fanning out to eight consumers:**

| Consumer | Effect |
|---|---|
| `dial_power_set_night()` | `DUTY_DAY {255,90,10}` → `DUTY_NIGHT {140,40,6}`; night standby follows `bri_night_clock_pct` through its own squared curve, not `bri_night_pct` |
| `dial_haptics_set_night()` (forwarded from power) | In AUTO: effect IDs tick 24→26, stop 10→7, confirm 1→7; error 16 unchanged on purpose; `REG_OD_CLAMP` → SOFT |
| `dial_palette_set_night()` | Whole-table swap to the warm/ember palette, plus a forced `dial_state_commit` (screens read `PAL()` at render time) |
| `scr_dial.c` | Chevron pulse 1.2s → 2.4s; staleness dot at 40% opacity |
| `scr_dial.c` / `scr_standby.c` | Ambient "Update available" line hidden entirely |
| `scr_standby.c` | Clock ink drops to `neutral_holding` |
| Update prompt | `!night` entry gate |
| Auto-update install | `!night` in the eligibility test |

**Everything above is gated on `dial_time_now()` succeeding, with no `else`.**
`s_night` defaults to false, so a dial with no valid clock never enters night
at all and runs day brightness through the small hours. Since `0.1.4`'s
setup gate a dial without a zone is rare; a dial with a zone but no SNTP
sync (internet down) is not. §7 handles both.

**Second finding, adjacent.** The auto-update install window is
`auto_start = 9*60, auto_end = 11*60` — 09:00 to 11:00 — while the setting is
labelled **"Overnight"** (`scr_update.c:441`), the log line says "attempting
v%s in the overnight window", and the comments call it an overnight install.
It is a morning window wearing overnight's name, inherited from Orion's
post-wake branch with the sleep schedule that justified it deleted. §6 fixes
it.

## 2. What is proposed

One Settings row — **Night mode** — opening a `dial_list` picker with three
choices:

```
Back
Off
9 pm – 7 am
10 pm – 6 am
```

Tap commits and returns. That is the whole UI. It is `scr_timezone.c`'s
shape exactly: Back at row 0, every other row a complete final choice, a
checkmark on the current value. No new widget, no time editing, no schedule
engine. This window governs **how the dial presents itself in a dark room**;
it never writes a setpoint and never touches the pad.

**Why presets and not a time editor.** Revision 2 specified two 15–18-row
lists of half-hour times; the owner then proposed an On/Off toggle with a
scrollable hours-then-minutes range. Both are more UI than this feature has
earned before it has been used once. Two presets cover the common case, the
storage below is the *same* storage a time editor would need, and a custom
editor is a fourth row plus one new screen later, with no migration. What
this gives up — day sleepers, unusual bedtimes — is accepted for a beta whose
real job is proving the beta channel works.

**Why not sunrise/sunset.** Raised and rejected 2026-09-03. A timezone is not
a location (America/Chicago spans over an hour of sunset), and the window is
about *sleep*, not darkness — Chicago's sun sets at 4:20 pm in December.
Only the user knows when they are in bed.

### Rejected earlier, still rejected

- **An HH:MM spinner** in the `scr_pad_address.c` octet-wheel shape.
- **A separate "Always night" mode** — a day sleeper is served by a window,
  which is the custom-editor follow-up, not a mode.
- **Per-day windows.**

## 3. State and persistence

Three new fields in `app_state_t`, alongside `screen_timeout_s`:

```c
// Night window. night_on == false disables night mode entirely (day
// palette, day duty, day haptics, around the clock); the two times are
// then dormant, not cleared, so switching back on restores the previous
// window. Minutes from local midnight. Defaults on / 21:00 / 07:00
// reproduce the fixed window this firmware hardcoded before the setting
// existed, so an OTA changes zero devices' behavior until the user taps
// a row — same discipline as screen_timeout_s's off-menu 90s default and
// rel_mode's absolute-stays-absolute migration. Persisted to NVS
// "ui"/"night_on" (u8), "ui"/"night_s" and "ui"/"night_e" (u16,
// nvs_set_u16/nvs_get_u16).
bool     night_on;
uint16_t night_start_min;
uint16_t night_end_min;
```

**Three values, deliberately.** Revision 2 carried Off as a `0xFFFF` sentinel
inside `night_start_min` to avoid a bool that could disagree with the times.
With a toggle-shaped model the bool *is* the state and the times are payload;
`on=false, start=22:00` is not a contradiction, it is a dormant window. The
sentinel is gone, and with it the `u8`-narrowing hazard it created.

**The NVS types are `u8` for the flag and `u16` for both times, end to
end.** A `u8` anywhere on the time path — an `nvs_get_u8` copied from a
neighbouring pref, a narrowing cast — turns 22:00 (1320) into 40 = 00:40
and the feature "works" with a phantom window. Grep for it at review time.

Getter/setter shape is copied from `screen_timeout_s`, not invented: clamp on
read, persist immediately in the setter, **no "changed" hook** — `worker_task`
re-reads the snapshot every steady-state tick, so a change takes effect
within one tick the same way `screen_timeout_s` reaches `power_task` within
100ms. **Clamp-on-read is the only corruption guard**: a stored time outside
`0..1439`, or a pair with `start == end`, snaps the **pair** back to
21:00/07:00 (never one value alone); a stored flag outside `{0,1}` snaps to
on. Fail back to the original fixed behavior. Everything downstream may
assume the triple is sane.

Migration: no keys → the defaults. There is nothing to migrate, because the
value being introduced *is* what the firmware already did.

**Preset table**, in `dial_state.h` beside `DIAL_SCR_TIMEOUT_CHOICES`:

```c
#define DIAL_NIGHT_PRESETS_N 2
static const uint16_t DIAL_NIGHT_PRESET_START[DIAL_NIGHT_PRESETS_N] = { 21*60, 22*60 };
static const uint16_t DIAL_NIGHT_PRESET_END  [DIAL_NIGHT_PRESETS_N] = {  7*60,  6*60 };
static const char *const DIAL_NIGHT_PRESET_LABEL[DIAL_NIGHT_PRESETS_N] = { "9 pm – 7 am", "10 pm – 6 am" };
```

Labels use whole hours with no `:00` because every preset is on the hour;
that keeps the Settings value column short. If a custom editor is added
later its values render `h:mm am` and the presets keep their short form.

## 4. The predicate

One `static inline` in `dial_state.h`, next to the preset table. `main.c`
must not open-code the comparison, and §6 reuses the wrap logic.

```c
// True when now_min (minutes from local midnight) falls inside [start, end).
// Windows wrap midnight — 21:00-07:00 is start > end, the normal case — so
// both orders are handled here rather than at each call site. A pure
// function of its arguments: the same "what should it be at 3:14am"
// discipline SPEC-dial-side-scheduling.md §5 argues for, so a reboot
// mid-night resolves correctly with no sequence state to restart.
static inline bool dial_in_window(uint16_t start, uint16_t end, int now_min)
{
    if (start == end) return false;   // unreachable after §3's clamp; belt-and-braces
    return (start < end) ? (now_min >= start && now_min <  end)
                         : (now_min >= start || now_min <  end);
}

static inline bool dial_night_active(const app_state_t *st, int now_min)
{
    return st->night_on && dial_in_window(st->night_start_min, st->night_end_min, now_min);
}
```

`main.c`'s line becomes:

```c
bool night = dial_night_active(&st, now_min);
```

`st` is already read one line above; `now_min` already exists. The
`s_ui_night` edge-detection block below it is untouched — it still exists
only to fire the palette commit on an actual transition, and it now
correctly fires when the *setting* changes mid-window, not just when the
clock crosses a boundary. Change the setting at 21:30 and the palette
follows on the next tick.

## 5. The screens

**Settings row.** One, inserted after **Brightness** and before **Screen
timeout** — brightness, night mode and screen timeout are all "what the dial
does in a dark room":

```
Brightness
Night mode          9 pm – 7 am
Screen timeout      1m
```

The value renders `Off`, or the matching preset label, or — if the stored
pair matches no preset (a future custom editor, or nothing today) — the
actual times as `h:mm am – h:mm pm`. The row never shows a preset label the
stored pair does not match.

**`SCR_NIGHT_MODE`.** One new screen file: a `dial_list` with Back, Off, and
one row per preset. Row tap: Off → `set_night_on(false)`; a preset →
`set_night_start_min/end_min` to the pair **then** `set_night_on(true)` (two
setters, flag last, so the worker never sees on=true with a half-written
pair). Checkmark on Off if `!night_on`, on the matching preset if on and the
pair matches one, on nothing otherwise. Written by reading `scr_timezone.c`
rather than from this description. Entered from Settings only (no packed
`arg` needed); returns to Settings.

**Cursor seeding.** `dial_list` opens on row 0 (Back) or on the checkmarked
row — check which. With four rows it barely matters here, but if it opens on
Back, seed it on the current value; Timezone and Screen timeout get the same
improvement for free.

### When night is Off, nothing that depends on it may look live

Setting Off leaves the **night brightness rows on the Brightness screen**
(`bri_night_pct`, `bri_night_clock_pct`) consumed by nothing —
`dial_power_set_night(false)` never selects them. Settable, persisted,
displayed, read by no live path is `sched_follow`'s grave exactly. So while
`night_on` is false those rows are **hidden** (item 9 established hiding as
the treatment for a dormant row). Their values are preserved. Turning night
back on restores the rows. This is §7's second question applied to the
*neighbours* of the new setting, not just the setting itself.

**Threading — this is NOT the timezone case, and the resemblance is a trap.**
`SPEC-timezone-source.md` needed `CMD_TZ_CHANGED` because
`dial_time_set_iana_tz()` mutates global libc TZ state that `worker_task`'s
`dial_time_now()` callers read concurrently. Nothing of the kind applies
here: these prefs live in `dial_state`'s own mutex-protected store, so the
picker calls the setters directly from the LVGL task exactly as
`scr_brightness.c` already calls `dial_state_set_bri_night_pct()`, and the
worker picks them up on its next `dial_state_get()`. **No new command, no
queue plumbing.** What makes a call site safe is what else is running, not
which function it looks like.

## 6. The auto-update window follows night end

Replace the fixed constants in `main.c`'s auto-update block with one
function, used by both `main.c` and the Update screen:

```c
// Post-wake install window: two hours after night ends, two hours wide.
// This is what Orion derived from the account's sleep schedule; the
// Somnus port lost the schedule and froze the result at 09:00-11:00,
// which is right only for a 07:00 riser. Falls back to that same fixed
// pair when night is off, since there is no wake time to derive from.
static inline void dial_auto_update_window(const app_state_t *st, int *start, int *end)
{
    if (!st->night_on) { *start = 9 * 60; *end = 11 * 60; return; }
    *start = (st->night_end_min + 120) % 1440;
    *end   = (*start + 120)            % 1440;
}
...
int auto_start, auto_end;
dial_auto_update_window(&st, &auto_start, &auto_end);
bool in_window = dial_in_window((uint16_t)auto_start, (uint16_t)auto_end, now_min);
```

Reusing `dial_in_window` for "is now inside a possibly-wrapping window" is
the point — a second hand-written wrap comparison is how the two drift
apart. Two copies of `+120 % 1440` is the same drift risk in a different
place, hence one function.

With the defaults this computes 09:00–11:00 — **byte-identical to today's
behavior on an unchanged device.** The 10 pm – 6 am preset gives 08:00–10:00.

Keep the `!night` term in the eligibility test. With today's two presets a
derived window can never overlap night and the term is redundant. It costs
one boolean and it is the guard that survives a custom editor with free
hours.

### The coupling has to be visible where it is chosen

This gives the night window a second consumer, and that consumer reboots
the device. Nothing on the Night mode row says so. So the Update screen's
row renders the derived window, from the same function as `main.c`:

```
After wake          9:00–11:00 am
```

When night is off it shows the fixed fallback pair. That is the whole cost
of making the derivation honest.

**Rename the setting while you are in here.** `scr_update.c:441`'s
`"Overnight"` becomes **`"After wake"`**, along with the log line
(`"auto-update: attempting v%s in the post-wake window"`), the comment at
`scr_update.c:174`, and `main.c`'s block header. The current label describes
behavior the code has never had — a user-facing string describing something
the code does not do — and it costs four string edits.

## 7. The recurring-bug check

The project's standing rule for any new setting is two questions.

**Can it be changed from the state the device will actually be in when it
needs changing?** Yes. Settings is reachable at steady state and, since
`0.1.4` (F3), during the connect loop. But there is a real failure mode:
**the setting is inert without a valid clock.** The night block runs only
when `dial_time_now()` succeeds, and that requires `dial_time_valid()` —
`s_synced && s_tz_set`, **both**. Two distinct ways to be inert, with two
distinct fixes:

- **No timezone persisted.** Rare since the setup gate; not impossible (skip
  the gate, reboot, skip again). Fix is one tap on the Timezone row.
- **Timezone set but SNTP never synced** — Wi-Fi up, internet down. The
  state a bedside device sits in during an outage. Checking only the zone
  would render a live-looking value while night silently never engaged —
  the recurring bug shape inside the paragraph written to prevent it.

So the row must not lie, and the check must test **the same condition the
consumer tests** — `!dial_time_valid()`, not a proxy for it. When it is
false, the Settings value gets a suffix naming the actual cause:

```
Night mode          9 pm – 7 am — set timezone      (no zone persisted)
Night mode          9 pm – 7 am — no clock          (zone set, not synced)
```

and the picker screen carries the same line under its title. "Set timezone"
names the action; "no clock" is the honest answer when there is no user
action to take. `scr_settings.c` already has the stacked-second-line
treatment Pad Address uses if the string does not fit on one row. The short
preset labels exist partly so that this suffix fits.

**Does anything actually read it?** This is `sched_follow`'s exact grave —
settable, persistent, displayed, consumed by nothing. Mitigation is
procedural, not architectural: **consumer before control.** Commit 1
changes `main.c` to call the predicate with the hardcoded defaults and adds
the §6 function with a hardcoded 07:00, byte-identical behavior, build
proves it. Commit 2 adds the prefs, the screen, the Settings row, the
Brightness-rows rule, the annotation and the Update-screen line, and swaps
the hardcoded arguments for the state. No "add the pref now, wire it later."

## 8. Not in scope

- **Any pad interaction.** Writing a setpoint on a schedule is
  `SPEC-dial-side-scheduling.md`, deferred to v1.5/v2, and it should keep
  its own window rather than borrow this one — "when the screen dims" and
  "when the bed cools" are different questions.
- **A custom time editor.** The follow-up if the presets prove too few:
  a fourth row, one new screen, the owner's On/Off-plus-scrollable-range
  idea (hours in 1 h steps, minutes in 15 min steps), no storage change.
- **Per-day windows.** A second dimension of list scrolling for a bedside
  knob. No.
- **Sunset/sunrise derivation.** §2.

## 9. Comment corrections owed nearby

- **`dial_state.h:501`** says Screen timeout offers "30s/1m/2m/5m/10m".
  `DIAL_SCR_TIMEOUT_CHOICES` is `{ 5, 15, 30, 60 }` — 5s/15s/30s/1m. The
  comment describes a menu that does not exist.
- **`main.c`'s auto-update block** calls itself the "overnight install"
  four times over a 09:00–11:00 window. §6 covers it.

## 10. Release: the first beta

This ships as **`0.1.5-beta.1`**, and the beta channel has never been used.
`SPEC-ota-readiness.md` §9 is the recipe; the load-bearing parts:

- `PROJECT_VER "0.1.5-beta.1"` — the workflow verifies tag == PROJECT_VER
  exactly, suffix included.
- A `## 0.1.5-beta.1` CHANGELOG section — the workflow hard-fails without
  it, and re-tagging is the expensive recovery.
- Tag `somnus-v0.1.5-beta.1`, push to `somnus` only. `release.yml`
  classifies any `-beta.` tag as a GitHub *prerelease*; stable dials poll
  `/releases/latest`, which excludes prereleases, so **no stable dial ever
  sees this build.**
- On the test dial: Settings → Update → Beta builds → On, then Check for
  updates. The dial scans the newest 5 releases for the highest version
  including prereleases; `is_newer()` ranks `0.1.5-beta.1` above `0.1.4`.
- **Getting off the beta later**: a stable `0.1.5` outranks any
  `0.1.5-beta.N`, so cutting stable graduates the dial automatically; Beta
  builds can then be switched off. Nothing to clean up.

**What the beta proved, 2026-09-03:** the prerelease classification worked;
the list-endpoint scan did **not** — GitHub's release list is not
newest-first, so `per_page=5` never contained the beta. `0.1.5-beta.2`
replaced the scan with a tags lookup (`SPEC-ota-readiness.md` §9.7) and
was pulled over the air by a dial on the fixed code, then again by a
factory-blank device (`HARDWARE-bringup-log.md` §18). The beta toggle, the
endpoint switch and the beta-vs-beta tiebreak are all verified on hardware.

## 11. Revision log

**2026-09-03, revision 3 (owner review, approved for beta).** The UI became
a three-choice picker (Off / 9 pm – 7 am / 10 pm – 6 am), replacing
revision 2's two time lists. Storage moved from a `0xFFFF` sentinel to an
explicit `night_on` flag plus two times, so a later custom editor needs no
migration and the `u8` narrowing hazard disappears. Predicate split into
`dial_in_window` (reused by §6) and `dial_night_active`. §6's derivation
pinned to one function shared with the Update screen. Sunrise/sunset
recorded as rejected with reasons. §10 added: this is the first beta-channel
release and what it has to prove.

**2026-09-03, revision 2 (second review pass).** §7 annotation gated on
`!dial_time_valid()` with two causes named; §5 Off rule for the neighbours;
cursor seeding instead of "decide on the board"; §6 derived window rendered
on the Update screen; NVS types pinned; `start == end` guard labelled
belt-and-braces.
