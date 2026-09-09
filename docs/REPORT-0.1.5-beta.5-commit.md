# REPORT: 0.1.5-beta.5 commit (step 1 of 2 — commit only)

**Verdict: COMMITTED. Exactly the 22 intended files, nothing else swept in. Not tagged, not pushed.**

Date: 2026-09-04
Branch: `firmware/somnus-port`
Commit: `afd23fffbee543d2de4ae9eb6becfda6ed55ad4c`
Parent: `490a3ec` (docs: power sensing section 10 built, verified, shipped as beta.4)

## Pre-commit check

`git --no-optional-locks status --short` before staging showed 18 modified + 13 untracked
paths. The list matched the expected set exactly: no unexpected modified or untracked
files appeared, so the commit proceeded.

## Staging

Each of the 22 paths was staged with an individual `git add <path>` (no `-A`, no wildcard).
`git --no-optional-locks diff --cached --stat` before committing:

```
 .gitignore                                         |   4 +
 CHANGELOG.md                                       |  22 ++
 docs/SPEC-power-sensing.md                         |  95 +++++++
 docs/screens/about-battery-pct.png                 | Bin 0 -> 21673 bytes
 docs/screens/about-battery-usb.png                 | Bin 0 -> 21782 bytes
 docs/screens/about-wifi-real.png                   | Bin 0 -> 24711 bytes
 docs/screens/about-wifi-worst.png                  | Bin 0 -> 25677 bytes
 docs/screens/about.png                             | Bin 17851 -> 19116 bytes
 firmware/dial-idf/CMakeLists.txt                   |   2 +-
 .../dial-idf/components/dial_power/dial_power.c    |  85 ++++++
 .../dial-idf/components/dial_power/dial_power.h    |  11 +
 .../dial-idf/components/dial_state/dial_state.c    |   8 +
 .../dial-idf/components/dial_state/dial_state.h    |  22 ++
 firmware/dial-idf/components/dial_ui/scr_about.c   | 285 +++++++++++++++++++--
 firmware/dial-idf/components/dial_ui/scr_dial.c    |  15 +-
 firmware/dial-idf/components/dial_ui/scr_standby.c |  14 +-
 .../components/dial_ui/ui_screens_internal.h       | 234 ++++++++++++++---
 simulator/CMakeLists.txt                           |   4 +-
 simulator/main.c                                   | 102 ++++++++
 simulator/sim_state.c                              |  34 +++
 simulator/sim_state.h                              |   6 +
 simulator/stubs.c                                  |  30 ++-
 22 files changed, 901 insertions(+), 72 deletions(-)
```

## Commit

SHA: `afd23fffbee543d2de4ae9eb6becfda6ed55ad4c`

Message used verbatim as supplied (subject `feat(power): battery percentage + About screen
redesign (0.1.5-beta.5)`, trailer `Co-Authored-By: Claude Sonnet 5 <noreply@anthropic.com>`
as given in the instruction).

`git --no-optional-locks show --stat HEAD` reports the same 22-file stat block as above:
`22 files changed, 901 insertions(+), 72 deletions(-)`.

## Explicit exclusion confirmation

None of the following were staged or committed (verified: absent from `diff --cached --stat`
and from `show --stat HEAD`, and still present as untracked after the commit):

- `docs/PLAN-screen-layout-fixes.md`
- `docs/REPORT-about-layout-battery-glyph.md`
- `docs/REPORT-battery-pct-about-redesign.md`
- `docs/REPORT-handoff.md`
- `docs/REPORT-night-face.md`
- `docs/REPORT-power-indicator.md`
- `docs/REPORT-0.1.5-beta.3-release.md` (then named `REPORT-release-beta3.md`)
- `docs/REPORT-0.1.5-beta.4-release.md` (then named `REPORT-release-beta4.md`)
- `docs/REPORT-screen-layout-audit.md`
- `Claude outputs/`

## Working tree after commit

`git --no-optional-locks status --short` shows no modified or staged files. Only the
deliberately excluded untracked paths remain:

```
?? "Claude outputs/"
?? docs/PLAN-screen-layout-fixes.md
?? docs/REPORT-about-layout-battery-glyph.md
?? docs/REPORT-battery-pct-about-redesign.md
?? docs/REPORT-handoff.md
?? docs/REPORT-night-face.md
?? docs/REPORT-power-indicator.md
?? docs/REPORT-release-beta3.md
?? docs/REPORT-release-beta4.md
?? docs/REPORT-screen-layout-audit.md
```

(This file, `docs/REPORT-0.1.5-beta.5-commit.md` — then named `REPORT-beta5-commit.md` — is now also untracked, per report convention.)

## Not done (by instruction)

- No `git tag`.
- No `git push`.

Step 2 (tag + push) awaits a separate explicit instruction.
