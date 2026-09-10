# Handoff — Sep 9 2026, end of session

## State of the tree

Branch `main`, HEAD `11a00da` (docs: track housekeeping report), pushed;
`somnus/main` == HEAD. Working tree clean: `git --no-optional-locks status
--short` is empty, nothing untracked. Latest tag `somnus-v1.0.0` (annotated,
object `e4a35eb`) points at `7c106fb`; it exists on the `somnus` remote
and nowhere else. `origin` (upstream) was never pushed; its push URL is still
`no_push`.

This file is the only thing this session leaves uncommitted — it was rewritten
after the last commit and is left for you to commit.

## What shipped this session

**Bedknob for Somnus 1.0.0 is published.** It is the 0.1.6 build renumbered
(`PROJECT_VER` 0.1.6 → 1.0.0, no code change), stable, non-prerelease, with
both assets on `matthewclaude/somnus-dial-releases`; `/releases/latest` is
`somnus-v1.0.0` and Pages `firmware/latest/` serves the 1.0.0 merged image
(1,743,152 bytes, HTTP 200). Release run 34412911352 succeeded
(build-and-release 5m45s, deploy-pages 11s). The published body is the
CHANGELOG 1.0.0 section byte-for-byte plus release.yml's standard footer.

Commits, oldest first, all on `main` and pushed to `somnus`:

| SHA | What |
|---|---|
| `c56cc4d` | release: somnus-v1.0.0 — PROJECT_VER, CHANGELOG 1.0.0 section + header rewrite, README release line + 1.0.0 paragraph, ARCHITECTURE beta-channel endpoint corrected to `/tags?per_page=50` + `/releases/tags/<tag>`, 0.1.6 graduation report tracked |
| `7c106fb` | chore: stale "1.0.0 is reserved" CMake comment replaced; 1.0.0 commit report tracked — **tag somnus-v1.0.0 is here** |
| `effb9c6` | sim: `simulator/CMakeLists.txt` reads PROJECT_VER at configure time (FATAL_ERROR if absent) and passes `SIM_APP_VERSION`=1.0.0 / `SIM_OTA_LATEST`=1.0.1 (patch+1); the hardcoded "0.1.4"/"1.4.3" literals in stubs.c/main.c replaced by `#error` guards; 6 of 49 screens regenerated (about, about-wifi-real/worst, update, update-prompt, update-failed), 43 byte-identical |
| `195e77c` | docs: sim version/screens report |
| `3fd1cc1` | docs housekeeping: five 0.1.5-era reports renamed to `REPORT-<version>-<step>.md` via git mv (prose refs updated, quoted git output left), `docs/TEST-F3-stuck-loop.md` now on disk (was Claude-Project-only), new `docs/REPORTS.md` index (40 entries) |
| `11a00da` | docs: track housekeeping report |

Reports written this session, all tracked and listed in `docs/REPORTS.md`:
`REPORT-1.0.0-commit.md`, `REPORT-1.0.0-tag-push.md`,
`REPORT-sim-version-screens.md`, `REPORT-sim-screens-push.md`,
`REPORT-docs-housekeeping.md`. No spec changed this session, so no spec
addendum.

CI: every ci.yml run on today's pushes is green (34412910378, 34414740070,
34416728040); the last one, 34418027434 for `11a00da` (a docs-only commit),
read `in_progress ` when this handoff was written.

## Conventions that changed today (also in memory)

- **Reports are committed now**, by the *next* docs commit, never by the
  commit they describe, and each gets a line in `docs/REPORTS.md`
  (hand-maintained: filename, first-commit date, H1, one-sentence verdict).
- **`PROJECT_VER` is on line 21** of `firmware/dial-idf/CMakeLists.txt`
  (was 22 until `7c106fb`). Gate checks that say "line 22" need updating.
- **The simulator can no longer show a stale version**: it fails to configure
  if it cannot read PROJECT_VER, and `CMAKE_CONFIGURE_DEPENDS` re-runs
  configure when the firmware CMakeLists changes. A version bump changes
  exactly six PNGs; if any other PNG changes on a regen, the build
  environment drifted.
- `per_page=5` still appears in historical docs (REPORT-0.1.5-ci-check,
  REPORT-ota-beta-not-found, SPEC-ota-readiness §9.3 (superseded by §9.7),
  SPEC-night-window). Intentional record of the bug, not a doc error.

## Not verified without hardware

- **No dial has been seen installing 1.0.0 over the air** (0.1.6 → 1.0.0, or
  0.1.5 → 1.0.0), and none was seen installing 0.1.6 either. The release is
  in place for it; the next time the bench dial (on 0.1.6) is powered,
  Menu → Update should read "v1.0.0 - tap to install" — worth one eyes-on
  check and a line in HARDWARE-bringup-log.md.
- Nothing was flashed this session. CI's build of `7c106fb` is the only
  build of 1.0.0; the host simulator build of `effb9c6` compiled the same
  UI sources with zero warnings.

## Still open (carried forward)

- **Pad restore** from the Sep 7 audit session: the pad was left `on @
  24 °C` when it had been `off @ 18 °C`; whether that was restored by hand
  is not recorded anywhere. Check the bed state.
- Overnight soak verdict on 0.1.6-beta.3 was never reported; 0.1.6 and then
  1.0.0 were graduated anyway.
- Layout audit Tier D (brightness "%" placement, pad-discovery relayout) and
  `SCR_SIDEPICK`/`side_picked` deletion — owner decisions, untouched.
- TEST-F3 (escape hatches while stuck in the connect loop) is now on disk at
  `docs/TEST-F3-stuck-loop.md`; whether it was ever run at the board is
  still only in HARDWARE-bringup-log.md if anywhere.
- Encoder detent-coalescing question from the Sep 7 audit: still untested,
  still no symptom.
- Next version line after 1.0.0 is your call (1.0.1 for fixes, 1.1.0 for a
  feature); first beta would be `X.Y.Z-beta.1`, and the simulator will
  advertise patch+1 automatically.

## Tooling / machine

- `tools/dial_display_audit.py` and `docs/dial-audit-run.log` are tracked
  (since the Sep 7 docs commit). No new tools this session.
- Repo-root `build/` holds a configured dial_sim at 1.0.0 (gitignored).
- Temp files removed: `/tmp/screens-before` (the pre-regen PNG backup).
  Nothing in `/tmp` belongs to this session; the session scratchpad is
  session-scoped and disposable.

## Nothing else in flight

No background jobs, no worktrees besides the main checkout, no open artifact
watches. gh is authed as matthewclaude.
