# Release report — 0.1.5-beta.3

**Verdict: RELEASED.** Release commit `1aafca5` ("release: 0.1.5-beta.3")
made, changing exactly `firmware/dial-idf/CMakeLists.txt` and
`CHANGELOG.md`. No tag created, no push performed (owner's job, per
instructions).

This release went through two gate runs — the first failed and was
correctly stopped; the blocker was cleared by an explicit follow-up
instruction (a docs-only commit), and the gate was re-run and re-verified
before proceeding. Both runs are recorded below in full, since this file
replaces the earlier stop-only report.

## Gate run #1 (initial — STOPPED)

```
$ git log --oneline -1
e31761a night face: restore menu dot at dawn, WATER word above numeral, snap on drag start (spec rev 3, commit 4)

$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.5-beta.2")

$ git tag --list 'somnus-v*' | sort -V | tail -1
somnus-v0.1.5-beta.2

$ git --no-optional-locks status --short | grep -v '^??'
 M docs/SPEC-ota-readiness.md
 M docs/SPEC-power-sensing.md
```

Result: 3 of 4 checks passed. The tracked-status check failed — two
tracked spec docs (`docs/SPEC-ota-readiness.md`, `docs/SPEC-power-sensing.md`)
had unstaged modifications. Untracked (`??`) entries present at the time
(`Claude outputs/`, `docs/REPORT-night-face.md`, `docs/SPEC-night-face.md`,
`docs/SPEC-voice.md`) were correctly excluded by the grep and were not the
problem. Per instructions, work stopped and a gate-only report was written.

## Interim: blocker cleared (owner-directed, outside the release commit)

The owner directed clearing the blocker with a separate docs commit:

```
$ git add docs/SPEC-ota-readiness.md docs/SPEC-power-sensing.md docs/SPEC-night-face.md docs/SPEC-voice.md
$ git commit -m "docs: night face spec (built, verified), voice analysis, power sensing §9-10, OTA downgrade note §9.8"
[firmware/somnus-port 4d85d79] docs: night face spec (built, verified), voice analysis, power sensing §9-10, OTA downgrade note §9.8
 4 files changed, 408 insertions(+), 1 deletion(-)
 create mode 100644 docs/SPEC-night-face.md
 create mode 100644 docs/SPEC-voice.md

$ git --no-optional-locks status --short | grep -v '^??'
(nothing — grep exit 1)
```

This moved HEAD from `e31761a` to `4d85d79`, which no longer literally
matches the gate's `git log --oneline -1` pin. Per instructions to STOP on
any gate mismatch, this was flagged rather than silently overridden. The
owner confirmed proceeding, with the substitute check that `4d85d79`'s
parent is `e31761a` and that `4d85d79` touches only `docs/`:

```
$ git log --oneline -2
4d85d79 docs: night face spec (built, verified), voice analysis, power sensing §9-10, OTA downgrade note §9.8
e31761a night face: restore menu dot at dawn, WATER word above numeral, snap on drag start (spec rev 3, commit 4)

$ git diff --stat e31761a..4d85d79
 docs/SPEC-night-face.md    |  90 ++++++++++++++++++++++++
 docs/SPEC-ota-readiness.md |  26 +++++++
 docs/SPEC-power-sensing.md | 124 ++++++++++++++++++++++++++++++++-
 docs/SPEC-voice.md         | 169 +++++++++++++++++++++++++++++++++++++++++++++
 4 files changed, 408 insertions(+), 1 deletion(-)
```

Confirmed: nothing under `firmware/dial-idf` changed in `4d85d79`.

## Gate run #2 (re-verified — PASSED, with the owner-approved HEAD substitution)

```
$ git log --oneline -1
4d85d79 docs: night face spec (built, verified), voice analysis, power sensing §9-10, OTA downgrade note §9.8

$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.5-beta.2")

$ git tag --list 'somnus-v*' | sort -V | tail -1
somnus-v0.1.5-beta.2

$ git --no-optional-locks status --short | grep -v '^??'
(nothing — grep exit 1)
```

All four checks pass under the substitution above (HEAD's parent is the
required `e31761a`, and the commit on top is docs-only).

## Release commit

1. `firmware/dial-idf/CMakeLists.txt`: `PROJECT_VER` `"0.1.5-beta.2"` →
   `"0.1.5-beta.3"`.
2. `CHANGELOG.md`: inserted the 0.1.5-beta.3 section verbatim, directly
   above `## 0.1.5-beta.2`.

## Build (raw tail)

```
[100%] Linking CXX executable somnus-dial.elf
[100%] Built target somnus-dial.elf
[100%] Generating binary image from built executable
esptool v5.3.1
Creating ESP32-S3 image...
Merged 2 ELF sections.
Successfully created ESP32-S3 image.
Generated /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/somnus-dial.bin
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x185a50 bytes. Smallest app partition is 0x400000 bytes. 0x27a5b0 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete. To flash, run:
 idf.py flash
or
 idf.py -p PORT flash
```

Build succeeded. App size: `0x185a50` bytes used, `0x27a5b0` bytes (62%)
free of the `0x400000`-byte app partition.

(Note: `idf.py` was not on PATH in this shell; had to
`source ~/esp/esp-idf/export.sh` first. Not a code issue, just an
environment note.)

## Binary version-string check

```
$ strings firmware/dial-idf/build/somnus-dial.bin | grep -m1 '0\.1\.5-beta\.3'
0.1.5-beta.3
```

Confirmed: the built binary carries the correct version string.

## Release commit diff/SHA

```
$ git diff --stat -- firmware/dial-idf/CMakeLists.txt CHANGELOG.md
 CHANGELOG.md                     | 17 +++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 2 files changed, 18 insertions(+), 1 deletion(-)
```

Commit: `1aafca5` — "release: 0.1.5-beta.3"

```
commit 1aafca53ae6c7b281abe4370b59063f6fc527abf
Author: Matthew Montgomery <mattseattle@icloud.com>
    release: 0.1.5-beta.3

    Co-Authored-By: Claude Sonnet 5 <noreply@anthropic.com>
    Claude-Session: https://claude.ai/code/session_01TU2gDrnsuxoqou6aaDa34R

 CHANGELOG.md                     | 17 +++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 2 files changed, 18 insertions(+), 1 deletion(-)
```

Exactly the two authorized files. No tag created. No push performed.

## Deviations

- Gate's `git log --oneline -1` pin (`e31761a`) no longer matched at
  release time because an owner-directed docs-only commit (`4d85d79`) was
  made on top of it, between the first (stopped) gate run and the second.
  This was flagged and explicitly approved by the owner before proceeding
  (see "Interim" section above), with the substitute verification that
  `4d85d79`'s parent is `e31761a` and that `4d85d79` touches only `docs/`.
  The release commit itself (`1aafca5`) still touches exactly the two
  authorized files, on top of `4d85d79`.
- Everything else: none.

## What could not be verified without hardware

- The OTA pull itself (a device on the beta channel actually discovering,
  downloading, and successfully applying this release) was not exercised
  or verifiable from this environment — that requires physical hardware
  running the beta-channel update check.
