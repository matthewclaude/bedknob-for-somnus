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
