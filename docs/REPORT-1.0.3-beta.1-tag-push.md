# REPORT: 1.0.3-beta.1 publish verification, six version-bearing screens, REPORTS.md index

**Verdict: DONE. All four gates passed. Part 1 found no mismatch: the Release is a published prerelease with the two expected assets, its body is the CHANGELOG section plus the workflow footer, `/releases/latest` is still `somnus-v1.0.2`, the release and ci runs for `5a6e692` both succeeded, and both Pages images match their Release digests and embed the right version. Part 2: exactly the six version-bearing screens changed, and "v1.0.3-beta.1" fits on all of them. No findings.**

Date: 2026-09-30. Repo: `~/Projects/somnus-waveshare-rotary-dial`, branch `main`. All `gh` commands ran with `--repo matthewclaude/bedknob-for-somnus` (left out of the quoted command lines below for width).

## Gate

| # | Check | Expected | Actual | Result |
|---|---|---|---|---|
| 1 | `sed -n 21p firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.3-beta.1")` | `set(PROJECT_VER "1.0.3-beta.1")` | PASS |
| 2 | `git rev-parse somnus-v1.0.3-beta.1^{commit}` | starts `5a6e692` | `5a6e6923120416e9d0adf5d154c47febce8f504b` | PASS |
| 3 | `git rev-parse HEAD` and `git ls-remote --heads somnus main` | starts `63aaed8`, same full SHA on both | `63aaed84459a178d35ca9514e31b492c5d287251` and `63aaed84459a178d35ca9514e31b492c5d287251	refs/heads/main` | PASS |
| 4 | `git --no-optional-locks status --short --untracked-files=no` | no output | no output, exit 0 | PASS |

Raw:

```
$ sed -n 21p firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.3-beta.1")
$ git rev-parse somnus-v1.0.3-beta.1^{commit}
5a6e6923120416e9d0adf5d154c47febce8f504b
$ git rev-parse HEAD
63aaed84459a178d35ca9514e31b492c5d287251
$ git ls-remote --heads somnus main
63aaed84459a178d35ca9514e31b492c5d287251	refs/heads/main
$ git --no-optional-locks status --short --untracked-files=no
[exit 0]
```

## Part 1: publish verification

### a. The Release object

```
$ gh release view somnus-v1.0.3-beta.1 --json tagName,isDraft,isPrerelease,publishedAt,body,assets
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/601387909","contentType":"application/octet-stream","createdAt":"2026-09-30T16:51:51Z","digest":"sha256:df3aafefebf66e2122efd7fe1dd970b32b5c8726f7b9e16ef722eccfefccf51f","downloadCount":0,"id":"RA_kwDOUCLPIM4j2HOF","label":"","name":"somnus-dial-merged.bin","size":1742512,"state":"uploaded","updatedAt":"2026-09-30T16:51:52Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.3-beta.1/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/601387912","contentType":"application/octet-stream","createdAt":"2026-09-30T16:51:51Z","digest":"sha256:31132e49d8e9abcd461861e76f4a564268e139b73b2e665db264cdfcfab701e1","downloadCount":1,"id":"RA_kwDOUCLPIM4j2HOI","label":"","name":"somnus-dial.bin","size":1611440,"state":"uploaded","updatedAt":"2026-09-30T16:51:52Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.3-beta.1/somnus-dial.bin"}],"body":"A new dial in Dual Sides mode no longer stops to ask \"Which side of the\nbed?\" after it first reaches the pad. It opens on the right side, as it\nalready did for anyone who never saw that question, and one swipe shows\nthe left. The dial still remembers the last side you looked at.\n\nAlso fixed: switching an existing dial from One Bed to Dual Sides and then\nswiping to the other side could change Scale from Relative to Absolute on\nthe next restart, even though you never changed it. Scale now stays where\nyou left it.\n\nInternal: SCR_SIDEPICK and the side_picked flag are removed; the \"relmode\"\nkey is now seeded alongside the first \"zone\" write in\ndial_state_set_ui_zone instead of by the side picker.\n\nDials already running the firmware pick this up on their own — see Menu → Update.\nFor a first install, use the [browser flasher](https://matthewclaude.github.io/bedknob-for-somnus/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.\n\nRequired Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)","isDraft":false,"isPrerelease":true,"publishedAt":"2026-09-30T16:51:51Z","tagName":"somnus-v1.0.3-beta.1"}
```

| Check | Expected | Actual | Result |
|---|---|---|---|
| tagName | `somnus-v1.0.3-beta.1` | `somnus-v1.0.3-beta.1` | PASS |
| isDraft | false | false | PASS |
| isPrerelease | true | true | PASS |
| publishedAt | (recorded) | 2026-09-30T16:51:51Z | — |
| asset count and names | 2: `somnus-dial.bin`, `somnus-dial-merged.bin` | 2: the same two | PASS |
| `somnus-dial-merged.bin` | (recorded) | 1742512 B, `sha256:df3aafefebf66e2122efd7fe1dd970b32b5c8726f7b9e16ef722eccfefccf51f`, downloadCount 0 | — |
| `somnus-dial.bin` | (recorded) | 1611440 B, `sha256:31132e49d8e9abcd461861e76f4a564268e139b73b2e665db264cdfcfab701e1`, downloadCount 1 | — |
| body vs CHANGELOG | CHANGELOG `## 1.0.3-beta.1` section, plus the workflow footer | matches | PASS |

Body comparison. I pulled the `## 1.0.3-beta.1 — 2026-09-30 (beta)` section out of CHANGELOG.md with the same awk range and blank-line trim that `release.yml` uses, saved the body with `gh release view somnus-v1.0.3-beta.1 --json body --jq .body`, and diffed the two:

```
$ diff changelog-section release-body
13a14,18
> 
> Dials already running the firmware pick this up on their own — see Menu → Update.
> For a first install, use the [browser flasher](https://matthewclaude.github.io/bedknob-for-somnus/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
> 
> Required Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)
[diff exit 1]
```

All 13 CHANGELOG lines match the body exactly. The only difference is the five lines the workflow adds after them: a blank line, the two-line install footer, another blank line, and the Required Notice.

### b. Latest stable

```
$ gh api repos/matthewclaude/bedknob-for-somnus/releases/latest --jq .tag_name
somnus-v1.0.2
```

Expected `somnus-v1.0.2`, got `somnus-v1.0.2`. PASS. The prerelease did not become "latest".

### c. Workflow runs

```
$ gh run list --limit 6 --json databaseId,name,headBranch,event,conclusion,createdAt
[{"conclusion":"success","createdAt":"2026-09-30T17:02:30Z","databaseId":36748562272,"event":"push","headBranch":"main","name":"ci"},{"conclusion":"success","createdAt":"2026-09-30T16:52:07Z","databaseId":36747327782,"event":"dynamic","headBranch":"gh-pages","name":"pages build and deployment"},{"conclusion":"success","createdAt":"2026-09-30T16:48:42Z","databaseId":36746919219,"event":"push","headBranch":"somnus-v1.0.3-beta.1","name":"release"},{"conclusion":"success","createdAt":"2026-09-30T16:48:40Z","databaseId":36746916175,"event":"push","headBranch":"main","name":"ci"},{"conclusion":"success","createdAt":"2026-09-30T16:11:11Z","databaseId":36742349944,"event":"push","headBranch":"main","name":"ci"},{"conclusion":"success","createdAt":"2026-09-17T23:22:32Z","databaseId":35286550160,"event":"push","headBranch":"main","name":"ci"}]
```

`ci` only runs on pushes to `main`, not on tags, so `headBranch` alone cannot tie a ci run to the tag. I ran one more read-only listing to get each run's commit:

```
$ gh run list --limit 6 --json databaseId,name,headSha
36748562272 ci 63aaed84459a178d35ca9514e31b492c5d287251
36747327782 pages build and deployment d9c870bb0ff148b96e249430942cd38fc1f70e84
36746919219 release 5a6e6923120416e9d0adf5d154c47febce8f504b
36746916175 ci 5a6e6923120416e9d0adf5d154c47febce8f504b
36742349944 ci 042840b3dd557cd24369cec6322903d53dfb5aee
35286550160 ci 547231f9a6db4dda9abcc9f3bd5020e374abefb4
```

| Run | Commit | Trigger | Conclusion |
|---|---|---|---|
| release 36746919219 | `5a6e692` (the tag) | push of tag `somnus-v1.0.3-beta.1`, 16:48:42Z | success |
| ci 36746916175 | `5a6e692` (the tagged commit) | push of `main`, 16:48:40Z | success |
| pages build and deployment 36747327782 | gh-pages `d9c870b` | dynamic, 16:52:07Z, just after publishedAt | success |
| ci 36748562272 | `63aaed8` (HEAD, the T3 docs commit) | push of `main`, 17:02:30Z | success |

PASS. Tag push to `publishedAt` was 3 min 09 s (16:48:42Z → 16:51:51Z).

### d. Pages images

Both files went into `bench-logs/verify-1.0.3b1/` (gitignored: `git check-ignore -v` → `.gitignore:28:bench-logs/`). Each channel got its own subfolder because both files have the same name.

```
$ curl -fsSL -o bench-logs/verify-1.0.3b1/beta/somnus-dial-merged.bin https://matthewclaude.github.io/bedknob-for-somnus/firmware/beta/somnus-dial-merged.bin
[exit 0]
$ shasum -a 256 bench-logs/verify-1.0.3b1/beta/somnus-dial-merged.bin
df3aafefebf66e2122efd7fe1dd970b32b5c8726f7b9e16ef722eccfefccf51f  bench-logs/verify-1.0.3b1/beta/somnus-dial-merged.bin
$ strings bench-logs/verify-1.0.3b1/beta/somnus-dial-merged.bin | grep -E '^1\.0\.[0-9](-beta\.[0-9])?$'
1.0.3-beta.1
size 1742512
$ curl -fsSL -o bench-logs/verify-1.0.3b1/latest/somnus-dial-merged.bin https://matthewclaude.github.io/bedknob-for-somnus/firmware/latest/somnus-dial-merged.bin
[exit 0]
$ shasum -a 256 bench-logs/verify-1.0.3b1/latest/somnus-dial-merged.bin
e9f88689b9313cb33c1e933db2394bcc24b481e16bb2f69b071187cebfe13462  bench-logs/verify-1.0.3b1/latest/somnus-dial-merged.bin
$ strings bench-logs/verify-1.0.3b1/latest/somnus-dial-merged.bin | grep -E '^1\.0\.[0-9](-beta\.[0-9])?$'
1.0.2
size 1743392
```

(`size` lines come from `ls -l`.) The 1.0.2 merged digest comes from `gh release view somnus-v1.0.2 --json assets`, quoted in full under e.

| Channel | Expected SHA-256 | Actual SHA-256 | Expected string | Actual string | Result |
|---|---|---|---|---|---|
| beta | 1.0.3-beta.1 merged asset `df3aafef…cf51f` | `df3aafef…cf51f` (full value above; identical) | `1.0.3-beta.1` | `1.0.3-beta.1` only | PASS |
| latest | 1.0.2 merged asset `e9f88689…13462` | `e9f88689…13462` (full value above; identical) | `1.0.2` | `1.0.2` only | PASS |

The manifest files were not used as evidence.

### e. Download counts

```
$ gh release view somnus-v1.0.2 --json assets
somnus-dial-merged.bin size=1743392 sha256:e9f88689b9313cb33c1e933db2394bcc24b481e16bb2f69b071187cebfe13462 downloadCount=0
somnus-dial.bin size=1612320 sha256:58e10172386fb5b8942789b25d05256b73fc68af397adcd7258f4a550f15cff9 downloadCount=2
$ gh release view somnus-v1.0.2-beta.1 --json assets
somnus-dial-merged.bin size=1743392 sha256:a0dac94c7df18ee247480883790ed07a6b8744807b5a545535b35e224b19405d downloadCount=0
somnus-dial.bin size=1612320 sha256:764f2228b8555a6b5591a722b959473ed97fd9b77898f24f0e67d3bcd6b973a0 downloadCount=1
$ gh release view somnus-v1.0.3-beta.1 --json assets
somnus-dial-merged.bin size=1742512 sha256:df3aafefebf66e2122efd7fe1dd970b32b5c8726f7b9e16ef722eccfefccf51f downloadCount=0
somnus-dial.bin size=1611440 sha256:31132e49d8e9abcd461861e76f4a564268e139b73b2e665db264cdfcfab701e1 downloadCount=1
```

(Each ran with `--jq '.assets[]|"\(.name) size=\(.size) \(.digest) downloadCount=\(.downloadCount)"'`.)

| Release | Asset | Owner's own | Actual | Above owner's |
|---|---|---|---|---|
| somnus-v1.0.2 | somnus-dial.bin | 2 | 2 | 0 |
| somnus-v1.0.2 | somnus-dial-merged.bin | 0 | 0 | 0 |
| somnus-v1.0.2-beta.1 | somnus-dial.bin | 1 | 1 | 0 |
| somnus-v1.0.2-beta.1 | somnus-dial-merged.bin | 0 | 0 | 0 |
| somnus-v1.0.3-beta.1 | somnus-dial.bin | 1 | 1 | 0 |
| somnus-v1.0.3-beta.1 | somnus-dial-merged.bin | 0 | 0 | 0 |

No count is above the owner's own.

Part 1 findings: none.

## Part 2: docs commit

### f. Simulator

Output below has the home prefix shown as `~`. The build step's output is cut to its last five lines (the full log is only LVGL compile lines).

```
$ cmake -B build -S simulator
-- dial_sim: firmware version 1.0.3-beta.1, OTA scenarios advertise 1.0.4
-- dial_sim: using local LVGL checkout at ~/Projects/somnus-waveshare-rotary-dial/simulator/../firmware/dial-idf/managed_components/lvgl__lvgl
CMake Warning (policy) at ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/lvgl__lvgl/env_support/cmake/custom.cmake:66 (install):
  Policy CMP0177 is not set: install() DESTINATION paths are normalized.  Run
  "cmake --help-policy CMP0177" for policy details.  Use the cmake_policy
  command to set the policy and suppress this warning.
Call Stack (most recent call first):
  ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/lvgl__lvgl/CMakeLists.txt:16 (include)
This warning is for project developers.  Use -Wno-author or -Wno-policy to
suppress it.

-- Configuring done (0.1s)
-- Generating done (0.1s)
-- Build files have been written to: ~/Projects/somnus-waveshare-rotary-dial/build
[exit 0]
$ cmake --build build
[100%] Building C object lvgl/CMakeFiles/lvgl_demos.dir/demos/widgets/assets/img_demo_widgets_avatar.c.o
[100%] Building C object lvgl/CMakeFiles/lvgl_demos.dir/demos/widgets/assets/img_lvgl_logo.c.o
[100%] Building C object lvgl/CMakeFiles/lvgl_demos.dir/demos/widgets/lv_demo_widgets.c.o
[100%] Linking C static library liblvgl_demos.a
[100%] Built target lvgl_demos
[exit 0]
$ ./build/dial_sim
I (ui_router) router up, screen 0
wrote welcome          ~/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      ~/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          ~/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (6 distinct colors sampled)
wrote passkey          ~/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote connecting       ~/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote dial-night-water ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-night-water.png (7 distinct colors sampled)
wrote rails-420-up     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=410
wrote rails-420-up-down ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up-down.png (10 distinct colors sampled)
wrote rails-423        ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=420
wrote rails-423-up     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423-up.png (10 distinct colors sampled)
wrote rails-drag-337-live ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337-live.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=340
wrote rails-drag-337   ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337.png (10 distinct colors sampled)
wrote menu             ~/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (3 distinct colors sampled)
wrote update           ~/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (7 distinct colors sampled)
wrote update-prompt    ~/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    ~/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (5 distinct colors sampled)
wrote settings         ~/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (4 distinct colors sampled)
wrote settings-pad     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (2 distinct colors sampled)
wrote settings-timezone-raw ~/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (2 distinct colors sampled)
wrote timezone         ~/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (5 distinct colors sampled)
wrote night-mode       ~/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (4 distinct colors sampled)
wrote night-face       ~/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (6 distinct colors sampled)
wrote standby-face     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-face.png (5 distinct colors sampled)
wrote settings-standby-face ~/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-standby-face.png (9 distinct colors sampled)
wrote pad-address      ~/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  ~/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (4 distinct colors sampled)
wrote pad-discovery    ~/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (3 distinct colors sampled)
wrote pad-degraded-real ~/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (4 distinct colors sampled)
wrote adjust-mode      ~/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (9 distinct colors sampled)
wrote brightness-menu  ~/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (5 distinct colors sampled)
wrote brightness       ~/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock ~/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off ~/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        ~/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (6 distinct colors sampled)
wrote wifi-confirm     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (6 distinct colors sampled)
wrote about            ~/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (4 distinct colors sampled)
wrote about-wifi-worst ~/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (5 distinct colors sampled)
wrote about-wifi-real  ~/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct ~/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb ~/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         ~/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          ~/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (4 distinct colors sampled)
wrote standby-update   ~/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (4 distinct colors sampled)
done: 48 screens rendered to ~/Projects/somnus-waveshare-rotary-dial/docs/screens
[exit 0]
```

The required configure line `dial_sim: firmware version 1.0.3-beta.1, OTA scenarios advertise 1.0.4` is there. The only warning is LVGL's own CMP0177 policy notice, which comes from the vendored LVGL CMake.

### g. docs/screens status

```
$ git --no-optional-locks status --short docs/screens
 M docs/screens/about-wifi-real.png
 M docs/screens/about-wifi-worst.png
 M docs/screens/about.png
 M docs/screens/update-failed.png
 M docs/screens/update-prompt.png
 M docs/screens/update.png
```

Exactly the six allowed PNGs changed. None were added or deleted and no other PNG changed. PASS, no restore needed.

### h. Per-screen fit (each PNG opened and looked at)

| Screen | Where the version appears | Fit |
|---|---|---|
| `about.png` | Firmware row, centred value `v1.0.3-beta.1` in the large font | Fits. It is centred with clear margin on both sides, on one line, and does not overlap the Firmware label above or the divider below. |
| `about-wifi-real.png` | Firmware row scrolled to the top edge: dimmed `v1.0.3-beta.1` under a faded "Firmware" label, above the ABOUT title | Fits. It is on one line inside the round mask at that height, with a visible gap above "ABOUT". |
| `about-wifi-worst.png` | Same top-edge Firmware row as about-wifi-real (the longest About layout) | Fits. It is on one line, not clipped by the circle, and does not overlap "ABOUT". The only truncation on this screen is the deliberate `ABCDEFGHIJKLMNO…` worst-case SSID ellipsis, which is what this screen exists to show and does not involve the version string. |
| `update.png` | Installed row, right-aligned value `v1.0.3-beta.1` | Fits. It ends inside the circle edge with a wide gap to the "Installed" label and stays on one line. |
| `update-failed.png` | Installed row (dimmed), right-aligned `v1.0.3-beta.1` | Fits, the same as update.png, with no overlap with the red "Update failed" lines above. |
| `update-prompt.png` | Does not show the installed version. The sheet shows the offered version `1.0.4`. | Not applicable. The only change from HEAD is that version label: a pixel diff against `git show HEAD:docs/screens/update-prompt.png` is confined to the box (163, 69)–(198, 81), where the old image read `1.0.3` and the new one reads `1.0.4`, because the OTA scenarios now advertise 1.0.4. It fits the same way. |

Screen findings: none.

### i. REPORTS.md

Four lines were added after the `REPORT-sidepick-bench.md` line, in this order (all dated 2026-09-30, in the order the work happened): `REPORT-sidepick-bench-flash.md` (**PARTIAL**), `REPORT-1.0.3-beta.1-commit.md` (**DONE**), `REPORT-sidepick-t3-flash.md` (**PASS**), `REPORT-1.0.3-beta.1-tag-push.md` (**DONE**). Each verdict word and summary comes from that report's own verdict line. No other line changed.

### j/k. Staged diff

```
$ git diff --cached --stat
 docs/REPORT-1.0.3-beta.1-tag-push.md | 311 +++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                      |   4 +
 docs/screens/about-wifi-real.png     | Bin 23180 -> 23596 bytes
 docs/screens/about-wifi-worst.png    | Bin 24148 -> 24564 bytes
 docs/screens/about.png               | Bin 18593 -> 19416 bytes
 docs/screens/update-failed.png       | Bin 21739 -> 22327 bytes
 docs/screens/update-prompt.png       | Bin 22145 -> 22170 bytes
 docs/screens/update.png              | Bin 23231 -> 23933 bytes
 8 files changed, 315 insertions(+)
```

## Deviations

- This report cannot carry its own commit SHA, because it is part of that commit. The same applies to its line in REPORTS.md.
- c: I ran one extra read-only `gh run list --limit 6 --json databaseId,name,headSha`. `ci` does not run on tag pushes, so `headBranch` alone could not identify the ci run for the tag. The extra listing ties ci 36746916175 to `5a6e692`.
- a: the body comparison used a scratch copy of the CHANGELOG section, cut out with the same awk range and trim as `release.yml`, and a `diff` against the saved body. Both files are in the session scratchpad, not the repo.
- d: the two downloads went into `beta/` and `latest/` subfolders of `bench-logs/verify-1.0.3b1/`, because both files are named `somnus-dial-merged.bin`. I added a `git check-ignore -v` check and an `ls -l` size line for each file.
- e: `gh release view … --json assets` ran with a `--jq` filter, so the output is one line per asset instead of raw JSON.
- f: the `cmake --build build` output is quoted as its last five lines only.
- h: I ran one extra read-only pixel diff of `update-prompt.png` against its HEAD version (via `git show HEAD:…` into the scratchpad). The screen shows no installed version, so the diff was needed to say what changed.
- Every git and strings call ran with `DEVELOPER_DIR=/Library/Developer/CommandLineTools` set, because of the unaccepted Xcode license on this Mac. No `git show` of a commit was needed, so `--format=` never came up.

## Not verifiable without hardware

- That a dial on 1.0.2 with Beta builds On is offered 1.0.3-beta.1. T3 already showed this once on Bedknob #1: an OTA from 1.0.2 to 1.0.3-beta.1 from ota_1, with rollback cancelled.
- A browser-flasher install of the beta image. This report only shows that the Pages beta image is byte-identical to the Release's merged asset and embeds 1.0.3-beta.1.
