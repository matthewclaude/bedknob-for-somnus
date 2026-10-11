# REPORT: docs touch-up after somnus-v1.0.4

**Verdict: DONE** — gate passed; dated owner-confirmed charger note added to `docs/SPEC-power-sensing.md` §9.3; the `REPORT-docs-sync-1.0.4.md` line in `docs/REPORTS.md` reworded; nothing staged, committed, tagged or pushed.

## Gate

```
$ sed -n 21p firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.4")

$ git tag --list somnus-v1.0.4
somnus-v1.0.4

$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

All three checks pass.

## Edit A — docs/SPEC-power-sensing.md, §9.3 "The hazard: a power-bank-class IC"

§9.3 is the section that treats the charger as an unidentified part (§9.5 later repeats "Still unknown: the charger IC … (§9.3)"). The note is appended as a new paragraph after §9.3's existing text, which is unchanged. It adds one framing sentence so the reader does not take the note as resolving §9.3's hazard.

Before (end of §9.3, unchanged):

> … **Any future "screen off on battery" or deep-sleep feature must be tested on battery for at least a minute before anyone believes it works** — the symptom would be the dial powering itself off, not sleeping. Unverified; recorded so the first person to try deep sleep does not spend an evening on it.

After (that paragraph, then the new paragraph before `### 9.4`):

> **2026-10-01, owner-confirmed:** with the side power switch off and USB plugged in, the cell charges. The charger sits ahead of the power switch; the switch only gates the battery feeding the board. That settles where the switch sits, not which part the charger is: the charger IC is still unknown, and the auto-power-off hazard above remains unverified.

## Edit B — docs/REPORTS.md, the `REPORT-docs-sync-1.0.4.md` line

Before:

> - `REPORT-docs-sync-1.0.4.md` — 2026-10-11 — REPORT: docs sync to somnus-v1.0.4 — **DONE** — README and SPEC-home-integration name `somnus-v1.0.4` as current; SPEC-local-api-hint, SPEC-sidepick-deletion, SPEC-standby-poll and SPEC-pad-discovery Status lines record where each shipped; SPEC-ota-readiness ground truth at 1.0.4; five version-bearing screens regenerated at 1.0.4; nothing committed.

After:

> - `REPORT-docs-sync-1.0.4.md` — 2026-10-11 — REPORT: docs sync to somnus-v1.0.4 — **DONE** — README and SPEC-ota-readiness name `somnus-v1.0.4` as current; SPEC-home-integration line 91 left as dated history under its 2026-09-14 heading; SPEC-local-api-hint, SPEC-sidepick-deletion, SPEC-standby-poll and SPEC-pad-discovery Status lines record where each shipped; five version-bearing screens regenerated at 1.0.4; nothing committed.

Checked against the tree: `README.md:5` and `docs/SPEC-ota-readiness.md:10` name `somnus-v1.0.4`. `docs/SPEC-home-integration.md` line 91 still reads "stable is `somnus-v1.0.1`" under "Checked on 2026-09-14 …", and commit `511066f` does not touch that file. No other REPORTS.md line changed.

## git diff --stat

```
 docs/REPORTS.md            | 2 +-
 docs/SPEC-power-sensing.md | 2 ++
 2 files changed, 3 insertions(+), 1 deletion(-)
```

(This report is untracked and not in the stat.)

## Deviations

- Edit B: the old clause "SPEC-ota-readiness ground truth at 1.0.4" was removed because the new "README and SPEC-ota-readiness name `somnus-v1.0.4` as current" says the same thing. Keeping both would repeat it. The rest of the line is unchanged.
- Edit A: one framing sentence was added around the owner's wording ("That settles where the switch sits, not which part the charger is: … and the auto-power-off hazard above remains unverified"). The owner's three statements are kept as given.

Observation, not changed: `docs/REPORT-docs-sync-1.0.4.md` §6 still describes line 91 as edited to `somnus-v1.0.4`. The committed tree shows it was left as history, which is what the reworded index line now says. That report was left as is because this task may edit only two files.

## Commit commands (for Matthew)

```
git add docs/SPEC-power-sensing.md docs/REPORTS.md
git commit -m "docs: SPEC-power-sensing §9.3 owner-confirmed charger-ahead-of-switch note; REPORTS.md docs-sync-1.0.4 line corrected"
```

Reports are not committed, so `docs/REPORT-docs-touchup-1.0.4.md` is left out of `git add`. No tag or push is needed for a docs-only change.
