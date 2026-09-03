# Spec: User-settable night window

Status: **PROPOSAL. Not v1** — no new features until v1 is functional and a
new user can easily add their pad (owner instruction, Sep 1 2026). Written
2026-09-03 at the owner's request as a design decision to hold until after
`0.1.4` is cut. **Revised 2026-09-03 after a second review pass** — §3, §4,
§5, §6 and §7 changed; the change list is at the end. Nothing here is built.

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
`s_night` defaults to false, so a dial with no timezone never enters night at
all and runs day brightness through the small hours. That is the open
iPhone-provisioning bug's knock-on (`SPEC-timezone-source.md`, "Observed
failure"; v1 item 11), and it is a precondition for this feature, not a
problem this feature introduces — see §7.

**Second finding, unrelated to the setting but adjacent to it.** The
auto-update install window is `auto_start = 9*60, auto_end = 11*60` — 09:00
to 11:00 — while the setting is labelled **"Overnight"** (`scr_update.c:441`),
the log line says "attempting v%s in the overnight window", and the comments
call it an overnight install. It is a morning window wearing overnight's
name, inherited from Orion's post-wake branch with the sleep schedule that
justified it deleted. §6 fixes it.

## 2. What is proposed

Two Settings rows — **Night starts** and **Night ends** — each opening a
`dial_list` picker of fixed times in 30-minute steps. The start list carries
**Off** as its first choice. Defaults reproduce today's behavior exactly.

That is the whole feature. No new widget, no time spinner, no schedule
engine. This is `SPEC-timezone-source.md`'s curated-list shape applied to a
second setting, and it deliberately stops short of
`SPEC-dial-side-scheduling.md` — this window governs **how the dial
presents itself in a dark room**. It never writes a setpoint and never
touches the pad.

### Rejected: an HH:MM spinner

Full one-minute granularity via a two-field knob-scrolled entry is the
`scr_pad_address.c` octet shape, which this project already judged
intolerable for a set-once value. Nobody needs their palette to warm at
22:17.

### Rejected: presets + Custom

"Off / 21:00–07:00 / 22:00–06:00 / Custom" reads like fewer taps, but
Custom still needs the two pickers behind it. Strictly more code than two
pickers alone, for a saving on one path.

## 3. State and persistence

Two new fields in `app_state_t`, alongside `screen_timeout_s`:

```c
// Night window, minutes from local midnight. night_start_min ==
// DIAL_NIGHT_OFF disables night mode entirely (day palette, day duty,
// day haptics, around the clock). Defaults 21:00/07:00 reproduce the
// fixed window this firmware hardcoded before the setting existed, so
// an OTA changes zero devices' behavior until the user taps a row —
// same discipline as screen_timeout_s's off-menu 90s default and
// rel_mode's absolute-stays-absolute migration. Persisted to NVS
// "ui"/"night_s" and "ui"/"night_e" as u16 (nvs_set_u16/nvs_get_u16).
uint16_t night_start_min;
uint16_t night_end_min;
```

`DIAL_NIGHT_OFF` is `0xFFFF`, carried **in `night_start_min` itself** rather
than as a separate `night_enabled` bool. One value cannot disagree with
itself; two can, and `SPEC-timezone-source.md`'s "Displaying the current
value" section already rejected a second independently-persisted copy of one
fact for exactly this reason.

**The NVS type is `u16`, end to end.** The Off sentinel is `0xFFFF`, so any
`u8` anywhere on the read or write path — an `nvs_get_u8` copied from a
neighbouring pref, a narrowing cast in the picker — silently turns Off into
`0xFF` = 04:15 and the feature "works" with a phantom window. Grep for it
at review time.

Getter/setter shape is copied from `screen_timeout_s`, not invented: clamp on
read, persist immediately in the setter, **no "changed" hook** — `worker_task`
re-reads the snapshot every steady-state tick, so a change takes effect
within one tick the same way `screen_timeout_s` reaches `power_task` within
100ms. **Clamp-on-read is the only corruption guard**: a stored value that
is neither `DIAL_NIGHT_OFF` nor in `0..1439` snaps back to the 21:00/07:00
default pair (both values together, not one at a time), not to the nearest
menu choice — same argument as `clamp_screen_timeout_s`: fail back to the
original fixed behavior. Everything downstream may assume the pair is sane.

Migration: no key → the defaults. There is nothing to migrate, because the
value being introduced *is* what the firmware already did.

## 4. The predicate

One `static inline` in `dial_state.h`, next to the choice tables. `main.c`
must not open-code the comparison, and §6 reuses this same function.

```c
#define DIAL_NIGHT_OFF 0xFFFFu

// True when now_min (minutes from local midnight) falls inside the window
// [start, end). Windows wrap midnight — 21:00-07:00 is start > end, the
// normal case — so both orders are handled here rather than at each call
// site. A pure function of (start, end, now): the same "what should it be
// at 3:14am" discipline SPEC-dial-side-scheduling.md §5 argues for, so a
// reboot mid-night resolves correctly with no sequence state to restart.
static inline bool dial_night_active(uint16_t start, uint16_t end, int now_min)
{
    if (start == DIAL_NIGHT_OFF) return false;
    if (start == end)            return false;   // unreachable from the tables; see below
    return (start < end) ? (now_min >= start && now_min <  end)
                         : (now_min >= start || now_min <  end);
}
```

The `start == end` line is belt-and-braces, not a design decision. With the
§5 tables (start ≥ 18:00, end ≤ 11:00) and §6's derivation (`end + 120`,
`+ 240`) the two can never be equal, and §3's clamp already catches corrupt
NVS before the predicate sees it. Keep the line — a one-branch guard against
a future table edit is cheap — but do not build UI on the idea that "equal
means off". Off is `DIAL_NIGHT_OFF`, only.

`main.c`'s line becomes:

```c
bool night = dial_night_active(st.night_start_min, st.night_end_min, now_min);
```

`st` is already read one line above; `now_min` already exists. The
`s_ui_night` edge-detection block below it is untouched — it still exists
only to fire the palette commit on an actual transition, and it now
correctly fires when the *setting* changes mid-window, not just when the
clock crosses a boundary. That falls out for free and is the behavior you
want: change the setting at 21:30 and the palette follows on the next tick.

## 5. The screens

**Settings rows.** Two, inserted after **Brightness** and before **Screen
timeout** — brightness, night window and screen timeout are all "what the
dial does in a dark room", and grouping them keeps that legible:

```
Brightness
Night starts        9:00 pm
Night ends          7:00 am
Screen timeout      1m
```

Values render 12-hour with `am`/`pm`. The standby clock is already 12-hour
(`scr_standby.c`'s `render_clock`), and a bedtime list spanning 18:00–02:00
is genuinely ambiguous without the suffix. Both strings are ASCII, so the
compiled Montserrat faces cover them.

**`SCR_NIGHT_WINDOW`.** One new screen file, two lists, selected by a packed
`arg` (0 = start, 1 = end) — `scr_adjust_mode.c`'s `s_origin` idiom, itself
citing `scr_brightness.c`'s. Row 0 is Back; every other row is a complete
final choice that commits and returns in one tap, with a checkmark on the
current value. That is `scr_timezone.c` exactly, and it should be written by
reading that file rather than from this description.

**Choice tables**, in `dial_state.h` beside `DIAL_SCR_TIMEOUT_CHOICES`:

- Start: `Off`, then **18:00 → 02:00** in 30-minute steps — 17 times + Off =
  18 rows.
- End: **04:00 → 11:00** in 30-minute steps — 15 rows.

### When night is Off, nothing that depends on it may look live

Setting Night starts to Off leaves two other controls on screen that are
now consumed by nothing: the **Night ends** row, and the **night brightness
rows on the Brightness screen** (`bri_night_pct`, `bri_night_clock_pct` —
`dial_power_set_night(false)` never selects them). Settable, persisted,
displayed, read by no live path is `sched_follow`'s grave exactly, and this
feature would dig two more of them by default.

So, while start is Off:

- **Night ends** renders its value as **`Off`** and tapping it is a no-op
  (or the row is hidden — item 9 established hiding as the treatment for a
  dormant row; either is acceptable, pick one and use it for both places).
  The stored `night_end_min` is untouched, so turning night back on restores
  the previous end time.
- The Brightness screen's night rows get the **same treatment** — hidden
  or shown as `Off`. Their values are likewise preserved.

This is the §7 second question ("does anything actually read it?") applied
to the *neighbours* of the new setting, not just the setting itself.

### The list must open on the current value

Eighteen rows is the longest list in the firmware (timezone's 11 is today's
champion), and it is entered by someone who wants to move their bedtime by
half an hour. Whether that is a grind depends far less on the step size than
on **where the cursor starts.** From Back (row 0), 9:00 pm → 10:30 pm is
twelve detents through the long list; from the current value it is three.

Before writing the screen, check whether `dial_list` (as `scr_timezone.c`
uses it) seeds the cursor on the checkmarked row or always on row 0.
**If it seeds:** keep 30-minute steps as specified. **If it does not:** add
cursor seeding to `dial_list` — it is the right fix, and it improves
timezone and screen timeout for free — *then* keep 30-minute steps. Do not
drop to hourly steps to compensate for a list that opens in the wrong place;
that treats the symptom and loses 10:30 pm for everyone. Hourly steps
(9 + Off, and 8 rows) remain the fallback only if seeding is impossible for
some reason found on the board.

**Threading — this is NOT the timezone case, and the resemblance is a trap.**
`SPEC-timezone-source.md` needed `CMD_TZ_CHANGED` because
`dial_time_set_iana_tz()` mutates global libc TZ state that `worker_task`'s
`dial_time_now()` callers read concurrently. Nothing of the kind applies
here: these two prefs live in `dial_state`'s own mutex-protected store, so
the picker calls `dial_state_set_night_start_min()` directly from the LVGL
task exactly as `scr_brightness.c` already calls
`dial_state_set_bri_night_pct()`, and the worker picks it up on its next
`dial_state_get()`. **No new command, no queue plumbing.** What makes a call
site safe is what else is running, not which function it looks like.

## 6. The auto-update window follows night end

Replace the fixed constants in `main.c`'s auto-update block:

```c
// Post-wake window: two hours after night ends, two hours wide. This is
// what Orion derived from the account's sleep schedule; the Somnus port
// lost the schedule and froze the result at 09:00-11:00, which is right
// only for a 07:00 riser. Falls back to that same fixed pair when night
// is off, since there is no wake time to derive from.
int auto_start, auto_end;
if (st.night_start_min == DIAL_NIGHT_OFF) {
    auto_start = 9 * 60;  auto_end = 11 * 60;
} else {
    auto_start = (st.night_end_min + 120) % 1440;
    auto_end   = (auto_start + 120)        % 1440;
}
bool in_window = dial_night_active((uint16_t)auto_start, (uint16_t)auto_end, now_min);
```

Reusing `dial_night_active` for "is now inside a possibly-wrapping window" is
the point — a second hand-written wrap comparison is how the two drift apart.
The name reads oddly at this call site; rename it `dial_in_window()` and let
night mode call it too, if that bothers you at implementation time.
**Whichever name, the derivation above must live in one function** (say
`dial_auto_update_window(&st, &start, &end)`) because §6's screen row below
has to compute the identical pair — two copies of `+120 % 1440` is the same
drift risk in a different place.

With the defaults (night ends 07:00) this computes 09:00–11:00 — **byte-identical
to today's behavior on an unchanged device.** A 05:00 riser gets 07:00–09:00
instead of an install three hours after they got up.

Keep the `!night` term in the eligibility test. Given the choice tables
(start ≥ 18:00, end ≤ 11:00) the shortest possible day is seven hours, so a
derived window can never overlap night and the term is redundant today. It
costs one boolean and it is the guard that survives a future edit to those
tables.

### The coupling has to be visible where it is chosen

This gives **Night ends** a second consumer, and that consumer reboots the
device. A user who moves Night ends from 7:00 am to 4:00 am has just moved
their auto-install from 9–11 am to 6–8 am, and nothing on the Night ends
row says so. Renaming the update setting helps only if the row shows what
it resolved to. So the Update screen's row renders the derived window:

```
After wake          9:00–11:00 am
```

computed from the same function as `main.c` (above), so the number on
screen is the number the worker will act on. When night is Off it shows the
fixed fallback pair. That is the whole cost of making the derivation honest;
without it "After wake" is a label for behavior the user can't see.

**Rename the setting while you are in here.** `scr_update.c:441`'s
`"Overnight"` becomes **`"After wake"`**, along with the log line
(`"auto-update: attempting v%s in the post-wake window"`), the comment at
`scr_update.c:174`, and `main.c`'s block header. The current label describes
behavior the code has never had. This is the bring-up log §12 pattern —
a comment (here, a *user-facing string*) describing something the code does
not do — and it costs four string edits.

## 7. The recurring-bug check

The project's standing rule for any new setting is two questions.

**Can it be changed from the state the device will actually be in when it
needs changing?** Yes. Settings is reachable at steady state and, since
review F3, during the connect loop. But there is a real failure mode:
**the setting is inert without a valid clock.** The night block runs only
when `dial_time_now()` succeeds, and `dial_time_now()` requires
`dial_time_valid()` — which is `s_synced && s_tz_set`, **both**. Two
distinct ways to be inert, with two distinct fixes:

- **No timezone persisted** (`dial_time_get_iana_tz()` reports none). The
  item 11 setup gate makes this rare, not impossible (skip the gate,
  reboot, skip again). Fix is one tap on the Timezone row.
- **Timezone set but SNTP never synced** — Wi-Fi up, internet down. This is
  the state a bedside device sits in during an outage, and it is the one
  the first draft of this spec missed: checking only the zone would render
  `9:00 pm` with no annotation while night silently never engaged. That is
  the recurring bug shape inside the paragraph written to prevent it.

So the rows must not lie, and the check must test **the same condition the
consumer tests** — `!dial_time_valid()`, not a proxy for it. When it is
false, the Settings values render with a suffix naming the actual cause:

```
Night starts        9:00 pm — set timezone      (no zone persisted)
Night starts        9:00 pm — no clock          (zone set, not synced)
```

and the picker screen carries the same line under its title. "Set timezone"
names the action; "no clock" is the honest answer when there is no user
action to take. Long, and it earns its length: a silent inert setting is
precisely the shape of five of the six bugs in the bring-up log.
`scr_settings.c` already has the stacked-second-line treatment Pad Address
uses if the string does not fit on one row.

**Does anything actually read it?** This is `sched_follow`'s exact grave —
settable, persistent, displayed, consumed by nothing. Mitigation is
procedural, not architectural: **the prefs, the predicate, the `main.c` call
site and the screens land in one commit.** No "add the pref now, wire it
later." If the work has to be split, split it the other way — change
`main.c` to read the predicate with hardcoded 21:00/07:00 arguments first,
so the consumer exists before the control does. And §5's Off rule applies
the same question to the setting's neighbours: Night ends and the night
brightness rows must not look live when nothing reads them.

## 8. Not in scope

- **Any pad interaction.** This window is presentation only. Writing a
  setpoint on a schedule is `SPEC-dial-side-scheduling.md`, deferred to
  v1.5/v2, and it should keep its own window rather than borrow this one —
  "when the screen dims" and "when the bed cools" are different questions and
  a user may reasonably want different answers.
- **A separate "Always night" mode.** Rejected with Off: a day sleeper is
  served by setting the window to their actual sleep hours, which is what the
  feature is for.
- **Per-day windows** (weekday vs weekend). A second dimension of list
  scrolling for a bedside knob. No.
- **Sunset/sunrise derivation.** Needs a location and a solar table; the
  project just finished removing one baked-in personal constant and is not
  adding a geo dependency for a palette swap.

## 9. Comment corrections owed nearby

Found while reading for this spec, unrelated to the feature, cheap to fix in
whatever pass touches these files:

- **`dial_state.h:501`** says Screen timeout offers "30s/1m/2m/5m/10m".
  `DIAL_SCR_TIMEOUT_CHOICES` is `{ 5, 15, 30, 60 }` — 5s/15s/30s/1m. The
  comment describes a menu that does not exist.
- **`main.c`'s auto-update block** calls itself the "overnight install"
  four times over a 09:00–11:00 window. §6 covers it.

## 10. Revision log

**2026-09-03, second review pass.** Four substantive changes and two small
ones, all from re-reading the first draft against the project's own
recurring-bug rule:

1. §7 — the "no clock" annotation now gates on `!dial_time_valid()` (the
   consumer's actual condition) instead of on the zone alone, and names
   which of the two causes applies. First draft would have shown a live
   value on an unsynced dial.
2. §5 — new "When night is Off" rule: Night ends and the Brightness
   screen's night rows may not look live while nothing reads them.
3. §5 — list length is no longer "decide on the board"; the decision is
   cursor seeding on the current value, with 30-minute steps kept.
4. §6 — the Update screen renders the derived window so the Night ends →
   install-time coupling is visible; derivation lives in one function.
5. §3 — NVS type pinned to `u16` with the `0xFF` narrowing hazard spelled
   out; clamp-on-read stated as the sole corruption guard.
6. §4 — `start == end` guard labelled as unreachable belt-and-braces, not a
   second Off.
