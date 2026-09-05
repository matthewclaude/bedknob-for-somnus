# REPORT: manual OTA check — serial capture

**Top-line finding: the OTA check DID fire during the window. Eight `ota:` lines were captured, every one reading `latest 0.1.4, running 0.1.4 -- up to date`. The device reports itself as running 0.1.4 (not the 0.1.5-beta.5 in CMakeLists.txt), and the server-side "latest" it resolved is 0.1.4 — the beta.5 prerelease published earlier was not surfaced to this client.**

Date: 2026-09-05 00:41–00:42 UTC (2026-09-04 evening local)
Read-only capture. No reflash, no reset, no source edits.

## Device

```
$ ls /dev/cu.usbmodem*
/dev/cu.usbmodem83401
```

Serial device used: `/dev/cu.usbmodem83401`

## Command sequence (exactly as run)

```
DEV=/dev/cu.usbmodem83401
stty -f $DEV 115200 raw
( cat $DEV & echo $! > /tmp/serial_cap.pid ) > /tmp/ota_check_capture.log 2>&1
echo "capture started $(date -u +%T) pid=$(cat /tmp/serial_cap.pid)"
sleep 45
kill $(cat /tmp/serial_cap.pid); echo "kill exit=$?"
echo "capture stopped $(date -u +%T)"
```

Note: this shell blocks a foreground `sleep`, so the stty/cat/sleep/kill sequence above was
run as a single background job. The "capture started, tap now" message was sent as soon as
the job launched. The kill and the 45 s window are otherwise identical to the requested
steps. `idf.py monitor` was NOT used (it would have reset the chip).

Job output:

```
capture started 00:41:54 pid=11780
kill exit=0
capture stopped 00:42:39
bytes captured:      524
```

Extraction:

```
grep -in "ota" /tmp/ota_check_capture.log
```

Cross-reference (read-only):

```
$ grep -n "PROJECT_VER" firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.5-beta.5")
```

## All "ota"-matching lines, verbatim, in order

(`grep -in "ota"`, prefix is the line number within `/tmp/ota_check_capture.log`; CRs stripped)

```
2:I (729460) ota: latest 0.1.4, running 0.1.4 -- up to date
3:I (732626) ota: latest 0.1.4, running 0.1.4 -- up to date
4:I (735766) ota: latest 0.1.4, running 0.1.4 -- up to date
5:I (739135) ota: latest 0.1.4, running 0.1.4 -- up to date
6:I (742516) ota: latest 0.1.4, running 0.1.4 -- up to date
7:I (745889) ota: latest 0.1.4, running 0.1.4 -- up to date
8:I (749839) ota: latest 0.1.4, running 0.1.4 -- up to date
9:I (755163) ota: latest 0.1.4, running 0.1.4 -- up to date
```

8 of the 9 captured lines match. The only non-matching line is line 1:

```
1:I (722822) app: side A: on=0 set=29.3C water=24.5C
```

The eight checks are spaced ~3–5 s apart (729460 → 755163 ms), consistent with repeated
taps on "Check for updates" during the window.

## Current PROJECT_VER

`firmware/dial-idf/CMakeLists.txt` line 22: `0.1.5-beta.5`

## Observations (facts from the capture only, no code inspected)

- The connected unit reports `running 0.1.4`. It is therefore not running the beta.5 build
  that PROJECT_VER currently names (or any beta), regardless of what was flashed to other
  units in earlier sessions.
- The OTA client resolved `latest 0.1.4`. At capture time the public releases repo held
  `somnus-v0.1.5-beta.5` as a prerelease (published 2026-09-05T00:14:40Z, see
  `docs/REPORT-beta5-ci-check.md`). GitHub's `releases/latest` endpoint excludes
  prereleases by definition, which matches what the device printed. Whether this unit is
  meant to be on the beta channel, and how the client selects a channel, was not examined
  here (read-only task).
- No error, HTTP-failure, or download lines appeared; every check completed as "up to date".

## Addendum -- root cause and SPEC update

Added 2026-09-05. `docs/SPEC-ota-readiness.md` gained a new `### 9.9 Stable channel never
received the beta-discovery fix`, appended after §9.8 (previously the last section). No
other file and no code was touched.

Every fact in §9.9 was independently re-verified against the repo before writing, not
copied from the request. What was checked, and how:

| Claim | Check | Result |
|---|---|---|
| `somnus-v0.1.4` -> `56aded3` | `git rev-parse somnus-v0.1.4` | `56aded3`, subject "release: 0.1.4 — v1 new-user path verified on hardware" |
| Tag date 2026-09-03 11:06:28 -0500 | `git log -1 --format='%ai / %ci' 56aded3` | **Qualified:** 11:06:28 is the *author* date; the *commit* date is 11:10:12 -0500. Both recorded in §9.9. |
| Fix is commit `819f102` | `git show --stat 819f102` | 2026-09-03 15:10:39 -0500, "ota: beta channel picks the newest tag, then fetches that release; the release list order is not newest-first"; touches only `dial_ota.c`/`dial_ota.h`; diff removes `releases?per_page=5` + `RELEASES_LIST_SCAN_CAP 5`, adds `tags?per_page=50` + `releases/tags/%s` |
| `819f102` is after the 0.1.4 tag | `git merge-base --is-ancestor 819f102 somnus-v0.1.4` | exits non-zero (not an ancestor); timestamps 15:10 vs 11:10 same day |
| Old path still in 0.1.4 code | `git show somnus-v0.1.4:.../dial_ota.c \| grep per_page` | line 44 `releases?per_page=5`, line 61 `RELEASES_LIST_SCAN_CAP 5` |
| Fix exists only in beta.2–beta.5, no stable | `git tag --contains 819f102`; `--is-ancestor` against every `somnus-v*` tag | contains: beta.2, beta.3, beta.4, beta.5 only. Lacking: v0.1.0–v0.1.4 and beta.1 |
| Flasher's unchecked option installs stable from `firmware/latest/` | `web-flasher/index.html`, `web-flasher/manifest.json`, `release.yml` channel-dir step | checkbox label is "Install beta build instead" (**wording corrected** from "Install beta build"); default manifest -> `firmware/latest/somnus-dial-merged.bin`; non-beta tags deploy to `firmware/latest`, beta tags to `firmware/beta` |
| Five stable releases fill the per_page=5 window | public releases API listing (this report, and `REPORT-beta5-ci-check.md`) | stable: 0.1.0, 0.1.1, 0.1.2, 0.1.3, 0.1.4 = 5; all betas listed after them |
| Unit logged `latest 0.1.4, running 0.1.4 -- up to date` on 2026-09-05 | this report, lines 2–9 of the capture | 8 occurrences, verbatim above |
| "with Beta builds toggled on" | `dial_ota.c` @ v0.1.4, lines 272–340 | **Qualified:** both channels fall through to the same `ESP_LOGI("latest %s, running %s -- up to date")`. The capture cannot show which channel was active, and the toggle state was not recorded during capture. §9.9 states the code consequence for toggle-on and flags the capture as consistent-with, not proof-of, the toggle position. |

Corrections/qualifications made relative to the requested text: (1) author vs commit
timestamp distinction on the 0.1.4 tag; (2) flasher checkbox label wording; (3) the
toggle-on claim is stated as code behaviour, with the capture's inability to distinguish
channels noted explicitly. Everything else checked out as described.

The stable-vs-fix decision (§9.9 last paragraph) is left open in the SPEC, as instructed.
