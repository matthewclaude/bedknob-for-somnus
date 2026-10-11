# REPORT: docs sync to somnus-v1.0.4

**Verdict: DONE.** Every current-version statement in `README.md` and `docs/*.md` (outside `REPORT-*.md`) now names `somnus-v1.0.4`. Four spec Status lines now say where each change shipped, citing commits and reports. Four index lines were added to `docs/REPORTS.md`. The five version-bearing screens were regenerated at 1.0.4. Nothing under `firmware/` or `.github/` changed. No git write commands were run.

Date: 2026-10-11. Repo `~/Projects/somnus-waveshare-rotary-dial`, branch `main`, HEAD `6f25928` (= `somnus/main`, per `git ls-remote --heads somnus main`).

## Ground-truth check

**Latest stable tag is `somnus-v1.0.4`: confirmed.**

```
$ git tag --list 'somnus-v*' --sort=-creatordate | head -3
somnus-v1.0.4
somnus-v1.0.4-beta.2
somnus-v1.0.4-beta.1
$ git ls-remote --tags somnus 'somnus-v1.0.4*'
e531e0575c03f82337d59ca024f8bede5bfb1839	refs/tags/somnus-v1.0.4
14e99004d269676f0e6c41b0598c3d077cab1eaf	refs/tags/somnus-v1.0.4^{}
ac499d80091e055a56bcf5c555670517fec5a720	refs/tags/somnus-v1.0.4-beta.1
1d01f3b664e6a217bbf10bb34152f683d1a4d91e	refs/tags/somnus-v1.0.4-beta.1^{}
1598b27f4a5ffac8afd4172e4ebf60d3b0fef1bb	refs/tags/somnus-v1.0.4-beta.2
$ gh release list --repo matthewclaude/bedknob-for-somnus --limit 4
somnus-v1.0.4	Latest	somnus-v1.0.4	2026-10-10T18:57:03Z
somnus-v1.0.4-beta.2	Pre-release	somnus-v1.0.4-beta.2	2026-10-04T13:01:44Z
somnus-v1.0.4-beta.1	Pre-release	somnus-v1.0.4-beta.1	2026-09-30T20:19:46Z
somnus-v1.0.3-beta.1	Pre-release	somnus-v1.0.3-beta.1	2026-09-30T16:51:51Z
$ gh api repos/matthewclaude/bedknob-for-somnus/releases/latest --jq '.tag_name,.prerelease,.published_at'
somnus-v1.0.4
false
2026-10-10T18:57:03Z
```

Tag → commit, from `git rev-parse <tag>^{commit}` and `git log -1 --format=%cs`:

| Tag | Commit | Date |
|---|---|---|
| `somnus-v1.0.2-beta.1` | `00bd94d` | 2026-09-12 |
| `somnus-v1.0.2` | `5512537` | 2026-09-17 |
| `somnus-v1.0.3-beta.1` | `5a6e692` | 2026-09-30 |
| `somnus-v1.0.4-beta.1` | `1d01f3b` | 2026-09-30 |
| `somnus-v1.0.4-beta.2` | `1598b27` | 2026-10-04 |
| `somnus-v1.0.4` | `14e9900` | 2026-10-10 |

**`release.yml` publishes from this repo, and `somnus-dial-releases` has been archived since 2026-09-13: confirmed.**

```
$ sed -n 10,17p .github/workflows/release.yml
# The tag is pushed to (and this workflow runs in) THIS repo, which is
# public. The Release object and the gh-pages site are published here and
# nowhere else (docs/SPEC-repo-consolidation.md, Phase 6a).
# matthewclaude/somnus-dial-releases (archived 2026-09-13) is the frozen pre-1.0.1 location that
# dials at or below 1.0.0 polled (docs/SPEC-ota-readiness.md §5 for why it
# existed). It is kept read-only and is never deleted or renamed, because
# its 1.0.1 Release is the permanent over-the-air migration path for any
# such dial. Nothing in this workflow writes to it.
$ git remote get-url somnus
git@github.com:matthewclaude/bedknob-for-somnus.git
$ gh api repos/matthewclaude/somnus-dial-releases --jq '.archived,.updated_at'
true
2026-09-13T19:40:48Z
```

Ancestry check for the code commits cited below (`git merge-base --is-ancestor`):

```
39af8f5 in somnus-v1.0.2-beta.1
39af8f5 in somnus-v1.0.2
39af8f5 NOT in somnus-v1.0.1
fe5b139 in somnus-v1.0.3-beta.1
fe5b139 NOT in somnus-v1.0.2
fe5b139 in somnus-v1.0.4
5e9bc69 in somnus-v1.0.4-beta.1
5e9bc69 NOT in somnus-v1.0.3-beta.1
5e9bc69 in somnus-v1.0.4
4e46454 in somnus-v1.0.4-beta.2
4e46454 NOT in somnus-v1.0.4-beta.1
4e46454 in somnus-v1.0.4
```

## Edits

### 1. README.md (line 5)

- Old: `` > `somnus-v1.0.2`. `` (continuing "Current release" from line 4)
- New: `` > `somnus-v1.0.4`. ``
- Evidence: ground-truth check above.
- Other version statements in README.md: `grep -n -i '1\.0\.[0-9]\|current release\|latest'` finds only line 225, "**1.0.0 shipped 2026-09-09**". That line is historical, so it is unchanged. Lines 269 and 289 are the licence version (PolyForm 1.0.0).

### 2. docs/SPEC-local-api-hint.md (Status paragraph, lines 3–8)

- Old (end of the paragraph): "…the dial face with its staleness dot and never shows the hint; §4 and §10 step 3 are corrected below."
- New: the same text, followed by: "Shipped in `somnus-v1.0.4-beta.1` (release commit `1d01f3b`, 2026-09-30; `docs/REPORT-1.0.4-beta.1-commit.md`, `docs/REPORT-1.0.4-beta.1-tag-push.md`) and on stable in `somnus-v1.0.4` (release commit `14e9900`, 2026-10-10; `docs/REPORT-stable-discovery.md`, `docs/REPORT-changelog-1.0.4.md`)."
- Evidence: code commit `5e9bc69` is in `somnus-v1.0.4-beta.1` and `somnus-v1.0.4` and is not in `somnus-v1.0.3-beta.1`. CHANGELOG `## 1.0.4-beta.1` describes the hint, and `## 1.0.4` repeats it ("brings everything from the 1.0.3 and 1.0.4 betas"). The REPORTS.md line for the tag-push report says it is published as a prerelease.

### 3. docs/SPEC-sidepick-deletion.md (Status paragraph, lines 3–7)

- Old (end of the paragraph): "Owner decisions recorded 2026-09-23 (section 9); ready for the code change."
- New: the same text, followed by: "Code committed as `fe5b139` (`docs/REPORT-sidepick-code.md`), §7 bench PASS (`docs/REPORT-sidepick-bench.md`). Shipped in `somnus-v1.0.3-beta.1` (release commit `5a6e692`, 2026-09-30; `docs/REPORT-1.0.3-beta.1-commit.md`, `docs/REPORT-1.0.3-beta.1-tag-push.md`) and on stable in `somnus-v1.0.4` (release commit `14e9900`, 2026-10-10; `docs/REPORT-changelog-1.0.4.md`)."
- Evidence: `fe5b139` is in `somnus-v1.0.3-beta.1` and `somnus-v1.0.4` and is not in `somnus-v1.0.2`. The verdict of REPORT-sidepick-code is "CODE COMMITTED — … commit B `fe5b139` (the change)". The verdict of REPORT-sidepick-bench is "PASS". REPORT-1.0.3-beta.1-tag-push says the Release is a published prerelease. CHANGELOG `## 1.0.4` includes the Dual Sides / Scale text from `## 1.0.3-beta.1`.

### 4. docs/SPEC-standby-poll.md (Status line 3, and line 113): shipped, so updated

- Old (Status line 3, bold part): "… amended 2026-09-11. Not built. Scheduled as `somnus-v1.0.2-beta.1`, queued behind the repo consolidation (`docs/SPEC-repo-consolidation.md`).**"
- New: "… amended 2026-09-11. Built and released: code `39af8f5` (`docs/REPORT-standby-poll-impl.md`), bench PASS (`docs/REPORT-1.0.2-bench-cadence.md`, `docs/REPORT-1.0.2-bench-items-2-3.md`, `docs/REPORT-1.0.2-bench-items-4-5.md`, `docs/REPORT-1.0.2-overnight-analysis.md`, `docs/REPORT-1.0.2-beta.1-ota-bench.md`). Shipped in `somnus-v1.0.2-beta.1` (release commit `c6261fa`, tag on `00bd94d`; `docs/REPORT-1.0.2-beta.1-commit.md`, `docs/REPORT-1.0.2-beta.1-tag-push.md`) and on stable in `somnus-v1.0.2` (release commit `5512537`, 2026-09-17; `docs/REPORT-1.0.2-commit.md`, `docs/REPORT-1.0.2-tag-push.md`).**"
- Old (line 3 later, and line 113, item 7): "and current stable is `1.0.1`." (two occurrences)
- New: "and current stable is `1.0.4`."
- Evidence: REPORT-standby-poll-impl verdict "DONE … commit 2 (firmware, `main/main.c` only) `39af8f5`". `39af8f5` is in both 1.0.2 tags and not in `somnus-v1.0.1`. Bench verdicts: cadence (§6 item 1) PASS; items 2–3 PASS; items 4–5 PASS; overnight analysis (item 6) PASS; 1.0.2-beta.1 OTA bench (item 7, install half) PASS. Commit subjects: `c6261fa` "release: 1.0.2-beta.1 — STANDBY poll cadence 300 s"; `5512537` "release: somnus-v1.0.2 — … STANDBY poll cadence 300 s goes stable". REPORT-1.0.2-tag-push says the Release was published as stable.
- Note: `somnus-v1.0.2-beta.1` points at `00bd94d` (the release-commit report commit, child of `c6261fa`), not at the release commit itself. The REPORTS.md line for REPORT-1.0.2-beta.1-tag-push records this too. The Status line states both.

### 5. docs/SPEC-pad-discovery.md (new paragraph after the Status paragraph, before the "As built vs. as specified" block)

- Old: (none; the Status paragraph ended "…Record in `docs/HARDWARE-bringup-log.md` and `V1-scope.md` item 1.")
- New: "**Amended 2026-10-11 — wide-subnet change.** On a subnet wider than /24, discovery now scans the subnet's first /24 plus the dial's own /24 instead of skipping (commit `4e46454`, `docs/REPORT-pad-discovery-wide-subnet.md`). Released in `somnus-v1.0.4-beta.2` (release commit `1598b27`, 2026-10-04; `docs/REPORT-1.0.4-beta.2-release-prep.md`) and on stable in `somnus-v1.0.4` (release commit `14e9900`, 2026-10-10; `docs/REPORT-stable-discovery.md`). Field note — 2026-10-11: a fresh setup after a dial factory reset found the pad on an eero 192.168.4.x network (owner report)."
- Evidence: commit subject `4e46454` "dial: pad discovery scans two /24 blocks on subnets wider than /24". The REPORT-pad-discovery-wide-subnet verdict wording supplies the "first /24 plus the dial's own /24" description. `4e46454` is in `somnus-v1.0.4-beta.2` and `somnus-v1.0.4` and is not in `somnus-v1.0.4-beta.1`. The beta.2 Release is a published prerelease (gh release list). The field note is the owner's report as given in the task. The repo has nothing that confirms or contradicts it, and the line is labelled as an owner report.

### 6. docs/SPEC-home-integration.md (line 91)

- Old: "… `somnus-v1.0.0` shipped on 2026-09-09 and stable is `somnus-v1.0.1`; this spec adds no feature …"
- New: "… `somnus-v1.0.0` shipped on 2026-09-09 and stable is `somnus-v1.0.4`; this spec adds no feature …"
- Evidence: ground-truth check. Context: this bullet sits under "Checked on 2026-09-14 …". The new text is true today, but the stable release on that check date was 1.0.1. The edit was made as instructed. If you want the dated check kept intact, revert this one line.

### 7. Other docs/*.md (not REPORT-*.md) stating the current or latest stable version

Searched with `grep -n -i -E '(current|latest|stable|now|today)[^.]{0,60}(v?1\.0\.[0-3]|0\.1\.[0-9])…'` and `grep -n -i -E 'current (stable|release|version)|latest (stable|release)|stable is|is the current|currently (on|at|ships)'` over `docs/*.md` minus `REPORT-*.md`.

Fixed (plain current-version statements):

| File:line | Old | New |
|---|---|---|
| `docs/SPEC-ota-readiness.md:10` (reader's guide, "Ground truth today") | `` current is stable `somnus-v1.0.2` (2026-09-17), with no current beta (`1.0.2-beta.1` is superseded) `` | `` current is stable `somnus-v1.0.4` (2026-10-10), with no current beta (`1.0.4-beta.2` is superseded) `` |
| `docs/SPEC-standby-poll.md:3` and `:113` | "current stable is `1.0.1`" | "current stable is `1.0.4`" (see edit 4) |
| `docs/SPEC-home-integration.md:91` | see edit 6 | see edit 6 |

The SPEC-ota-readiness "no current beta" claim holds: `somnus-v1.0.4` is newer than `somnus-v1.0.4-beta.2`, and there is no later `somnus-v*` tag.

Left unchanged (historical or dated in context):

- `docs/SPEC-ota-readiness.md:1342`: "`somnus-v0.1.4` — the current stable release" is inside §9.9, "Added 2026-09-05".
- `docs/SPEC-sidepick-deletion.md:101`: "Fresh single-zone device, today (1.0.2)" is the spec's analysis of the tree it was written against (`547231f`).
- `docs/SPEC-night-window.md:10, 394`: pre-1.0 plan text about stable `0.1.5`.
- `docs/SPEC-voice.md:15`: the §0.2 scope-rule history (`0.1.4`).
- `docs/SPEC-repo-consolidation.md`: every `1.0.1` mention is the dual-publish plan and gate, not a current-version statement.
- `docs/SPEC-update-prompt.md:166`: "v1.0.10" is upstream Orion's version, not Somnus's.
- `docs/REPORTS.md`: index lines quote each report's own verdict as of its date.

### 8. docs/REPORTS.md

The two existing uncommitted 2026-10-04 lines (`REPORT-pad-discovery-wide-subnet.md`, `REPORT-1.0.4-beta.2-release-prep.md`) are kept unchanged. Four lines were added after them, in the existing format:

```
- `REPORT-stable-discovery.md` — 2026-10-10 — REPORT: somnus-v1.0.4 stable release prep (wide-subnet pad discovery) — **READY — 1.0.4** — gate passed (`somnus-v1.0.4-beta.2` at `1598b27` on `somnus`, only the screens commit `94241fc` after it); `PROJECT_VER` set to 1.0.4 and a `## 1.0.4 — 2026-10-10` CHANGELOG section added, `idf.py build` and the host test pass; not committed, tagged or pushed (later committed as `14e9900`).
- `REPORT-changelog-1.0.4.md` — 2026-10-10 — REPORT — CHANGELOG 1.0.4 section rewrite — **DONE** — the `## 1.0.4` section now covers everything a stable user gets from 1.0.2 to 1.0.4 (1.0.3-beta.1, 1.0.4-beta.1 and 1.0.4-beta.2), the "Discovery on non-192.168.1.x subnets" sentence is gone, and the beta sections below it are unchanged.
- `REPORT-support-notice.md` — 2026-10-11 — Support notice in the README and the web flasher page — **DONE** — hobby-project / no-setup-support notice added to `README.md` and `web-flasher/index.html` (committed as `6f25928`); one deliberate deviation: "open an issue" is plain text, not a link.
- `REPORT-docs-sync-1.0.4.md` — 2026-10-11 — REPORT: docs sync to somnus-v1.0.4 — **DONE** — README and SPEC-home-integration name `somnus-v1.0.4` as current; SPEC-local-api-hint, SPEC-sidepick-deletion, SPEC-standby-poll and SPEC-pad-discovery Status lines record where each shipped; SPEC-ota-readiness ground truth at 1.0.4; five version-bearing screens regenerated at 1.0.4; nothing committed.
```

Missing-from-index check: `ls docs | grep '^REPORT-'` compared with the backticked `REPORT-*.md` names in REPORTS.md (`comm -23`). Before the edit it found exactly `REPORT-changelog-1.0.4.md`, `REPORT-stable-discovery.md` and `REPORT-support-notice.md`, so no other report was missing. Dates: REPORT-support-notice has its own date line (2026-10-11). REPORT-stable-discovery and REPORT-changelog-1.0.4 have no date line, so their date (2026-10-10) comes from the `## 1.0.4 — 2026-10-10` section they write about and from the file mtimes (2026-10-10 13:49 and 13:52). The "later committed as `14e9900`" and "committed as `6f25928`" notes come from `git show --stat` of those commits (CHANGELOG + CMakeLists; README + web-flasher/index.html).

### 9. Version-bearing screenshots

Method (from REPORT-sim-version-screens, REPORT-1.0.3-beta.1-tag-push and REPORT-1.0.4-beta.2-release-prep): back up `docs/screens/*.png` to the scratchpad (48 files), run `cmake -B build -S simulator && cmake --build build`, run `./build/dial_sim`, then `cmp` every PNG against the backup. The simulator reads `PROJECT_VER` (`set(PROJECT_VER "1.0.4")`, `firmware/dial-idf/CMakeLists.txt:21`) at configure time. `simulator/main.c` was not touched.

**Changed (5):** `about.png`, `about-wifi-real.png`, `about-wifi-worst.png`, `update.png`, `update-failed.png`.

**Unchanged (43),** including `update-prompt.png`. That screen shows only the offered version, patch+1 with the suffix stripped. It was 1.0.5 at 1.0.4-beta.2 and is still 1.0.5, the same case as beta.1→beta.2. The screens with no version text (`standby-update`, `updating`, `about-battery-*`) are byte-identical, so the environment has not drifted. No non-version screen changed.

**"v1.0.4" fits on each.** I looked at all five side by side. `about.png`: the Firmware row reads "v1.0.4" on one line, centred. `about-wifi-real.png` and `about-wifi-worst.png`: "v1.0.4" under the faded Firmware label at the top. `update.png`: Installed "v1.0.4" right-aligned, with "v1.0.5 - tap to install" and "Skip 1.0.5". `update-failed.png`: Installed "v1.0.4" right-aligned. "v1.0.4" is six characters shorter than the "v1.0.4-beta.2" these screens showed before, so nothing clips or overlaps.

Simulator output, raw. Configure lines from the build log (the build had no compiler warnings; the log's only "warning" text is CMake's developer policy notice):

```
-- dial_sim: firmware version 1.0.4, OTA scenarios advertise 1.0.5
-- dial_sim: using local LVGL checkout at /Users/matthew/Projects/somnus-waveshare-rotary-dial/simulator/../firmware/dial-idf/managed_components/lvgl__lvgl
CMake Warning (policy) at /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/lvgl__lvgl/env_support/cmake/custom.cmake:66 (install):
```

`./build/dial_sim` (exit 0):

```
I (ui_router) router up, screen 0
wrote welcome          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (6 distinct colors sampled)
wrote passkey          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote connecting       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote dial-night-water /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-night-water.png (7 distinct colors sampled)
wrote rails-420-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=410
wrote rails-420-up-down /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up-down.png (10 distinct colors sampled)
wrote rails-423        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=420
wrote rails-423-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423-up.png (10 distinct colors sampled)
wrote rails-drag-337-live /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337-live.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=340
wrote rails-drag-337   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337.png (10 distinct colors sampled)
wrote menu             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (3 distinct colors sampled)
wrote update           /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (5 distinct colors sampled)
wrote update-prompt    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (4 distinct colors sampled)
wrote settings         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (4 distinct colors sampled)
wrote settings-pad     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (2 distinct colors sampled)
wrote settings-timezone-raw /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (2 distinct colors sampled)
wrote timezone         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (5 distinct colors sampled)
wrote night-mode       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (4 distinct colors sampled)
wrote night-face       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (6 distinct colors sampled)
wrote standby-face     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-face.png (5 distinct colors sampled)
wrote settings-standby-face /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-standby-face.png (9 distinct colors sampled)
wrote pad-address      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (4 distinct colors sampled)
wrote pad-discovery    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (3 distinct colors sampled)
wrote pad-degraded-real /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (3 distinct colors sampled)
wrote adjust-mode      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (9 distinct colors sampled)
wrote brightness-menu  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (5 distinct colors sampled)
wrote brightness       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (6 distinct colors sampled)
wrote wifi-confirm     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (6 distinct colors sampled)
wrote about            /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (4 distinct colors sampled)
wrote about-wifi-worst /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (5 distinct colors sampled)
wrote about-wifi-real  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (4 distinct colors sampled)
wrote standby-update   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (4 distinct colors sampled)
done: 48 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
```

`cmp` against the backup:

```
CHANGED about-wifi-real.png
CHANGED about-wifi-worst.png
CHANGED about.png
CHANGED update-failed.png
CHANGED update.png
```

## Checks

```
$ git diff --stat
 README.md                         |   2 +-
 docs/REPORTS.md                   |   6 ++++++
 docs/SPEC-home-integration.md     |   2 +-
 docs/SPEC-local-api-hint.md       |   6 +++++-
 docs/SPEC-ota-readiness.md        |   2 +-
 docs/SPEC-pad-discovery.md        |   9 +++++++++
 docs/SPEC-sidepick-deletion.md    |   5 +++++
 docs/SPEC-standby-poll.md         |   4 ++--
 docs/screens/about-wifi-real.png  | Bin 23661 -> 23241 bytes
 docs/screens/about-wifi-worst.png | Bin 24628 -> 24206 bytes
 docs/screens/about.png            | Bin 19608 -> 18499 bytes
 docs/screens/update-failed.png    | Bin 22488 -> 21774 bytes
 docs/screens/update.png           | Bin 24028 -> 23218 bytes
 13 files changed, 30 insertions(+), 6 deletions(-)
$ git diff --name-only -- firmware .github | wc -l
       0
```

Untracked files (`git ls-files --others --exclude-standard docs`, plus this report):

```
docs/REPORT-1.0.4-beta.2-release-prep.md
docs/REPORT-changelog-1.0.4.md
docs/REPORT-pad-discovery-wide-subnet.md
docs/REPORT-stable-discovery.md
docs/REPORT-support-notice.md
docs/REPORT-docs-sync-1.0.4.md
```

Every changed file appears in `git diff --stat`. Nothing under `firmware/` or `.github/` changed. `build/` (the simulator build) is git-ignored and does not appear in `git --no-optional-locks status --short`. **There is no commit SHA: this task does not commit.** No surname was added to any file.

## Deviations

None. Two notes:

- Edit 6 follows the instruction, but that line sits in a list headed "Checked on 2026-09-14" (see edit 6). Revert that one line if you would rather keep the dated check as it was.
- `somnus-v1.0.4-beta.2` is a lightweight tag (`git cat-file -t` = `commit`); the other recent tags are annotated. REPORT-stable-discovery already records this. It is out of scope here and nothing was changed.

## Commands for Matthew

```
git add README.md docs/REPORTS.md docs/SPEC-home-integration.md docs/SPEC-local-api-hint.md docs/SPEC-ota-readiness.md docs/SPEC-pad-discovery.md docs/SPEC-sidepick-deletion.md docs/SPEC-standby-poll.md docs/screens/about.png docs/screens/about-wifi-real.png docs/screens/about-wifi-worst.png docs/screens/update.png docs/screens/update-failed.png docs/REPORT-1.0.4-beta.2-release-prep.md docs/REPORT-changelog-1.0.4.md docs/REPORT-pad-discovery-wide-subnet.md docs/REPORT-stable-discovery.md docs/REPORT-support-notice.md docs/REPORT-docs-sync-1.0.4.md
git diff --cached --stat
git commit -m "docs: sync to somnus-v1.0.4 - current-version lines, spec Status lines, five version-bearing screens, REPORTS.md index"
git push somnus main
```
