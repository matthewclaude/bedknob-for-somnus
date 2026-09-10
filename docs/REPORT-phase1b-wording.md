# REPORT: Phase 1b wording — repair six SPEC sentences, scrub the home pad address from the audit tool

**DONE** — all six SPEC sentences matched the quoted text and were repaired exactly as specified, the four occurrences in `tools/dial_display_audit.py` were replaced as specified with `py_compile` exit 0, and the final sweep shows the home address only in `docs/REPORT-*.md` files; committed as `4e86dbe` on `main`. No firmware file touched, no history rewritten. Push and CI result: see the post-push addendum at the end (committed separately, as instructed).

Date: 2026-09-10
Repo: `~/Projects/somnus-waveshare-rotary-dial` (matthewclaude/bedknob-for-somnus), branch `main`
Starting HEAD: `51a85e9f7d0f2f668905fe1544af6447d9982c31` = `somnus/main` (tree clean, nothing untracked)

## Gate check

| Check | Command | Result |
|---|---|---|
| 1 | `sed -n '21p' firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.0")` — PASS |
| 2 | `git tag -l 'somnus-v1.0.0'` | `somnus-v1.0.0` — PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | (empty) — PASS |

## STEP 1 — the six SPEC sentences

Pre-edit match check (`git grep -n -F` on each quoted "from" string, restricted to `docs/SPEC-*.md`): all six found, one hit each, at the line numbers the task predicted. **None failed to match.**

```
docs/SPEC-connect-phases.md:512:- Leaves the temporary `192.168.1.100` default (since reverted to `192.168.1.100`), the `main.c` connect-loop
docs/SPEC-ota-readiness.md:954:`http://192.168.1.100:8080` — a real home IP — while pad discovery was
docs/SPEC-ota-readiness.md:989:Zero occurrences of `192.168.1.100` anywhere in the binary. `0.1.0` (§7)
docs/SPEC-ota-readiness.md:1011:if so whether they still describe `192.168.1.100` as a live,
docs/SPEC-pad-discovery.md:62:`192.168.1.100` compiled default
docs/SPEC-pad-discovery.md:724:- Not touching the temporary `192.168.1.100` default (since reverted to
```

Each substitution was applied as an exact single-occurrence string replace (asserted count == 1), leaving indentation and line wrapping untouched. The full sentence after each edit, as it now reads on disk:

**1. `docs/SPEC-connect-phases.md` lines 512–514**

```
- Leaves the temporary home-address default (since reverted to `192.168.1.100`), the `main.c` connect-loop
  fix, the relative-scale fix, and the timezone work untouched, per the
  standing constraints.
```

**2. `docs/SPEC-pad-discovery.md` lines 61–66** (line 62 is the changed line; the joined sentence reads "This also retires the temporary home-address compiled default currently sitting uncommitted in the working tree", as required)

```
retires the need for either outright). This also retires the temporary
home-address compiled default
currently sitting uncommitted in the working tree: once discovery exists, a
fresh device needs no working compiled-in address at all — a scan finds the
real one, and the compiled default reverts to the neutral, documentation-only
placeholder it was always meant to be.
```

**3. `docs/SPEC-pad-discovery.md` lines 724–726**

```
- Not touching the temporary home-address default (since reverted to
  `192.168.1.100`, 2026-09-02), the `main.c` connect-loop fix, the
  relative-scale fix, or the timezone work.
```

**4. `docs/SPEC-ota-readiness.md` lines 952–958** (line 954 changed)

```
**2026-09-01.** `DIAL_PAD_DEFAULT_BASE_URL` (`dial_state.h:343`) and
`SOMNUS_DEFAULT_BASE_URL` (`dial_somnus.h:53`) were temporarily set to
the real home pad address — while pad discovery was
being built and verified on hardware, per commit `9ff178e`'s neutralization
of the *previous* hardcoded default. That reason is now gone: discovery
(`components/dial_pad_discovery/`, `docs/SPEC-pad-discovery.md`) is proven
on real hardware, so both constants are reverted to `192.168.1.100` — the
```

**5. `docs/SPEC-ota-readiness.md` lines 989–990**

```
Zero occurrences of the home address anywhere in the binary. `0.1.0` (§7)
is still present and unaffected by this change. Built, not flashed, per
```

**6. `docs/SPEC-ota-readiness.md` lines 1009–1015** (line 1011 changed)

```
**A future session: do not assume this note-worthy warning has been
removed from either document** — check whether the files exist yet, and
if so whether they still describe the home address as a live,
must-not-be-committed concern, since as of this pass that concern is
resolved in code (§8 above) but may still read as open in those two docs
once they're found or written.
```

Reading note on sentence 4 (not changed; the task's "to" text was applied verbatim): the original had a paired em-dash parenthetical ("… set to `http://…` — a real home IP — while …"). The replacement removed the value and the first dash, so one dash remains: "set to the real home pad address — while pad discovery was being built …". It reads acceptably as a single aside; flagged only so the owner knows it was noticed.

## STEP 2 — `tools/dial_display_audit.py`

Four occurrences, at lines 12, 15, 16 and 88 as predicted. Whitespace was verified with `sed -n l` before editing; every replacement was an exact single-occurrence match.

**2a. Docstring, lines 12–13 → line 12**

```
before: hardcoded default, even though the bench pad's current address (192.168.1.169)
        is visible in this repo's bench-logs/*.log. Pass it explicitly:
after:  hardcoded default. Pass the pad's address explicitly:
```

**2b. Usage examples, lines 15–16 → 14–15** (four-space indentation kept)

```
before:     PAD_HOST=192.168.1.169 python3 tools/dial_display_audit.py
            python3 tools/dial_display_audit.py --host 192.168.1.169
after:      PAD_HOST=192.168.1.100 python3 tools/dial_display_audit.py
            python3 tools/dial_display_audit.py --host 192.168.1.100
```

**2c. Runtime print, lines 88–90 → 87–89** (8/14-space indentation, implicit concatenation across two lines and `file=sys.stderr` all kept)

```
before:         print("(bench-logs/*.log shows this dial's pad at 192.168.1.169 as of "
                      "2026-09-06 -- verify before reuse, pad addresses change with DHCP.)",
                      file=sys.stderr)
after:          print("(pad addresses change with DHCP -- check the dial's Settings screen "
                      "or your router's lease table for the current one.)",
                      file=sys.stderr)
```

Resulting region, lines 10–15 and 85–90:

```
Pad host comes ONLY from --host or the PAD_HOST env var -- deliberately no
hardcoded default. Pass the pad's address explicitly:

    PAD_HOST=192.168.1.100 python3 tools/dial_display_audit.py
    python3 tools/dial_display_audit.py --host 192.168.1.100

    if not host:
        print("ERROR: pad host not given. Use --host <ip> or set PAD_HOST.", file=sys.stderr)
        print("(pad addresses change with DHCP -- check the dial's Settings screen "
              "or your router's lease table for the current one.)",
              file=sys.stderr)
        sys.exit(2)
```

Parse check:

```
$ python3 -m py_compile tools/dial_display_audit.py
py_compile exit 0
```

## STEP 3 — final sweep

```
$ git grep -n "192\.168\.1\.169"
docs/REPORT-0.1.5-upgrade-path-verify.md:109:179:I (3429) app: pad connected at http://192.168.1.169:8080
docs/REPORT-dial-display-audit.md:281:$ PAD_HOST=192.168.1.169 python3 tools/dial_display_audit.py --once
docs/REPORT-dial-display-audit.md:367:`PAD_HOST=192.168.1.169`, 1 Hz `GET /api/state` only. No writes were issued
docs/REPORT-layout-a1b.md:713:I (2768) app: pad connected at http://192.168.1.169:8080
docs/REPORT-layout-phase1-flash.md:542:I (3085) app: pad connected at http://192.168.1.169:8080
docs/REPORT-phase1-scrub.md:113:docs/REPORT-0.1.5-upgrade-path-verify.md:109:179:I (3429) app: pad connected at http://192.168.1.169:8080
docs/REPORT-phase1-scrub.md:114:docs/REPORT-dial-display-audit.md:281:$ PAD_HOST=192.168.1.169 python3 tools/dial_display_audit.py --once
docs/REPORT-phase1-scrub.md:115:docs/REPORT-dial-display-audit.md:367:`PAD_HOST=192.168.1.169`, 1 Hz `GET /api/state` only. No writes were issued
docs/REPORT-phase1-scrub.md:116:docs/REPORT-layout-a1b.md:713:I (2768) app: pad connected at http://192.168.1.169:8080
docs/REPORT-phase1-scrub.md:117:docs/REPORT-layout-phase1-flash.md:542:I (3085) app: pad connected at http://192.168.1.169:8080
docs/REPORT-phase1-scrub.md:118:docs/REPORT-rails-fix.md:882:I (5715) app: pad connected at http://192.168.1.169:8080
docs/REPORT-phase1-scrub.md:119:docs/REPORT-standby-face-c1.md:489:I (4178) app: pad connected at http://192.168.1.169:8080
docs/REPORT-phase1-scrub.md:120:docs/REPORT-standby-face-c2.md:554:I (5286) app: pad connected at http://192.168.1.169:8080
docs/REPORT-phase1-scrub.md:121:docs/REPORT-sticky-brightness.md:449:I (2645) app: pad connected at http://192.168.1.169:8080
docs/REPORT-phase1-scrub.md:122:docs/REPORT-sticky-night-pickers.md:478:I (3027) app: pad connected at http://192.168.1.169:8080
docs/REPORT-phase1-scrub.md:123:docs/SPEC-connect-phases.md:512:- Leaves the temporary `192.168.1.169` default (since reverted to `192.168.1.100`), the `main.c` connect-loop
docs/REPORT-phase1-scrub.md:124:docs/SPEC-ota-readiness.md:954:`http://192.168.1.169:8080` — a real home IP — while pad discovery was
docs/REPORT-phase1-scrub.md:125:docs/SPEC-ota-readiness.md:989:Zero occurrences of `192.168.1.169` anywhere in the binary. `0.1.0` (§7)
docs/REPORT-phase1-scrub.md:126:docs/SPEC-ota-readiness.md:1011:if so whether they still describe `192.168.1.169` as a live,
docs/REPORT-phase1-scrub.md:127:docs/SPEC-pad-discovery.md:62:`192.168.1.169` compiled default
docs/REPORT-phase1-scrub.md:128:docs/SPEC-pad-discovery.md:724:- Not touching the temporary `192.168.1.169` default (since reverted to
docs/REPORT-phase1-scrub.md:129:reference/bed_web_app.py:18:BED_IP = "192.168.1.169"
docs/REPORT-phase1-scrub.md:130:reference/somnus_bed.py:8:    def __init__(self, ip="192.168.1.169", port="8080"):
docs/REPORT-phase1-scrub.md:131:reference/somnus_bed.py:156:    bed = SomnusPad(ip="192.168.1.169", port="8080")
docs/REPORT-phase1-scrub.md:132:tools/dial_display_audit.py:12:hardcoded default, even though the bench pad's current address (192.168.1.169)
docs/REPORT-phase1-scrub.md:133:tools/dial_display_audit.py:15:    PAD_HOST=192.168.1.169 python3 tools/dial_display_audit.py
docs/REPORT-phase1-scrub.md:134:tools/dial_display_audit.py:16:    python3 tools/dial_display_audit.py --host 192.168.1.169
docs/REPORT-phase1-scrub.md:135:tools/dial_display_audit.py:88:        print("(bench-logs/*.log shows this dial's pad at 192.168.1.169 as of "
docs/REPORT-rails-fix.md:882:I (5715) app: pad connected at http://192.168.1.169:8080
docs/REPORT-standby-face-c1.md:489:I (4178) app: pad connected at http://192.168.1.169:8080
docs/REPORT-standby-face-c2.md:554:I (5286) app: pad connected at http://192.168.1.169:8080
docs/REPORT-sticky-brightness.md:449:I (2645) app: pad connected at http://192.168.1.169:8080
docs/REPORT-sticky-night-pickers.md:478:I (3027) app: pad connected at http://192.168.1.169:8080
(exit 0)
```

**Is every remaining hit in a `docs/REPORT-*.md` file? YES.** 33 hits across 11 files, all `docs/REPORT-*`. `git grep -n "192\.168\.1\.169" | grep -v '^docs/REPORT-'` returns nothing (exit 1). Of the 33, 23 are inside `docs/REPORT-phase1-scrub.md`, which quotes the previous pass's full grep output (including the then-tracked `reference/*.py` and `tools/` lines that no longer exist in that form). Nothing outside `docs/REPORT-*` was found, so nothing was left unfixed by the rule.

Note also that this report itself, once committed, adds further `docs/REPORT-*` hits by quoting the sweep above. That is the same historical-record convention.

## Commits — `git diff --stat` per commit, every SHA

**Commit A (STEPS 1–2)** — `4e86dbe059843a368f89eaa523ee996a4c999864` — "docs: name the home-address default instead of printing a value; scrub it from the audit tool"

```
 docs/SPEC-connect-phases.md |  2 +-
 docs/SPEC-ota-readiness.md  |  6 +++---
 docs/SPEC-pad-discovery.md  |  4 ++--
 tools/dial_display_audit.py | 11 +++++------
 4 files changed, 11 insertions(+), 12 deletions(-)
```

No file under `firmware/` is touched. Working tree clean after commit A.

**Commit B (this report + its `docs/REPORTS.md` line)** — SHA recorded in the addendum below and in the chat reply.

**Commit C (post-push addendum)** — SHA in the chat reply only.

## Push and CI

See the post-push addendum at the end of this report.

## Deviations

- **A third, docs-only addendum commit follows commit B**, exactly as the task instructed (it cannot be avoided: a commit cannot carry its own push result). Its second push triggers one more ci.yml run that is not tracked in this report.
- Sentence 4 (`SPEC-ota-readiness.md:954`) is left with one unpaired em dash; the task's exact "to" text was applied and no further wording change was made. Noted under STEP 1.
- Otherwise none: all six SPEC strings matched, all four audit-tool occurrences matched, no `.gitignore` or firmware change, nothing pushed to `origin`.

## Cannot verify from disk

- **Whether the audit tool still behaves identically at runtime** against a live pad. Only `py_compile` was run (the task's stated check). The changed lines are a docstring and an error-path `print`; no logic changed. A live run needs a pad on the network and was not in scope.
- **Whether `192.168.1.100` occurs in the 1.0.0 release binary.** Sentence 5 now says "zero occurrences of the home address", which restores the original claim's meaning; the actual scan was done in the 2026-09-01 session on that session's build, and has not been re-run here.
- **The public repo's rendered state** after the push — see addendum for what the API showed.

## Post-push addendum (written after commit B, committed separately)

Commit B (this report + its `docs/REPORTS.md` line): **`b5248a1e2f318512c1381f79b68d503e15851b4e`** — "docs: phase1b-wording report"

```
 docs/REPORT-phase1b-wording.md | 223 +++++++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                |   1 +
 2 files changed, 224 insertions(+)
```

Push:

```
$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   51a85e9..b5248a1  main -> main
```

Commits carried (`git log --oneline 51a85e9..HEAD`, `somnus/main` before the push was `51a85e9`):

```
b5248a1 docs: phase1b-wording report
4e86dbe docs: name the home-address default instead of printing a value; scrub it from the audit tool
```

`git ls-remote somnus refs/heads/main` after the push: `b5248a1e2f318512c1381f79b68d503e15851b4e`. `origin` was not touched (push URL is `no_push`).

CI (`gh run list --repo matthewclaude/bedknob-for-somnus --limit 3`, columns: id, workflow, sha, event, status, conclusion, created):

```
34505156131 ci b5248a1 push in_progress  2026-09-10T16:56:31Z
34504551360 ci 51a85e9 push completed success 2026-09-10T16:50:27Z
34503940658 ci f77e7ee push completed success 2026-09-10T16:44:31Z
```

`gh run watch 34505156131 --exit-status` returned 0. Final state: **ci.yml run 34505156131 on `b5248a1` — completed, conclusion `success`** (single job `build`, success). (Run 34504551360 is the previous task's addendum push; also success.)

Remote check via the GitHub contents API after the push: the served `tools/dial_display_audit.py` contains zero lines matching the home address. This resolves the "public repo's rendered state" item under Cannot verify from disk.

This addendum is committed as a third docs-only commit ("docs: phase1b-wording report — push and CI addendum") whose SHA is reported in the chat reply, and pushed to `somnus` so the remote matches the tree. That push triggers one further ci.yml run, not tracked here.
