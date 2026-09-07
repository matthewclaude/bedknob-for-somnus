# REPORT — Release somnus-v0.1.6-beta.3 (2026-09-06)

## 1. Verdict

**RELEASED.** Release commit `53fc0c9dad016dac148a9887d91773b2a835c51c`, annotated tag `somnus-v0.1.6-beta.3` on it, both pushed to the remote named `somnus` only. Release workflow run 34037342861 concluded **success** (build-and-release 5m14s, deploy-pages 8s). GitHub release is a prerelease, not a draft, with both firmware assets attached. No code changes in this task.

## 2. Gate check (raw)

```
$ git rev-parse --abbrev-ref HEAD
main
$ git --no-optional-locks status --short
?? docs/REPORT-layout-tier-bc.md
$ git log --oneline -3
fe7c556 fix(discovery): mute per-host HTTP-stack errors during a scan; stale comments
01c6f3a ui: layout fixes B1-B7, C1, C2 from the screen audit
f46a779 docs: release report 0.1.6-beta.2
$ git tag --list 'somnus-v*' --sort=-v:refname | head -1
somnus-v0.1.6-beta.2
$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.6-beta.2")
$ git remote -v
origin	https://github.com/chris023/orion-waveshare-rotary-dial.git (fetch)
origin	no_push (push)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (fetch)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (push)
```

All checks agree. The only untracked file was `docs/REPORT-layout-tier-bc.md`; `docs/SPEC-standby-poll.md` does not exist on disk (`ls: docs/SPEC-standby-poll.md: No such file or directory`). Gate passed.

## 3. Regenerated screenshots

Simulator rebuilt (`cmake --build build`, exit 0, zero warnings) and run once (`./build/dial_sim`, exit 0, "done: 49 screens rendered to docs/screens").

```
$ git --no-optional-locks status --short docs/screens
 M docs/screens/adjust-mode.png
 M docs/screens/connecting.png
 M docs/screens/netpick.png
 M docs/screens/night-mode.png
 M docs/screens/pad-degraded-real.png
 M docs/screens/pad-unreachable.png
 M docs/screens/standby-update.png
 M docs/screens/standby.png
 M docs/screens/update-failed.png
 M docs/screens/update.png
 M docs/screens/wifi-confirm.png
```

Eleven PNGs changed. That is exactly the expected set (update, update-failed, netpick, adjust-mode, night-mode, wifi-confirm, standby, standby-update, connecting, pad-unreachable, pad-degraded-real), every one a screen touched by 01c6f3a. **No unexpected PNG changed.** The other 38 renders are byte-identical to the checked-in files.

## 4. CHANGELOG.md entry added

Inserted directly above the `## 0.1.6-beta.2 — 2026-09-06 (beta)` heading, same heading form (`## <version> — <date> (beta)`, `### Fixed` subsection, bold-lead prose bullets). Verified against release.yml's awk extractor (`$0 == "## " ver || index($0, "## " ver " ") == 1`), which matched: the "Extract release notes from CHANGELOG.md" step passed.

```markdown
## 0.1.6-beta.3 — 2026-09-06 (beta)

### Fixed

- **Screen layout audit, Tier B.** The Update row's "tap to install" text no
  longer overflows the row with a long version number; network picker rows
  are sized to the row width and ellipsize long network names instead of
  running off both ends; the wrong-password message is shortened so it
  fits on one line whole; the Adjust mode, Night mode and Wi-Fi confirm
  screens have their vertical spacing corrected (the Back pill no longer
  touches the bezel, the Night mode note clears the row under it, the
  Wi-Fi confirm text clears the Continue button); and the standby clock
  block is recentred on the screen.
- **Screen layout audit, Tier C.** The Update row is rebuilt as a flex block
  (the same approach the About screen rows use) so its lines centre in the
  row in every state. The Connecting/error screen's offsets are fixed so
  multi-line degraded text no longer overlaps the headline, and its colours
  now come from the palette — the background matches the chassis colour
  instead of pure black, so booting no longer flashes from black into the
  dial face, and the text follows the night palette.
- Pad discovery no longer floods the serial log with per-host connection
  errors during a subnet scan.
- Stale comments corrected (screen timeout choices, auto-update window,
  simulator update scenario). No behaviour change.
```

## 5. Version-string carriers

```
$ grep -rn '0.1.6-beta.2' --exclude-dir=build --exclude-dir=.git .
firmware/dial-idf/CMakeLists.txt:22:set(PROJECT_VER "0.1.6-beta.2")
docs/SPEC-standby-face.md:3:... Ships as `0.1.6-beta.2`. ...
CHANGELOG.md                       (the beta.2 entry heading)
docs/REPORT-release-0.1.6-beta.2.md
docs/REPORT-layout-tier-bc.md
```

The beta.2 release commit (`ee42066`) touched only `CHANGELOG.md` and `firmware/dial-idf/CMakeLists.txt`. So the one carrier bumped is:

- `firmware/dial-idf/CMakeLists.txt:22` — `set(PROJECT_VER "0.1.6-beta.2")` → `set(PROJECT_VER "0.1.6-beta.3")`

The `docs/SPEC-standby-face.md` mention is historical ("Ships as 0.1.6-beta.2" — that feature did ship in beta.2) and the two reports record beta.2 as history; none is a version carrier and none was edited. `CHANGELOG-orion.md` untouched.

## 6. Release commit and tag

Release commit: `53fc0c9dad016dac148a9887d91773b2a835c51c`

```
53fc0c9dad016dac148a9887d91773b2a835c51c Release somnus-v0.1.6-beta.3

 CHANGELOG.md                       |  25 ++
 docs/REPORT-layout-tier-bc.md      | 524 +++++++++++++++++++++++++++++++++++++
 docs/screens/adjust-mode.png       | Bin 25054 -> 25019 bytes
 docs/screens/connecting.png        | Bin 13891 -> 14039 bytes
 docs/screens/netpick.png           | Bin 18724 -> 18723 bytes
 docs/screens/night-mode.png        | Bin 17945 -> 17916 bytes
 docs/screens/pad-degraded-real.png | Bin 19447 -> 19561 bytes
 docs/screens/pad-unreachable.png   | Bin 17237 -> 17244 bytes
 docs/screens/standby-update.png    | Bin 20143 -> 20168 bytes
 docs/screens/standby.png           | Bin 19113 -> 19138 bytes
 docs/screens/update-failed.png     | Bin 21741 -> 21750 bytes
 docs/screens/update.png            | Bin 23727 -> 23110 bytes
 docs/screens/wifi-confirm.png      | Bin 22447 -> 22426 bytes
 firmware/dial-idf/CMakeLists.txt   |   2 +-
 14 files changed, 550 insertions(+), 1 deletion(-)
```

Tag: `somnus-v0.1.6-beta.3`, **annotated** (`git cat-file -t` → `tag`), same as beta.2 (`git cat-file -t somnus-v0.1.6-beta.2` → `tag`; its message is the bare tag name, and so is this one's). Target: `git rev-parse somnus-v0.1.6-beta.3^{commit}` → `53fc0c9dad016dac148a9887d91773b2a835c51c`.

## 7. Push output (raw)

```
$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   f46a779..53fc0c9  main -> main
exit=0

$ git push somnus somnus-v0.1.6-beta.3
To github.com:matthewclaude/bedknob-for-somnus.git
 * [new tag]         somnus-v0.1.6-beta.3 -> somnus-v0.1.6-beta.3
exit=0
```

Both went to `github.com:matthewclaude/bedknob-for-somnus.git` (remote `somnus`). Nothing was pushed to `origin` (its push URL is `no_push`).

## 8. CI

Tag-triggered release workflow on `matthewclaude/bedknob-for-somnus`:

| Run | Workflow | Trigger | Conclusion |
|---|---|---|---|
| **34037342861** | release | push of tag `somnus-v0.1.6-beta.3` | **success** — build-and-release 5m14s (job 101497710226), deploy-pages 8s (job 101498421571) |
| 34037342472 | ci | push of `main` (53fc0c9) | success |

`gh run watch 34037342861 --exit-status` returned 0; every step green (Verify tag matches PROJECT_VER, Extract release notes from CHANGELOG.md, Classify release channel, Build firmware and merge flashable image, Upload merged image, Publish GitHub Release, then Download merged image / Channel directory / Assemble Pages site / Deploy to gh-pages). One annotation, informational only: "Node.js 20 is deprecated … forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2".

```
$ gh release view somnus-v0.1.6-beta.3 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,isDraft,assets
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/547256069","contentType":"application/octet-stream","createdAt":"2026-09-06T13:56:13Z","digest":"sha256:4c830d5ee8fdfc05bfab9e1846955be8f70f615e54b8bf7b90e769b5abafbf0d","downloadCount":0,"id":"RA_kwDOULeAcc4gnncF","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-06T13:56:14Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.3/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/547256070","contentType":"application/octet-stream","createdAt":"2026-09-06T13:56:13Z","digest":"sha256:ca8d3f5f10da43582d8d767ae624460f6c42674d626d6df735723fd5a75f4239","downloadCount":0,"id":"RA_kwDOULeAcc4gnncG","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-06T13:56:14Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.3/somnus-dial.bin"}],"isDraft":false,"isPrerelease":true,"tagName":"somnus-v0.1.6-beta.3"}
```

isPrerelease **true**, isDraft **false**, assets `somnus-dial-merged.bin` (1,743,152 bytes) and `somnus-dial.bin` (1,612,080 bytes), both `state: uploaded`.

## 9. Post-push verification (raw)

```
$ git log somnus/main..HEAD
(empty)

$ git --no-optional-locks status --short
(empty at the time of the check; after this report is written it shows only)
?? docs/REPORT-release-0.1.6-beta.3.md
```

## 10. Deviations

Deviations: none.

Two notes that are not deviations: `gh run list` with no `--repo` defaulted to the `origin` remote (chris023 upstream) and listed that repo's runs; it was re-run with `--repo matthewclaude/bedknob-for-somnus`, which is where the somnus runs are and what §8 reports. `docs/SPEC-standby-poll.md` was conditional ("if it exists") and does not exist, so it is not in the release commit.

## 11. Not verified here

The OTA pull onto the bench dial. Owner: Menu → Update → Beta builds On → Check for updates; expect "latest 0.1.6-beta.3, running 0.1.6-beta.2 — update available", install, then About shows 0.1.6-beta.3. Also the on-device look of the eleven regenerated screens under the physical bezel (listed in docs/REPORT-layout-tier-bc.md §8) and a real subnet scan's serial line count.
