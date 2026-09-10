# Spec: Pad auto-discovery

Status: **IMPLEMENTED AND VERIFIED ON HARDWARE (2026-09-01).** Shipped in
`somnus-v0.1.x` as `components/dial_pad_discovery/`. Verified end to end: pad
address set to a wrong value, dial power-cycled, it scanned with the progress
screen, found the real pad, went to the dial face, and re-persisted the
address itself. Record in `docs/HARDWARE-bringup-log.md` and `V1-scope.md`
item 1.

> **As built vs. as specified — read this before citing the integration
> section.** What shipped is the design in this document *except* the phase
> model: the connect loop still sets `PH_SOMNUS_CONNECTING`, and the trapped-
> user bug was fixed with the four-line `nav_policy()` case-group patch that
> the collapsed "Original §9.2 analysis" below describes — **not** the
> `PH_PAD_UNREACHABLE` redesign from `SPEC-connect-phases.md` that the
> "Concrete integration point" section shows. `PH_PAD_UNREACHABLE` does not
> exist in the tree. `SPEC-connect-phases.md` remains an unadopted proposal.
> Not yet exercised on hardware: the pass-2 slow sweep, the found-nothing
> path, and navigating away mid-scan. The 300ms pass-1 timeout has not been
> re-measured from the ESP32 itself (Refinement 2's standing note).

Everything below this line is the investigation as written on 2026-09-01,
kept intact as the design record — the numbers come from reading the actual
sdkconfig, the actual `main.c` loop, and the actual reference implementation,
not from an estimate.

**Revised same day:** added probe ordering (Refinement 1) and a two-pass
timeout (Refinement 2), both scoped to attack the 38.4s sweep time without
reopening the socket budget, 4-way concurrency, or 256-host cap. See the
Sweep time subsection for the before/after numbers and the Two-pass
subsection for an explicit, non-hand-wavy assessment of whether the two
passes are actually independent.

**Revised again same day:** the independence question above was answered
with a real measurement against the real pad, not left as an open risk. The
synchronous-server-work concern did not materialize (pad think time ~23ms
mean, 47ms worst), but the measurement showed the originally-proposed 200ms
pass-1 timeout was too tight against measured cold-connection latency
(61-189ms ttfb) — **the pass-1 timeout is now 300ms**, not 200ms, with the
worst-case sweep revised to 57.6s accordingly. See Refinement 2 for the full
data and arithmetic.

**Revised again same day:** the §9.2 blocker (connect-phase flapping) has
been fully analyzed and proposed in its own spec, **`docs/SPEC-connect-
phases.md`** — the bug and its fix are independent of discovery, so they're
documented there rather than growing further inside this file. The original
§9.2 analysis here is kept, collapsed, for history; the recommendation it
converged on (a `nav_policy` patch) is superseded by that spec's fuller
fix (collapsing the flapping phases themselves). This file's own "Concrete
integration point" section is updated to use the new phase name,
`PH_PAD_UNREACHABLE`.

## Goal

A factory-reset dial should reach the bed face having been given only a
Wi-Fi password. The captive portal (`docs/SPEC-timezone-source.md`) already
captures credentials and timezone with zero typing; the pad address is the
last thing still requiring roughly 25 spin-and-commit operations on a
character wheel (a *smaller* fix to that same problem — a per-octet
redesign of the entry screen — was drafted but never built; this approach
retires the need for either outright). This also retires the temporary
home-address compiled default
currently sitting uncommitted in the working tree: once discovery exists, a
fresh device needs no working compiled-in address at all — a scan finds the
real one, and the compiled default reverts to the neutral, documentation-only
placeholder it was always meant to be.

## Reference implementation — behavior yes, concurrency no

`~/Projects/SomnusDialPreview/Sources/BedknobMac/PadDiscovery.swift` (Bedknob for Mac)
(+ `PadClient.swift`) is a real, working implementation against the actual
pad. Its **behavior** is the reference:

- No mDNS/SSDP to lean on (confirmed against real hardware — the pad answers
  neither) — brute-force subnet scan is the only option.
- Cache the last-known IP; on every launch, re-validate it with one probe
  before falling back to a full scan (`findPadBaseURL()`) — a scan only runs
  "when it has to."
- Subnet derived from the **real interface address + netmask**
  (`getifaddrs`/`ifa_netmask`), not an assumed `/24` — `network = self &
  netmask`, `broadcast = network | ~netmask`, host range is everything
  between, excluding self/network/broadcast.
- Capped so a misdetected huge subnet can't become an unbounded scan (Swift:
  skip anything with >1024 hosts, i.e. looser than `/22`).
- A hit is validated by **decoding** the response into `PadStateResponse`
  (`side0`/`side1`/`error`), not by trusting a 200 — `isPad(atBaseURL:)`
  explicitly does `try JSONDecoder().decode(...)`, discards on any decode
  failure.
- First qualifying hit wins: `withTaskGroup` races up to 40 concurrent
  probes, and the moment one returns non-nil, the driving loop breaks and
  calls `group.cancelAll()`.

**What does not carry over:** the concurrency number. 40 simultaneous probes
is a Swift `TaskGroup` on a phone/laptop TCP/IP stack with an effectively
unbounded socket table. That number is meaningless on this target and is the
entire reason this pass exists as investigation before implementation — see
below.

## Concurrency — the real constraint

**`CONFIG_LWIP_MAX_SOCKETS=10`** (`sdkconfig:2618`) — this is the *entire
system's* BSD-sockets-API budget, shared by every consumer, not a per-task or
per-component allowance. (`CONFIG_LWIP_MAX_ACTIVE_TCP=16` and
`CONFIG_LWIP_MAX_UDP_PCBS=16` exist too, but `MAX_SOCKETS` is the tighter,
binding constraint — it's the size of the fd-to-pcb table the sockets layer
itself is built on.)

### What else holds a socket, and when

Traced every consumer against the actual timeline discovery would run in
(after Wi-Fi connects, during the pad connect retry loop, before `PH_READY`):

| Consumer | Socket(s) | Held when | Evidence |
|---|---|---|---|
| SNTP (`esp_netif_sntp`) | 1 UDP | Continuously, from `dial_time_start()` (main.c:780, runs immediately after `dial_net_bringup()` returns, i.e. *before* the pad connect loop begins) for the rest of the app's life | `dial_time.c`'s `dial_time_start()` |
| Wi-Fi setup portal (`esp_http_server`) | 1 listening + up to a few accepted | Only *during* provisioning; `portal_stop()` runs before `dial_net_bringup()` returns (`dial_wifi.c:770`, "take the portal DOWN before dialling the real network") | `dial_wifi.c` |
| **DNS hijack task** | **1 UDP, leaked** | **Permanently, for the rest of the boot, on any device that went through the captive portal at all this session** | see below |
| `dial_somnus`'s HTTP client | 1 TCP, transient | Only for the duration of one `do_request()` call (`esp_http_client_init` → `perform` → `cleanup`, `dial_somnus.c:104-114`) — released immediately after, never held between calls | `dial_somnus.c` |
| OTA auto-check | 1 TCP, transient | Only in the steady-state loop (`main.c`, gated on `clock_valid`), never during the initial connect loop — not concurrent with discovery | `main.c` |

**The DNS hijack finding is the one that actually matters here, and it's a
real, pre-existing leak, not a hypothetical:** `dial_wifi.c:680` starts
`dns_hijack_task` once (`xTaskCreate`, never re-created — guarded by
`if (!s_dns_task)`), and that task is an unconditional `for (;;) { recvfrom
(...); ... }` with **no exit path** except its own bind-failure branch
(`dial_wifi.c:294`). `portal_stop()` (`dial_wifi.c:684`) stops the httpd
server only — it never touches `s_dns_task`. So on **exactly the boot this
whole feature targets** (fresh device, just came through the captive
portal), one of the ten socket slots is already gone, permanently, before
discovery's first probe. This isn't being fixed in this pass (out of scope,
per the instructions), but the socket budget below is computed *with* it
gone, not against a clean 10.

Baseline consumption during the discovery window: **1 socket (SNTP) always,
2 on a fresh just-provisioned device (SNTP + the leaked DNS task).**
Worst-case starting budget: **10 − 2 = 8 free slots.**

One assumption stated explicitly rather than silently relied on: LWIP's own
internal DHCP client and ARP do not go through the BSD sockets layer (they
use raw PCBs directly), so they should not draw from this same pool. I
believe this to be correct for this IDF version but have not instrumented it
on real hardware to confirm — flagging it as a desk-review assumption, not a
verified fact, the same distinction this project has insisted on everywhere
else.

### Recommended concurrency: 4 concurrent probes

Not 8 (the full worst-case headroom). Reasoning: 8 consumes the *entire*
remaining budget the instant a scan starts, leaving zero margin for anything
this desk review might have missed, for the leaked-socket count being worse
than estimated, or for the normal jitter of a live system. **4** leaves 4
slots of margin above the worst-case 6-consumed (2 baseline + 4 probes) —
defensible without being maximally aggressive, and it's a single named
constant, easy to raise later once real hardware can be instrumented for
actual `socket()`/`ENOMEM`-class failures rather than estimated blind.

Each probe is a plain `esp_http_client` call against a **numeric IP literal**
(never a hostname) — no DNS resolution per probe, so no transient extra
socket for that, matching `dial_somnus.c`'s own request shape
(`esp_http_client_config_t{ .url, .method, .timeout_ms, .event_handler,
.user_data }`) with `.timeout_ms` set to the probe timeout instead of
`SOMNUS_HTTP_TIMEOUT_MS`'s 5000.

### Sweep time (base formula, before the two refinements below)

Formula: `ceil(candidate_count / concurrency) * probe_timeout`. This is
close to the realistic case, not just the worst case — on an actual
residential `/24`, the large majority of the 254 addresses are simply unused
(no host there at all), and a `connect()` to a dead address doesn't fail
fast the way a live host's closed port does; it waits out close to the full
timeout. So "worst case" and "typical case" are close together here, not far
apart the way they'd be on a fully-populated network. **This same
observation is exactly what both refinements below exploit** — the search
order (Refinement 1) puts the statistically likely candidates where 4-way
concurrency reaches them in seconds instead of tens of seconds; the two-pass
timeout (Refinement 2) stops making every one of the many *dead* addresses
pay the full 600ms when a live host answers in a fraction of that.

At the base numbers — `/24` (254 usable hosts, excluding
network/self/broadcast), 4 concurrent, 600ms timeout, no ordering:

```
ceil(254 / 4) = 64 rounds
64 * 0.6s = 38.4 seconds
```

That's the number the two refinements below revise. See "Revised timing"
at the end of the Two-pass section for the numbers that actually ship.

**Subnet size cap:** tighter than Swift's `/22` (1024 hosts) cap, and
deliberately so — Swift's number assumed 40-way concurrency; this scan runs
at 4-way, roughly a tenth of that. A `/22` here would be ~1024/4*0.6s ≈ 153
seconds (2.5 minutes) — not tolerable at any progress-screen quality. **Cap
the scan at 256 hosts (`/24` or tighter).** Looser than that (`/23`, `/22`,
`/16`, …) is rare on a consumer router in the first place; skip the scan
entirely rather than attempt it, same as Swift does for its own cap, just at
a number sized to this hardware's real concurrency instead of copied from
the reference. **Not reopened by either refinement below**, per the
instructions — both operate strictly within this cap and this concurrency.

Derive the actual subnet from the live interface, not an assumed `/24`:
`dial_net.c`'s `dial_net_ip()` (`dial_wifi.c:238`) already calls
`esp_netif_get_ip_info(s_sta_netif, &ip)` for the IP alone; this needs a
sibling getter exposing the netmask too (`s_sta_netif` is file-static in
`dial_wifi.c` today, not reachable from outside it) — e.g. a new
`dial_net_subnet(uint32_t *ip_host_order, uint32_t *netmask_host_order)`
returning the same `esp_netif_ip_info_t` fields as raw host-order integers,
so the scanner can do exactly the bitwise math `PadDiscovery.swift` does
(`network = ip & netmask`, `broadcast = network | ~netmask`) without
re-deriving it from a string.

### Refinement 1 — probe order (2026-09-01)

The base sweep above treats the candidate set as an unordered range, which
in practice becomes a linear `.1 → .254` walk. That leaves the statistically
likely locations — where a DHCP server actually tends to put a device — no
better placed than the least likely ones. Reordering the *same* candidate
set costs nothing and changes nothing about correctness:

> **This is a heuristic ordering only. It does not remove any candidate, add
> any candidate, or change the worst-case sweep time — a pad at the very end
> of the ordering still takes exactly as long to find as it would under a
> linear sweep. It only changes where a pad is likely to be found relative
> to how much of the sweep has to run first, which is a real, common-case
> difference precisely because most of a `/24` is dead address space (the
> same finding the base formula above already establishes).**

Tiers, checked in this order, each skipping any address already added by an
earlier tier (a single ordered candidate list built once, before the scan
starts — not a re-scan per tier):

| Tier | Range | Size | Why this boundary |
|---|---|---|---|
| 0 (re-scan only) | the previously-persisted address, ± 5 | ≤ 11 | Only applies when the trigger was a *real* address going stale (§ Scan trigger), never the compiled default. DHCP servers overwhelmingly re-lease a *freed* address from their own pool when a client's lease expires and it requests again — that's usually the same address or one immediately adjacent, not a distant one. Re-probing the address that JUST failed (once, cheaply) also guards against the trigger itself having been a transient blip rather than a real move. ±5 (not wider): this tier is testing a specific, narrow hypothesis — "moved a little" — not a general one; the general one is tier 1. |
| 1 | the dial's own last octet, ± 10 | ≤ 21 | The dial just received its own address from this exact DHCP pool. Devices that join a network around the same general period, or whose leases get renewed/reassigned by the same server, commonly cluster nearby — not guaranteed, but the single strongest available heuristic signal, since it's derived from the dial's own real, current network state rather than a guessed convention. ±10, wider than tier 0: this is a broader hypothesis (general pool clustering, not "moved slightly"), so it gets a broader window, while staying small enough to resolve in a handful of concurrent rounds. |
| 2 | `.1`–`.20` | 20 | The conventional low end of a subnet: `.1` is the gateway on the vast majority of consumer routers, and `.2`–`.20` is the range most vendor documentation and most manually-configured static devices (NAS, printer, a second AP) actually use. The pad is unlikely to *be* the gateway, but the whole tier is 20 probes — cheap enough to check rather than special-case out `.1` specifically. |
| 3 | `.100`–`.150` | 51 | The modal default DHCP pool start across major consumer router vendors is `.100`; pool sizes vary (some run to `.149`, some to `.199`, some to `.254`), but `.100`–`.150` covers the pool's *start*, where a device that's been connected longest (a fixed appliance like the pad, plugged in once and left alone, rather than a phone that joins and leaves) is statistically more likely to sit, across the largest number of common vendor defaults. |
| 4 | everything else | remainder | Ascending, exactly the base sweep's order, over whatever tiers 0–3 didn't already cover. |

Note the likely overlap between tiers 1 and 3 in practice: the dial itself
almost certainly got its own address from the same DHCP pool the pad would
be in, so the dial's last octet is itself likely to fall inside or near
`.100`–`.150` — meaning the *effective* unique candidate count across tiers
0–3 is often smaller than the naive sum, not larger.

**Worked cumulative timing** (4-way concurrency; using the base 600ms
timeout here to isolate ordering's effect alone — Refinement 2 below
shortens this further), fresh-scan case (no tier 0):

| After tier | Cumulative candidates | `ceil(N/4)` rounds | Cumulative time |
|---|---|---|---|
| 1 | 21 | 6 | 3.6s |
| 2 | 41 | 11 | 6.6s |
| 3 | 92 | 23 | 13.8s |
| 4 (full sweep) | 254 | 64 | 38.4s (unchanged worst case) |

If the pad sits anywhere in tiers 1–3 — roughly 92 of 254 addresses, ~36%
of the space, concentrated on the statistically likely locations — it's
found in well under 14 seconds even at the base 600ms timeout, without
touching completeness or the worst case at all.

Mechanically, this only changes what the shared cursor in the Concurrency
mechanism section below walks: a precomputed ordered list instead of a raw
numeric range. The 4 worker tasks still just claim "the next candidate,"
unaware of tiers — the ordering is entirely in how the list was built, not
in the workers' logic.

### Refinement 2 — two-pass timeout (2026-09-01)

The base 600ms timeout is sized for the worst case — a pad on a busy,
congested network, or fielding retries. But it's paid by *every dead
address*, which is most of the sweep, since a `connect()` to a nonexistent
host waits out close to the full timeout regardless of what that timeout
is. A live host on a healthy LAN answers far faster than that.

**Two passes over the same ordered candidate list** (Refinement 1's
ordering benefits both passes, not just the first):

- **Pass 1:** the full ordered list at a short timeout.
- **Pass 2:** only if pass 1 found nothing — the same ordered list again, at
  the original 600ms timeout, for a pad that's genuinely slow to answer
  (weak signal, congested Wi-Fi, a busy moment).

**Pass-1 timeout: 300ms — MEASURED, not guessed (2026-09-01).** The
independence question below was resolved empirically rather than left as a
desk-review assumption: real timed requests against the real pad, from a
Mac on the same LAN.

| | n | min | max | mean |
|---|---|---|---|---|
| Warm (ARP cached) — pad think time (ttfb − connect) | 10 | 16ms | 47ms | ~26ms |
| Cold (ARP flushed before each) — connect time | 10 | 42ms | 145ms | — |
| Cold — ttfb (full first-contact latency) | 10 | 61ms | 189ms | ~109ms |
| Cold — pad think time (isolated from connect) | 10 | 19ms | 43ms | ~23ms |

**Are the two passes actually independent? Answered with measurement, not
assumed:**

The risk named in the original draft of this section — that the pad might
do enough synchronous work inside its `GET /api/state` handler (a blocking
sensor read, say) to add a fixed latency floor independent of network
conditions — **does not materialize.** Pad think time is ~23-26ms mean, 47ms
worst, essentially identical warm vs. cold. The pad itself is fast. The two
passes are independent in the way that matters: pass 1's timeout only needs
to clear network/connection latency, not any hidden server-side floor.

**But the timeout has to be sized against cold ttfb, not think time**, and
this is the part the original 200ms estimate got wrong: every probe in a
real scan is a cold first contact with an address whose ARP entry doesn't
exist yet, never a warm re-request. Cold ttfb ranged **61-189ms, mean
~109ms** — the connect handshake (42-145ms of it) dominates, not the pad's
own response generation. 200ms would have caught all 10 cold samples, but
with only 11ms of margin on the worst one (189ms) — and that measurement
came from a Mac Studio's Wi-Fi/network stack; the ESP32 dial has a smaller
antenna and a much lighter TCP/IP stack, and should be expected to do worse
on connect time, not better. A pass-1 timeout that occasionally misses a
live pad is worse than skipping pass 1 entirely: it converts what should be
a fast path into wasted time *plus* the full slow sweep, on exactly the
runs where the fast path should have worked.

**300ms** — roughly 1.6× the observed cold-ttfb maximum (189ms) — is real
margin without being timid about it. Chosen over 200ms specifically because
of the measured cold-ttfb ceiling, not the (already-cleared) think-time
concern.

**This constant should be re-measured from the dial itself once discovery
runs on real hardware — every sample above came from a Mac, not the ESP32.**
If the ESP32's own cold-connect times turn out materially worse than the
Mac's (plausible, given the antenna/stack difference just noted), 300ms is
the constant to revisit, not the two-pass design itself — the failure mode
of a too-tight pass-1 timeout is graceful (pass 2 always catches what pass 1
misses; only the "typical case" number degrades, not correctness), so this
is a tuning task, not a re-derivation.

**Revised timing:**

- **Pass 1 full sweep:** `ceil(254/4) * 0.3s` = **19.2s** (was 12.8s at the
  discarded 200ms).
- **Worst case** (nothing found by either pass): pass 1 (19.2s) + pass 2
  (`ceil(254/4) * 0.6s` = 38.4s) = **57.6 seconds**, up from the single-pass
  baseline of 38.4s and up from the previous draft's 51.2s. Stated plainly,
  not hidden in the arithmetic: **worst case is now ~19 seconds worse than
  a single-pass scan would be** — a real cost, paid only when nothing is
  found by either pass (a pad that's genuinely absent or unreachable, not
  the common case).
- **Typical case** (a live pad, Refinement 1's ordering, pass 1's now-300ms
  timeout — no longer a contingent claim, since the independence risk is
  resolved by measurement above): tier 1's cumulative 6 rounds becomes `6 *
  0.3s = 1.8s`. **This is the number that matters, and it is still
  excellent** — under 2 seconds for a pad in the dial's own DHCP
  neighborhood, up to `ceil(92/4) * 0.3s ≈ 6.9s` if it takes until the end
  of tier 3, up to the full pass-1 sweep at 19.2s if it's outside all three
  heuristic tiers. A real improvement over the base ~38.4s across the whole
  range, not just in the best case.

## Validation — decode, never trust a 200

Matches the Swift reference and, better, an **existing firmware parser**
already does almost exactly this: `dial_somnus.c:176` (`dial_somnus_get_state
()`) already `cJSON_Parse`s the body and requires `side0`/`side1` sub-objects
each with `is_on`/`target_t`/`current_t` fields (`parse_side()`,
`dial_somnus.c:148`) plus a top-level `error`. That function isn't directly
reusable for scanning (it reads/writes `dial_somnus.c`'s own single
`s_base_url` and is bound by `dial_somnus.h`'s explicit "not reentrant,
worker-task-only" contract — fundamentally incompatible with N concurrent
probes against N different candidate addresses), but its **validation
shape** is exactly what a probe should require: parse the body, confirm
`side0` and `side1` are present and each contains `is_on` (bool) and
`target_t` (number) at minimum. A probe that gets a 200 with an unrelated
JSON body, or a 200 with no body, or a non-200 (a printer, a camera, a dev
server on 8080 — the exact false-positive risk the instructions name) fails
this check and is not a match, full stop. `current_t`'s nullability
(confirmed nullable in the real spec) means it's checked for presence as a
key, not required to be a number.

## Scan trigger — on failure, not on every boot, not on every retry

Three-tier policy, sized to avoid "sweep the subnet every 5 seconds" while
still recovering fast from the common case (DHCP lease moved):

1. **Every boot:** try the persisted address directly first, exactly as
   today (`main.c`'s existing `dial_state_get_pad_url()` → `dial_somnus_
   connect()`). No scan if this succeeds — this is "scan on failure," not
   "scan on boot."
2. **First failure of the persisted address:** trigger exactly one scan
   *invocation* immediately, before falling into the normal exponential
   backoff — "one scan" means the whole two-pass sweep (Refinement 2), not
   one pass; pass 2 only runs if pass 1 comes back empty, but both together
   count as the single scan this policy gates. This is the fast path for the
   single most likely real-world cause of a stale address — a DHCP lease
   that moved — and it's also the path a genuinely fresh device (persisted
   address = the compiled default, which after this ships is never a real
   pad) takes on its very first connection attempt.
3. **Subsequent failures:** do **not** scan again on every retry — that's
   exactly the "sweep every 5 seconds" failure mode named in the
   instructions. Fall back to the existing `backoff_wait`/exponential-backoff
   loop untouched, and gate any *further* scan behind a wall-clock cooldown
   (proposed: 5 minutes, tracked the same way `main.c`'s own OTA auto-check
   already tracks `last_ota_check_us` via `esp_timer_get_time()` — an
   existing, already-accepted pattern in this exact file, not a new idiom).
   A pad that's been gone for more than 5 minutes gets re-scanned
   periodically rather than never again; a pad that's merely mid-reboot for
   30 seconds doesn't trigger a second sweep on top of the first.

## First-hit-wins — decided explicitly

Whichever of the concurrent probe workers gets a validated hit first sets a
shared `found` flag (and the found URL) under `dial_state`'s existing store
mutex semantics (or a small dedicated one — see Concurrency mechanism
below); every other worker checks that flag before starting its *next*
probe and stops if it's set. A probe already in flight when the flag flips
is allowed to finish (its own short timeout bounds this to at most one
probe-timeout of tail latency — 300ms during pass 1, 600ms if pass 2 is the
one running) rather than force-closing another task's live socket from
outside — simpler and safer than trying to cancel in-flight blocking I/O
across tasks. Every hit — including a same-scan
runner-up, if the timing ever produces one — gets logged at INFO; only the
first is returned/persisted.

## Concurrency mechanism (concrete enough to implement from, not final code)

`esp_http_client_perform()` is blocking/synchronous (confirmed —
`dial_somnus.c`'s own usage is a plain init→perform→cleanup sequence, and
ESP-IDF's http_client has no first-class async/multiplexed API for many
concurrent targets). The idiomatic way to get bounded concurrency out of
blocking I/O on FreeRTOS is N worker tasks pulling from a shared cursor, not
a single task multiplexing sockets itself:

- A small shared struct: a **precomputed ordered candidate list**
  (Refinement 1's tiers 0–4, built once per scan before any worker starts —
  the workers themselves stay tier-unaware, just consuming "the next slot"),
  a `volatile uint32_t` next-candidate cursor into that list, a `volatile
  bool found` + found-URL buffer, a `volatile uint32_t checked` counter for
  progress, and a mutex protecting the cursor/found/checked fields.
- 4 short-lived worker tasks (spawned fresh per pass — see below — torn
  down after; scans are rare under the backoff policy above, so
  task-creation overhead is a non-issue, not worth a persistent pool), each
  looping: claim the next candidate under the mutex (or exit if `found` or
  exhausted), skip self/network/broadcast, probe it, and on a validated hit
  set `found` under the mutex (first one wins, per above).
- **Two passes, same list, same worker shape, different timeout**
  (Refinement 2): the driver builds the ordered list once, runs the 4-worker
  sweep at the pass-1 timeout (300ms, measured — see Refinement 2) over it;
  if `found` is still false
  when all 4 workers exit, it resets the cursor to the front of the *same*
  list and runs the identical 4-worker sweep again at the pass-2 timeout
  (600ms). The list is not rebuilt or reordered between passes — Refinement
  1's ordering benefits pass 2 exactly as much as pass 1.
- Progress reporting reuses machinery that already exists rather than
  inventing new IPC: `dial_state_set_phase(conn_phase_t, const char *err)`
  (`dial_state.c:728`) already takes the store mutex internally, writes into
  `app_state_t.phase_err` (128 bytes), and unconditionally bumps the
  generation counter the LVGL dispatcher polls. It's already used this way
  for `PH_DEGRADED`'s error text. Each worker calls `dial_state_set_phase
  (PH_PAD_DISCOVERY, progress_buf)` after finishing its own probe — safe to
  call from multiple tasks concurrently since `dial_state`'s own mutex
  serializes it. The progress text now **must carry which pass is running**
  (see New phase and screen below) — a bare `"134/254"` that silently resets
  to `"1/254"` when pass 2 starts would read as the scan restarting from
  scratch, not continuing.
- The function called from the connect loop's failure branch (running on
  `worker_task`, as it already does) drives both passes sequentially,
  blocking until each pass's 4 workers have all exited (a counting "done"
  semaphore each worker gives on exit is the simplest join) before deciding
  whether to run the second. `worker_task` stays synchronous for the whole
  two-pass scan, matching the existing loop's style — only the *helper*
  tasks are new, `worker_task`'s own control flow doesn't change shape.
- Proposed home for this: a **new component**, `components/dial_pad_
  discovery/`, not folded into `dial_somnus` — `dial_somnus.h`'s single-
  instance, worker-task-only, non-reentrant contract is fundamentally
  incompatible with N concurrent probes, and forcing them into the same
  module would mean either violating that contract or maintaining two
  entirely separate code paths inside one file. A separate component keeps
  `dial_somnus`'s existing contract untouched and gives the scanner its own
  clean surface (`bool dial_pad_discovery_scan(char *out_url, size_t sz)`,
  roughly).

## New phase and screen (constraint #4)

New `conn_phase_t` value, **`PH_PAD_DISCOVERY`**, added next to the new
`PH_PAD_UNREACHABLE` (`docs/SPEC-connect-phases.md`) in `dial_state.h`'s
enum — not next to `PH_SOMNUS_CONNECTING`, which that spec retires
entirely (its one call site goes away with the flapping it caused; nothing
else should be left setting it). Same file, same "worker_task is doing
exactly one describable thing right now" idiom every existing phase
already follows.

New screen, `SCR_PAD_DISCOVERY`, shown while that phase is active. What it
shows:

- A short, calm headline — "Looking for your Somnus pad…" — not an error
  framing; this is an expected step, especially on a first boot, not a fault.
- **Real progress**, not a spinner: the `phase_err` text set by the workers
  above, rendered as a fraction/percentage and, ideally, a simple progress
  arc (this UI already has the exact widget for that — the same
  display-only `lv_arc` pattern `scr_brightness.c` and `scr_boost`'s
  duration ring use, just fed `checked/total` instead of a live-adjustable
  value — no drag handle needed here, it's read-only progress).
- **Two-pass-aware progress text, not just a bare fraction** (the concern
  named in the instructions is real: a naive `"134/254"` that silently
  drops back to `"1/254"` when pass 2 starts looks exactly like the scan
  restarting or hanging, not continuing with a purpose). Proposed copy —
  the pass number is carried in the `phase_err` string itself, not a
  separate field, so no new state channel is needed beyond what's already
  proposed:
  - Pass 1: `"134/254"` under the existing "Looking for your Somnus pad…"
    headline — unchanged from the single-pass design, since this is the
    common path and doesn't need extra explanation.
  - Pass 2 (only reachable if pass 1 truly found nothing): the **headline
    itself changes**, not just the fraction, so the transition reads as a
    deliberate escalation rather than a glitch — e.g. "Still looking
    (checking more slowly)…" with `"67/254"` beneath it. The arc/counter
    resetting to 0 is then *explained* by the headline change instead of
    being the only signal the user gets.
- No knob/tap interaction *of its own* — nothing to adjust mid-scan — but
  it must still be escapable to Settings — see `docs/SPEC-connect-phases.md`.

## The §9.2 blocker — moved to its own spec (2026-09-01)

This section originally analyzed and proposed a fix for the connect-phase
flapping bug here. That analysis grew into its own spec,
**`docs/SPEC-connect-phases.md`** — the bug predates pad discovery and is
independent of it (`PH_SOMNUS_CONNECTING`/`PH_DEGRADED` have flapped every
`BACKOFF_MIN_S` since the connect loop was written; discovery is *blocked*
by it, not the cause of it), so it's documented on its own terms there
rather than as a subsection here.

**What that spec concludes, relevant to this one:** the fix is not a patch
to `nav_policy`'s escape list — it's collapsing `PH_SOMNUS_CONNECTING` and
the pre-`PH_READY` use of `PH_DEGRADED` into one new stable phase,
`PH_PAD_UNREACHABLE`, that never flaps while retrying (retry status is
already carried by the existing `retry_in_s` countdown, independent of
phase — no new plumbing needed). `PH_DEGRADED` itself is untouched; it has
a second, distinct, already-correct meaning post-`PH_READY` that must not
be disturbed. `PH_PAD_DISCOVERY` (this spec) needs the identical one-line
`nav_policy` sticky-group addition `PH_PAD_UNREACHABLE` gets — with that fix
in place, one addition is durably sufficient, because there's no longer a
second, unprotected phase for the loop to flap back into between scans.

The rest of this section (below, "Concrete integration point") is updated
to reflect `PH_PAD_UNREACHABLE` rather than `PH_SOMNUS_CONNECTING`/
`PH_DEGRADED` for that reason — see `SPEC-connect-phases.md` for the full
trace, the dependency audit (what else reads these phases), the first-boot
tone question, and the reasoning, not repeated here.

<!-- Original analysis below retained only as history; superseded by
     docs/SPEC-connect-phases.md above. -->

<details>
<summary>Original §9.2 analysis (superseded, kept for history)</summary>

### Root cause, traced precisely (not just restated)

### Root cause, traced precisely (not just restated)

`main.c`'s `nav_policy()` (`main.c:150`) has a case group —
`case PH_READY: case PH_DEGRADED: case PH_WIFI_LOST:` — that, when
`!st->have_state` (true for any device that has never completed a
successful poll, which is exactly the pre-first-connect state), falls
through to an explicit "never trap the user" block (`main.c:300-307`,
literally commented `// Never trap the user (field incident 2026-07-28)`)
that keeps the CURRENT screen if it's `SCR_MENU`/`SCR_SETTINGS`/`SCR_ABOUT`/
`SCR_WIFI`/`SCR_BRIGHTNESS`/`SCR_BRIGHTNESS_MENU`/`SCR_UPDATE`/`SCR_PAD_
ADDRESS`. **`PH_DEGRADED` already has this protection.**

`PH_SOMNUS_CONNECTING` does not — it isn't in that case group at all, so it
falls to the bare `default: return SCR_CONNECTING;` (`main.c:309`), which has
**no sticky-screen check whatsoever**. It unconditionally forces
`SCR_CONNECTING` regardless of what the user is doing.

The actual user-visible cycle: the connect loop sets `PH_SOMNUS_CONNECTING`
at the top of *every* iteration (`main.c:814`, unconditionally, before even
attempting the connect) → forced to `SCR_CONNECTING`, no escape → connect
fails fast → `PH_DEGRADED` set → *now* the escape hatch is live, so a user
who navigates to Settings during this window can stay there → `backoff_wait`
blocks `worker_task` for the current backoff interval (5s initially,
doubling to a 60s cap) → loop repeats, `PH_SOMNUS_CONNECTING` set again →
**forcibly yanks the user off Settings/Pad Address back to `SCR_CONNECTING`,
no matter what they were doing** → repeats indefinitely while the pad stays
unreachable. This matches the reported symptom exactly, traced to the one
specific line (`main.c:309`'s bare default) that causes it.

### Why discovery makes this worse, not just inherits it

Without the nav-policy touch, `PH_PAD_DISCOVERY` isn't reachable at all —
falling to the same bare `default: return SCR_CONNECTING`, the new progress
screen would never actually show; `nav_policy` has to be touched just to
route to it. So **some** change here is not optional, only its scope is.
And once a real scan (up to ~38s) is what's happening during the trapped
window instead of a quick failed-connect blip, the existing bug becomes
substantially more painful to hit — the "safe" `PH_DEGRADED` window between
scans is a single ~5-second backoff tick; the "trapped" `PH_SOMNUS_
CONNECTING` window now includes an entire scan.

### Recommendation: ship the two together, with the smallest fix that exists

**They should ship together.** Discovery's failure path is unusable without
some nav-policy change (nothing would even show the new screen), and the
minimal version of that change is genuinely small — add both
`PH_SOMNUS_CONNECTING` and `PH_PAD_DISCOVERY` to the exact same case group
`PH_DEGRADED` already sits in:

```c
case PH_READY:
case PH_DEGRADED:
case PH_WIFI_LOST:
case PH_SOMNUS_CONNECTING:   // NEW
case PH_PAD_DISCOVERY:       // NEW
```

This is safe specifically because `PH_SOMNUS_CONNECTING` is set from exactly
one call site in the whole codebase (`main.c:814`, confirmed by grep), only
ever during the pre-`PH_READY` connect loop — `have_state` is
architecturally guaranteed false whenever this phase is active, so folding
it into the `if (st->have_state)` branch's sibling case group never actually
takes the `have_state`-true path for it; it always falls through to the same
"never trap" block `PH_DEGRADED` already relies on. Same reasoning extends
cleanly to the new `PH_PAD_DISCOVERY`. Four added lines, reusing an
already-load-bearing code path, not a new mechanism.

### The larger question this pass is surfacing, not resolving

The instructions' own framing is right to flag this separately: **a
first-boot device that has never had a pad is not in a fault state** the way
a previously-working device that lost its pad is. `nav_policy` already
treats first-boot specially in one place — the `fresh_device && !welcomed`
gate to `SCR_WELCOME`, checked ahead of the phase switch entirely
(`main.c:192`). Whether "still hasn't found a pad yet" should get similarly
distinct treatment from "was working, now isn't" — different copy, different
urgency, maybe no forced navigation away from wherever the user is at all,
rather than the reactive "patch the escape hatch" framing above — is a real
product question with a real existing precedent (`fresh_device`) to build on,
but it's a bigger decision than this pass should make unilaterally. The
four-line patch above is the smallest change that makes the failure path
escapable *today*, ships safely alongside discovery, and does not foreclose
a more deliberate first-boot-specific redesign later — it's a floor, not
intended to be read as the final word on first-boot phase semantics.

*(This four-line-patch conclusion is what `SPEC-connect-phases.md`
supersedes — its recommendation collapses the phase itself rather than
patching around it, for the reasons summarized above. Both the first-boot
question and the "what else depends on `PH_DEGRADED`" question this
original analysis flagged are answered in full in that spec — including a
real dependency, `DIAL_NET_EV_LOST`'s phase check, found by actually tracing
every reader rather than assumed absent.)*

</details>

## Concrete integration point in `main.c`

**Updated to reflect `docs/SPEC-connect-phases.md`'s `PH_PAD_UNREACHABLE`**
(replacing the flapping `PH_SOMNUS_CONNECTING`/`PH_DEGRADED` pair this
section originally showed — see that spec for the full reasoning). The
existing loop is still the right integration point and still does **not**
need restructuring in shape — only which phase constant is set, and where.
Today's loop (the exact structure the Sep 1 self-healing fix already
established):

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

Proposed shape, both specs combined (the phase set once before the loop,
never flapping — `SPEC-connect-phases.md` §5 has this same snippet with its
own framing; reproduced here so this spec's integration section stays
self-contained):

```c
dial_state_set_phase(PH_PAD_UNREACHABLE, NULL);   // once, not inside the loop
for (;;) {
    dial_state_get_pad_url(pad_url, sizeof(pad_url));
    if (dial_somnus_connect(pad_url)) { dial_state_set_phase(PH_READY, NULL); break; }

    if (pad_discovery_should_attempt(...)) {          // policy from "Scan trigger" above
        dial_state_set_phase(PH_PAD_DISCOVERY, NULL);
        char found[DIAL_PAD_URL_MAX_LEN + 1];
        if (dial_pad_discovery_scan(found, sizeof found)) {
            dial_state_set_pad_url(found);             // next iteration's dial_state_get_pad_url
            dial_state_set_phase(PH_PAD_UNREACHABLE, NULL);  // back to the stable phase
            continue;                                   // above re-reads this automatically --
        }                                                // no restructuring needed, this IS the
        pad_discovery_mark_attempted();                  // Sep 1 fix's self-healing loop
        dial_state_set_phase(PH_PAD_UNREACHABLE, NULL);  // scan failed, resume waiting
    }

    dial_state_set_phase(PH_PAD_UNREACHABLE, dial_somnus_last_error());  // same phase, new err text
    backoff_wait(backoff_s);
    backoff_s = (backoff_s * 2 > BACKOFF_MAX_S) ? BACKOFF_MAX_S : backoff_s * 2;
}
```

Every `dial_state_set_phase(PH_PAD_UNREACHABLE, ...)` call after the first
is a generation bump with no navigation behind it (`ui_router_go`'s
same-screen no-op) — the phase value never changes while retrying, only
`phase_err` and, via `backoff_wait`'s own separate per-second commits,
`retry_in_s`. This is what makes the loop's own structure not need
restructuring: only *which* constant gets passed to `dial_state_set_phase`,
and where the very first call sits, changed.

The `continue` on a found address skips the backoff entirely and retries
immediately with the freshly-persisted URL — a successful scan shouldn't
still pay a 5-second wait on top of whatever the scan itself already cost
(typically a few seconds to low tens of seconds for a successful find, per
the Sweep time numbers above — the 57.6s worst case only applies when
*nothing* is found and both passes run to exhaustion). Function names above
are illustrative, not final.

## Out of scope for this pass (confirmed against the instructions, not assumed)

- No implementation. This is the spec only.
- Not fixing the DNS-hijack-task socket leak found above — noted as a
  contributing factor to the socket budget, not something this pass resolves.
- Not implementing the connect-phase fix — proposed in
  `docs/SPEC-connect-phases.md`, pending review, not written here.
- Not deciding the first-boot-phase-tone question — proposed in that same
  spec (a persisted `pad_ever_connected` flag), pending review.
- Not touching the temporary home-address default (since reverted to
  `192.168.1.100`, 2026-09-02), the `main.c` connect-loop fix, the
  relative-scale fix, or the timezone work.

## Open questions for the owner

**Resolved by shipping (2026-09-01):** Q1 — accepted as specified (4 probes,
256-host cap, two passes). Q2 — *not* adopted; the minimal `nav_policy`
patch shipped instead, see the as-built note at the top. Q3 — deferred; no
`pad_ever_connected` flag exists. Q4 — resolved as recorded below. Kept for
history:

1. Does the 4-concurrent-probe / 256-host-cap / 57.6-second-worst-case-for-a-
   `/24` combination (up from the 38.4s single-pass baseline, and up again
   from the previous draft's 51.2s now that the pass-1 timeout is 300ms
   instead of 200ms — see Refinement 2's honest tradeoff) read as
   acceptable, given the socket budget itself can only be truly verified
   with real hardware instrumentation (actual socket-exhaustion behavior)?
2. Approve or amend `docs/SPEC-connect-phases.md`'s proposal — the
   `PH_PAD_UNREACHABLE` redesign, not a patch to the escape list — to ship
   in the same change as discovery, as that spec recommends?
3. That spec's §4 proposes a small new persisted `pad_ever_connected` flag
   so first-boot and lost-my-pad-later can read with different tone. Worth
   deciding now, or deferred once discovery is live and the current
   trapped-user bug is no longer masking how first-boot actually feels?
4. ~~The real pad's `GET /api/state` response latency has never been
   measured.~~ **Resolved (2026-09-01):** measured against the real pad —
   see Refinement 2's timing table. Pad think time is ~23-26ms mean, 47ms
   worst; cold ttfb (the number that actually sizes the timeout) is
   61-189ms. Pass-1 timeout set to 300ms on that basis. The remaining
   open item from this measurement: it was taken from a Mac, not the ESP32
   — worth re-measuring from the dial itself once discovery runs on
   hardware, per Refinement 2's note, since the antenna/stack difference
   could push the ESP32's own cold-connect times higher.
