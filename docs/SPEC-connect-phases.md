# Spec: Connect-phase flapping and the escape-hatch bug

Status: **PROPOSAL, NOT ADOPTED (as of 2026-09-02).** The bug this spec
analyzes was fixed on 2026-09-01 by the *minimal* alternative it argues
against — adding `PH_SOMNUS_CONNECTING` and `PH_PAD_DISCOVERY` to
`nav_policy()`'s existing "never trap the user" case group (`main.c` ~297),
plus `SCR_TIMEZONE` added to the sticky-screen list. That fix is verified on
hardware. `PH_PAD_UNREACHABLE` and the `pad_ever_connected` first-boot flag
proposed here do **not** exist in the tree. Whether to still do the fuller
redesign is an open decision, not a scheduled one; the trace below is the
reference for that state machine either way. No code written against this
spec. This is a standalone spec, not a subsection of
`docs/SPEC-pad-discovery.md` — see "Why its own spec" below for why.

## Why its own spec, not a section of SPEC-pad-discovery.md

The bug analyzed here predates pad discovery and is independent of it:
`main.c`'s connect retry loop has flapped `PH_SOMNUS_CONNECTING`/
`PH_DEGRADED` every `BACKOFF_MIN_S` (5) seconds since that loop was written,
with no relationship to scanning. Pad discovery is *blocked* by this bug
(its own failure path lands in the same trap), which is why this analysis
was commissioned now, but the bug, its root cause, and its fix are about the
connect-phase state machine broadly — `main.c`, `dial_state.h`'s
`conn_phase_t`, `nav_policy()`, `scr_connecting.c`, and the Wi-Fi event
handler all participate, none of it specific to scanning. Keeping this as
its own spec means the design of *that* state machine is citable on its own
terms later, and `SPEC-pad-discovery.md` stays focused on subnet-scan
mechanics rather than growing a second, unrelated design inside it.
`SPEC-pad-discovery.md`'s own §9.2 is being replaced with a pointer here.

> **One live gap this trace found, independent of the proposal (verified in
> source 2026-09-02):** `main.c`'s `DIAL_NET_EV_LOST` handler only moves to
> `PH_WIFI_LOST` from `PH_READY`/`PH_DEGRADED`, so a Wi-Fi drop during
> `PH_SOMNUS_CONNECTING` or `PH_PAD_DISCOVERY` is never reflected as a Wi-Fi
> problem — the connect loop just keeps failing. Benign in practice (the loop
> retries), not v1, untracked anywhere else.

## 1. The mechanism, traced precisely

### How a phase change becomes a navigation

`dial_state_set_phase(conn_phase_t phase, const char *err)`
(`dial_state.c:728`) writes `s_state.phase`, optionally `s_state.phase_err`,
and **unconditionally** increments `s_state.generation`, all under the
store's mutex.

`ui_router.c`'s `dispatch_tick` (`ui_router.c:151`, an `lv_timer` firing
every 50ms on the LVGL task — this is the *only* place a worker-task commit
reaches the UI) reads a fresh `app_state_t` snapshot every tick and compares
`st.generation` against the last-rendered generation. On any change:

```c
if (gen_changed || power_changed) {
    ...
    if (s_nav_policy) {
        void *arg = s_current_arg;
        screen_id_t want = s_nav_policy(&st, &arg);
        ui_router_go(want, arg, LV_SCR_LOAD_ANIM_FADE_ON);  // no-op if unchanged
        ...
    }
    if (scr && scr->on_state) scr->on_state(&st);
}
```

`nav_policy()` (`main.c:150`) is a pure function: given the current
snapshot, which screen *should* be showing right now. It runs on **every**
generation bump, not just phase changes — but every `dial_state_set_phase`
call is itself a generation bump, so a phase change always triggers a fresh
`nav_policy()` evaluation on the very next 50ms tick.

`ui_router_go(id, arg, anim)` (`ui_router.c:101`) is cheap to call
repeatedly with an unchanged destination — `if (id == s_current && arg ==
s_current_arg) return;` is the first line, so calling it every tick with the
*same* answer is a harmless no-op (no screen destroyed/recreated, no
animation, no leak). It only does real work — destroy the old screen,
create the new one, animate — when `nav_policy()`'s answer actually changes
from the previous tick.

### Why a user on `SCR_SETTINGS` gets pulled off it

`nav_policy()`'s `switch (st->phase)` has one case group with an explicit
escape hatch and one bare default with none:

```c
case PH_READY:
case PH_DEGRADED:
case PH_WIFI_LOST:
    if (st->have_state) { ... }
    // "Never trap the user (field incident 2026-07-28)"
    {
        screen_id_t cur = ui_router_current();
        if (cur == SCR_MENU || cur == SCR_SETTINGS || cur == SCR_ABOUT ||
            cur == SCR_WIFI || cur == SCR_BRIGHTNESS ||
            cur == SCR_BRIGHTNESS_MENU || cur == SCR_UPDATE ||
            cur == SCR_PAD_ADDRESS)
            return cur;
    }
    return st->phase == PH_READY ? SCR_CONNECTING : SCR_ERROR;
default:                    return SCR_CONNECTING;
```

(`main.c:220-309`.) `PH_DEGRADED` is in the protected group.
`PH_SOMNUS_CONNECTING` is not — it falls to the bare `default`, which has no
`cur` check at all and unconditionally returns `SCR_CONNECTING`.

The connect loop (`main.c:810-820`, established Sep 1) sets
`PH_SOMNUS_CONNECTING` at the **top of every iteration**, unconditionally,
before even attempting the connect:

```c
for (;;) {
    dial_state_get_pad_url(pad_url, sizeof(pad_url));
    dial_state_set_phase(PH_SOMNUS_CONNECTING, NULL);
    if (dial_somnus_connect(pad_url)) break;
    dial_state_set_phase(PH_DEGRADED, dial_somnus_last_error());
    backoff_wait(backoff_s);
    backoff_s = (backoff_s * 2 > BACKOFF_MAX_S) ? BACKOFF_MAX_S : backoff_s * 2;
}
```

Full cycle: `PH_SOMNUS_CONNECTING` set → next dispatch tick forces
`SCR_CONNECTING`, no escape, regardless of where the user is → connect
fails (fast — an unreachable local address doesn't hang) → `PH_DEGRADED`
set → *now* the escape hatch is live: a swipe (see below) or an
already-deliberate visit to Settings/Menu/etc. sticks → `backoff_wait`
blocks `worker_task` for the current backoff interval (5s → doubling to a
60s cap) → loop repeats, `PH_SOMNUS_CONNECTING` set again → **the very next
dispatch tick, regardless of what the user is doing, forces `SCR_CONNECTING`
again** → repeats for as long as the pad stays unreachable.

### One nuance worth being precise about: there IS an escape gesture, and it works — briefly

`scr_connecting.c` (which renders both `SCR_CONNECTING` and `SCR_ERROR` —
registered as the same implementation) has its own `on_gesture`:

```c
static bool on_gesture(lv_dir_t dir)
{
    if (dir != LV_DIR_LEFT && dir != LV_DIR_RIGHT) return false;
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    return true;
}
```

This isn't gated on phase — a swipe works from this screen regardless of
whether the headline currently says "Connecting to your bed..." or "Pad
unreachable." So the accurate description of the bug is **not** "there is no
escape" — it's: **a swipe genuinely reaches `SCR_MENU`, and it genuinely
stays there for as long as the phase happens to be `PH_DEGRADED` (which has
the sticky protection) — but the instant the loop's next iteration sets
`PH_SOMNUS_CONNECTING` again, the very next dispatch tick yanks the user
straight back to `SCR_CONNECTING`, undoing the escape.** To stay off
`SCR_CONNECTING` at all, a user would have to re-swipe every single retry
cycle, forever, for as long as the pad stays unreachable — which is not a
usable escape hatch even though the mechanism technically fires every time.
This matches the reported "~5-second windows" symptom exactly, just traced
to its precise cause rather than restated.

### The screen-leak connection (contained, not the same bug)

`ui_router_go` also documents the field incident this flapping already
caused once: two animated loads inside LVGL's ~220ms `auto_del` window
orphan the older screen, and enough flaps eventually crash on `lv_obj_create
()` returning NULL (`LoadProhibited @0x20`). The fix in place — forcing
`LV_SCR_LOAD_ANIM_NONE` whenever the previous animated load could still be
in flight (a 400ms guard) — contains the *leak*. It does nothing for the
*navigation theft*, which is a separate consequence of the same underlying
flap: one bug, two symptoms, one already patched, one not.

## 2. Proposal: stop flapping at the source, not at the escape hatch

Recommendation, matching the instructions' own lean and now backed by the
full trace above: **one stable phase for the entire pre-`PH_READY` "trying
to reach the pad" period, replacing the `PH_SOMNUS_CONNECTING`/`PH_DEGRADED`
flip entirely.** Not a patch on `nav_policy`'s escape list — eliminating the
thing that needs escaping from in the first place.

### Why this, over patching the escape hatch alone

A version of "the smallest change" was already sketched (add
`PH_SOMNUS_CONNECTING` to `nav_policy`'s sticky case group, four lines) in
`SPEC-pad-discovery.md`'s original §9.2. It's smaller as a diff, but it
leaves the actual defect in place: `worker_task` would still flap the phase
every 5-60 seconds, still generate a full navigation-policy re-evaluation
and (until the 400ms guard) an animation attempt on every single cycle, and
still rely on every future phase that behaves like this (pad discovery's own
`PH_PAD_DISCOVERY` already needed the identical treatment) being manually
added to the same list — a maintenance trap, not a fix. `worker_task`
*already knows*, deterministically, that it's retrying — it's the one
calling `dial_state_set_phase` twice, on purpose, every cycle. There's no
reason to model "actively attempting" and "waiting to retry" as two
different externally-observed phases when they're two sub-states of one
activity, especially when the retry-countdown text is *already* driven by a
separate, lower-churn signal that doesn't need a phase change at all (next
section).

### The mechanism this reuses already exists

`backoff_wait` (`main.c:589`) already publishes a live per-second countdown
independent of phase:

```c
static void backoff_wait(int seconds)
{
    for (int s = seconds; s > 0; s--) {
        dial_state_commit(mut_retry_in, &s);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ...
}
```

`retry_in_s` already bumps `generation` every second (via
`dial_state_commit`) and `scr_connecting.c`'s `PH_DEGRADED` branch already
branches on it (`retry_in_s > 0` → "Retrying in Ns", else → "Retrying...").
**The live countdown UI already doesn't depend on a phase transition** — it
rides `dial_state_commit`'s generation bump exactly like a phase change
would, just without navigating anywhere, because `nav_policy` resolving to
the same screen on every tick is already a no-op in `ui_router_go`. Merging
`PH_SOMNUS_CONNECTING`'s "actively trying" text and `PH_DEGRADED`'s "waiting
to retry" text into one phase's `on_state`, branching on the exact same
`retry_in_s` signal that already exists, is not new plumbing — it's
deleting a phase transition that was never necessary for the UI it drives.

### Concrete shape

New `conn_phase_t` value: **`PH_PAD_UNREACHABLE`**, replacing
`PH_SOMNUS_CONNECTING`'s use in the pre-`PH_READY` connect loop.
**`PH_SOMNUS_CONNECTING` itself should be removed from the enum**, not just
left unreferenced — `main.c:814` (confirmed by grep) is its only call site
in the whole tree, so once that line is changed to
`PH_PAD_UNREACHABLE`, nothing sets `PH_SOMNUS_CONNECTING` again; keeping a
dead enum value around invites a future reader to assume it still means
something. `PH_DEGRADED` **is not touched, renamed, or removed** — see §3,
it has a second, distinct, correctly-working meaning that must survive
unchanged.

The loop (today's exact structure, `main.c:810-820`):

```c
for (;;) {
    dial_state_get_pad_url(pad_url, sizeof(pad_url));
    dial_state_set_phase(PH_SOMNUS_CONNECTING, NULL);
    if (dial_somnus_connect(pad_url)) break;
    dial_state_set_phase(PH_DEGRADED, dial_somnus_last_error());
    backoff_wait(backoff_s);
    backoff_s = (backoff_s * 2 > BACKOFF_MAX_S) ? BACKOFF_MAX_S : backoff_s * 2;
}
```

becomes (set once, before the loop; the loop's failure branch updates
`phase_err` via the same phase value — a no-navigation no-op after the
first tick, per `ui_router_go`'s early-return):

```c
dial_state_set_phase(PH_PAD_UNREACHABLE, NULL);   // ONCE — not inside the loop
for (;;) {
    dial_state_get_pad_url(pad_url, sizeof(pad_url));
    if (dial_somnus_connect(pad_url)) { dial_state_set_phase(PH_READY, NULL); break; }
    dial_state_set_phase(PH_PAD_UNREACHABLE, dial_somnus_last_error());  // same phase, new err text
    backoff_wait(backoff_s);
    backoff_s = (backoff_s * 2 > BACKOFF_MAX_S) ? BACKOFF_MAX_S : backoff_s * 2;
}
```

`scr_connecting.c`'s `on_state` gets one merged case (replacing the
separate `PH_SOMNUS_CONNECTING`/`PH_DEGRADED` branches for this phase only —
`PH_DEGRADED`'s own case, serving its distinct post-`PH_READY` meaning,
stays exactly as it is):

```c
case PH_PAD_UNREACHABLE: {
    if (st->retry_in_s > 0) {
        main_txt = "Pad unreachable";
        main_color = dial_palette_is_night() ? PAL()->ink_secondary : PAL()->warning;
        // ...phase_err echo + "Retrying in Ns" + "Swipe left for menu", ported verbatim
        // from PH_DEGRADED's existing branch.
    } else {
        main_txt = "Connecting to your bed...";   // actively attempting right now
    }
    break;
}
```

`nav_policy`'s sticky case group gains exactly one entry:

```c
case PH_READY:
case PH_DEGRADED:
case PH_WIFI_LOST:
case PH_PAD_UNREACHABLE:   // NEW — and it now only needs to be added ONCE,
                           // because there is no second, unprotected phase
                           // for the loop to flap back into.
```

Because the phase no longer changes value on every cycle, this one addition
is now durably sufficient — not a patch that has to be re-applied every time
a new "worker is busy trying" phase is invented (pad discovery's own
`PH_PAD_DISCOVERY`, below, is the proof: it needs the identical one-line
treatment, and nothing else).

### A smaller, secondary idea considered and not required

The instructions' second option — phase-driven navigation yielding to a
user-initiated screen in general, not just for specific known phases — is
worth naming as a real, separate hardening idea: `dial_state_last_input_us
()` (already used for the quiet-period poll gate) could let `nav_policy`
check "did the user do something recently" before forcibly navigating away
from wherever they are, for *any* phase, not just the ones explicitly
listed. This would be defense-in-depth against the same class of bug
recurring for a phase nobody thought to add to the sticky list. It's not
proposed as part of this change — the primary fix above removes the actual
defect at its source, and a general "recent input wins" rule is a bigger,
more speculative surface (it would need to distinguish "the user is
deliberately here" from "the user touched something in passing 4 seconds
ago and has since walked away," which `dial_state_last_input_us()` alone
doesn't disambiguate). Worth keeping in mind if a similar bug shows up
again in a phase this spec didn't anticipate; not necessary to build now.

## 3. What else depends on these phase values (the part most likely to bite)

Traced every reader of `conn_phase_t`/`.phase` in the tree, not just
grepped for `PH_DEGRADED` by name:

| Site | What it checks | Affected by this change? |
|---|---|---|
| `scr_connecting.c` | `switch (st->phase)`, all values | **Yes, by design** — gets the new `PH_PAD_UNREACHABLE` case (§2); `PH_DEGRADED`'s own case is untouched. |
| `nav_policy` sticky group (`main.c:221`) | `PH_READY`/`PH_DEGRADED`/`PH_WIFI_LOST` | **Yes** — gains `PH_PAD_UNREACHABLE` (§2). `PH_DEGRADED`'s membership and behavior in this group is unchanged. |
| `scr_dial.c:431` | `st->phase != PH_READY` (the staleness dot) | **No.** Only cares whether phase equals `PH_READY`, treats every other value identically. `PH_PAD_UNREACHABLE` is just one more "not ready" value; this check was never reached during the pre-`PH_READY` window anyway (the dial face isn't shown until `have_state`). |
| OTA ambient-prompt gate (`main.c:987`) | `st.phase == PH_READY` | **No.** Same reasoning — exact-match on `PH_READY` only. |
| OTA auto-update-overnight gate (`main.c:1050`) | `st.phase == PH_READY` | **No.** Same. |
| OTA auto-check window gate (`main.c:1191`) | `ota_st.phase == PH_READY` | **No.** Same. |
| **`dial_somnus` steady-state poll failure** (`main.c:1114`, inside the post-`PH_READY` poll loop) | sets `PH_DEGRADED` after 3 consecutive poll failures | **No — and this is the one that most needed tracing carefully.** See below. |
| `net_event_cb`'s `DIAL_NET_EV_LOST` handler (`main.c:1229`) | `st.phase == PH_READY \|\| st.phase == PH_DEGRADED` before setting `PH_WIFI_LOST` | **Yes, needs one line added** — see below. This is the dependency most likely to be missed. |
| `net_event_cb`'s `DIAL_NET_EV_GOT_IP` handler (`main.c:1234`) | `st.phase == PH_WIFI_LOST` | No direct dependency on the flapped values, but see the interaction note below. |

### Why `PH_DEGRADED`'s post-`PH_READY` meaning must not be touched

This is the finding that reshaped the proposal: `PH_DEGRADED` is not one
thing. It fires from two structurally different places:

1. **Pre-`PH_READY`, the connect loop** (`main.c:816`, being replaced by
   `PH_PAD_UNREACHABLE` above) — `have_state` is architecturally guaranteed
   `false` here (no successful poll has ever happened yet this boot), so
   `nav_policy`'s `if (st->have_state)` branch never triggers; it always
   falls to the "never trap" sticky-screen block. This is the buggy case.
2. **Post-`PH_READY`, the steady-state poll loop** (`main.c:1114`, after
   3 consecutive poll failures on a connection that was previously working)
   — `have_state` is `true` here (a successful poll already happened
   earlier this boot to reach `PH_READY` in the first place), so
   `nav_policy` takes the *other* branch entirely: it keeps the user on the
   dial face, with its staleness dot (`scr_dial.c:431`), through the outage.
   **No forced navigation happens at all in this case — it already works
   correctly today**, and it's a completely different code path from the
   bug being fixed.

Reusing `PH_DEGRADED` for the new pre-`PH_READY` stable phase would have
required either accepting that collision (two different real-world
situations sharing one enum value and hoping `have_state` always
disambiguates them correctly downstream) or auditing every `PH_DEGRADED`
reader to make sure a third meaning didn't slip in unnoticed. Introducing
`PH_PAD_UNREACHABLE` as a distinct value sidesteps this entirely — the
working, correct, already-shipped post-`PH_READY` degraded behavior is
provably untouched because nothing about it changes.

### The one real gap found: `DIAL_NET_EV_LOST`

```c
case DIAL_NET_EV_LOST:
    dial_state_get(&st);
    if (st.phase == PH_READY || st.phase == PH_DEGRADED)
        dial_state_set_phase(PH_WIFI_LOST, NULL);
    break;
```

Today, if Wi-Fi drops while the pre-`PH_READY` connect loop happens to be
sitting in `PH_DEGRADED` (physically possible — a router reboot during the
first few minutes of setup, say), this correctly moves to `PH_WIFI_LOST`.
Under the new design, that same moment would have phase
`PH_PAD_UNREACHABLE`, not `PH_DEGRADED`, and this condition would silently
stop firing — a real regression if not caught. **Needs one line**:

```c
if (st.phase == PH_READY || st.phase == PH_DEGRADED || st.phase == PH_PAD_UNREACHABLE)
```

(And, once `PH_PAD_DISCOVERY` exists per the discovery spec, arguably that
too — a Wi-Fi drop mid-scan should also announce itself as `PH_WIFI_LOST`
rather than leaving a scan silently running over a dead network interface.
Flagging this as part of the same one-line fix, not a separate task.)

This is exactly the kind of dependency that's invisible from `main.c`'s
connect loop alone and only turns up by tracing every phase reader —
consistent with the instructions calling this "the part most likely to
bite." It's a small, mechanical fix, but a real one, and it's now written
down rather than left to be discovered by a Wi-Fi drop during someone's
actual setup.

## 4. First boot vs. lost-my-pad-later — should they be distinct?

**Yes, in presentation. No, not as a separate `conn_phase_t` value** — the
*mechanical* behavior (retry cadence, escapability, which screen) is
identical in both cases; only the *tone* should differ, and that's a
smaller fork than a whole new phase.

### The signal that already exists doesn't quite answer this

`fresh_device` (`dial_state.h:419`) is computed once per boot from whether
Wi-Fi credentials exist yet (`main.c:1273`, before the dev-seed injection).
It's an accurate signal for "this is the very first boot after Wi-Fi setup"
— but it's `false` on every subsequent reboot, **even if the pad was never
found in that first session** (device set up, pad not yet powered on, dial
reboots overnight — next boot, `fresh_device` is already `false`, but the
pad still has never been seen). `have_state` doesn't help either — it's a
per-boot runtime flag, `false` at the start of *every* boot including a
routine restart of a device that's connected successfully hundreds of times
before. **Neither existing signal actually answers "has this device ever,
in its whole life, successfully reached a pad."**

### Proposed: a new persisted flag, small and in the existing idiom

`pad_ever_connected` — a `bool`, NVS-backed the same way `rel_mode`/
`haptics_level`/etc. already are, set once (never cleared except by Factory
Reset, which wipes all NVS anyway) the first time `PH_READY` is actually
reached with a real poll behind it. This is the honest signal the other two
don't provide, and it's a small, well-precedented addition, not a new kind
of state for this codebase.

### The fork this enables

`PH_PAD_UNREACHABLE`'s `retry_in_s > 0` ("waiting to retry") branch reads
`pad_ever_connected` for tone, not for any change in cadence or
escapability:

- `!pad_ever_connected` (this device has never once found a pad): calmer,
  setup-toned copy — no "unreachable," no warning color. Something like
  "Looking for your Somnus pad — make sure it's powered on and nearby" in
  `ink_secondary`, not `PAL()->warning`. This is expected, ordinary setup,
  not a fault.
- `pad_ever_connected` (this device had one, and it's gone): today's
  existing framing, unchanged — "Pad unreachable," warning tone at
  daytime, error text echoed, "Retrying in Ns." Something that used to work
  has stopped working; that *should* read as more urgent than first-time
  setup.

Both share the same phase, the same escapability, the same retry cadence —
only `on_state`'s copy/color branches on the extra flag. This is
deliberately the smallest fork that makes the distinction real rather than
cosmetic-only, and it doesn't require `nav_policy` or the escape mechanism
to know or care which case it's in.

## 5. Interaction with pad discovery's `PH_PAD_DISCOVERY`

`SPEC-pad-discovery.md` already proposes `PH_PAD_DISCOVERY` as its own
distinct phase (own screen, own live progress via repeated `phase_err`
updates under the *same* phase value during a scan — up to ~58 seconds
worst case per that spec's latest numbers). That design already avoids the
flapping problem on its own: a scan holds `PH_PAD_DISCOVERY` as one stable
value for its entire duration, and `dial_state_set_phase(PH_PAD_DISCOVERY,
progress_text)` called repeatedly with the *same* phase is exactly the
"generation bump, no navigation" no-op `ui_router_go` already handles safely
(§2, "the mechanism this reuses already exists" — the same property that
makes the primary fix above work is what already makes discovery's own
progress reporting safe).

What discovery's failure path needs from **this** spec: `PH_PAD_DISCOVERY`
added to `nav_policy`'s sticky case group, the exact same one-line addition
`PH_PAD_UNREACHABLE` gets (§2) — with this fix in place, that's now
genuinely sufficient (no companion phase to flap back into), where against
today's code it would only have been a partial patch.

The connect loop's integration point, combining both specs (illustrative,
not final — see each spec's own integration section for the fuller
picture):

```c
dial_state_set_phase(PH_PAD_UNREACHABLE, NULL);   // once
for (;;) {
    dial_state_get_pad_url(pad_url, sizeof(pad_url));
    if (dial_somnus_connect(pad_url)) { dial_state_set_phase(PH_READY, NULL); break; }

    if (pad_discovery_should_attempt(...)) {
        dial_state_set_phase(PH_PAD_DISCOVERY, NULL);
        char found[DIAL_PAD_URL_MAX_LEN + 1];
        if (dial_pad_discovery_scan(found, sizeof found)) {
            dial_state_set_pad_url(found);
            dial_state_set_phase(PH_PAD_UNREACHABLE, NULL);   // back to the stable phase
            continue;
        }
        pad_discovery_mark_attempted();
        dial_state_set_phase(PH_PAD_UNREACHABLE, NULL);       // scan failed, resume waiting
    }

    dial_state_set_phase(PH_PAD_UNREACHABLE, dial_somnus_last_error());
    backoff_wait(backoff_s);
    backoff_s = (backoff_s * 2 > BACKOFF_MAX_S) ? BACKOFF_MAX_S : backoff_s * 2;
}
```

Two phases total across the whole pre-`PH_READY` window
(`PH_PAD_UNREACHABLE` and, only while a scan is actually running,
`PH_PAD_DISCOVERY`), each stable for its own duration, each needing exactly
one `nav_policy` case-group entry, neither ever flapping.

## Out of scope for this pass

- No implementation — analysis and proposal only, per the instructions.
- Not implementing the `pad_ever_connected` flag or the `DIAL_NET_EV_LOST`
  one-line fix — specified, not written.
- Not building the "general nav yields to recent input" idea from §2 — named
  as a considered alternative, not recommended for this change.
- Leaves the temporary home-address default (since reverted to `192.168.1.100`), the `main.c` connect-loop
  fix, the relative-scale fix, and the timezone work untouched, per the
  standing constraints.

## Summary for the owner

- Root cause: `nav_policy`'s bare `default` case has no escape protection,
  and `PH_SOMNUS_CONNECTING` falls into it every single retry cycle,
  undoing whatever escape the user found during the previous `PH_DEGRADED`
  window.
- Fix: collapse `PH_SOMNUS_CONNECTING`/pre-`PH_READY`-`PH_DEGRADED` into one
  new stable phase, `PH_PAD_UNREACHABLE`, reusing the countdown/error-text
  machinery that already exists and already doesn't require navigation to
  update. One `nav_policy` case-group addition, durably sufficient because
  there's no second phase left to flap into.
- `PH_DEGRADED` itself: untouched. Its post-`PH_READY` meaning (a
  previously-working pad going quiet) is a different, already-correct code
  path and must stay that way — confirmed by tracing, not assumed.
- One real dependency found that needs a one-line fix alongside the main
  change: `DIAL_NET_EV_LOST`'s phase check.
- First boot vs. lost-later: distinct tone, same mechanism — a new small
  persisted `pad_ever_connected` flag is the honest way to tell them apart,
  since neither `fresh_device` nor `have_state` actually does.
- Pad discovery's own `PH_PAD_DISCOVERY` slots into the same pattern with
  no changes to its own spec — it already avoids flapping internally; it
  just needed this fix to exist so its sticky-list addition is worth
  making.
