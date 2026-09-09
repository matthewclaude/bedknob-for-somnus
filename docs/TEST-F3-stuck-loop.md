# Test plan: F3 — escape hatches work while stuck in the connect loop

Written 2026-09-03. Owner-run, at the board. This is the last hardware check
before `0.1.4` (V1-scope item 11, "still owed"). Lives in `docs/` in the
firmware repo (this file), mirrored in the Claude Project; results go to
`HARDWARE-bringup-log.md` as a new §14.x entry and V1-scope item 11 gets
closed.

## What is being proven

Commit `f08dbb7` changed `backoff_wait()` in `main.c` to wait on the command
queue instead of `vTaskDelay`, so the initial connect loop services
`CMD_WIFI_RESET`, `CMD_FACTORY_RESET`, `CMD_OTA_CHECK`, `CMD_OTA_APPLY`,
`CMD_OTA_CLEAR_FAILED` and `CMD_TZ_CHANGED` **between attempts** — during the
backoff countdown only, never during the probe or the discovery scan. Before
this, a dial that could not reach its pad had Menu, Wi-Fi, Settings and Update
all reachable and every button on them dead. That is the sixth instance of
the recurring bug and it broke the fresh-install escape hatch.

The claim to falsify: *on a dial that cannot reach the pad, Update → Check for
updates produces "Checking…" and a result, and Wi-Fi → Change network reboots
into the portal.* Until this runs on the board, F3 is "compiles clean".

## Test design — block the pad at the router, not a wrong IP

V1-scope phrases the test as "set Pad Address to a wrong IP, reboot". **That
will not produce a stuck dial** on the home network: item 1 verified that a
wrong address triggers discovery, which finds the real pad in ~5 s and
re-persists the address. The loop self-heals before there is a countdown to
tap during.

To be stuck, the pad has to be unreachable and not discoverable, without
touching the pad — it is a real bed. **Primary method: move the dial to an
isolated network** — the router's guest SSID (client isolation on) or a
phone hotspot. Either gives the dial internet (SNTP and the GitHub release
check both need it) and no route to the pad, so discovery finds nothing.
Getting onto it uses Change network at steady state (already known to work);
getting back off it *is* the destructive half of the test.

Ruled out Sep 3: the router's "pause internet" on the pad's MAC — checked,
it cuts WAN only and LAN traffic keeps flowing, so the dial still reaches
the pad.

**Confirm the isolation before starting.** From a laptop on the guest SSID
(or the hotspot):

    curl -m 3 http://<pad-ip>/api/state

No response (timeout) means the network is isolated and the plan works as
written. A JSON reply means the guest network is not isolated; use the
hotspot instead.

**Fallback only: unplug the pad.** Simplest for the dial, but it
power-cycles a real bed, and whether setpoint, power state and the 3-stage
schedule survive a power loss has not been observed in this project. Only
use it if that is already known. Allow a minute or two for the pad to
rejoin Wi-Fi afterwards.

Nothing in this plan writes to the pad. The pad's state at the start is the
pad's state at the end; step 12 confirms it anyway.

## Preconditions

- Board on the bench, USB attached, serial monitor running **before** step 1
  so the whole run is captured: `idf.py -p /dev/cu.usbmodem83401 monitor`
  (or whatever the port enumerates as; note that attaching the monitor hard
  resets the board, which is fine here).
- Firmware on the board is `f08dbb7` or later — confirm under Menu → About
  (should read 0.1.3) and in the boot log's `App version`.
- Isolated network ready — guest SSID or phone hotspot — with the curl
  check above confirming the pad is unreachable from it. Know its SSID and
  password; you type them into the dial's portal in step 2.
- Pad is at its normal state. Write down the current setpoint and power state
  from the Somnus app before starting (step 12 compares against this).
- Daytime, bed not in use, so a 20-minute detour on the dial bothers nobody.

## Steps

**1. Baseline.** Dial at steady state on the home network, dial face showing
live pad data. Note the pad address under Settings → Pad Address.

**2. Move the dial to the isolated network.** Menu → Wi-Fi → Change
network. The dial reboots into the SoftAP portal (steady-state path, expected
to work). From the phone, join the dial's SoftAP, pick the guest or hotspot
SSID, enter the password, save. (Unplug fallback: skip this, unplug the pad,
and power-cycle the dial instead.)

**3. Watch it get stuck.** After the reboot the dial joins the isolated
network, tries the pad address, fails, runs discovery, finds nothing, and
lands in the connect loop with a visible countdown. Record: how long from
Wi-Fi join to the first countdown, and what the countdown screen says. This
is the state the rest of the test runs in. If instead the dial reaches the
dial face, the network is not isolated — stop and switch to the hotspot.

**4. Wait for SNTP.** The OTA check now reads `dial_time_valid()` directly,
which needs sync plus a zone. Give it 30 s on the hotspot before step 5. If
you want to see the branch, go to step 5 immediately instead and note what
the Update screen says when the clock is not yet valid — that message is
worth recording either way, but it is not the pass criterion.

**5. Check for updates, during the countdown.** Menu → Update → Check for
updates. Tap **while the countdown is visible**, not during a probe or a scan.

PASS: "Checking…" appears, followed by a result — expected "up to date" (or
equivalent) since the newest published release is 0.1.3 and the board is on
0.1.3. Record the exact strings and the delay from tap to "Checking…".

FAIL: no "Checking…" at all by the time the *next* countdown begins. A delay
of up to one scan length is not a fail — draining only happens in the backoff
— but write the delay down.

Serial: look for the OTA check log lines (the success-path lines added in
0.1.2) while the loop is still in its pre-connect phase. That is the proof
the worker serviced the command from inside `backoff_wait()`.

**6. Repeat step 5 once**, tapping deliberately during a scan or probe
instead, to see the delay case and confirm it still lands. Record the delay.

**7. Tap timing sanity.** Post one command and then immediately open Settings
and toggle nothing — just confirm the UI stays responsive and the sticky
screen is not yanked by the loop's next iteration (item 2's fix, but this is
the first time it is exercised alongside draining).

**8. Change network, during the countdown.** Menu → Wi-Fi → Change network.
Tap while the countdown is visible.

PASS: the button reads "Restarting…" and the dial actually reboots into the
SoftAP portal within a few seconds. Serial shows the reboot.

FAIL: "Restarting…" and nothing else. This is exactly the F3 symptom.

**9. Re-provision the home network** from the portal on the phone (unplug
fallback: plug the pad back in first and give it a minute). The dial should then: join Wi-Fi, try the
stored pad address, and reach the pad (or discover it). Record whether the pad address survived the Wi-Fi reset —
it should, `CMD_WIFI_RESET` wipes Wi-Fi credentials only — and whether the
timezone survived (Settings → Timezone should not read "Not set").

**10. Dial face.** Live pad data, correct side/zone label, clock local.

**11. Factory reset — optional, skip by default.** It is the third command in
the drained set and the most destructive: it wipes everything and costs a
full re-provision plus timezone plus pad address. Only run it if step 8
passed and you want the full set proven. If you do, run it as steps 2–3 then
Settings → Factory reset during the countdown, expect a reboot into the
portal, and then re-provision everything.

**12. Restore check.** Compare the Somnus app's setpoint and power state to
what you wrote down in the preconditions. They should be identical — nothing
in this test writes. If they differ, something else changed them (the pad's
own schedule can, at its stage boundaries); note the time and move on.

## What to write down

Keep it to one list in the bring-up log entry: firmware version; time from
hotspot join to first countdown; step 5 result strings and delay; step 6
delay; step 8 result and reboot time; whether pad address and timezone
survived step 9; step 11 run or skipped; step 12 match. Plus the serial log
saved to a file (not pasted into chat — garbled paste has bitten before).

## Pass criteria for closing F3

Steps 5 and 8 both PASS. Everything else is observation. If either fails,
F3 goes back to open, `0.1.4` waits, and the serial log is the artifact for
the next session.

## After a pass

- Bring-up log: new §14.x entry with the list above.
- V1-scope item 11: remove "still owed", mark F3 VERIFIED on hardware with
  the date.
- Then `0.1.4`: `PROJECT_VER` bump, `## 0.1.4` CHANGELOG section (F1/F2/F3/
  F6/F7 fixes, the flasher page fixes from `155385a`, flip both manifests'
  `new_install_prompt_erase` to `false` in the same pass), tag
  `somnus-v0.1.4`, push to `somnus` only, hard-refresh the flasher page
  after the Pages deploy and confirm the cable line changed.
