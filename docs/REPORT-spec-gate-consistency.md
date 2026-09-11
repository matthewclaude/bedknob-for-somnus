# REPORT: SPEC-repo-consolidation — §3.1 and §7 item 1 made consistent with the reworded gate

**DONE** — `docs/SPEC-repo-consolidation.md`'s §3.1 closing sentence and §7 item 1 no longer contradict the Beta-builds-off 404 check that §7 item 2 became in `4a61622`; spec committed on its own as `d4abc6dc836dfb9bab74f42e4335fe7b364a88de`, this report and its index line in a second commit; documentation only, not pushed.

Date: 2026-09-11. Repo `~/Projects/somnus-waveshare-rotary-dial`, branch `main`, starting HEAD `4a6162266378b696d195fab1c38ba3593512295a`.

## 1. Gate checks — raw output

```
$ git log -1 --format=%H
4a6162266378b696d195fab1c38ba3593512295a
exit=0

$ git --no-optional-locks status --short --untracked-files=no
exit=0
```

Both passed. Premise checks before editing: §3.1's last sentence (line 70) ended "keeps Beta builds **on** so the gate actually exercises the new repo rather than a 404 that looks like success"; §7 item 1 (line 147) asked the serial log to show the tags request going to `api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50`. Both as described.

## 2. §3.1 closing sentence

### Before

Consequence: a repointed dial with Beta builds **off** reports "up to date" on every check until `1.0.1` stable exists in the new repo. For the one bench unit this is acceptable, and the hardware gate (§7) keeps Beta builds **on** so the gate actually exercises the new repo rather than a 404 that looks like success.

### After

Consequence: a repointed dial with Beta builds **off** reports "up to date" on every check until `1.0.1` stable exists in the new repo. For the one bench unit this is acceptable, and that 404 is what §7 item 2 uses to tell the two repos apart: with Beta builds **off** only the new repo answers `/releases/latest` with a 404, the old repo would report its `1.0.0` stable Release. Because the dial displays that outcome as "up to date" like any other, the serial log — not the screen — is what proves which repo answered.

## 3. §7 item 1

### Before

1. Bench dial on `1.0.0`, Beta builds **on** → Menu → Update → Check for updates → finds `1.0.1-beta.1` in the **old** repo (the serial log shows the tags request going to `api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50` and the Release fetch for `somnus-v1.0.1-beta.1` returning 200) → installs → reboots showing `1.0.1-beta.1` under Menu → About.

### After

1. Bench dial on `1.0.0`, Beta builds **on** → Menu → Update → Check for updates → finds `1.0.1-beta.1` in the **old** repo → installs → reboots showing `1.0.1-beta.1` under Menu → About. The evidence is the Release fetch for `somnus-v1.0.1-beta.1` succeeding — the serial log's `ota: latest 1.0.1-beta.1, running 1.0.0 -- update available` line — and the reboot into `1.0.1-beta.1`. No log line carries a URL, but a dial still running `1.0.0` polls only the old repo, so only `somnus-dial-releases` could have served it that release; that is what makes the observation meaningful.

## 4. Diff and commit

`git diff --stat` of the spec commit:

```
 docs/SPEC-repo-consolidation.md | 4 ++--
 1 file changed, 2 insertions(+), 2 deletions(-)
```

Spec commit: `d4abc6dc836dfb9bab74f42e4335fe7b364a88de` on `main`, parent `4a6162266378b696d195fab1c38ba3593512295a`. Not pushed; `somnus/main` remains at `fb38c83`. This report and its `docs/REPORTS.md` line follow in a separate commit, per the repo convention.

## 5. Deviations from the instructions

- Item 1 names the specific observable serial line for "the Release fetch succeeding": `ota: latest 1.0.1-beta.1, running 1.0.0 -- update available`. The instructions said "the Release fetch … succeeding" without naming a line; the line was added so item 1 does not rest on another unobservable claim. It was verified in source: `dial_ota.c:285` inside `finish_from_release()`, which the beta path calls at line 470 once the Release JSON for the chosen tag has been fetched.

Otherwise, no deviations. Nothing under `firmware/`, `.github/` or `web-flasher/` was touched; no other file under `docs/` besides the spec, this report and `docs/REPORTS.md`; §1 and §4(b) unchanged; no version bump, no tag, no push.

## 6. Not verifiable without hardware

- That the bench dial on `1.0.0` actually printed `ota: latest 1.0.1-beta.1, running 1.0.0 -- update available` during the 2026-09-10 pass. The format string and call path are confirmed in source; the bench capture was not re-examined here.
- That the old repo, asked `/releases/latest` by a Beta-builds-off dial, reports its `1.0.0` stable Release. Inferred from the API's semantics and the 2026-09-10 tag-push report, not observed on a dial.
- That the 404 outcome renders as "up to date" on the display, as §3.1 states. Not observed on hardware in this session.
