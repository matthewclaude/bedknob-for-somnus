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
