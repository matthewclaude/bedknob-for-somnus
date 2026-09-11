# REPORT: SPEC-repo-consolidation status and §7 gate reword

**DONE** — `docs/SPEC-repo-consolidation.md` now says §2–§4 shipped in `somnus-v1.0.1-beta.1` on 2026-09-10 and were hardware-confirmed, and §7 item 2 is the Beta-builds-off `ota: no releases published yet (HTTP 404)` check with item 3's serial capture keyed to it; documentation only, committed, not pushed.

Date: 2026-09-11. Repo `~/Projects/somnus-waveshare-rotary-dial`, branch `main`, starting HEAD `fb38c83` (docs: 1.0.1-beta.1 tag-push report).

## 1. Gate checks — raw output

```
$ grep -n 'PROJECT_VER' firmware/dial-idf/CMakeLists.txt
20:# Somnus versioning: tag somnus-vX.Y.Z must equal PROJECT_VER exactly (release.yml verifies). 1.0.0 shipped 2026-09-09; see CHANGELOG.md.
21:set(PROJECT_VER "1.0.1-beta.1")
exit=0

$ git tag -l 'somnus-v1.0.1-beta.1'
somnus-v1.0.1-beta.1
exit=0

$ git --no-optional-locks status --short --untracked-files=no
exit=0
```

All three passed: line 21 sets `PROJECT_VER` to `1.0.1-beta.1`, the tag exists by exact name, the tree was clean.

Premise checks, also before editing: the status line began `Status: **Draft, 2026-09-10. Not built. Supersedes the queue order …`; §7 was numbered 1–5 with items 2 and 3 as described; the log string exists verbatim in firmware — `firmware/dial-idf/components/dial_ota/dial_ota.c:309` is `ESP_LOGI(TAG, "no releases published yet (HTTP 404)");` with `TAG = "ota"` (line 28), and the file's only uses of the URL constants are as `ota_http_get()` arguments (lines 301, 364, 440) — no log statement prints a URL or repo name.

## 2. Status line

### Before

Status: **Draft, 2026-09-10. Not built. Supersedes the queue order in the 1.0.x line: this ships as `1.0.1-beta.1` BEFORE the standby-poll cadence change (`SPEC-standby-poll.md`), because it has a closing window and that spec does not.** The window is the one in §3.3 and §6: every dial that installs any release while both repos are publishing migrates itself over the air; every dial that does not is stranded on the old repo the day the old repo stops publishing. The standby-poll change has no such clock, so it waits.

### After

Status: **Shipped as `somnus-v1.0.1-beta.1` on 2026-09-10 and hardware-confirmed. §2–§4 are built and released in both repos; the dual-publish window of §3.3 is open. Still open: the Phase 6 cut-over listed in §5, and the §7 gate re-run against `1.0.1` stable. This work jumped the queue in the 1.0.x line — it shipped BEFORE the standby-poll cadence change (`SPEC-standby-poll.md`) because it has a closing window and that spec does not.** The window is the one in §3.3 and §6: every dial that installs any release while both repos are publishing migrates itself over the air; every dial that does not is stranded on the old repo the day the old repo stops publishing. The standby-poll change has no such clock, so it waited, and still queues behind this.

## 3. §7 item 2

### Before

2. Same dial → Check for updates → the serial log must show the request going to `api.github.com/repos/matthewclaude/bedknob-for-somnus/…` and the result `latest 1.0.1-beta.1, running 1.0.1-beta.1 -- up to date`.

### After

2. Same dial, now on `1.0.1-beta.1` → Menu → Update → set Beta builds **off** → Check for updates. The serial log must show, verbatim, `ota: no releases published yet (HTTP 404)`. That line is what tells the two repos apart: with Beta builds off the dial asks `/releases/latest`, and `bedknob-for-somnus` has no non-prerelease Release yet, so GitHub answers 404 — whereas the same check pointed at `somnus-dial-releases` would find its `1.0.0` stable Release and report up to date, never logging that line.

## 4. §7 item 3

### Before

3. **A serial capture of item 2 is required** and goes into the bring-up record. A silent "up to date" against the wrong host is indistinguishable from success on the screen — that was the failure shape of 2026-09-03 (`docs/REPORT-ota-beta-not-found.md`: eight `ota:` lines, every one "up to date", while the newer beta sat unseen on the server). Capture with the cat-based serial method from the bring-up notes, not `idf.py monitor`.

### After

3. **A serial capture of item 2 is required** and goes into the bring-up record: it must contain the `ota: no releases published yet (HTTP 404)` line from the Beta-builds-off check. A silent "up to date" against the wrong repo is indistinguishable from success on the screen — that was the failure shape of 2026-09-03 (`docs/REPORT-ota-beta-not-found.md`: eight `ota:` lines, every one "up to date", while the newer beta sat unseen on the server) — and the 404 outcome of item 2 is likewise shown as "up to date" on the display (§3.1), so only the log proves which repo answered. Capture with the cat-based serial method from the bring-up notes, not `idf.py monitor`.

## 5. Note added to §7 (new paragraph after item 5, before §8)

*Note, recorded after the 2026-09-10 bench pass:* item 2 as originally written asked the serial log to show the request going to `api.github.com/repos/matthewclaude/bedknob-for-somnus/…`; that is unobservable on this hardware. The OTA client never logs the URL or the repo name it queried, only the outcome, and both repos are reached at the same host, `api.github.com`, so no serial capture can distinguish them by hostname — which is why item 2 is now the Beta-builds-off 404 check.

## 6. Diff and commit

`git diff --stat` for the spec edit alone, before the report and index line were staged:

```
 docs/SPEC-repo-consolidation.md | 8 +++++---
 1 file changed, 5 insertions(+), 3 deletions(-)
```

`git diff --cached --stat` for the whole commit: the spec at `8 ++--` (5 insertions, 3 deletions) as above, `docs/REPORTS.md` at `1 +`, and this report added in full. The report's own line count is not quoted here: any figure written into this file changes the file, so it cannot be exact by construction. `git show --stat HEAD` gives it.

Commit: this report is part of the commit it describes, so the resulting SHA cannot be written inside it (any edit to embed it would change the SHA). The commit's parent is `fb38c8308369dd3ee3a9af3f1b2cc540483c8351`; the commit SHA is given in the session's closing chat line, and `git log -1 -- docs/REPORT-spec-gate-reword.md` recovers it. Not pushed.

## 7. Deviations from the instructions

- The commit SHA is not recorded in this report, for the reason in §6 above. The instructions asked for the SHA in the report and for the report to be in the same commit; both cannot hold, so the same-commit instruction was kept and the SHA is given in chat.
- `docs/REPORTS.md`'s preamble says a report "is committed in the next docs commit rather than in the commit it describes". The instructions said the same commit; the instructions were followed. The preamble was not edited (it is a file under `docs/` other than the three the task names).
- Placement choice, not a change of scope: the "unobservable as originally written" note sits as a short paragraph after §7 item 5, immediately before §8, rather than inside the numbered list, so the list items stay single-purpose. It is one paragraph of two sentences.

Otherwise, no deviations. Nothing under `firmware/`, `.github/` or `web-flasher/` was touched; no other file under `docs/` was touched; §1 and §4(b) are unchanged; no version bump, no tag, no push.

## 8. Related statements left as-is (flagged for the owner, not changed)

These were outside the task's two edits and were deliberately not touched:

- §3.1, last sentence (line 70): "the hardware gate (§7) keeps Beta builds **on** so the gate actually exercises the new repo rather than a 404 that looks like success." With item 2 now being the Beta-builds-off 404 check, that sentence now describes the opposite of what §7 asks. It is the same 404 the sentence calls a false positive, turned into the discriminator; the reasoning that made it a false positive (both channels say "up to date" on screen) is exactly why item 3 still demands the serial capture.
- §7 item 1 still asks the serial log to show the tags request going to `api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50`. The same limitation applies: the OTA client does not log its URL. Item 1's observable evidence is the Release fetch for `somnus-v1.0.1-beta.1` succeeding and the dial rebooting into `1.0.1-beta.1`, since only the old repo could have served that to a `1.0.0` dial.
- §1 line 18 ("This spec is the 'spec on disk before code' step. It changes no code and no workflow.") and §1 line 11 ("holds no Releases, serves no Pages site, and has no `gh-pages` branch") are now historical. The task said to leave §1 alone because its statements are dated 2026-09-10; they were left alone.

## 9. Not verifiable without hardware

- That a dial on `1.0.1-beta.1` with Beta builds off actually logs `ota: no releases published yet (HTTP 404)` against `bedknob-for-somnus`. Taken from the task's background (stated as observed on the bench 2026-09-10); the log string and the 404 → `OTA_IDLE` mapping were confirmed in `dial_ota.c`, the bench result was not re-run here.
- That the same check pointed at `somnus-dial-releases` reports the `1.0.0` stable Release as latest. Inferred from `/releases/latest` semantics and the 2026-09-10 tag-push report (old repo's `/releases/latest` = `somnus-v1.0.0`), not observed on a dial.
- That the 404 outcome renders as "up to date" on the display. Item 3 cites §3.1 for this; it was not observed on hardware in this session.
