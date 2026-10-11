# REPORT — CHANGELOG 1.0.4 section rewrite

**Verdict: DONE**

The `## 1.0.4 — 2026-10-10` section of `CHANGELOG.md` was rewritten to
cover everything a stable user gets going from 1.0.2 to 1.0.4 (combining
1.0.3-beta.1, 1.0.4-beta.1 and 1.0.4-beta.2). The sentence beginning
"Discovery on non-192.168.1.x subnets" is gone (`grep -c` returns 0).
All lines in the section are 72 columns or fewer. The beta sections below
it are unchanged: `git diff CHANGELOG.md` against HEAD contains no
removed lines, only the added 1.0.4 section.

## New 1.0.4 section, as it now reads

```markdown
## 1.0.4 — 2026-10-10


This release brings everything from the 1.0.3 and 1.0.4 betas to dials
on stable updates.

The dial can now find your pad by itself on routers that use larger
networks, such as eero mesh systems. Before, on those networks the dial
gave up looking and you had to type the pad's address in by hand.

When the dial cannot reach your pad, the "Pad unreachable" screen now
asks "Is the pad's Local API enabled?" and says Somnus support can turn
it on, instead of showing a technical error code. The Local API has to
be on for the dial to work, and today it is turned on by asking Somnus
support.

A new dial in Dual Sides mode no longer stops to ask "Which side of the
bed?" after it first reaches the pad. It opens on the right side, and
one swipe shows the left. The dial still remembers the last side you
looked at.

Also fixed: switching an existing dial from One Bed to Dual Sides and
then swiping to the other side could change Scale from Relative to
Absolute on the next restart, even though you never changed it. Scale
now stays where you left it.

```

## awk extraction check (release.yml awk, VER=1.0.4), raw output

```

This release brings everything from the 1.0.3 and 1.0.4 betas to dials
on stable updates.

The dial can now find your pad by itself on routers that use larger
networks, such as eero mesh systems. Before, on those networks the dial
gave up looking and you had to type the pad's address in by hand.

When the dial cannot reach your pad, the "Pad unreachable" screen now
asks "Is the pad's Local API enabled?" and says Somnus support can turn
it on, instead of showing a technical error code. The Local API has to
be on for the dial to work, and today it is turned on by asking Somnus
support.

A new dial in Dual Sides mode no longer stops to ask "Which side of the
bed?" after it first reaches the pad. It opens on the right side, and
one swipe shows the left. The dial still remembers the last side you
looked at.

Also fixed: switching an existing dial from One Bed to Dual Sides and
then swiping to the other side could change Scale from Relative to
Absolute on the next restart, even though you never changed it. Scale
now stays where you left it.

```

The output stops before `## 1.0.4-beta.2`; the exact-match rule keeps
it from matching the beta headings.

## git diff --stat

```
 CHANGELOG.md                     | 25 +++++++++++++++++++++++++
 docs/REPORTS.md                  |  2 ++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 3 files changed, 28 insertions(+), 1 deletion(-)
```

`docs/REPORTS.md` and `firmware/dial-idf/CMakeLists.txt` were already
modified before this task and were not touched. Untracked report files
(including this one) don't appear in `diff --stat`.

No commit SHA: per the task, nothing was committed, staged or tagged.

## Deviations

- Added a one-sentence opening line ("This release brings everything
  from the 1.0.3 and 1.0.4 betas to dials on stable updates.") so
  stable readers know where the changes came from.
- Left out the beta sections' "Internal:" lines (serial-log note,
  SCR_SIDEPICK / relmode seeding) because the section is meant to be
  user-facing.
- This report is not added to `docs/REPORTS.md`, because the task said
  not to touch that file.
