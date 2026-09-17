# REPORT: docs index commit — REPORT-mcp-integration, REPORT-spec-location and their REPORTS.md lines

Date: 2026-09-14. Task: index the two outstanding reports in `docs/REPORTS.md` and commit them together with the index in one docs commit. No push.

## 1. Verdict

**DONE.** Three index lines added to `docs/REPORTS.md` in the existing format (mcp-integration, spec-location, this report); `docs/REPORTS.md`, `docs/REPORT-mcp-integration.md` and `docs/REPORT-spec-location.md` staged one path at a time and committed together as `ff42f6d` on top of `abac7a1`, which is neither amended nor reverted. Not pushed. This report file itself is untracked and rides the next docs commit (section 6).

## 2. Gate check — the no-features-before-v1 rule

**Not implicated. Confirmed explicitly.** The commit changes three Markdown files under `docs/` and nothing else (`git diff --cached --stat` in section 3: 3 files, 356 insertions, all `docs/`). No firmware, simulator, companion, workflow or configuration file is touched, so there is no feature to gate. Separately, the only in-repo record of the rule, `docs/SPEC-voice.md` §0.2 ("no new features until v1 is functional and a new user can easily add their pad"), has had its precondition met since `somnus-v1.0.0` shipped on 2026-09-09; current stable is `somnus-v1.0.1`.

## 3. Raw, unfiltered output of every git command run

Before any edit (state check and the one pre-existing uncommitted index line):

```
$ git --no-optional-locks status --short --branch
## main...somnus/main [ahead 1]
 M docs/REPORTS.md
?? docs/REPORT-mcp-integration.md
?? docs/REPORT-spec-location.md

$ git diff -- docs/REPORTS.md
diff --git a/docs/REPORTS.md b/docs/REPORTS.md
index 62c2288..8df3058 100644
--- a/docs/REPORTS.md
+++ b/docs/REPORTS.md
@@ -81,5 +81,6 @@ A `REPORT-*.md` file is Claude Code's fixed-path output for one task: a verdict
 - `REPORT-1.0.2-beta.1-ota-bench.md` — 2026-09-13 — REPORT: 1.0.2-beta.1 beta-channel OTA bench verification — **PASS** — gate passed (1.0.2-beta.1 / tag → `00bd94d` / clean / serial device present) and the Release object fetched with `gh release view` (isPrerelease true, two assets, downloadCount 0 before the run); closes SPEC-standby-poll §6 item 7 — the first over-the-air install served from this repo's own Release: bench dial on 1.0.1 (39af8f5 wire flash in `ota_0`), Beta builds On, `ota: latest 1.0.2-beta.1, running 1.0.1 -- update available` → `esp_https_ota: Writing to <ota_1> partition at offset 0x420000` → written and verified in 20.4 s → software reset → `App version: 1.0.2-beta.1` loaded from 0x420000 → `app marked valid; rollback cancelled` → manual re-check `latest 1.0.2-beta.1, running 1.0.2-beta.1 -- up to date`; 209-line capture with one attach marker and no loss (the USB-Serial-JTAG device survives the OTA reset, so the reboot shows only as the ROM banner and tick reset), zero assert/panic/abort/Guru, zero `E (`, three benign `W (` lines attributed, `POST /api/` count 0, pad side A off / 18.0 °C before and after; install ≈ 21.3 s from the download line to the version banner by tick arithmetic across the reset; pre-run slot `ota_0` inferred, not quoted; six deviations (rev-parse added after `describe` matched, grep widened to `esp_https_ota:`, nohup reader stopped by pid, log truncated first, step 3b re-asked, manual re-check needed because the first automatic check is ~24 h after boot); not verified: a Beta-Off dial staying on 1.0.1, and the browser flasher; Beta builds left On at the owner's choice; nothing built, flashed by wire, committed, tagged or pushed in that block.
 - `REPORT-phase6b.md` — 2026-09-13 — REPORT: SPEC-repo-consolidation Phase 6b — secret, PAT, frozen-redirect README, Pages redirect, archive — **DONE** — gate re-run this session (1.0.2-beta.1 / grep exit 1 / clean / 1.0.1 Release with both assets) found `SOMNUS_RELEASES_TOKEN` still listed, so step 1 ran here (deleted, list empty); owner confirms the PAT "somnus-dial-releases publish" is revoked; frozen-redirect README pushed to the old repo's `main` as `e0f0b51` from the rewritten `web-flasher/RELEASES-README.md`, meta-refresh `index.html` pushed to its `gh-pages` as `6aa3a12` (firmware/ and manifests kept) and confirmed live by curl and by the owner in a browser; `somnus-dial-releases` archived after an explicit yes; after archiving, `somnus-v1.0.1` still present with both assets, `/releases/latest` still answers it and `somnus-dial.bin` downloads with the recorded SHA-256; old URL serves the redirect, new URL the flasher; main 0 ahead of `somnus/main`; no build, no code change; eight deviations (step 1 undone earlier, `web-flasher/README.md` fixed, two questions at steps 2 and 5, hung `cat >`, redirect source not kept in-tree, one extra asset download, this line); not verified: a real 1.0.0 dial migrating via the archived repo.
 - `REPORT-phase7-doc-sweep.md` — 2026-09-13 — REPORT: SPEC-repo-consolidation Phase 7 — doc sweep, point the repo's own docs at the single repo — **DONE** — gate passed (1.0.2-beta.1 / clean tracked tree / HEAD `7ff0c71` Phase 6b docs commit / `isArchived: true`); every line number in the block matched disk; seven files reworded, not string-replaced: both READMEs now send a first install to `matthewclaude.github.io/bedknob-for-somnus/` and link this repo's Releases page, `ARCHITECTURE.md` describes `dial_ota` as querying `bedknob-for-somnus` with the archived repo kept as the `1.0.0` migration path, `SPEC-ota-readiness.md` line 6 only carries the dual-publish history (`1.0.1-beta.1`–`1.0.1` dual, single-repo from `1.0.2-beta.1`), `release.yml` line 13 and `NAMING.md` line 46 note the 2026-09-13 archive without changing what they assert, `dial_ota.c` header comment reworded with every changed line beginning `//` (checked by `diff -U0` filter; no `#define`, no `PROJECT_VER`, no build, no tag); sweep grep leaves 39 hits outside reports and CHANGELOG, each listed with why it stays (SPEC-repo-consolidation body and five SPEC-ota narrative lines as as-built record, migration-path clauses, RELEASES-README as the live redirect's source); `CHANGELOG.md` and every `REPORT-*.md` untouched; committed `b53b06e`, pushed to `somnus`, CI run 34778960227 success (build 3m8s); the "ask the maintainer" plan item was already fixed by `ec8ce37` on 2026-09-10; two deviations (line 6's join with the unchanged line 7 reads as a list, three wrapped sentences changed their neighbouring lines); nothing hardware could verify; report itself untracked, riding the next docs commit.
+- `REPORT-mcp-integration.md` — 2026-09-14 — REPORT: Home Assistant MCP server integration — docs/SPEC-home-integration.md — **DONE, WITH ONE STRUCTURAL DEVIATION.** The requested content is written and committed as `abac7a1`, but as a **new** `docs/SPEC-home-integration.md`: the file the task asked to update does not exist in this repo or in any ref of its history, and neither the Alexa RangeController finding nor a "section 7.5" exists in any file here; both are recorded in the spec as not present rather than paraphrased; gate check: no-features-before-v1 rule not implicated (docs only, and 1.0.0 already shipped); nothing pushed.
 
 Regenerate this index when adding reports; it is hand-maintained, not built.

$ git log --diff-filter=A --format=%ad --date=short -- docs/REPORT-mcp-integration.md docs/REPORT-spec-location.md   # both untracked, so date = today
```

After the two new lines were appended to `docs/REPORTS.md` (section 5), staging and commit:

```
$ git add docs/REPORTS.md
exit 0
$ git add docs/REPORT-mcp-integration.md
exit 0
$ git add docs/REPORT-spec-location.md
exit 0

$ git diff --cached --stat
 docs/REPORT-mcp-integration.md | 124 ++++++++++++++++++++++
 docs/REPORT-spec-location.md   | 229 +++++++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                |   3 +
 3 files changed, 356 insertions(+)

$ git commit -F <message>
[main ff42f6d] docs: MCP integration and spec-location reports, and their REPORTS.md lines
 3 files changed, 356 insertions(+)
 create mode 100644 docs/REPORT-mcp-integration.md
 create mode 100644 docs/REPORT-spec-location.md
exit 0

$ git rev-parse HEAD
ff42f6dfcb443ea3211419c07fd0f205f38d24f4

$ git show --stat --format="%H%n%P%n%s" HEAD
ff42f6dfcb443ea3211419c07fd0f205f38d24f4
abac7a1b232d1665cac898b3e74e0be434d2d244
docs: MCP integration and spec-location reports, and their REPORTS.md lines

 docs/REPORT-mcp-integration.md | 124 ++++++++++++++++++++++
 docs/REPORT-spec-location.md   | 229 +++++++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                |   3 +
 3 files changed, 356 insertions(+)

$ git log --oneline -3
ff42f6d docs: MCP integration and spec-location reports, and their REPORTS.md lines
abac7a1 docs: SPEC-home-integration - Home Assistant MCP server (Assist) via Claude Desktop + mcp-remote, 2026-09-14
bcd50c4 docs: Phase 7 report, its REPORTS.md line, and a stale "ground truth" version in SPEC-ota-readiness

$ git --no-optional-locks status --short --branch
## main...somnus/main [ahead 2]

$ git diff --stat
exit 0

$ git ls-remote --heads somnus main   # remote main untouched
bcd50c4580fabef8f1dd8e5bf2de7f68206c4cab	refs/heads/main
exit 0
```

## 4. `git diff --stat`, new commit SHA, push status

- New commit: **`ff42f6dfcb443ea3211419c07fd0f205f38d24f4`** (`ff42f6d`), parent `abac7a1`, on `main`.
- `git diff --cached --stat` immediately before the commit:

```
 docs/REPORT-mcp-integration.md | 124 ++++++++++++++++++++++
 docs/REPORT-spec-location.md   | 229 +++++++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                |   3 +
 3 files changed, 356 insertions(+)
```

- `git diff --stat` after the commit: empty (tree clean except this report, written afterwards).
- **Nothing was pushed.** `git status` reports `main...somnus/main [ahead 2]` (`abac7a1` and `ff42f6d`), and `git ls-remote --heads somnus main` still answers `bcd50c4`, the last pushed commit. No `git push` was run.
- `abac7a1` is untouched: `git log --oneline -3` shows it unchanged as the parent, and no `--amend`, `revert` or `reset` was run.

## 5. Index lines added, verbatim

Format matched from the surrounding entries: `` - `FILE` — YYYY-MM-DD — H1 minus a leading "REPORT — " prefix — verdict sentence(s) ``, appended in file order before the trailing "Regenerate this index…" sentence. Date is today for files with no earlier first-commit date.

Line 1 (for `REPORT-mcp-integration.md`) was already present uncommitted from the previous task and is committed unchanged here:

```
- `REPORT-mcp-integration.md` — 2026-09-14 — REPORT: Home Assistant MCP server integration — docs/SPEC-home-integration.md — **DONE, WITH ONE STRUCTURAL DEVIATION.** The requested content is written and committed as `abac7a1`, but as a **new** `docs/SPEC-home-integration.md`: the file the task asked to update does not exist in this repo or in any ref of its history, and neither the Alexa RangeController finding nor a "section 7.5" exists in any file here; both are recorded in the spec as not present rather than paraphrased; gate check: no-features-before-v1 rule not implicated (docs only, and 1.0.0 already shipped); nothing pushed.
```

Lines 2 and 3 were added by this task:

```
- `REPORT-spec-location.md` — 2026-09-14 — REPORT: Where does SPEC-home-integration.md actually live? — read-only investigation — **NO.** No pre-existing "real" `SPEC-home-integration.md` exists on disk anywhere under `~/Projects`; the only copy is the one `abac7a1` added, `git log --follow` shows that single commit, the two `ls` globs and two `find` sweeps return only it and no `GUIDE-home-assistant-setup.md`; recommendation: `abac7a1` stands (one new file, 93 insertions, nothing duplicated or modified, self-describing provenance); read-only, nothing changed, reverted, committed or pushed; three deviations (zsh "no matches found" from the second `ls`, no REPORTS.md line added under "do not change any files", exit codes echoed); not determinable from disk: the Claude Project store, paths outside `~/Projects`, whether the GUIDE file or the Alexa/§7.5 material exist anywhere.
- `REPORT-docs-index-commit.md` — 2026-09-14 — REPORT: docs index commit — REPORT-mcp-integration, REPORT-spec-location and their REPORTS.md lines — **DONE.** Three index lines added to `docs/REPORTS.md` in the existing format (mcp-integration, spec-location, this report); `docs/REPORTS.md`, `docs/REPORT-mcp-integration.md` and `docs/REPORT-spec-location.md` staged one path at a time and committed together in a single docs commit on top of `abac7a1`, which is neither amended nor reverted; gate check: no-features-before-v1 rule not implicated (docs only, and 1.0.0 already shipped); not pushed; one deviation: this report file itself is untracked, because it has to carry the new commit's SHA and so can only be written after the commit — it rides the next docs commit.
```

Entry count after the commit: 82 index lines against 81 `REPORT-`/`REVIEW-`/`PLAN-`/`TEST-` files then on disk; the 82nd line is for this report, whose file now exists, so the count is 82 against 82 as of writing.

## 6. Deviations from the instructions

1. **This report file is not in the commit.** The task allowed for this: the report must quote the new commit's SHA, which exists only after committing, so the file was written afterwards. Its `REPORTS.md` line *was* added before the commit and is in `ff42f6d`; the file `docs/REPORT-docs-index-commit.md` is untracked and rides the next docs commit. Its verdict line was fixed in advance to match the index line, and it did.
2. **One read-only network call.** `git ls-remote --heads somnus main` was run after the commit to show that the remote head is still `bcd50c4`. It reads only; it is included because the task asks for confirmation that nothing was pushed.
3. **Index line for `REPORT-mcp-integration.md` was not re-written.** It was already in the working tree from the previous task in the correct format; it is committed as found rather than duplicated or reworded.

No other deviations.

## 7. Not verifiable here

- Whether the remote `somnus/main` will accept the two pending commits fast-forward when the owner pushes: it is at `bcd50c4`, the local parent of `abac7a1`, so a fast-forward is expected, but no push was attempted.
- Nothing else. All facts in this report come from the commands shown in section 3 and from files on disk in this repo.
