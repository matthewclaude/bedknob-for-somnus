# Handoff — Sep 5 2026 (UTC), end of session

## State of the tree

Branch `firmware/somnus-port`, HEAD `1f527bb` (release: 0.1.5). Tags
`somnus-v0.1.5-beta.5` (`afd23ff`) and `somnus-v0.1.5` (`1f527bb`) both
exist locally and on the `somnus` remote. `PROJECT_VER` = `0.1.5`.

Tracked modifications left uncommitted, for the owner to commit:

```
 M docs/SPEC-ota-readiness.md   (§9.7 update paragraph + §9.9 "Resolved" paragraph, session close)
```

Untracked, by convention never committed: `docs/REPORT-*.md`,
`docs/PLAN-screen-layout-fixes.md`, `"Claude outputs/"`. `bench-logs/` is
gitignored and now holds the two serial captures from this session.

## What shipped this session

1. **0.1.5-beta.5** — commit `afd23ff`, tag pushed, CI run 33932029987
   success, prerelease published (`REPORT-beta5-commit.md`,
   `REPORT-beta5-tag-push.md`, `REPORT-beta5-ci-check.md`).
2. **0.1.5 stable** — commit `1f527bb` (PROJECT_VER, CHANGELOG rollup of
   beta.1–5, SPEC §9.9), tag pushed, CI run 33935053719 success in 5m20s,
   published non-prerelease; `/releases/latest` → `somnus-v0.1.5`; Pages
   `firmware/latest/` serves the 0.1.5 merged image
   (`REPORT-0.1.5-graduation-commit.md`, `REPORT-0.1.5-tag-push.md`,
   `REPORT-0.1.5-ci-check.md`).

Why stable now: a serial capture showed the bench dial on 0.1.4 could not
see beta.5 (`REPORT-ota-beta-not-found.md`); root cause is that the §9.7
beta-discovery fix (`819f102`) post-dates the 0.1.4 tag and had only ever
shipped in prereleases (SPEC §9.9). Shipping 0.1.5 stable is what closes
that for every stable unit.

## What is on the dial right now

**0.1.5, installed over the air.** The bench dial was wire-restored to
0.1.4 (built in a temporary worktree at the tag, NVS untouched), then
found and installed 0.1.5 on its first manual check: offer at 57 s,
image verified at 88 s, rebooted, `App version: 0.1.5` (compile time
matches the CI build), pad reconnected, rollback cancelled at 3.9 s
(`REPORT-0.1.5-upgrade-path-verify.md`,
`bench-logs/2026-09-05-upgrade-0.1.4-to-0.1.5.log`).

## Tooling changes on this machine

- `gh` 2.100.0 installed via Homebrew, authenticated as `matthewclaude`
  (keyring). `gh run list/view`, `gh release view`, `gh api` all work
  against both repos.

## Still open (carried forward)

- **Layout fixes** — `docs/PLAN-screen-layout-fixes.md` still awaiting the
  owner's review; nothing implemented. Four open questions at its end.
- **OTA paths not yet observed on hardware:** beta.N → stable graduation;
  0.1.4 with Beta builds **on** → 0.1.5 (API shows 0.1.5 first in the
  five-entry list so it should work); the 404/draft fallback.
- **BATT_CURVE 100 % anchor** (4200 mV) may be unreachable on some
  cell/charger combos — monitoring, not recalibrating yet.
- Time from plug-in to 4280 mV on a depleted cell (SPEC-power-sensing §10.2).
- CI annotation noise: actions pinned to Node 20 (checkout@v4,
  upload/download-artifact@v4, action-gh-release@v2) are being forced onto
  Node 24. Harmless today; bump when convenient.

## Nothing in flight

No background jobs, no serial capture running (both pid files removed),
no worktrees besides the main checkout (`git worktree list` verified),
`/tmp/somnus-0.1.4-wt` gone. Main checkout HEAD and status unchanged by
the hardware test.
