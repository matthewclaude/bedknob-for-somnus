# OTA readiness — investigation and plan

Status: **INVESTIGATION, 2026-09-01.** No code changed. Written to answer one
question: what in the OTA path is proven to work, versus what has only ever
compiled, ahead of the v1 release.

**2026-09-01, second pass:** §7 below investigates version/release identity
specifically — where the running firmware's version string actually comes
from, how it collides (or doesn't) with the 23 tags inherited from the
upstream fork, and whether starting the Somnus lineage at `0.1.0` is safe
given what's already flashed. Still no code changed.

**2026-09-01, third pass — §7's recommendation (Option A) applied.** Code
changed this time. `PROJECT_VER` is now `0.1.0`
(`firmware/dial-idf/CMakeLists.txt:22`); the four `dial-v` → `somnus-v`
spots from §7.3 are all changed together
(`release.yml:14,32,52`, `dial_ota.c`'s `TAG_PREFIX`); `CHANGELOG.md` is
split per §7.5, upstream history moved verbatim to `CHANGELOG-orion.md`.
Built (not flashed — see below) and verified structurally with `esptool
image-info`, not just `strings`:

```
Project name: somnus-dial
App version: 0.1.0
Compile time: Sep  1 2026 19:11:31
```

No `1.4.2` and no `dial-v` survive anywhere in `build/somnus-dial.bin`
(`strings` over the whole binary, zero matches for either); exactly one
`somnus-v` occurrence, from the new `TAG_PREFIX` literal.

Left untouched at this point, deliberately narrower than the four required
spots: two purely cosmetic mentions of `dial-v...` as prose/example text
(`release.yml`'s top-of-file comment, and its "Classify release channel"
step's inline comment) and `dial_ota.h`'s doc comments (lines naming
`"dial-v1.1.0-beta.1"` and `"dial-v"` as examples) — none of these execute,
but see the fourth pass below for the first two.

**2026-09-01, fourth pass — flashed, and fleet is now consistent.**
`idf.py -p /dev/cu.usbmodem83401 flash` (plain flash, not `erase-flash` —
NVS was never in the write list, so Wi-Fi creds/timezone/pad address
survived). All four written segments hash-verified: bootloader
(`0x0`), partition table (`0x8000`), otadata (`0x19000`), app
(`0x20000`). The binary written was confirmed to be the exact one the
third pass verified with `strings`/`image-info` — same size and mtime,
no rebuild occurred in between. **User has independently verified the
board now reports `v0.1.0`.** The board and this document agree as of
this pass; the caution in the previous paragraph about not trusting this
file over the hardware no longer applies, but the general practice (check
the hardware, don't just read the doc) still holds for any future change.

Also this pass: the two cosmetic `dial-v` mentions flagged above
(`release.yml` lines 1 and 82) were fixed to say `somnus-v`, comment-only,
no rebuild/reflash (confirmed via `grep -n "dial-v" release.yml` returning
nothing). **Still outstanding, not yet fixed:** `dial_ota.h`'s doc
comments naming `"dial-v1.1.0-beta.1"` and `"dial-v"` as examples — same
category of stale prose, lower priority since that file is farther from
the CI trigger this project got bitten by, but worth the same treatment
next time that file is touched.

**2026-09-02, fifth pass — reflash independently confirmed on-device, not
just by the flasher's own hash check.** The fourth pass's "user has
independently verified" line above was true but under-specified — it
didn't say how, which is exactly the kind of unverifiable claim this
document otherwise tries not to make. Confirmed method: the dial's own
**Settings → About screen**, on `/dev/cu.usbmodem83401`, reading `v0.1.0`
directly off the device UI. That screen (`scr_about.c:102-104`) formats
`esp_app_get_description()->version` as `"v%s"` — the identical
`esp_app_desc_t.version` field §7.1's serial boot log and this session's
`esptool image-info` check both read, just surfaced through the UI a real
user would actually look at rather than a developer tool. **The
fleet-inconsistent state from the third/fourth passes is resolved as of
this date and this check** — a future session may treat the board and
this document as agreeing, though the general practice (verify against
the hardware before relying on this file for anything load-bearing) still
holds for any change after this one.

**Why this was worth catching before any tag was pushed, not after** —
the two consequences that made the versioning pass (§7) more than
housekeeping:

- **A silently-bricked update channel, forever, for every future 0.x
  release.** `PROJECT_VER` was inherited at `1.4.2` from the upstream
  fork and never bumped down, so the one physical board that existed was
  claiming to be a Somnus Dial `1.4.2` that never shipped. `is_newer()`
  (§7.2) has no concept of a renumbering — a `0.1.0` release would have
  compared as older and been silently refused forever, indistinguishable
  from "already current," with no error anywhere. This was caught while
  "reflash the whole fleet to fix it" meant one board on a workbench, by
  wire, in the time this conversation took. It stops being that cheap the
  moment a second unit exists anywhere it isn't your own desk.
- **The wrong project's release notes on your first real release.**
  `release.yml`'s "Extract release notes from CHANGELOG.md" step
  (`release.yml:49-65`) does an *exact* `## <version>` heading match
  against `CHANGELOG.md` and publishes whatever it finds as that
  release's public GitHub description — it doesn't know or care whose
  history is in the file. Before §7.5's split, that file already
  contained chris023's `## 1.4.2` section. Since your own scheme reserves
  `1.0.0` for real v1 and versions climb from there, a future
  `somnus-v1.4.2` was a real, reachable version number for this project
  to eventually hit — and without the split, tagging it would have made
  `release.yml` publish **chris023's Rotation-setting bugfix notes as a
  Somnus release's description**, silently, because that was the first
  matching section in the file. The split (`CHANGELOG-orion.md` for the
  inherited history, a fresh `CHANGELOG.md` starting at `0.1.0`) makes
  this collision structurally impossible rather than merely unlikely —
  it was a correctness fix for a real, demonstrated failure mode of the
  actual release pipeline, not a filing preference.

## 0. Summary, if you read nothing else

- The private-repo diagnosis is **confirmed**, and it's worse than "zero
  releases 404s the same as no access" — see §1. GitHub 404s *every*
  endpoint under a private repo to an unauthenticated client, including the
  list endpoint, so `dial_ota.c` **cannot tell** "no releases yet" from "no
  access" from "repo doesn't exist." That's true today and will stay true
  after you cut the first release, for as long as the repo stays private.
- `docs/SPEC-update-prompt.md` is **not stale in the way you'd expect** — go
  read §2 first. Nearly everything it specifies is actually built (ambient
  indicator, sheet, wake-edge gating, auto-update, persistence keys). The one
  real staleness is its test plan referencing "the Orion app" to set a test
  bedtime — that app doesn't exist in this fork; the night window is now a
  hardcoded 21:00–07:00 constant with no way to override it from the device
  or any app.
- The OTA *mechanism itself* — download, redirect-follow, write, reboot,
  confirm, rollback — is not merely COMPILED-ONLY. It shipped and was
  battle-tested for real, repeatedly, under this codebase's previous
  identity (`chris023/orion-waveshare-rotary-dial`, public, with a real
  release history through `dial-v1.4.2`). Two of the field bugs that fix
  now-load-bearing code (the redirect buffer size, the rollback timing) are
  in `CHANGELOG.md` with "verified on hardware." See §3 — the honest label
  for most of the pipeline is **PROVEN (under the old identity) / UNVERIFIED
  IN THIS CONFIGURATION**, not COMPILED-ONLY.
- What genuinely has never run even once: anything downstream of the rename
  to `somnus-dial.bin` / the `matthewclaude/somnus-waveshare-rotary-dial`
  repo. No `dial-v*` tag has ever been pushed there; the release workflow
  has never executed against it; there is no `gh-pages` branch on that
  remote at all.
- The asset-naming trap you asked about (§4) **did not spring**: the OTA
  path wants `somnus-dial.bin` (app-only image) and the web flasher wants
  `somnus-dial-merged.bin` (bootloader+partition-table+app, for flashing at
  offset 0). These are two different, correctly-divergent artifacts, and
  `.github/workflows/release.yml` already builds and attaches both under
  the post-rename names. The trap that *did* spring: the release workflow's
  own generated release notes still hardcode a link to
  `https://chris023.github.io/orion-waveshare-rotary-dial/` — the old
  public fork's flasher page, not anything under your control. That's a
  live bug, independent of everything else in this doc.
- Recommendation for §5: split off a separate **public** repo holding only
  binaries + a manifest, keep the source private. It is the smallest diff
  (two URL constants in firmware, one cross-repo publish step in CI) and it
  reuses `dial_ota.c`'s existing GitHub-shaped parsing verbatim. Making the
  source fork public is very slightly less CI work but is a one-way
  disclosure decision that contradicts the reason you went private in the
  first place; self-hosting a bespoke manifest is architecturally cleaner
  but is strictly more firmware code to write and verify before v1.
- §6 gives an ordered test plan, ending with a way to force the rollback
  path deliberately by building a trap image that crashes before it can
  confirm — I traced the exact ESP-IDF bootloader logic that fires (with
  file:line citations) rather than assuming.

---

## 1. The private-repo diagnosis, confirmed

Live requests, run today (2026-09-01) from this machine, unauthenticated,
with the same `Accept`/`User-Agent` shape `dial_ota_check()` sends:

```
GET https://api.github.com/repos/matthewclaude/somnus-waveshare-rotary-dial/releases/latest
-> HTTP/2 404
   {"message":"Not Found","documentation_url":".../releases#get-the-latest-release"}

GET https://api.github.com/repos/matthewclaude/somnus-waveshare-rotary-dial/releases?per_page=5
-> HTTP/2 404
   {"message":"Not Found","documentation_url":".../releases#list-releases"}

GET https://api.github.com/repos/matthewclaude/somnus-waveshare-rotary-dial
-> HTTP/2 404
   {"message":"Not Found","documentation_url":".../repos#get-a-repository"}
```

All three are byte-for-byte the same shape: status 404, identical
`message`. This is deliberate GitHub behavior — a private repo 404s
unauthenticated (and unauthorized-but-authenticated) requests rather than
403ing, specifically so an outside observer can't distinguish "exists,
private" from "doesn't exist." The cost is that **your own device can't
distinguish it from "exists, public, but has no releases yet" either** —
that third case is the only one `dial_ota.c`'s comments actually name.

Concretely, in `dial_ota_check()` (`components/dial_ota/dial_ota.c:216-226`):

```c
if (err == ESP_OK && status == 404) {
    ESP_LOGI(TAG, "no releases published yet (HTTP 404)");
    set_status(OTA_IDLE, NULL, NULL);
    ...
```

This fires identically for:
1. A public repo with genuinely zero releases (what the comment describes).
2. A private repo, any number of releases, hit unauthenticated (today's
   actual state).
3. A typo'd or renamed-away repo name (not today's state, but equally
   silent if it ever happens).

**Can `dial_ota.c` distinguish them? No — not from this response, and not
from any response GitHub will ever send an unauthenticated client against a
private repo.** There is no header, no body field, no status-code
distinction available. The only way to tell them apart is out-of-band
knowledge (you, reading this, knowing the repo is private) or by making an
*authenticated* request — which the dial deliberately never does ("the dial
is unauthenticated by design," per your framing, and per `dial_ota.h`'s own
"public repo's GitHub API" comment).

One more wrinkle worth naming precisely: your comment at
`dial_ota.c:32-38` says the list endpoint "unlike `/releases/latest`...
returns 200 with an empty array for a repo with zero releases." That's true
for a *public* repo. It is **not** what we just measured — the list
endpoint 404s too, because the repo is private, so GitHub 404s everything
under it uniformly. The good news: because the 404-handling `if` block sits
*before* the `beta`/`!beta` branch splits (line 260), both channels already
route through the exact same graceful path today. Nothing is currently
broken by this gap — it just means the comment's stated reasoning for that
code path doesn't apply to the situation actually triggering it right now.

**Does this change the error-reporting story?** Only in what you should
expect once you cut a first release: making the repo private and cutting a
release does *not* make `OTA_AVAILABLE` reachable. The device will report
`OTA_IDLE` — indistinguishable from "you haven't released yet" — forever,
until either the repo goes public or the binary lives somewhere reachable
unauthenticated. This is the practical proof that §5 (repointing at
something public) isn't optional polish; it's required before OTA can ever
leave `OTA_IDLE` on a real device pointed at this repo name.

### 1.1 Seen for real: a pre-repoint dial can never self-heal over the air

This ambiguity stopped being theoretical on 2026-09-01, cutting
`somnus-v0.1.1`: the bring-up board was still running a build made before
`dial_ota.c` was repointed from the private source repo to the public
`matthewclaude/somnus-dial-releases` binaries-only repo. That board queried
the private repo, got the 404 described above, and reported `OTA_IDLE` —
"no releases published yet" — even with `0.1.0` and then `0.1.1` genuinely
published and public at the new location. It had no way to tell the
difference, for exactly the reason in §1: the 404 is indistinguishable from
"not repointed yet" from the device's side.

**The deployment constraint this proves: OTA cannot deliver the repoint
that would let OTA work.** Any dial flashed before the repoint is
permanently stuck querying the wrong repo — no release published anywhere
it's reachable, however many `somnus-v*` tags accumulate at the new
location, will move it out of `OTA_IDLE`. There is no over-the-air fix for
a device that doesn't know where to look. The one-time fix is a single wire
flash of any post-repoint build (`0.1.1` or later) over USB via
`idf.py flash` — a **plain flash, no `erase-flash`**, since NVS (Wi-Fi
credentials, timezone, the pad address discovery already wrote back) must
survive it. From that build onward, the dial is pointed at the public repo
and every later release reaches it over the air normally.

Practically: **every dial that shipped or was hand-flashed before the
repoint needs this one-time USB flash before its first real OTA update can
ever succeed.** This is a real fact about the fleet, not just about
tonight's bring-up board — worth checking for on any device whose flash
history predates the repoint commit, not something the on-device UI can
detect or route around.

## 2. What `docs/SPEC-update-prompt.md` already specifies, and what's stale

Read it in full before touching this feature — it is detailed and current
in every load-bearing way. Verified against the actual code
(`main.c`, `dial_state.c/h`, `scr_update.c`, `scr_update_prompt.c`,
`ui_router.c`, `dial_power.c`):

| Spec says | Built? |
|---|---|
| Ambient "Update available" line on `scr_dial.c`/`scr_standby.c`, not-night only | Yes — matches, including the `pending_verify` mutual-exclusion note |
| Wake-edge-triggered sheet (`SCR_UPDATE_PROMPT`), sticky until one of three exit conditions | Yes — `main.c:988-1075`, matches the gate table verbatim (`ota.status`, `ota_skip`, `ota_defer`, `ota_shown` 24h ceiling, night, `PH_READY`+`have_state`, `dial_time_valid()`, `ota_auto==0`) |
| "Later" = now+23h defer; "Skip this version"; "Update options" → `SCR_UPDATE` | Yes — `scr_update_prompt.c`, `scr_update.c` |
| Auto-update Off/Overnight, default Off, two-failed-installs brake | Yes — `main.c:1085-1141` (`s_ota_auto_fail_ver`/`s_ota_auto_fail_count`) |
| NVS keys `ota_auto`/`ota_defer`/`ota_skip`/`ota_shown`, namespace `"ui"` | Yes — `dial_state.c:95-98,128-130,160-164,243-246,623-678` |
| `SCR_UPDATE` grows Check/Auto-update/Skip/Beta rows | Yes — `scr_update.c` |

So: **do not re-specify any of this.** It's built and matches the spec
closely enough that the spec is a reliable reference for *behavior*, not
just intent.

**What's actually stale**, and it's exactly the kind of thing you flagged
today already having turned up elsewhere — a comment/doc describing a
mechanism this fork deleted:

1. **Test plan step 2** says: *"Verify the prompt does NOT appear inside the
   sleep window (temporarily set a bedtime that covers now, via the Orion
   app)."* There is no Orion app in this fork — the entire Orion
   OAuth/MCP/phone-app pipeline was removed in the Somnus port
   (`b8c2a0f`, and see `dial_ota.h`'s own "now removed" note). More to the
   point, **there is no bedtime schedule of any kind to set**: `main.c:994-997`
   hardcodes the night window to 21:00–07:00 unconditionally, with a comment
   explaining exactly why —
   *"Somnus's local API has no sleep-schedule endpoint to derive a real
   window from (unlike Orion's `get_sleep_schedules`)."* Same story for
   the Overnight auto-update window (`main.c:1086-1088`): fixed 09:00-11:00,
   not derived from any wakeup schedule, for the same reason.

   The practical fix for testing isn't code — it's testing method: to
   exercise the "inside the sleep window" gate, set the *device's clock*
   (not a phone app) to a time between 21:00 and 07:00, e.g. via whatever
   your dev/test build does for wall-clock override, or simply run the test
   between real 21:00 and 07:00. For the Overnight auto-update window,
   likewise there's no wakeup schedule to fake — it's always 09:00-11:00
   local, so just arrange the clock (or wait) for that window.

2. Nothing else in the doc references a removed mechanism. The rest of its
   "stale" flag (the status line at the top) was already self-corrected —
   it says the earlier "not built" status line was superseded, which
   checks out against the code.

## 3. End-to-end trace, honestly labeled

Legend:
- **PROVEN** — exercised for real, on real hardware, with evidence (a
  CHANGELOG entry, a live deployed artifact, or both).
- **PROVEN (old identity)** — the exact same code path was exercised for
  real under this codebase's previous branding/repo, and nothing about the
  Somnus rename touches the mechanism itself (only URL/asset strings
  change) — but it has not been re-run since the rename.
- **COMPILED-ONLY** — builds, has never executed on hardware in any
  configuration.
- **UNREACHABLE-TODAY** — can't currently be reached given the private repo
  and/or other current gates, independent of whether the code is correct.

| Step | Label | Evidence |
|---|---|---|
| TLS validation, `api.github.com` | **PROVEN (old identity)** | `trust_roots.pem`'s own header: chains "verified live against ... github.com ... 2026-07-09," widened 2026-07-27; monthly `.github/workflows/certs.yml` probe. Host is identical regardless of which repo you point at — renaming the repo doesn't touch this. |
| TLS validation, `objects.githubusercontent.com` | **PROVEN (old identity)** | Same PEM file, same live-verified chain; this is also the asset CDN the v1.0.3 fix (below) had to actually download from to prove itself. |
| Redirect `esp_https_ota` follows to the asset host | **PROVEN (old identity)** | `CHANGELOG.md:389-391` (v1.0.3, 2026-07-16): *"Updates couldn't download at all. GitHub redirects release assets to a long signed URL that didn't fit in the dial's request buffer, so every install failed before downloading a byte."* That's a field failure and fix of exactly this step; `dial_ota.c:359-361`'s `buffer_size_tx = 4096` comment is the residue of that fix. |
| Write to the inactive OTA slot | **PROVEN (old identity)** | Implied by every one of the 23 `dial-v*` releases in this repo's history (`dial-v1.0.0` through `dial-v1.4.2`) actually landing on hardware — `CHANGELOG.md` narrates hardware behavior at nearly every version. |
| Reboot into it | **PROVEN (old identity)** | Same evidence — a released version becoming "what the dial runs" requires this step to have worked repeatedly. |
| `ota_confirm_once()` cancelling the pending-verify timer | **PROVEN (old identity)** | `CHANGELOG.md:283-292` (v1.0.10, 2026-07-28): *"A good update could silently roll back. The new firmware marked itself valid only after a successful connection to Orion — 30-60s after reboot ... A power cycle inside that window reverted it. It now confirms itself once the system has demonstrably booted healthy."* This is the exact mechanism now in `main.c`'s `ota_confirm_once()`/30s timer — built in direct response to a real rollback happening on a real device. |
| Rollback path if confirm never happens | **PROVEN (old identity), mechanism independently verified against ESP-IDF v6.0 source** | The v1.0.10 note above is itself proof a rollback fired for real once. I also read the actual bootloader logic (`~/esp/esp-idf` @ tag `v6.0`, matching `sdkconfig.defaults`' pinned toolchain) rather than trust my memory of it — see §6's rollback-test section for the exact mechanism and file:line citations. |

**What has never run, in any form:** everything specific to the *current*
configuration — `somnus-dial.bin` as an asset name, the
`matthewclaude/somnus-waveshare-rotary-dial` repo as a host, and the
`dial-v1.4.2`-and-newer commits as the thing being shipped. Concretely:

- `git tag -l 'dial-v*'` lists 23 tags through `dial-v1.4.2`, all of them
  reachable from history — but `git log` shows `origin` (`chris023/orion-
  waveshare-rotary-dial`) is where those releases actually happened:
  `origin/gh-pages` exists, deployed from commit `4a32427` (v1.4.2), and its
  tree genuinely contains `firmware/latest/orion-dial-merged.bin` and
  `firmware/beta/orion-dial-merged.bin` — a real, working release pipeline
  output, under the old name.
- `somnus` (`matthewclaude/somnus-waveshare-rotary-dial`, the private
  remote this branch tracks) has **no `gh-pages` branch at all** — the
  deploy job in `release.yml` has never completed against it, which means
  it's never even been *triggered* against it (no `dial-v*` tag has been
  pushed there since the rename).
- Everything downstream of `9844aa9` ("rename build identity orion-dial ->
  somnus-dial") through `be2fcda`/`b417841` (asset-name and doc fixes) is
  therefore genuinely **COMPILED-ONLY**: the CI YAML parses and the local
  build produces `somnus-dial.bin`/`somnus-dial-merged.bin` correctly, but
  no tag push has run it for real, so the cross-repo mechanics (private-repo
  Actions permissions, `softprops/action-gh-release@v2` behavior, the
  Pages deploy) are unverified in this exact shape.
- And, independent of all of the above: **no device has ever received an
  `OTA_AVAILABLE` against the current repo**, because of §1 — so even if
  you fix nothing else, the write/reboot/confirm/rollback steps remain
  UNREACHABLE-TODAY in this configuration specifically because the check
  step can never leave `OTA_IDLE`.

## 4. What a release must look like

From `dial_ota.c` and `.github/workflows/release.yml`, cross-checked against
each other (they agree, which is the answer to your asset-naming-trap
question):

- **Tag format:** `dial-vX.Y.Z` or `dial-vX.Y.Z-beta.N`. Required —
  `release.yml`'s "Verify tag matches PROJECT_VER" step fails the whole
  workflow if the tag's version doesn't exactly match
  `CMakeLists.txt`'s `PROJECT_VER`. `dial_ota.c`'s `TAG_PREFIX "dial-v"`
  strips exactly this prefix before comparing.
- **Version comparison (`is_newer()`, `dial_ota.c:133-149`):** numeric
  major.minor.patch compare, then a prerelease tiebreak — equal cores: a
  plain release outranks any `-beta.N` of the same core, and between two
  betas the higher `N` wins. This is genuinely semver §11-shaped; nothing
  about it needs to change for any of the proposals below, since it
  operates purely on the tag string, not on where the tag came from.
- **Beta vs. stable channel:** a tag containing `-beta.` publishes as a
  GitHub *prerelease* (`release.yml`'s "Classify release channel" step) —
  that flag alone is what separates the channels. Stable dials poll
  `/releases/latest`, which excludes prereleases by GitHub's own
  definition; only dials with "Beta builds" on poll the list endpoint and
  scan for the newest tag among prereleases-included entries.
- **Required asset:** exactly one asset literally named `somnus-dial.bin`
  attached to the release (`dial_ota.c:41`, `ASSET_NAME`) — this must be the
  **app-only** image (`idf.py build`'s `build/somnus-dial.bin`), not the
  merged one. `dial_ota_check()` walks `assets[]` for an exact name match
  and fails with `"no somnus-dial.bin asset in latest release"` if it's
  missing, regardless of what else is attached.
- **CHANGELOG requirement:** `release.yml` refuses to publish a tag whose
  version has no matching `## X.Y.Z` section in `CHANGELOG.md` — checked
  *before* the ~3 minute build, deliberately, so a forgotten entry fails in
  seconds.

**Do the OTA path and the web flasher want the same asset name? No, by
design, and correctly so:**

| Consumer | Asset | Contents | Why different |
|---|---|---|---|
| `dial_ota.c` (device OTA) | `somnus-dial.bin` | App partition only | `esp_https_ota` writes into the *inactive OTA app partition* — it must never see a bootloader or partition table, or it would corrupt the flash layout of a device that's already running. |
| `web-flasher/manifest.json` / `manifest-beta.json` (ESP Web Tools) | `somnus-dial-merged.bin` | Bootloader + partition table + `otadata` + app, `esptool merge-bin`'d, flashed at offset `0x0` | A first flash of a blank chip has no bootloader yet — ESP Web Tools writes raw bytes at a fixed offset with no ESP-IDF-aware partition logic, so the *whole* image has to be pre-merged. |

`release.yml` already builds and attaches both, correctly named, in one
job (`somnus-dial.bin` and `somnus-dial-merged.bin`, lines 107 and
121-123). **The trap didn't spring here.** It sprang one line earlier: the
release-notes template this same workflow generates
(`release.yml:76`) still reads

```
"For a first install, use the [browser flasher](https://chris023.github.io/orion-waveshare-rotary-dial/)..."
```

— a hardcoded link to the *old public fork's* Pages URL. That's stale
regardless of anything else in this doc, and it's the one item here I'd
call an outright bug rather than an open design question: fix it to point
at wherever your own flasher ends up living (which §5 also has to decide).

## 5. Repo-topology proposal: separate public binaries-only repo

**Recommendation: do this.** Reasoning below, then the mechanics.

### What breaks / what has to change

Nothing breaks structurally — the two repos don't interact at runtime, only
at release-cut time. Concretely:

- **Firmware:** exactly the two URL constants you named,
  `GITHUB_API_URL` and `GITHUB_API_URL_LIST` (`dial_ota.c:30-40`), repointed
  at `owner/public-binaries-repo`. `ASSET_NAME`, `TAG_PREFIX`, `is_newer()`,
  and every byte of the JSON-parsing logic are unchanged — they only ever
  cared about the shape of a GitHub release, never which repo it lives in.
- **CI:** `release.yml` currently runs `softprops/action-gh-release@v2`
  with no `repository:` input, which defaults to publishing into the repo
  the workflow is running in (the private source repo) using the ambient
  `GITHUB_TOKEN`, which is scoped only to that repo. To publish into a
  *different* repo you need:
  1. A fine-grained PAT (or a GitHub App install token) scoped to
     `contents: write` on the public binaries repo, stored as a secret in
     the private repo (e.g. `RELEASE_REPO_TOKEN`).
  2. `softprops/action-gh-release@v2`'s `repository:` input set to
     `owner/public-binaries-repo`, and `token:` set to that secret instead
     of the default `GITHUB_TOKEN`.
  3. Nothing else about the job changes — same build, same two assets, same
     CHANGELOG-derived notes, same tag-matches-`PROJECT_VER` gate.
- **Web flasher / Pages:** move `web-flasher/`'s deploy target to the new
  public repo too (its own `gh-pages`, or a `docs/` dir served from `main`).
  This is a genuine side-benefit, not just a consequence: GitHub Pages for a
  *private* repo requires a paid plan (Pro/Team/Enterprise) — if the
  `matthewclaude` account or org is on Free, `deploy-pages`'s job as
  currently written may not even be able to publish a reachable page
  against the private repo at all. Publishing to a repo that's public from
  the start sidesteps that question entirely rather than leaving it as a
  dependency to verify.
- **Fix the stale link** from §4 while you're touching this file, since
  you'll know the real target URL at that point.
- **Nothing else.** Version comparison, channel classification, the
  CHANGELOG gate, the tag-verification step — all untouched.

### Does the cert bundle cover it?

Yes, unchanged, zero action needed. Both hosts involved —
`api.github.com` for the version check and `objects.githubusercontent.com`
for the asset download — are **identical regardless of which repo you
point at**; GitHub's API and release-asset CDN are shared infrastructure
across every repo on the platform, public or private. `trust_roots.pem`'s
header already documents both chains as live-verified and specifically
widened in 2026-07 "to survive a routine CA rotation on either host." A
repo rename changes a URL path segment, not a hostname.

### Why not make the source fork public instead?

It's less CI work (no cross-repo token, no second Pages target) — that's
the entire case for it. Against it: you told me you want the source repo
private, and I'm taking that as a deliberate decision (this is a
reverse-engineered/adapted client against another vendor's local device
API — `dial_somnus`'s history is full of comments about matching Somnus's
actual observed behavior). Making the fork public is a one-way disclosure
of that reverse-engineering work and every comment describing it. I don't
think the CI convenience is worth reversing a decision you've already made
for reasons that have nothing to do with OTA. If the privacy reasoning
changes independently, revisit; don't let OTA plumbing be the reason it
changes.

### Why not self-host (S3/Cloudflare Pages/your own server) instead?

This is the architecturally cleaner long-term answer — no dependency on
GitHub's anonymous rate limit (60 req/hour, shared across **all**
unauthenticated `api.github.com` traffic from a given source IP; not a v1
risk with a handful of dials on one home network, but worth knowing it's
shared, not per-device, if that ever changes), full control over the
manifest shape, no cross-repo token to rotate. Against it for *v1*
specifically: `dial_ota_check()`'s parsing is entirely GitHub-shaped
(`tag_name`, `assets[].name`, `assets[].browser_download_url`, the
`/releases/latest` vs. `/releases?per_page=N` split, the prerelease flag as
the channel signal). Self-hosting means designing and shipping a new
manifest schema and rewriting a meaningful fraction of `dial_ota_check()` —
strictly more firmware work to write *and verify on hardware* before a v1
ship than the two-constant change above. The cert bundle happens to already
have headroom for this path too if you want it later (GTS R1-R4 and ISRG
X1/X2 cover Cloudflare and most Let's-Encrypt-issued static hosts already),
so it's not blocked — just not the smallest v1 diff.

## 6. First test, ordered so a failure is diagnosable

Ground rule for every step below: change one variable at a time, cheapest
first, so a failure narrows to one suspect instead of re-opening the whole
private-repo/rename/rewrite question.

**0. Off-device, before touching hardware.** Once you've stood up the
public binaries repo (§5) and cut a real release there, `curl` its
`/releases/latest` and `/releases?per_page=5` the same way I did in §1 and
eyeball the JSON: is `tag_name` exactly `dial-vX.Y.Z`, is there an asset
named exactly `somnus-dial.bin`, does it have a `browser_download_url`
under `objects.githubusercontent.com`? This catches a wrong asset name, a
draft release, or a malformed tag — the cheapest possible mistakes — before
any hardware or TLS is involved.

**1. Same-version check, on real hardware, zero install risk.** Flash the
dial with the current build (whatever `PROJECT_VER` already is), cut a
release at that *exact* version on the new public repo, repoint the two
URL constants at it, and tap "Check for updates." Expect `OTA_IDLE` (not
`OTA_AVAILABLE` — versions match, `is_newer()` correctly says no). This
step alone proves DNS, TLS handshake against both hosts, cert validation
against the real `trust_roots.pem`, JSON parsing, and asset-URL extraction
— the entire check path — with **no possibility of anything being
installed**, because there's nothing newer to install. If this fails, the
failure is unambiguously in the check path, not the install path.

**2. Real install, one version bump.** Cut `dial-vX.Y.(Z+1)` for real
(bump `PROJECT_VER`, add the CHANGELOG section, tag, push — the actual
release process). Let the dial discover `OTA_AVAILABLE`, tap-confirm
install manually via `SCR_UPDATE`. Watch it through `OTA_DOWNLOADING` →
`OTA_READY_REBOOT` → reboot. This is the first real exercise of the write
and reboot steps in this configuration.

**3. Confirm the confirm.** Immediately after the reboot in step 2, watch
the serial log (or just wait ~35s and check `SCR_UPDATE`/`scr_dial.c`'s
"Finalizing update" notice) for `dial_ota_init()`'s boot-time log
(`"boot pending-verify: true"`) followed by `ota_confirm_once()`'s
(`"app marked valid; rollback cancelled"`) — from whichever fires first,
the 30s timer or a successful pad poll. This is the step that has a real
field-incident history (v1.0.10) behind it, so it's worth explicitly
watching rather than assuming it fired silently.

**4. Deliberate rollback.** This is the one step nothing above exercises,
and it's the one you specifically don't want discovered by accident on a
nightstand. I read the actual mechanism in ESP-IDF v6.0 (the pinned
toolchain, per `sdkconfig.defaults`) rather than go from memory, at
`~/esp/esp-idf/components/bootloader_support/src/bootloader_utility.c:395-404`
and `bootloader_common_loader.c:72-90`. The relevant behavior:

   - On **every** bootloader boot, before it picks which app partition to
     run, it scans both OTA slots. Any slot currently marked
     `PENDING_VERIFY` (meaning: last boot into it never called
     `esp_ota_mark_app_valid_cancel_rollback()` before rebooting again, for
     *any* reason — crash, panic, watchdog, or a plain `esp_restart()`) gets
     unconditionally marked `ABORTED` right there, in that same boot.
   - Partition selection then treats `ABORTED` as invalid
     (`bootloader_common_ota_select_invalid`), so it falls back to the
     other slot — the previously-confirmed good image. This happens within
     a single boot, not after N failed attempts.
   - Net effect: rollback is guaranteed on the **second** boot after any
     image that reboots (however it reboots) before it manages to call
     `dial_ota_mark_valid_if_pending()` — i.e., before the earlier of {30s
     uptime past the point in `app_main` where the timer is armed, a
     successful pad poll}.

   To force this on purpose, without writing any code that ships: on a
   **throwaway branch, never merged**, add one line very early in
   `app_main()` — before `dial_ota_init()` even runs — that unconditionally
   crashes or resets (e.g. `abort();`, or an infinite `while(1);` to trip
   the task watchdog). Bump `PROJECT_VER` to something clearly a test build
   (e.g. append `-beta.99` if you want it beta-channel-only so a real dial
   can never pick it up by accident), build, and cut a release from that
   branch the same way as step 2. Let a **test** dial (not a bedside unit)
   install it. Expect: the dial reboots into the trap image, which
   immediately crashes/resets; the *next* boot after that should land back
   on the previous good firmware automatically, with no manual
   intervention. Confirm via `SCR_MENU → About` (or serial log) that the
   running version reverted. This is the one test in this plan I'd insist
   on running before calling v1 done — an OTA whose rollback has never
   fired for real, in *this* configuration, is a claim, not a fact, right
   up until you watch it happen.

   One honest caveat: because the abort-scan is unconditional on state
   (not on *why* the previous boot ended), this also means a perfectly
   good new release that happens to reboot once for an unrelated reason
   inside that same window will also get rolled back. That's a property of
   ESP-IDF's mechanism, not a bug in this codebase, and it errs in the
   direction you want for a bedside device — but it's worth knowing that
   "rolled back" doesn't always mean "the image was bad," only "it didn't
   confirm before something reset it."

**5. Only after 1-4 pass**, layer the SPEC-update-prompt.md behaviors on
top (§2's test plan, steps 2-5 there) — they're independent of the OTA
mechanism itself and testing them earlier just adds noise to any failure
in the parts above.

---

## 7. Version and release identity

Same conclusion as §0: get this wrong once and it's expensive, because a
git tag pushed to a real remote and a device that's already checked in on a
number are both hard to take back. Investigated end to end, live, against
the actual connected hardware — nothing here is inferred from reading code
in the abstract.

### 7.1 Where the running firmware's version comes from, traced exactly

It is a **hardcoded CMake string, full stop — not `git describe`, not
Kconfig, not derived from any tag.** The trace:

`firmware/dial-idf/CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)   # line 6 — defines project(), doesn't run it yet
add_compile_options("-Wno-format")
set(PROJECT_VER "1.4.2")                            # line 12
project(somnus-dial)                                # line 13 — THIS is where PROJECT_VER gets consumed
```

I read ESP-IDF's own build logic (`~/esp/esp-idf/tools/cmake/project.cmake`,
tag `v6.0`, matching this project's pinned toolchain) rather than assume:
the `project()` macro, when it runs (line 13, *after* line 12's `set`),
checks `if(NOT DEFINED PROJECT_VER)` (`project.cmake:696`). Since line 12
already defined it, that whole branch — which would otherwise fall back to
a `version.txt` file, then a CMake `project(... VERSION ...)` argument,
then `git_describe()` of the repo, then finally the literal string `"1"` —
never runs. `git describe` is in the fallback chain **only for a project
that never sets `PROJECT_VER` itself**; this one does, so git plays no part
at all. Confirmed also from `sdkconfig`: `CONFIG_APP_PROJECT_VER_FROM_CONFIG
is not set` (line 820) — the Kconfig override path isn't active either, and
there's no `version.txt` in the project directory to race with it.

That string flows into `esp_app_desc_t.version` at build time (embedded in
the image header), which is what `esp_app_get_description()->version`
returns at runtime — the value `dial_ota_check()` compares against and the
value `CMakeLists.txt`'s own comment (lines 8-11) says must match the
pushed tag: *"Baseline app version (M6 OTA): ... Bump per release; the tag
pushed to GitHub ('dial-vX.Y.Z') must match this for the update check to
ever consider a release 'current' once installed."*

**What the currently flashed build actually reports:** I connected to the
dev board on `/dev/cu.usbmodem83401` (present and idle this session) and
captured its boot log directly — a hard reset via the standard DTR/RTS
pulse (the same one `idf.py monitor`/`esptool --after hard_reset` issue),
then read the UART for a few seconds. No flash was written; nothing was
erased; the board just rebooted into whatever's already on it, same as a
power cycle. The relevant lines, verbatim:

```
I (761) app_init: Application information:
I (765) app_init: Project name:     somnus-dial
I (769) app_init: App version:      1.4.2
I (773) app_init: Compile time:     Aug 30 2026 18:19:31
I (782) app_init: ESP-IDF:          v6.0
```

So: **the project name says `somnus-dial` (renamed), but the version
still says `1.4.2` — inherited verbatim, never bumped down.** `PROJECT_VER`
in `CMakeLists.txt` is still the literal string left over from the fork;
nothing about the Somnus rename (`9844aa9` and after) touched it. This one
board is the concrete, present-tense version of the trap you're asking
about in the abstract.

### 7.2 How `dial_ota.c` compares versions, and what a lower-sorting release does

`is_newer()` (`dial_ota.c:133-149`) does a pure numeric `major.minor.patch`
compare via `sscanf`, then a prerelease tiebreak (§4 of this doc already
covers the tiebreak rules) — no epoch, no namespace, no concept of "this
number belongs to a different product lineage." It returns true **only**
when the offered version's core is strictly greater, or the cores are equal
and the prerelease tiebreak favors it. There is no downgrade path, and
there is no error path either — a lower or equal offered version is not
refused with a message, it is **silently treated as "already current"**:
`dial_ota_check()` (line 315-324) falls through to `set_status(OTA_IDLE,
latest, NULL); ok = true;` — same status as "checked, nothing to do."
Nothing distinguishes "this release is old" from "there's genuinely nothing
new" anywhere in the status, the log, or the UI.

**Your named trap, checked against the actual running value from §7.1:**
running version `1.4.2` (real, on the board right now), offered
`somnus-v0.1.0` → stripped to `0.1.0` → `sscanf` gives `am=0` vs `bm=1` →
`0 != 1` → `return am > bm` → `0 > 1` → **false.** `dial_ota_check()` sets
`OTA_IDLE`. No error, no log line distinguishing this from "you're already
up to date," no visible sign anything is wrong. **Confirmed exactly as you
feared, against the real number currently on the real board** — not a
hypothetical.

### 7.3 What `release.yml` keys off, and what a `somnus-v*` tag does today

The trigger (`.github/workflows/release.yml:14`):
```yaml
on:
  push:
    tags:
      - 'dial-v*'
```
A tag matching anything else — `somnus-v0.1.0` included — **fires no
workflow run at all.** Not a failure, not a skipped-with-message run;
GitHub simply never dispatches the `push` event to this workflow, because
the ref doesn't match the glob. Silence, exactly as you suspected.

If the trigger pattern *were* changed to `somnus-v*` without also updating
the rest of the file, here's precisely what would still need to move
together — I found four spots keyed to the literal string `dial-v`, not
one:

1. **`release.yml:14`** — the trigger glob itself.
2. **`release.yml:32`** — `TAG_VERSION="${GITHUB_REF_NAME#dial-v}"` (the
   "Verify tag matches PROJECT_VER" step). Bash's `${VAR#prefix}` is a
   no-op when the prefix doesn't match, so a `somnus-v0.1.0` tag here would
   leave `TAG_VERSION` as the **full literal string** `somnus-v0.1.0`,
   compared against `PROJECT_VER` (`0.1.0`) — a guaranteed mismatch. This
   fails **safe**, loudly, in seconds, refusing to publish — not a silent
   trap, but it does mean the workflow is currently unusable with the new
   prefix until this line changes too.
3. **`release.yml:52`** — the same strip, same prefix, in "Extract release
   notes from CHANGELOG.md." Same failure mode: the `## <version>` lookup
   would search for a section literally named `## somnus-v0.1.0` instead of
   `## 0.1.0`, find nothing, and correctly refuse to publish rather than
   publish with an empty body.
4. **`firmware/dial-idf/components/dial_ota/dial_ota.c:42`** —
   `#define TAG_PREFIX "dial-v"`. This is the one that does **not** fail
   safe. It runs on the *device*, not in CI, with no CHANGELOG or
   `PROJECT_VER` gate to catch it. If the GitHub tag prefix becomes
   `somnus-v` but this macro doesn't change with it, `release_version()`
   (`dial_ota.c:181-189`) finds no `dial-v` prefix on `somnus-v0.1.0`, skips
   the strip, and hands the **entire literal string** `"somnus-v0.1.0"` to
   `is_newer()`'s `sscanf("%d.%d.%d", ...)` — which matches zero leading
   digits (`s` isn't one) and returns `< 1`, so `is_newer()` returns `false`
   *unconditionally, for every release, forever*, indistinguishable again
   from "already current." This is the one line that must not be forgotten
   if the prefix changes, because nothing downstream will ever tell you it
   was.

(Two more mentions are cosmetic only and don't affect behavior:
`release.yml`'s top-of-file comment and its "Classify release channel"
comment both name `dial-v...` as prose/example text, not logic — worth
fixing for a future reader, not load-bearing.)

### 7.4 The 23 inherited tags — confirmed local-and-upstream only

```
$ git --no-optional-locks tag -l 'dial-v*' | wc -l
23
$ GIT_SSH_COMMAND="ssh -o BatchMode=yes ..." git --no-optional-locks ls-remote --tags origin | grep dial-v | wc -l
23
$ GIT_SSH_COMMAND="ssh -o BatchMode=yes ..." git --no-optional-locks ls-remote --tags somnus | wc -l
0
```

All 23 exist locally (inherited when this checkout was forked/fetched from
`origin`, `chris023/orion-waveshare-rotary-dial`) and all 23 exist on
`origin`. **`somnus` — the private `matthewclaude/somnus-waveshare-rotary-
dial` remote — has zero tags of any kind, confirmed by a live query, not
inferred.** They cannot currently collide with anything in your CI or your
version comparison, because neither reads git tags at all: CI only reacts
to a tag *push event* against whichever remote it's watching, and the
device-side comparison only ever reads `PROJECT_VER`/`esp_app_get_
description()` and a GitHub Release's `tag_name` field (a Release, not a
raw git tag — those are different objects; pushing a tag alone doesn't
create a Release).

**The concrete way they *could* still cause harm, and how to close it off
for good:** nothing stops a future `git push somnus --tags` (or `--follow-
tags`) from pushing all 23 for the first time. Since `release.yml`'s
trigger today is still `- 'dial-v*'`, every one of those 23 pushes would be
a real tag-push event matching the glob — GitHub would dispatch 23 separate
workflow runs, each checking out an old Orion-era commit and attempting to
build and publish it as a release **on the private repo**, under whatever
`PROJECT_VER` and asset names existed at that historical commit (pre-rename
`orion-dial.bin`, mismatched against post-rename expectations). That's a
real mess, not a hypothetical one, and it's the actual mechanism behind the
worry you stated.

**This is resolved automatically, as a side effect, by adopting a new tag
prefix** (§7.3's four-spot change) — once `release.yml`'s trigger is
`somnus-v*` instead of `dial-v*`, none of the 23 `dial-v*` tags matches it,
by construction, regardless of how or when they're pushed. That's a genuine
additional argument for changing the prefix, independent of the numbering
question: it's not just naming hygiene, it's what makes the inherited
history CI-inert. Belt-and-suspenders on top of that: push new tags by
exact name (`git push somnus somnus-v0.1.0`), never with a bare `--tags`/
`--follow-tags` flag against `somnus`, so an accidental push of inherited
tags can't happen regardless of the trigger pattern.

Keeping all 23 in history (both locally and on `origin`, which you don't
control and shouldn't need to) costs nothing and is exactly the right way
to preserve real lineage without pretending this code appeared from
nowhere — nothing above argues for deleting or rewriting any of them, and
the constraints on this task correctly forbid it anyway.

### 7.5 CHANGELOG.md — split it, don't just add a banner

The file today is chris023's, `## 1.0.0` (2026-07-15) through `## 1.4.2`
(2026-08-05), newest-first, and `release.yml`'s "Extract release notes"
step (`release.yml:49-65`) parses it directly with an `awk` script matching
`$0 == "## " ver` — an **exact** heading match, nothing fuzzier.

Here's why a shared file with a "Somnus starts here" banner isn't safe
enough, demonstrated rather than hypothesized: **your own proposed scheme
reserves `v1.0.0` for when v1 scope is complete — and `CHANGELOG.md`
already has a section literally titled `## 1.0.0`**, chris023's
very first release. The moment a real Somnus `## 1.0.0` section gets added,
this file contains two sections with the identical heading. The `awk`
match would resolve correctly *only* by accident of ordering — whichever
`## 1.0.0` appears first (top-to-bottom) in the file wins, silently, for
every tag named `1.0.0` from then on. That's a landmine specifically for
the version number you've explicitly said matters most (the real v1), and
it's the kind of mistake that only surfaces the day someone reorders or
edits the file for an unrelated reason.

**Recommendation: physically split the file**, not just visually separate
it:

1. `git mv CHANGELOG.md CHANGELOG-orion.md`. Add one line at the top:
   *"Inherited changelog from the upstream project this firmware was
   forked from (`chris023/orion-waveshare-rotary-dial`, tags `dial-v*`).
   Preserved for lineage; nothing in this file describes Somnus firmware.
   Somnus's own history starts fresh in `CHANGELOG.md`."* Otherwise
   untouched — same content, same order, same authorship, just relocated.
2. Write a new `CHANGELOG.md`, starting empty except a matching top note:
   *"Somnus firmware changelog, starting at `0.1.0` — this project forked
   from chris023/orion-waveshare-rotary-dial (see `CHANGELOG-orion.md` for
   that project's history through `dial-v1.4.2`); Somnus versioning is
   unrelated to those numbers and starts fresh here."* First real entry
   goes in when you cut the first real tag.
3. No change needed to `release.yml`'s parsing logic — it already only
   ever reads `CHANGELOG.md` by that exact filename, which from this point
   forward contains only Somnus sections, so the exact-heading-match
   `awk` script is now safe by construction: there is exactly one universe
   of version numbers in that file, ever again.

This is a `git mv` plus one new file — not a rewrite of history, not an
erasure of chris023's entries (they move, verbatim, into their own clearly-
labeled file), and it's the only option that makes the §7.4 concern and
this one both structurally impossible rather than merely unlikely.

### 7.6 Your proposed scheme — what it breaks, and the fix

**`somnus-v*` prefix:** doesn't break anything that isn't already going to
be broken by the current `dial-v*`-shaped CI on a repo that's never had a
`dial-v*` tag pushed to it. Requires the four synchronized changes in
§7.3 (three CI lines, one firmware macro) — none of them subtle once
listed, but all four have to move together or the result is either a loud
CI refusal (safe) or a silently-dead OTA check (not safe, and it's the
firmware macro that's the silent one). Also closes off the §7.4 tag-
collision risk as a side effect. No downside I can find to recommend
against it.

**Starting at `0.1.0`: yes, this breaks the version comparison, exactly as
you suspected, and I can now say so with the actual number in hand rather
than "it depends."** §7.1 measured the currently-flashed board at `1.4.2`.
§7.2 confirmed `is_newer()` on this exact pair (`0.1.0` offered, `1.4.2`
running) returns `false`. **Any device that has ever been flashed with a
build whose `PROJECT_VER` was left at the inherited `1.4.2` (or anything
`>= 1.0.0`) can never be moved onto a `0.x` release by OTA — there is no
code path in this firmware that offers a downgrade, by design, and a
lower-numbered epoch looks exactly like a downgrade to `is_newer()`.** This
isn't a bug to fix in the comparison logic; it's the comparison logic
correctly doing its one job (never install something older) applied to a
case it wasn't designed to know about (a renumbering, not an actual
downgrade).

**Options, since I'd rather hand you the choice than pick one silently:**

- **A — Bump `PROJECT_VER` now, reflash every existing board by wire
  (recommended).** Change `CMakeLists.txt:12` to `set(PROJECT_VER
  "0.1.0")` before cutting anything, and re-flash every physical unit that
  currently exists — including the dev board measured in §7.1 — via the
  USB/web flasher (a full flash, not OTA; OTA can't get you there, see
  above). This is trivial *right now, specifically because nothing has
  shipped* — by your own account every existing unit is a bench/dev board
  you have physical access to. Zero code changes to the comparison logic;
  the ordering trap only exists for a device whose on-flash version
  disagrees with the epoch you're now using, and after a one-time wired
  reflash, none do. This is the only option that costs nothing in
  complexity and closes the gap permanently rather than working around it.
- **B — Teach `is_newer()`/`dial_ota_check()` about an epoch reset** (e.g.
  treat the running version as "ignore, this is inherited" above some
  threshold, or add a manual override). More code, more surface to get
  wrong, more to verify before a first release — solving a problem that
  Option A solves for free at this stage of the project. I'd only reach
  for this if there were field units you couldn't physically touch, which
  by your own statement isn't the situation.
- **C — Start numbering above `1.4.2` instead (e.g. `2.0.0`)**, so the
  ordering trap can't occur even without reflashing anything by hand.
  Mechanically sound, but it's solving the wrong problem for the wrong
  reason — it exists only to avoid a five-minute reflash of a handful of
  bench units, at the cost of the exact confusion your `0.x`-until-v1
  reasoning was designed to prevent (a `somnus-v1.4.2` that never shipped,
  implying nine inherited-sounding minor versions of a product that's never
  been in anyone's hands). Rejected for the same reason you gave for not
  starting at `1.4.2` in the first place.

**Recommendation: A.** Nothing about your `0.1.0`-first / `1.0.0`-reserved
scheme needs to change — it just has to be paired with bumping
`PROJECT_VER` and reflashing existing boards *before*, not after, the
first tag is pushed. Do it in this order: (1) split `CHANGELOG.md` per
§7.5, (2) make the four `dial-v` → `somnus-v` changes per §7.3, (3) bump
`PROJECT_VER` to `0.1.0` in the same commit, (4) reflash every existing
board by wire and confirm each one now reports `0.1.0` the same way §7.1
confirmed `1.4.2` — then, and only then, tag and push the first
`somnus-v0.1.0` release.

---

## 8. The hardcoded pad-address default was a release blocker, not tidiness

**2026-09-01.** `DIAL_PAD_DEFAULT_BASE_URL` (`dial_state.h:343`) and
`SOMNUS_DEFAULT_BASE_URL` (`dial_somnus.h:53`) were temporarily set to
`http://192.168.1.169:8080` — a real home IP — while pad discovery was
being built and verified on hardware, per commit `9ff178e`'s neutralization
of the *previous* hardcoded default. That reason is now gone: discovery
(`components/dial_pad_discovery/`, `docs/SPEC-pad-discovery.md`) is proven
on real hardware, so both constants are reverted to `192.168.1.100` — the
address the official Somnus local-API spec itself uses as its documented
example (`dial_somnus.h`'s surrounding comments already said so, at lines
12 and 87; confirmed rather than assumed, and neither needed touching).

This belongs in an OTA-readiness document, not a separate note, because
**it is specifically a release-blocking problem, not a cosmetic one, and
the reason is the release mechanism this whole file is about:** a
compile-time default is baked into every copy of `somnus-dial.bin` that
`release.yml` builds and attaches to a GitHub Release. Ship a tag with a
real home IP compiled in, and *every device that ever flashes that
release* — via OTA per §3-§6, or via the web flasher — carries that same
address as its fresh-boot fallback. It doesn't matter that NVS overrides
it the moment Settings has a real value; a brand-new board, or an
existing one after an NVS erase, has no NVS value yet and would try to
reach somebody's actual home network. That's not a bug that surfaces on
your bench — it's a bug that surfaces on a stranger's Wi-Fi, from a
release you can't unpublish. **No tag gets pushed with anything but the
spec's example address compiled in.**

Verified by artifact inspection, not just a clean build (`strings` over
`build/somnus-dial.bin`, same check as `9ff178e`'s original
neutralization):

```
192.168.1.100   (1 occurrence — the restored default, both macros
                 collapse to one string literal since they're identical)
192.168.4.1     (1 occurrence — dial_wifi.c's SoftAP/captive-portal
                 address, unrelated and correctly still present)
```

Zero occurrences of `192.168.1.169` anywhere in the binary. `0.1.0` (§7)
is still present and unaffected by this change. Built, not flashed, per
this session's instructions — NVS already holds the real discovered
address on the one physical board, so the compiled default doesn't affect
it either way until a fresh flash or an NVS erase, and the user is
flashing on their own schedule.

**Still owed, not done in this pass:** two doc updates were requested
alongside this revert — the v1-scope planning doc's note (item 3 there)
about the temporary pad-address default, and the uncommitted-state warning
in `somnus-dial-project-summary.md`. Neither of those two documents exists
anywhere this session could find: not in this checkout, not untracked, and
not on any ref (`origin/main`, `origin/firmware/dial-product`, `somnus/
main`, `somnus/firmware/somnus-port` all checked via `git
--no-optional-locks ls-tree -r`, no match) — confirmed 2026-09-02 to be by
design: the v1-scope doc lives in the user's Claude Project deliberately,
not on disk, precisely so a planning document that changes often doesn't
drift out of sync with a second on-disk copy. Rather than fabricate either
file's content from scratch, this was surfaced back to the user mid-task;
the user chose to skip both for now rather than have them created blind.
**A future session: do not assume this note-worthy warning has been
removed from either document** — check whether the files exist yet, and
if so whether they still describe `192.168.1.169` as a live,
must-not-be-committed concern, since as of this pass that concern is
resolved in code (§8 above) but may still read as open in those two docs
once they're found or written.
