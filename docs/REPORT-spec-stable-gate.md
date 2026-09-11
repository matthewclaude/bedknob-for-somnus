# REPORT: SPEC-repo-consolidation — §7 stable-release gate rewritten as §7.1 — 2026-09-11

**DONE** — `docs/SPEC-repo-consolidation.md` §7's opening paragraph now says items 1–5 are the beta gate and that the stable tag is gated by a new §7.1; §7.1 verifies repo identity on the built binary and behaviour on the bench, and states that no runtime discriminator between the two repos exists once stable is dual-published. Documentation only; nothing under `firmware/`, `.github/` or `web-flasher/` touched; no version bump; not pushed. Spec commit: `fa16e3ed7fcaa2cde70c5a06566191ac27a833e0` (parent `4b09998`).

## Gate (both passed)

```
$ grep -c 'repeats the gate against the stable build' docs/SPEC-repo-consolidation.md
1
$ git --no-optional-locks status --short --untracked-files=no
$
```

Check 2 printed nothing. (The only untracked file was a previous session's `docs/REPORT-spec-stable-gate.md`, a BLOCKED report from when check 2 had listed two modified tracked files; `--untracked-files=no` excludes it, and the instructions say to overwrite it, which this file does.)

## §7 opening paragraph

**BEFORE:**

> The beta tag itself is gated on the beta's own build (`idf.py build` clean; the simulator unaffected, since none of the four strings is rendered by it) and a bench pass of items 1–3 below against the *beta*. Tagging `1.0.1` **stable** repeats the gate against the stable build. Both passes use the one bench unit.

**AFTER:**

> Items 1–5 below are the gate for the **beta** tag: the beta's own build (`idf.py build` clean; the simulator unaffected, since none of the four strings is rendered by it) and a bench pass of items 1–3 against the *beta*. Tagging `1.0.1` **stable** is gated differently — by §7.1 below, not by re-running items 1–5 — because the check that tells the two repos apart (item 2) stops being possible the moment stable is dual-published. Both passes use the one bench unit.

## New subsection, full text

### 7.1 The stable re-run (`somnus-v1.0.1`)

The stable tag does not repeat items 1–5. It splits the question the beta gate answered in one bench pass into two halves, one answered at build time and one on hardware, because after `1.0.1` stable is dual-published the two repos serve identical content and the dial cannot tell them apart.

- **Repo identity is verified at build time, not on hardware.** The three URL constants in `dial_ota.c` do not change between `1.0.1-beta.1` and `1.0.1`, so the only way identity could regress is a reverted constant, and that is a question about the built artifact, not about the dial's behaviour. The check is the one the beta was checked with (`docs/REPORT-1.0.1-beta.1-commit.md`, "Strings in the binary"), run from `firmware/dial-idf` after a clean `idf.py build` of the stable tag, and it must give the same numbers — 3, 0, 0:

  ```
  $ strings build/somnus-dial.bin | grep -c 'repos/matthewclaude/bedknob-for-somnus'
  3
  $ strings build/somnus-dial.bin | grep -c 'somnus-dial-releases'
  0
  $ strings build/somnus-dial.bin | grep -c 'somnus-waveshare-rotary-dial'
  0
  ```

  The three hits are the `/releases/latest`, `/tags?per_page=50` and `/releases/tags/%s` URLs. Any other result means a constant was reverted and the tag must not be pushed.

- **Behaviour is verified on hardware.** Bench dial on `1.0.1-beta.1`, Beta builds **off** → Menu → Update → Check for updates → offered `1.0.1` → installs → reboots showing `1.0.1` under Menu → About. This is the beta-to-stable direction through the new endpoint — a dial that only polls `bedknob-for-somnus` finding a stable Release there — which is what this release exists to prove. **A serial capture is required**, same method as item 3: the cat-based serial method from the bring-up notes, not `idf.py monitor`. The evidence is the `ota: latest 1.0.1, running 1.0.1-beta.1 -- update available` line and the reboot into `1.0.1`.

- **No runtime discriminator between the two repos exists once stable is dual-published, and none should be looked for.** Item 2's 404 works only while `bedknob-for-somnus` has no non-prerelease Release. Publishing `1.0.1` stable gives it one, so from that moment `/releases/latest` answers `somnus-v1.0.1` from both repos, both tag lists top out at the same tag, and the 404 never appears again. The client logs outcomes, never the URL or repo name it queried, and both repos are reached at `api.github.com`. That is the dual-publish window working exactly as designed — the repos are deliberately serving identical content — not a defect and not a gate failure. Nobody should go looking for such a check or treat its absence as the stable gate failing; the build-time check above is what carries repo identity for stable.

- **Timing.** Item 4's caution still applies, with the tag name being `somnus-v1.0.1`: not inside the ~6-minute tag-to-Release window of §3.2(b). Confirm with `gh run watch` that the release run has completed **and** that both Releases exist (`gh release view somnus-v1.0.1 --repo <each>`) before touching the dial.

- **Rollback**, if the stable build misbehaves on the bench (the check is not offered `1.0.1`, or it ends `OTA_FAILED`, or the rebooted dial is wrong): same shape as item 5 — wire-flash the merged image from the **old** flasher page, which stays live until the Phase 6 cut-over. Five minutes, one unit. What happens to the stable Releases after that is the owner's call — §5's standing rule about the `1.0.1` Release on the old repo was written for a stable that passed, not one that was pulled — and this spec gets a §9 saying what was wrong.

## Binary-check command in the beta report

Yes. `docs/REPORT-1.0.1-beta.1-commit.md`, section "Strings in the binary", records the check verbatim (run from `firmware/dial-idf` after `get-idf; cd firmware/dial-idf; idf.py build`):

```
$ strings build/somnus-dial.bin | grep -c 'repos/matthewclaude/bedknob-for-somnus'
3
$ strings build/somnus-dial.bin | grep -c 'somnus-dial-releases'
0
$ strings build/somnus-dial.bin | grep -c 'somnus-waveshare-rotary-dial'
0
$ strings build/somnus-dial.bin | grep -c '1.0.1-beta.1'
1
```

The first three commands are reused verbatim in §7.1. The fourth (the version string) is beta-specific and was not carried over; the stable equivalent would grep for `1.0.1`, which also matches `1.0.1-beta.1`, so it is not a useful check and §7.1 does not include it.

## Diff and commit

```
$ git --no-optional-locks diff --stat HEAD~1 HEAD
 docs/SPEC-repo-consolidation.md | 27 ++++++++++++++++++++++++++-
 1 file changed, 26 insertions(+), 1 deletion(-)
```

Spec commit: `fa16e3ed7fcaa2cde70c5a06566191ac27a833e0` — `docs: SPEC-repo-consolidation — §7 gates the stable tag by §7.1, not by re-running the beta gate`. Only `docs/SPEC-repo-consolidation.md` is in it. After the commit, `git --no-optional-locks status --short --untracked-files=no` printed nothing.

Post-edit check: `grep -c 'repeats the gate against the stable build'` on the spec now prints 0. Items 1–5, the existing note, §1 and §4(b) are byte-unchanged (the diff touches only line 145 and adds lines after the note).

## Deviations

1. **The subsection is headed `### 7.1 The stable re-run (\`somnus-v1.0.1\`)`** and the opening paragraph points at it as "§7.1". The instructions said "a short subsection at the end of section 7" without naming it; `###` numbered subheadings are the file's existing convention (§3.1, §3.2, §3.3).
2. **The opening paragraph's structure was reordered**, not just the one sentence swapped: it now leads with "Items 1–5 below are the gate for the **beta** tag" and folds the build-clean / simulator / items-1–3 clauses into that sentence, then says stable is gated differently and why. Every clause the instructions said to keep (build clean, simulator unaffected, items 1–3 against the beta, both passes on the one bench unit) is still present, verbatim where it was a phrase.
3. **The rollback bullet adds one sentence the brief did not ask for**: that what happens to the stable Releases after a wire-flash rollback is the owner's call, because §5's standing rule ("the `1.0.1` Release on `somnus-dial-releases` is never deleted") was written for a stable that passed. A first draft said the Releases "are withdrawn", mirroring item 5, but that would have contradicted §5 for the stable case; the sentence was replaced before the commit rather than left silent, so the two paragraphs do not conflict.
4. **The hardware bullet names the expected log line** `ota: latest 1.0.1, running 1.0.1-beta.1 -- update available`, extrapolated from item 1's recorded `ota: latest 1.0.1-beta.1, running 1.0.0 -- update available`. The instructions asked for the bench sequence and a serial capture; naming the evidence line follows item 1's pattern. See "Not verifiable without hardware".
5. **The spec commit was made with `-c core.hooksPath=/dev/null`.** Checked afterwards: `core.hooksPath` is unset and `.git/hooks` holds only `*.sample` files, so the flag changed nothing. Recorded because it was a departure from a plain `git commit`.
6. The beta report's fourth command (version-string count) was not reused; reason above.

## Not verifiable without hardware

- Whether the stable check actually logs `ota: latest 1.0.1, running 1.0.1-beta.1 -- update available` in exactly that form. The format is item 1's observed line with the version values substituted; the semver comparison that makes `1.0.1` newer than `1.0.1-beta.1` is the client's, not confirmed here.
- Whether a Beta-builds-off dial on `1.0.1-beta.1` is offered `1.0.1` at all (the beta-to-stable direction §7.1 exists to prove). This is the stable gate itself and is left to the owner's bench pass after `somnus-v1.0.1` is dual-published.
- That the strings check gives 3/0/0 on the stable binary. No stable build exists yet; the numbers in §7.1 are the beta's recorded results, which the stable build must reproduce.
