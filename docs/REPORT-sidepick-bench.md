# REPORT: sidepick bench — SPEC-sidepick-deletion §7 on Bedknob #1

Date: 2026-09-30. Written from the Cowork session that ran the bench with the owner at the dial.

**Verdict: PASS (T1, T1-r1, T1-r2, T4, T4-r1, T4-r2, T2, T2-a, T2-r1, T2-r2, T2-z, Close), with the deviations listed below. T3 (OTA from 1.0.2) is still open and can only run after 1.0.3-beta.1 is published.**

## Setup
- Candidate: HEAD 042840b (includes fe5b139), wire-flashed with `idf.py flash` to /dev/cu.usbmodem83401. See docs/REPORT-sidepick-bench-flash.md for the gate, build (0 warnings, 0 errors) and flash output.
- The image reports App version 1.0.2, because PROJECT_VER has not been bumped yet. The flashed binary contains no "Which side" string. Its sources (dial_state.c 15:23:10Z, main.c 15:23:35Z on 2026-09-23) predate the binary (15:24:38Z), and `git diff fe5b139 HEAD -- firmware/dial-idf` touches only README.md.
- Serial: a reattaching `stty` + `cat` capture, one file per USB attach: bench-logs/2026-09-30-sidepick-attach1..8.log (bench-logs/ is gitignored). Every RST press dropped and re-created the port, so each RST boot has its own file.
- Scale was read from the dial face: the relative number −10 for 17.0 °C (27 − 10). The owner never tapped the Scale row.

## Results
- T1 (factory reset, One Bed): no side picker, BOTH SIDES, −10. attach1: `factory reset requested — erasing NVS`, SW reset, `App version: 1.0.2`, pad found at .169, `zone mode set to single (One Bed)`. No panic.
- T1-r1: BOTH SIDES, −10 (attach2, fresh banner). T1-r2: BOTH SIDES, −10 (attach3).
- T4 (Dual Sides on the non-fresh device, swipe right): no picker, LEFT SIDE, −10. attach3: `zone mode set to dual (Dual Sides)`, no POST.
- T4-r1: LEFT SIDE, −10 (attach4, no POST). This is the case that read Absolute on 1.0.2.
- T4-r2: LEFT SIDE, −10 (attach5). Three side1 power POSTs — see deviation 3.
- T2 (factory reset, Dual Sides in the first session): no picker, opened on RIGHT SIDE, −10. attach5: erase, reboot, pad found, One Bed then Dual Sides, no POST.
- T2-a: left on LEFT SIDE. T2-r1: LEFT SIDE, −10 (attach6, no POST). T2-r2: LEFT SIDE, −10 (attach7, no POST).
- T2-z: swiped to RIGHT SIDE, RST: RIGHT SIDE, −10 (attach8, no POST).
- Close: `zone mode set to single (One Bed)` (attach8). Capture stopped.

## Pad state, before and after
- Before: side0 off, target 17.0, current 23.53; side1 off, target 17.0, current 23.33; error false.
- After:  side0 off, target 17.0, current 23.72; side1 off, target 17.0, current 23.51; error false.
- Power and target are unchanged. Only the water temperature drifted.

## Deviations
1. Spec error: §7 expects `sb_face: no key -> default` after a factory reset. That line cannot appear. After an NVS erase the "ui" namespace does not exist, so `nvs_open(NVS_NS, NVS_READONLY)` fails and `dial_state_restore_prefs` returns at its first line, before any logging (dial_state.c:186). The line's absence, together with the erase line, is the proof of an empty namespace. §7 should be corrected.
2. T1-r1: the first RST press did not reset the dial (uptime kept climbing, no new attach). A second, firmer press produced attach2.
3. T4-r2: the glass-only rule was broken. After the reboot the dial sent `POST /api/power {"side1":{"is_on":true}}`, then `{"side1":{"is_on":false}}` twice (the first off timed out with ESP_ERR_HTTP_EAGAIN). The pad (single-zone, mirrored) ran briefly on both sides, then returned to off. The cause (knob press or power-disc tap) is unconfirmed. The Scale result stands: a power toggle does not write "relmode", and the boot was real.
4. T2-z: the owner reports that the knob turned one detent while he was handling the dial. There was no POST in attach7 or attach8, and both sides stayed at 17.0.
5. T2 swipe sequence: the owner reported the final state (LEFT SIDE). The intermediate right-then-left swipe was not reported step by step, and swipes do not log.
6. The capture is one file per USB attach rather than per boot. The factory-reset SW resets did not drop USB, so their boots are inside attach1 and attach5.
7. The image identifies as 1.0.2, not 1.0.3-beta.1 (the version bump comes with the release commit).

## Not verified
- T3: OTA from a settings-kept 1.0.2 to the published 1.0.3-beta.1, keeping the same side and Scale. This runs after publication.
- The Settings Scale row text. Scale was read from the face number rather than the row.
