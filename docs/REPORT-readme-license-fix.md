# REPORT: README license and contributing text fix

**BLOCKED — the text on disk does not match the task's premise: `README.md`'s License and Contributing sections are already reworded (no commercial-license offer, no CLA, no rights assignment), the disclaimer already reads "Somnus Lab", and the strings `project maintainer` / `licensed to the project` appear nowhere in the tree. Per the task's own rule ("do NOT guess and do NOT edit"), no README was edited. STEP 0 was done (`3cc247e`). STEPS 3–5 were not run. This is a docs-task premise bug, not a repo defect.**

Date: 2026-09-10
Repo: `~/Projects/somnus-waveshare-rotary-dial` (matthewclaude/bedknob-for-somnus), branch `main`
Starting HEAD: `0b6c62f8cf7b084d4af1910bcfeea9d8ec5e5131` (= `somnus/main` at start, see Deviations)

## Gate check

| Check | Command | Result |
|---|---|---|
| 1 | `sed -n '21p' firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.0")` — PASS |
| 2 | `git tag -l 'somnus-v1.0.0'` | `somnus-v1.0.0` — PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | (empty) — PASS |

Untracked at gate time (expected, not a failure): `docs/REPORT-docs-amend.md`, `docs/REPORT-readme-license-fix.md` (the previous run's BLOCKED report, now overwritten by this file).

## STEP 0 — clear the owed report

`docs/REPORT-docs-amend.md` existed and was untracked. Added this line to `docs/REPORTS.md` directly after the `REPORT-docs-commit.md` line:

```
- `REPORT-docs-amend.md` — 2026-09-10 — REPORT: amend the docs commit message; track the docs-commit report — **BOTH STEPS DONE.** Step 1: `361a4cb` amended (message only, tree unchanged) to `eff6df3`; step 2: `docs/REPORT-docs-commit.md` + `docs/REPORTS.md` committed as `0b6c62f`. Neither commit pushed at the time of writing.
```

Committed as `3cc247e1cb38671360f2772e11acfe0019b5d7e1` — "docs: track the docs-amend report".

```
 docs/REPORT-docs-amend.md | 223 ++++++++++++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md           |   1 +
 2 files changed, 224 insertions(+)
```

## STEP 1 — raw output

```
$ git grep -n -iE "(commercial license|project maintainer|licensed to the project|Orion Sleep)"
CHANGELOG-orion.md:6:describes the Orion Sleep dual-zone-topper client this codebase started as,
CHANGELOG-orion.md:430:First public release. A standalone bedside dial for an Orion Sleep dual-zone
LICENSE:3:# PolyForm Noncommercial License 1.0.0
README.md:20:by Chris Meyer, which does the same job for an Orion Sleep topper through
README.md:265:beyond that license and **cannot offer a commercial license** to anyone;
THIRD_PARTY_LICENSES.md:5:© 2026 Chris Meyer, licensed under the PolyForm Noncommercial License 1.0.0
THIRD_PARTY_LICENSES.md:13:navigation fixes in `main.c` — under the same PolyForm Noncommercial License
docs/REPORT-releases-repo-readme.md:118: licensed under the **PolyForm Noncommercial License 1.0.0** — free for
firmware/dial-idf/docs/design-spec.md:95:Night window: Orion sleep schedule via MCP (bedtime −30min → wake +30min), manual override in quick-actions. [Not this firmware: fixed 21:00–07:00 window, no quick-actions override — see the provenance note at the top and docs/SPEC-night-window.md.] Backlight PWM (`lcd_bl_pwm_bsp`) tiers compound with the palette swap: day ceiling / night floor / 150ms "night-active" intermediate on any input, decaying after 8s (D2). Display never fully black.
web-flasher/RELEASES-README.md:48:licensed under the **PolyForm Noncommercial License 1.0.0** — free for
(exit 0)

$ git ls-files | grep -iE "readme"
README.md
docs/REPORT-releases-repo-readme.md
firmware/README.md
firmware/backups/README.md
firmware/dial-idf/README.md
firmware/dial-idf/docs/brand/README.md
firmware/dial-idf/test/README.md
reference/LOGGING_README.md
simulator/README.md
web-flasher/README.md
web-flasher/RELEASES-README.md
(exit 0)
```

Reading of the hits, per predicted string:

- `commercial license` — one hit, `README.md:265`, and it is the negation: "**cannot offer a commercial license** to anyone". The other "Noncommercial License" hits are the license's own name (LICENSE, THIRD_PARTY_LICENSES.md, RELEASES-README.md, a past report). None offers a commercial license.
- `project maintainer` — zero hits anywhere in the tree.
- `licensed to the project` — zero hits anywhere in the tree.
- `Orion Sleep` — `README.md:20` is the provenance sentence ("does the same job for an Orion Sleep topper through Orion's cloud"), not the disclaimer. The two `CHANGELOG-orion.md` hits are the preserved upstream changelog. The `design-spec.md` hit is "Orion sleep schedule" (lowercase, a feature description). No disclaimer names Orion Sleep.

Secondary scan of every README from `git ls-files` for `affiliat|contribut|## …licen|commercial|Orion Sleep|maintainer`: `firmware/README.md`, `firmware/backups/README.md`, `firmware/dial-idf/README.md`, `firmware/dial-idf/test/README.md`, `reference/LOGGING_README.md`, `simulator/README.md`, `web-flasher/README.md` — no hits; none has a License, Contributing or disclaimer section. `firmware/dial-idf/docs/brand/README.md:38` is a brand-guideline bullet ("Carry 'not affiliated with Somnus Lab' wherever the name is public"). `docs/REPORT-releases-repo-readme.md` is a past report quoting diffs, not a README.

Affected files in the task's sense (carrying the inherited upstream text): **none**.

## Before text, verbatim, per file

### `README.md` (unchanged; this is also the after text)

Disclaimer, lines 3–5:

```
> **Not affiliated with, endorsed by, or supported by Somnus Lab or Waveshare.**
> An independent, community-built project. Current release
> `somnus-v1.0.0`.
```

License section, lines 259–278 (outer fence is four backticks because the section itself contains a fenced block):

````
## License

**Source-available, not open source.**
[PolyForm Noncommercial 1.0.0](LICENSE), inherited from Orion Dial and
passed through unchanged. Free to use, build, modify and share for
**personal and other noncommercial purposes**. This fork holds no rights
beyond that license and **cannot offer a commercial license** to anyone;
commercial use of the inherited code is a question for its author.

```
Required Notice: Copyright © 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)
```

Third-party components are under their own licenses — see
[THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).

**Provided as is, without warranty.** You flash and use it at your own
risk. The pad clamps every setpoint to its own safe range regardless of
what a client asks for, and the Somnus app remains the authoritative
control for your bed.
````

Contributing section, lines 280–284 (end of file):

```
## Contributing

Contributions are accepted under PolyForm Noncommercial 1.0.0, the same
terms as the rest of the code. There is no contributor agreement and no
assignment of rights — there is nothing this fork could do with them.
```

### `web-flasher/RELEASES-README.md` (unchanged; this is also the after text)

Disclaimer, inside the opening paragraph, lines 3–6:

```
Firmware releases and the browser flasher for **Bedknob for Somnus** — a bedside
dial that turns a Waveshare ESP32-S3 round touch-LCD knob into a standalone
temperature control for a Somnus Pad. Not affiliated with, endorsed by, or
supported by Somnus Lab or Waveshare.
```

License section, lines 44–52 (end of file). There is no Contributing section in this file.

```
## License and attribution

This firmware is a fork of
[chris023/orion-waveshare-rotary-dial](https://github.com/chris023/orion-waveshare-rotary-dial),
licensed under the **PolyForm Noncommercial License 1.0.0** — free for
personal, noncommercial use. See [`LICENSE`](LICENSE) for the full terms
and required notice, and [`THIRD_PARTY_LICENSES`](THIRD_PARTY_LICENSES)
for the hardware bring-up code, fonts, data, and libraries this project
builds on, each under its own terms.
```

## After text, verbatim, per file

Identical to the before text. No file was edited.

How the on-disk text compares with the requested replacement, for the owner's judgement:

- README License: already says source-available not open source, names Orion Dial and Chris Meyer with the upstream URL, PolyForm Noncommercial 1.0.0 with a LICENSE link, the Required Notice, "cannot offer a commercial license", and links THIRD_PARTY_LICENSES.md. It adds a warranty/safety paragraph the requested text lacks. It does not use the literal phrase "no right to sublicense".
- README Contributing: already PolyForm-only, no contributor agreement, no rights assignment. It does not say "Issues and pull requests are welcome".
- README disclaimer: already exactly `Not affiliated with, endorsed by, or supported by Somnus Lab or Waveshare.` (bold, inside a blockquote, followed by two more lines).
- RELEASES-README: has a "License and attribution" section (different heading), no commercial-license sentence to remove, no Contributing section, disclaimer already names Somnus Lab.

If the owner wants the README wording replaced with the task's exact text anyway, that is a wording-preference change on already-correct text and needs a new task that states so.

## Commits — `git diff --stat` per commit, every SHA

| Commit | SHA | Subject | Stat |
|---|---|---|---|
| STEP 0 | `3cc247e1cb38671360f2772e11acfe0019b5d7e1` | docs: track the docs-amend report | `docs/REPORT-docs-amend.md +223`, `docs/REPORTS.md +1` (2 files, 224 insertions) |
| STEP 5 commit 1 (README changes) | — | not created | — |
| STEP 5 commit 2 (this report + index line) | — | not created | — |

## Push and CI

Not pushed. No `git push` was run. `somnus/main` remains at `0b6c62f`; local `main` is one commit ahead (`3cc247e`). No CI run was triggered; no run id.

## Deviations

- **Premise mismatch, so STEPS 3, 4 and 5 were not executed** and no README was edited. This is the instructed behaviour under the task's "text on disk does not match" rule.
- **`somnus/main` was not two commits behind.** The task said local `main` was expected to be ahead of `somnus/main` by two unpushed docs commits (`eff6df3`, `0b6c62f`). At gate time `somnus/main`, both the local tracking ref and `git ls-remote somnus refs/heads/main`, already resolved to `0b6c62f` = HEAD, so those two commits had been pushed before this run. Nothing was done about it; noted only.
- **STEP 1 was run before STEP 0 committed.** Both were read-only/independent; STEP 0's commit does not touch any file STEP 1 greps for except `docs/REPORTS.md` (no hits either way). Order of the report follows the task's order.
- **The public releases repo README was checked via the GitHub API, not from disk** (see next section). Read-only `gh api` call; nothing there was changed.
- The first `git rev-parse --short somnus/main HEAD` invocation errored ("Needed a single revision") because `--short` takes one revision; re-run without it. No effect on anything.
- This report is left **untracked**, per the repo's convention that a report is committed by the next docs commit.

## Cannot verify from disk

- **Public releases repo README (`matthewclaude/somnus-dial-releases`).** Not on disk. Checked instead with `gh api repos/matthewclaude/somnus-dial-releases/readme` (blob sha `046158dfc671edb88ee8b5d26b535f1e078d66ac`): the served `README.md` is **byte-identical** to this repo's `web-flasher/RELEASES-README.md` (`diff` clean). So it carries the same already-correct text: Somnus Lab disclaimer, "License and attribution" section, no commercial-license offer, no Contributing section, and the line "If you want the source, ask the maintainer." (that is "maintainer", not the task's "project maintainer" string, and it is about obtaining the source, not licensing). No change is needed there for this task; it would need its own hand-maintained update if the owner later changes the wording here.
- Whether the owner's intended wording (the exact replacement text in the task) is meant to supersede the current, already-correct README wording. That is a preference, not something the tree can answer.
- Nothing about hardware or CI was in scope; nothing was built or flashed.
