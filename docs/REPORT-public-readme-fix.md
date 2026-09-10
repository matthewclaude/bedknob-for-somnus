# REPORT: public README fix — the source is public now

**DONE** — both README paragraphs that still said the source was private were replaced exactly as specified (`web-flasher/RELEASES-README.md` whole paragraph; `web-flasher/README.md` one clause), committed as `ec8ce37` on `main`; the corrected `RELEASES-README.md` was mirrored byte-identical to `matthewclaude/somnus-dial-releases` as its `README.md` (commit `306985b`) and the public copy re-fetched through the API shows the new paragraph. The only "private" mention left in the three READMEs is `README.md:243` about the two Mac apps, which is still true. Firmware-repo push and CI: see the post-push addendum at the end.

Date: 2026-09-10
Repo: `~/Projects/somnus-waveshare-rotary-dial` (matthewclaude/bedknob-for-somnus), branch `main`
Starting HEAD: `bb1215e57a613c6f3ebb8717b66a25fe9ecab598` = `somnus/main` (tree clean)

## Gate check

| Check | Command | Result |
|---|---|---|
| 1 | `sed -n '21p' firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.0")` — PASS |
| 2 | `git tag -l 'somnus-v1.0.0'` | `somnus-v1.0.0` — PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | (empty) — PASS |
| 4 | `gh repo view matthewclaude/bedknob-for-somnus --json visibility --jq .visibility` | `PUBLIC` — PASS (premise holds) |

## STEP 1 — `web-flasher/RELEASES-README.md`

The paragraph on disk (lines 8–14) matched the task's quotation exactly; it was replaced as a single exact-match operation.

Before:

```
**This repo holds binaries, a manifest, and this flasher page only — it is
not the firmware's source.** The source lives in a private repository.
That's a deliberate choice about this project's development, not a
statement about the license below: the firmware is source-available
under the same PolyForm Noncommercial terms it always has been, this repo
just isn't where the buildable source happens to live. If you want the
source, ask the maintainer.
```

After (lines 8–13):

```
**This repo holds binaries, a manifest, and this flasher page only — it is
not the firmware's source.** The source is public at
[matthewclaude/bedknob-for-somnus](https://github.com/matthewclaude/bedknob-for-somnus),
source-available under the PolyForm Noncommercial terms described below.
Releases are published here rather than alongside the source because the
dial's over-the-air update client resolves them by this repo's URL.
```

## STEP 2 — `web-flasher/README.md`

Lines 3–9 matched the task's quotation exactly. Only the clause "This project's source stays private;" changed, to "This repo is public, but"; the line break moved one word to keep both lines under the existing width.

Before (lines 4–5 of the paragraph):

```
(https://matthewclaude.github.io/somnus-dial-releases/). This project's
source stays private; the release workflow publishes this directory's
```

After:

```
(https://matthewclaude.github.io/somnus-dial-releases/). This repo is
public, but the release workflow publishes this directory's
```

Full paragraph as it now reads (lines 3–9):

```
Source of the GitHub Pages browser-flasher site
(https://matthewclaude.github.io/somnus-dial-releases/). This repo is
public, but the release workflow publishes this directory's
contents, plus the built merged firmware image, to the `gh-pages` branch of
the public `matthewclaude/somnus-dial-releases` repo instead — see
`docs/SPEC-ota-readiness.md` §5 for why, and that repo's own
`RELEASES-README.md` (drafted alongside this file) for what ships there.
```

## STEP 3 — verify

**3a.** The unrestricted grep is not zero: every remaining hit is inside a historical `docs/REPORT-*.md` file that quotes the old wording (diffs and "cannot verify" notes from the 2026-09-04 and 2026-09-10 passes). Restricted to everything except `docs/REPORT-*`, it is zero.

```
$ git grep -n -iE "source (stays|lives in a) private|ask the maintainer"
docs/REPORT-publish-audit.md:146:docs/REPORT-readme-license-fix.md:185:- **Public releases repo README (`matthewclaude/somnus-dial-releases`).** … "If you want the source, ask the maintainer." …
docs/REPORT-readme-license-fix.md:185:- **Public releases repo README (`matthewclaude/somnus-dial-releases`).** … "If you want the source, ask the maintainer." …
docs/REPORT-releases-repo-readme.md:58:-the firmware's source.** The source lives in a private repository. That's a
docs/REPORT-releases-repo-readme.md:60:-license below. If you want the source, ask the maintainer.
docs/REPORT-releases-repo-readme.md:62:+not the firmware's source.** The source lives in a private repository.
docs/REPORT-releases-repo-readme.md:67:+source, ask the maintainer.
docs/REPORT-releases-repo-readme.md:544:happens to live. If you want the source, ask the maintainer.
docs/REPORT-releases-repo-readme.md:553:source, ask the maintainer.
(exit 0)

$ git grep -n -iE "source (stays|lives in a) private|ask the maintainer" -- . ':!docs/REPORT-*'
(no output, exit 1)
```

(The two long lines at `REPORT-publish-audit.md:146` and `REPORT-readme-license-fix.md:185` are abridged here with "…"; they are the same sentence, the audit report quoting the earlier report's grep output.) All eight are historical record, kept by this repo's convention; none is live documentation.

**3b.**

```
$ git grep -n -i "private" -- README.md web-flasher/README.md web-flasher/RELEASES-README.md
README.md:243:Mini**, a read-only menu bar reader. Both are private for now.
(exit 0)
```

One hit. `README.md:243` is the "Related" section's sentence about **Bedknob for Mac** and **Bedknob Mini**, the two macOS companion apps, which live in their own repositories and are still private — that statement is still correct and was not changed (as the task required). `web-flasher/README.md` and `web-flasher/RELEASES-README.md` no longer contain the word "private" at all.

## STEP 4 — mirror

```
$ gh repo clone matthewclaude/somnus-dial-releases /tmp/somnus-dial-releases-mirror
clone exit 0; origin = https://github.com/matthewclaude/somnus-dial-releases.git; HEAD 21c239d37550fc4a6f066bd0d57220c5041d69f9
$ cp web-flasher/RELEASES-README.md <clone>/README.md
$ cmp <clone>/README.md web-flasher/RELEASES-README.md
identical
 README.md | 11 +++++------
 1 file changed, 5 insertions(+), 6 deletions(-)
mirror commit: 306985bebf31fef5afdfc3d7923c971a6a74dbe6  docs: the source is public now
$ git push origin main          (that clone's origin = the releases repo)
To https://github.com/matthewclaude/somnus-dial-releases.git
   21c239d..306985b  main -> main
git ls-remote origin refs/heads/main → 306985bebf31fef5afdfc3d7923c971a6a74dbe6
```

Clone directory removed afterwards (confirmed absent).

## Commits — `git diff --stat` per firmware-repo commit, every SHA

**Commit A (STEPS 1–2)** — `ec8ce37e2e27829528798c30c121d2dfde5a796e` — "docs: the source is public now" (body notes the visibility change today, 2026-09-10)

```
 web-flasher/README.md          |  4 ++--
 web-flasher/RELEASES-README.md | 11 +++++------
 2 files changed, 7 insertions(+), 8 deletions(-)
```

No firmware file touched.

**Commit B (this report + its `docs/REPORTS.md` line)** — SHA in the addendum below and in the chat reply.

**Commit C (post-push addendum)** — SHA in the chat reply only.

**Releases repo:** `306985bebf31fef5afdfc3d7923c971a6a74dbe6` on `main` (previous HEAD `21c239d`).

## Firmware-repo push and CI

See the post-push addendum at the end of this report.

## Deviations

- **STEP 3a's "zero hits" holds only outside `docs/REPORT-*`.** The unrestricted grep returns eight lines, all inside three historical reports that quote the old paragraph or its diff. They are the repo's deliberate historical record and were not edited; the live READMEs are clean. Reported rather than forced to zero.
- **Three commits after STEP 2 rather than two**, as instructed (addendum pattern). The addendum push triggers one further ci.yml run not tracked here.
- Otherwise none: both paragraphs matched the quoted text, `README.md:243` untouched, nothing pushed to the firmware repo's `origin`.

## Cannot verify from disk

- **The public GitHub page for `somnus-dial-releases`.** Checked over the network after the mirror push: `gh api repos/matthewclaude/somnus-dial-releases/readme` returns blob sha `d67c297030df55e8d2217752b29b749c60d0d2cc` (the same blob git assigned to the edited `web-flasher/RELEASES-README.md`), its raw content is byte-identical to the firmware file (`cmp` clean), and the first line of the new paragraph reads:

  > **This repo holds binaries, a manifest, and this flasher page only — it is

  followed by "not the firmware's source.** The source is public at". So the API serves the new text. Whether GitHub's rendered HTML page has refreshed its cache was not checked visually.
- **The flasher site itself** (`matthewclaude.github.io/somnus-dial-releases`) is built from the `gh-pages` branch, not from this README, so it is unaffected by this change and was not re-checked.
- **Whether any other public-facing text still implies the source is private** — outside the three READMEs the task named, no search was made beyond the STEP 3a grep, which found the old phrasing only in historical reports.

## Post-push addendum (written after commit B, committed separately)

Commit B (this report + its `docs/REPORTS.md` line): **`6fabbb976057809e75cd53a847b1507109f837fb`** — "docs: public-readme-fix report"

```
 docs/REPORT-public-readme-fix.md | 162 +++++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                  |   1 +
 2 files changed, 163 insertions(+)
```

Push:

```
$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   bb1215e..6fabbb9  main -> main
```

Commits carried (`git log --oneline bb1215e..HEAD`, `somnus/main` before the push was `bb1215e`):

```
6fabbb9 docs: public-readme-fix report
ec8ce37 docs: the source is public now
```

`git ls-remote somnus refs/heads/main` after the push: `6fabbb976057809e75cd53a847b1507109f837fb`. `origin` was not touched (push URL is `no_push`).

CI (`gh run list --repo matthewclaude/bedknob-for-somnus --limit 3`, columns: id, workflow, sha, event, status, conclusion, created):

```
34509791509 ci 6fabbb9 push in_progress  2026-09-10T17:41:51Z
34508885630 ci bb1215e push completed success 2026-09-10T17:32:58Z
34508416097 ci 68b854d push completed success 2026-09-10T17:28:19Z
```

`gh run watch 34509791509 --exit-status` returned 0. Final state: **ci.yml run 34509791509 on `6fabbb9` — completed, conclusion `success`** (single job `build`, success). This is the first CI run since the repo became public.

This addendum is committed as a third docs-only commit ("docs: public-readme-fix report — push and CI addendum") whose SHA is reported in the chat reply, and pushed to `somnus`. That push triggers one further ci.yml run, not tracked here.
