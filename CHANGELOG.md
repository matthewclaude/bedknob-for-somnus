# Changelog

Somnus firmware changelog: what changed in each release, in the terms you'd
notice using it. Versions match the `somnus-vX.Y.Z` tags and the number the
dial shows under Menu → About.

This project forked from `chris023/orion-waveshare-rotary-dial`. That
project's own changelog (versions 1.0.0–1.4.2, tags `dial-v*`) is real
lineage, kept in full at `CHANGELOG-orion.md` — but nothing has ever shipped
under the Somnus name, so Somnus versioning starts fresh here at 0.1.0,
unrelated to those numbers. **1.0.0 is reserved for when v1 scope is
actually complete** — i.e. when a new user can genuinely buy a board,
flash it from a URL, and add their pad without typing an IP. Everything
before that is 0.x.

This file is the source of the release notes on GitHub: the release
workflow extracts the section matching the tag and publishes it as that
release's description, and refuses to publish a tag with no section here.
Add the new section in the same commit that bumps `PROJECT_VER`.

Releases marked **(beta)** are prereleases, visible only to dials with
"Beta builds" turned on.

## 0.1.4 — 2026-09-03

This release completes v1 scope: every step on the new-user path — buy a
board, flash it from a URL, join Wi-Fi from a phone, pick a timezone, add the
pad without typing an IP, get an over-the-air update — has been verified on
hardware as of 2026-09-03.

### Fixed

- **A dial set up from an iPhone had no clock and could not see updates.**
  iPhone Wi-Fi provisioning never fills in the timezone, so the dial came up
  with no zone at all. Now a dial that has never had a zone set goes straight
  to the timezone picker the first time it reaches the pad, and stays there
  until you pick one — after which the clock is right and update checks work.
  The picker is raised only once the dial is actually connected, so the
  choice is applied right away instead of sitting unapplied and being lost on
  the next reboot.

- **Dials upgraded from the Orion-era firmware count as having a zone.** Those
  builds saved the timezone rule without the zone name. The clock on such a
  dial was already right, so it is not sent to the picker, and Settings shows
  the rule it is using instead of "Not set".

- **Change network, Factory reset and Check for updates now work while the
  dial is still trying to reach the pad.** They used to be ignored between
  connection attempts — exactly when a dial stuck on the wrong network needed
  them. Verified on hardware 2026-09-03.

- **Bed Mode is re-read after the dial connects.** A One Bed / Dual Sides choice
  made while the dial was still trying to reach the pad used to be applied
  late: the first poll after connecting still used the old mode. Changing
  Pad Address or Bed Mode during a retry now also cuts the wait short and
  reconnects straight away.

- **Flasher page: cable instruction.** It now asks for a USB-A to USB-C data
  cable and explains why a C-to-C cable will not work with this board.

- **Flasher page: it always erases.** The old copy suggested erasing was a
  choice for brand-new dials. Flashing from the page always erases everything,
  including Wi-Fi, timezone and pad address, and the page now says so; the
  flasher's own "erase first" option is gone because it never made a
  difference. Over-the-air updates from the dial's Update menu keep your
  settings and remain the normal way to update.

- **Flasher page: two colors were off the palette.** The accent tint was still
  the old orange and the button text was pure white; both now follow the brass
  palette.

- **Flasher page: the third-party notices link was broken.** It pointed at a
  file name that does not exist.

### Changed

- **The PolyForm required notice now appears in the flasher page footer and in
  every GitHub release's notes.** The footer also states that the firmware is
  provided as-is, with no warranty.

## 0.1.3 — 2026-09-02

### Fixed

- **A side you had switched off was still adjustable, and still wrote to the
  pad.** The setpoint stayed drawn while a zone was off, and both the knob and
  the drag handle still moved it — so turning the dial on a side you had just
  turned off sent a new target temperature to that side of the bed, with
  nothing on screen saying so. Both inputs now check power first. The knob
  gives the same soft-stop pulse it gives at a range limit, and the power
  button breathes twice to point at the control that unblocks things.

- **A crash during connection flapping.** Phase-driven screen changes loaded
  with an animation, leaving a load pending for about a fifth of a second; if
  the connection state changed again in that window, the dial could finalize a
  screen that had already been torn down and reset. Those loads are now
  synchronous.

### Changed

- **An off side is now quiet, not just dimmer.** The temperature numeral, its
  unit, and the WATER caption drop back while a side is off, so the power
  button is the brightest thing on the face.

- **One Bed mode says BOTH SIDES.** It previously said RIGHT SIDE, naming
  something that does not exist — in One Bed mode the dial writes one side and
  the pad mirrors it. If the label ever disagrees with your bed, the Bed Mode
  setting is wrong.

Ported from Orion Dial (chris023/orion-waveshare-rotary-dial): 8615c3b,
07c3d14, cd8acf8, 76162de.

## 0.1.2 — 2026-09-01

### Changed

- **Update checks now report what they found, in the device log.** Checking
  for an update previously logged nothing at all when the answer was "up to
  date" or "update available" — only failures showed up in the log. Now
  every outcome logs the latest version seen, the version you're running,
  and the verdict (e.g. `latest 0.1.2, running 0.1.1 -- update available`),
  so an OTA problem can actually be diagnosed from a serial log instead of
  guessing whether the check ran at all.

### Fixed

- **The monthly cert-sentinel check, which had been failing since the
  Somnus port.** It was still looking for the trust-anchor bundle at its old
  pre-port location and checking a cloud host this fork no longer contacts;
  both are corrected, and it no longer fires on release tags.

## 0.1.1 — 2026-09-01

No firmware behavior change. This release fixes wording on the flasher page
and corrects the project's third-party attribution.

### Changed

- **Flasher page copy cleaned up.** The install page and its README no
  longer say "dual-zone bed" — the pitch and after-flashing instructions
  now describe setting Bed Mode (One Bed or Dual Sides) to match your
  setup, matching what the dial actually asks you to configure.
- **`THIRD_PARTY_LICENSES.md` corrected.** Credits this fork's own
  original work (the Somnus pad client, pad auto-discovery, on-device
  timezone handling, the temperature-scale rework, and connect-flow
  navigation fixes) alongside the upstream project it's built on; notes
  that the boost-icon fonts are shipped as source but currently unused;
  and updates the CA bundle entry to its current name, location, and
  purpose (`dial_ota`'s TLS verification), reflecting that the upstream
  cloud-auth pipeline it originally also served no longer exists in this
  fork.

## 0.1.0 — 2026-09-01

First Somnus-numbered version. No behavior change — this release exists to
fix the version identity itself before anything real is cut:

### Changed

- **`PROJECT_VER` reset from the inherited `1.4.2` to `0.1.0`.** Nothing has
  ever shipped under the Somnus name; the prior value was carried over
  verbatim from the upstream fork this project started from and never
  bumped down, which would have made every device permanently reject any
  real Somnus 0.x release as a downgrade (`is_newer()` has no concept of a
  renumbering, only "older" — see `docs/SPEC-ota-readiness.md` §7).
- **Release tags move from `dial-v*` to `somnus-v*`.** The 23 releases
  inherited from the upstream project stay in git history for real lineage,
  but can no longer match this repo's release workflow trigger or be
  mistaken for a Somnus release.
- **This changelog splits accordingly.** The upstream project's history
  moves to `CHANGELOG-orion.md`, unedited; this file carries only Somnus
  releases from here on.
