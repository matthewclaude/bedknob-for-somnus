# REPORT: post-publication doc sweep — dial_ota comment, SPEC-power-sensing paths, NAMING item 6

**DONE** — all four changes applied as comments and docs only: the `dial_ota.c` comment above `GITHUB_API_URL` no longer describes the source repo as private (the three `#define` values are byte-identical before and after, proven below), the two dangling `reference/` paths in `SPEC-power-sensing.md` now say the material is kept locally by the maintainer and name each public source, `NAMING.md` gains item 6 and the table row notes "public since Sep 10 2026", and `REPORTS.md` indexes this report. One commit, not pushed.

Date: 2026-09-10
Repo: `~/Projects/somnus-waveshare-rotary-dial` (matthewclaude/bedknob-for-somnus, PUBLIC), branch `main`

## 1. Verdict

DONE. (No BLOCKED gate; no PARTIAL item.)

## 2. Gate check

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.0")
$ git tag -l somnus-v1.0.0
somnus-v1.0.0
$ git describe --tags --abbrev=0 --match 'somnus-v1.0.0' HEAD
somnus-v1.0.0
(exit 0)
$ git --no-optional-locks status --short --untracked-files=no
(no output)
$ git rev-parse --abbrev-ref HEAD
main
$ git remote get-url --push origin
no_push
```

All five pass. HEAD at gate time: `492416bf7eb7ec03156b1113177512aab1059c15`, equal to `somnus/main`.

## 3. Raw outputs

**B, before:**

```
$ grep -n 'reference/' docs/SPEC-power-sensing.md
96:- **The schematic** (five sheets, re-read Sep 3; the five PNGs are now in `reference/waveshare-schematic/`, copied from Sandjab/Waveshare-Knob on GitHub because Waveshare's own download was unreachable): the same five sheets §5 describes. Sheet 1 has USB-C, the TLV62569 buck from the `5V` net to 3V3, the backlight FET and the two encoder switches. Sheet 4 has the `BATT_ADC` divider (R62/R63, 10K/10K, off `5V`), the PDM mic (MSM261D4030H1CPM), the TF socket. **No battery socket, no charger, no boost, no power button, no charge-status net on any sheet.** Sheet 2's ESP32-S3 net list confirms it: the S3's only power-related pin is `GPIO1 = BATT_ADC`. There is no `CHRG`, `PGOOD`, `PWR_KEY` or PMIC I2C. The battery section of the board is simply not in the published schematic, exactly as §5 concluded.
213:**Supersedes §9.5's "no percentage, no low warning, no setting" and §10.6's matching exclusions.** Those were the right call with only this project's own measurements in hand. Overtaken by finding [chris023/orion-waveshare-rotary-dial#4](https://github.com/chris023/orion-waveshare-rotary-dial/pull/4) (author: borski, unmerged as of this writing) — an independent battery implementation on the same board, same pin, same divider, whose measured thresholds land within noise of this project's own (§9.5, §10.2). Full PR content fetched read-only and kept for reference at `reference/upstream-pr4-battery-diag/` (not tracked, not built against directly — see that folder's own README). Owner's decision: adopt the percentage curve and the low-battery visual treatment; do not adopt their detector, their separate diagnostics screen, or their new dial_battery component wholesale.
(exit 0)
```

**B, after:**

```
$ grep -n 'reference/' docs/SPEC-power-sensing.md
(no output, exit 1)
```

The two sentences now read, in the parts that changed (technical content around them untouched):

- line 96: "(five sheets, re-read Sep 3; the five PNGs are kept locally by the maintainer and are not in this repository — the public source is Sandjab/Waveshare-Knob on GitHub, used because Waveshare's own download was unreachable)"
- line 213: "Full PR content was fetched read-only and is kept locally by the maintainer, not in this repository, and not built against directly; the public source is the PR itself, linked above."

**A, full diff of the one firmware file:**

```
$ git diff -- firmware/dial-idf/components/dial_ota/dial_ota.c
diff --git a/firmware/dial-idf/components/dial_ota/dial_ota.c b/firmware/dial-idf/components/dial_ota/dial_ota.c
index 27ea989..7ba408c 100644
--- a/firmware/dial-idf/components/dial_ota/dial_ota.c
+++ b/firmware/dial-idf/components/dial_ota/dial_ota.c
@@ -28,9 +28,11 @@ extern const char trust_roots_pem_start[] asm("_binary_trust_roots_pem_start");
 static const char *TAG = "ota";
 
 // Points at the public binaries-only release repo (docs/SPEC-ota-readiness.md
-// §5), not the private source repo this firmware is built from -- GitHub
-// 404s every endpoint under a private repo to an unauthenticated client
-// (§1), and both the dial and ESP Web Tools are unauthenticated by design.
+// §5). The source repo is public too (since 2026-09-10), but releases keep
+// publishing to somnus-dial-releases because every shipped dial resolves
+// updates by exactly this URL; consolidating releases into the source repo
+// is a separate, planned step (the OTA repoint), not a change this constant
+// can make on its own.
 #define GITHUB_API_URL \
     "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest"
 // Beta channel only (docs/SPEC-ota-readiness.md §9.7, 2026-09-03 finding): GitHub's
```

Every changed line begins with `//`. No `#define` line is added, removed or altered; the three `#define GITHUB_API_URL*` lines and their continuation strings, extracted from HEAD and from the working tree without line numbers, have the same md5 (`b686893a20bbb8c5b3f9b60dbb34a1a4`). Their line numbers shift by +2 (now 36, 44, 48) because the comment grew from four lines to six; the `(§1)` cross-reference in the old comment, which explained the private-repo 404 behaviour, is dropped with the sentence it belonged to, and the `(docs/SPEC-ota-readiness.md §5)` pointer is kept.

**C, `docs/NAMING.md`:** item 6 appended to "What is still outstanding" directly after item 5 (same numbered-item style, bold lead, three-space continuation indent), and the first "Old names → new" row's Status cell now reads "… renamed to `matthewclaude/bedknob-for-somnus` Sep 5, public since Sep 10 2026 (default branch `main` = the ported code); …". Full diff is in the commit.

**Personal-data check on the diff:** a grep of the full working diff for private addresses, home paths, MAC addresses, token prefixes or "open source" hit only two unchanged context lines (the existing Bedknob Mini table row with the spec's example address `192.168.1.100` and `~/Projects/…`, and the existing REPORTS.md audit line mentioning a masked `gho_****`). None of the added lines contain any of these.

## 4. `git diff --stat` and commit SHA

Working-tree stat before this report file was added (the commit also carries `docs/REPORT-post-public-sweep.md`):

```
 docs/NAMING.md                                   | 12 +++++++++++-
 docs/REPORTS.md                                  |  1 +
 docs/SPEC-power-sensing.md                       |  4 ++--
 firmware/dial-idf/components/dial_ota/dial_ota.c |  8 +++++---
 4 files changed, 19 insertions(+), 6 deletions(-)
```

HEAD before the commit: `492416b`. The commit SHA itself cannot be written into a file that the same commit carries; it is the single commit on top of `492416b` — `git rev-parse --short HEAD` on `main` after this run, with subject "docs: post-publication sweep — dial_ota comment, SPEC-power-sensing paths, NAMING item 6".

## 5. Deviations

- **The commit SHA is not inside this report** (see §4): the spec asks for the SHA in the report and for the report to be inside the commit, which cannot both hold; the report records HEAD-before and how to read the SHA instead.
- **The `dial_ota.c` comment grew from four lines to six**, so the `#define` lines below it moved by two line numbers. The spec required the values to be byte-identical, which they are; nothing said the line numbers must not move.
- **The old comment's `(§1)` cross-reference was dropped** along with the private-repo-404 sentence it annotated; only the `§5` pointer was required to stay.
- Otherwise none: nothing pushed, no file renamed, no history rewritten, `release.yml` and `DIAL_CERT_ERR_MSG` untouched, no new feature, every reference reworded sentence-by-sentence rather than string-replaced (each edit was an exact single-occurrence match of the quoted sentence).

## 6. False premises

False premises: none. Every quoted passage was on disk as described: the `dial_ota.c` comment at lines 30–33 above `GITHUB_API_URL` with the "private source repo" wording; `SPEC-power-sensing.md` lines 96 and 213 citing the two `reference/` directories; `NAMING.md` items 1–5 under "What is still outstanding" and the table row's "Sep 5" Status cell; `DIAL_CERT_ERR_MSG` at `dial_state.h:358` still carrying `matthewclaude/somnus-waveshare-rotary-dial`; `REPORTS.md` in the expected one-line-per-report format.

## 7. Could not be verified without hardware

Nothing. No behaviour changed: the only firmware edit is a C comment, and the three URL constants it sits above are byte-identical, so the built binary's OTA requests are unchanged. No build or flash was needed or performed.

## 8. Push and CI addendum

Written after §1–§7 were committed as `11448b911f48d8f3780327b2835765e567367e8c` (short `11448b9`, subject "docs: post-publication sweep — dial_ota comment, SPEC-power-sensing paths, NAMING item 6"). That SHA could not go into the file its own commit carried (§4, §5); it is recorded here instead.

Gate before the push: `git rev-parse --short HEAD` = `11448b9`; `git --no-optional-locks status --short --untracked-files=no` printed nothing; `git remote get-url --push origin` = `no_push`; `git log --oneline somnus/main..HEAD` = exactly the one line `11448b9`.

First push (raw output):

```
$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   492416b..11448b9  main -> main
```

Only `main` was pushed, only to `somnus`; no tags, `origin` untouched.

CI (raw output of `gh run list --repo matthewclaude/bedknob-for-somnus --limit 3`, taken a few seconds after the push):

```
in_progress		docs: post-publication sweep — dial_ota comment, SPEC-power-sensing p…	ci	main	push	34517958212	16s	2026-09-10T19:02:06Z
completed	success	docs: public-readme-fix report — push and CI addendum	ci	main	push	34510140392	3m10s	2026-09-10T17:45:21Z
completed	success	docs: public-readme-fix report	ci	main	push	34509791509	3m8s	2026-09-10T17:41:51Z
```

The ci.yml run for `11448b9` is **34517958212** (`headSha` `11448b911f48d8f3780327b2835765e567367e8c`, event `push`). `gh run watch 34517958212 --repo matthewclaude/bedknob-for-somnus --exit-status` exited 0. Final state: **completed, conclusion `success`** — single job `build` (ID 103008043459, 3m1s: checkout, "Build firmware (firmware/dial-idf)"). One annotation, unrelated to this change: the runner warns that `actions/checkout@v4` targets Node.js 20 and is being forced onto Node.js 24. There was no release run; nothing was tagged.

This section is committed on its own as "docs: post-public-sweep report — push and CI addendum" and pushed to `somnus` afterwards.

**Deviation:** the instruction asked for the second push's raw output to be recorded in this section before that commit is made. That is impossible in this order — the second push carries the addendum commit, so its output exists only after the commit. The second push output is therefore reported in the chat reply, not in this file. It triggers one further ci.yml run, not tracked here.
