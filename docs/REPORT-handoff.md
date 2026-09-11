# Handoff — Sep 11 2026, end of session

## State of the tree

Branch `main`, HEAD `b35002a` (docs: spec-gate-consistency report), pushed;
`somnus/main` == HEAD (`git rev-list --count somnus/main..HEAD` = 0). Working
tree clean apart from this file and its one-line entry in `docs/REPORTS.md`,
both rewritten after the last commit and left for you to commit. No
`.git/index.lock`. Latest tag `somnus-v1.0.1-beta.1` (annotated, object
`f78b7b4`) points at `2716be9`; it exists on the `somnus` remote and nowhere
else. `origin` (upstream) was never pushed; its push URL is still `no_push`.
`PROJECT_VER` is `1.0.1-beta.1` on line 21 of `firmware/dial-idf/CMakeLists.txt`.

## What this session did

Documentation only. `docs/SPEC-repo-consolidation.md` now matches what
shipped, in three commits, all pushed by you at the end:

| SHA | What |
|---|---|
| `4a61622` | Status line: §2–§4 shipped in `somnus-v1.0.1-beta.1` on 2026-09-10, hardware-confirmed, dual-publish window open; still open = Phase 6 (§5) and the §7 re-run against 1.0.1 stable. §7 item 2 is now the Beta-builds-off check whose evidence is the verbatim `ota: no releases published yet (HTTP 404)` line; item 3's cat-based serial capture is keyed to that line; a note after item 5 records why the original item 2 (hostname in the serial log) was unobservable. Report `REPORT-spec-gate-reword.md` is in the same commit, at your instruction, so it carries no SHA. |
| `d4abc6d` | §3.1's closing sentence no longer says the gate keeps Beta builds on to avoid "a 404 that looks like success"; it says that 404 is item 2's discriminator and the serial log, not the screen, proves which repo answered. §7 item 1's evidence is now the observable `ota: latest 1.0.1-beta.1, running 1.0.0 -- update available` line plus the reboot into 1.0.1-beta.1, with the clause that only the old repo could have served that to a 1.0.0 dial. |
| `b35002a` | `REPORT-spec-gate-consistency.md` + its `REPORTS.md` line (separate commit, per the repo convention). |

CI for `b35002a`: run 34629151192, `ci`, success. Nothing under `firmware/`,
`.github/` or `web-flasher/` changed. Nothing was built or flashed.

## Two things verified while closing that were listed as owner-open

- **Pages is enabled on `bedknob-for-somnus`.** `gh api repos/.../pages`
  reports `status=built`, source `gh-pages` root;
  `https://matthewclaude.github.io/bedknob-for-somnus/` answers 200 and
  `firmware/beta/somnus-dial-merged.bin` answers 200 at 1,743,152 bytes, the
  same size as the old repo's copy. The §4(b) prerequisite is done.
- **The §7 beta gate's serial capture exists on disk:**
  `bench-logs/1.0.1-beta.1-ota.log` (gitignored via `.gitignore:28`, so it is
  not in the repo), 9 lines, written 2026-09-10 15:29 local. Line 3 is
  `I (129680) ota: no releases published yet (HTTP 404)`, the discriminating
  line item 2 now asks for. The other eight lines are pad state:
  `side A: on=0 set=18.0C water=24.3C`. The capture contains no item-1 line
  (no `update available`, no App version banner): it starts after the
  install and reboot. If you want the item-1 evidence on record, it is not in
  this file.

Because `bench-logs/` is ignored, the spec's "goes into the bring-up record"
requirement is satisfied only if that file, or its `ota:` line, is copied into
HARDWARE-bringup-log.md (Claude-Project-only) or somewhere tracked. Not done
here; your call.

## Release state (unchanged this session)

`somnus-v1.0.1-beta.1` is a prerelease in both `somnus-dial-releases` and
`bedknob-for-somnus`, byte-identical assets. `/releases/latest` is
`somnus-v1.0.0` on the old repo and 404 on the new one, verified again today
with `gh api`. That 404 is what makes §7 item 2 work; it disappears the
moment 1.0.1 stable is published in the new repo.

## Conventions confirmed or added today (also in memory)

- **The OTA client never logs a URL or repo name.** `dial_ota.c` uses the
  three URL `#define`s only as `ota_http_get()` arguments and logs outcomes
  (`latest X, running Y -- update available / up to date`,
  `no releases published yet (HTTP 404)`, the `-- trying next tag` warnings).
  Both repos sit behind `api.github.com`. Do not write a gate item that reads
  a hostname out of the serial log.
- A report that must sit in the same commit as the change it describes
  cannot carry that commit's SHA and cannot state its own line count; the
  repo's normal convention (report in the *next* docs commit) exists for this
  reason and was restored for the second task.

## Still open

- **§7 re-run against 1.0.1 stable needs a new discriminator.** Once 1.0.1
  stable exists in `bedknob-for-somnus`, a Beta-off check there returns the
  stable Release, and the old repo (also dual-published) returns the same
  tag. Neither the 404 nor the version string will tell the repos apart.
  Candidates: a release published to the new repo only (the first non-dual
  release, which is Phase 6 territory), or an asset-level difference. Decide
  before cutting 1.0.1 stable, and write the item so it is observable.
- Phase 6 cut-over (§5 items 1–5), after 1.0.1 stable and its gate.
- The tags-endpoint 50-per-page cap (§3.2(c)) is on the record, not built.
- `fb38c83`'s `Claude-Session` trailer has a one-character typo; pushed,
  left as-is.
- Layout audit Tier D and `SCR_SIDEPICK`/`side_picked` deletion; both owner
  decisions, untouched since Sep 7.
- The Sep 7 pad-restore question: the bench capture from 2026-09-10 shows the
  pad at `on=0 set=18.0C`, i.e. off at 18 °C, which is the state the Sep 7
  handoff said it should have been restored to. Treat as closed unless the
  bed says otherwise.
- Standby-poll cadence change (`SPEC-standby-poll.md`) still queues behind
  this line.

## Not verified without hardware

- No dial has been seen installing a *stable* release over the air; the bench
  dial's 1.0.0 → 1.0.1-beta.1 install on 2026-09-10 is the only OTA install
  observed since 0.1.5 → 0.1.6-era testing.
- That a Beta-off check pointed at the *old* repo reports 1.0.0 stable is
  inferred from the API, not observed on a dial.

## Tooling / machine

- No new tools. `gh` is authed as matthewclaude; one worktree (the main
  checkout); no background jobs; no artifact watches.
- Session scratchpad emptied (before/after snippets and the two edit scripts
  used for the spec rewrites). Nothing in `/tmp` belongs to this session.
