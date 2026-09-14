# REPORT: Home Assistant MCP server integration — docs/SPEC-home-integration.md

Date: 2026-09-14. Task: document the HA MCP server integration in `docs/SPEC-home-integration.md`; documentation only.

## 1. Verdict

**DONE, WITH ONE STRUCTURAL DEVIATION.** The requested content is written and committed as `abac7a1`, but as a **new** `docs/SPEC-home-integration.md`: the file the task asked to update does not exist in this repo or in any ref of its history, and neither the Alexa RangeController finding nor a "section 7.5" exists in any file here. Both are recorded in the spec as not present (§2 of the spec) rather than paraphrased. Nothing pushed.

## 2. Gate check — the no-features-before-v1 rule

**Not implicated. Confirmed explicitly.** Two independent reasons:

1. This task changed one Markdown file under `docs/`. No firmware, simulator, companion-app, workflow or Home Assistant configuration was touched (`git show --stat abac7a1` below: one file, 93 insertions). No feature exists to gate.
2. The only in-repo record of the rule is `docs/SPEC-voice.md` §0.2, quoting the Sep 1 rule as "no new features until v1 is functional and a new user can easily add their pad." `somnus-v1.0.0` shipped 2026-09-09 and current stable is `somnus-v1.0.1` (`docs/REPORTS.md`, `README.md`), so the rule's precondition has already been met regardless.

The unverified climate-entity idea in the spec's §1.5 is recorded as an option and explicitly marked "not to be implemented"; no HA entity was created or changed.

## 3. Raw output of every check run

There is no build, lint or test target for `docs/` in this repo: `ci.yml` only builds `firmware/dial-idf`, and no `markdownlint`, `markdownlint-cli2` or `mdl` is installed. The checks below are the ones run, unfiltered:

```
$ git diff --check
exit 0

$ git status --short
?? docs/SPEC-home-integration.md

$ grep -n "^#" docs/SPEC-home-integration.md
1:# Spec: Home integration — Home Assistant MCP server (Assist)
7:## 1. Home Assistant MCP server, reached from Claude Desktop (2026-09-14)
9:### 1.1 Server side
16:### 1.2 Client side: Claude Desktop on the Mac, bridged by mcp-remote
45:### 1.3 LAN-only by design
49:### 1.4 Constraint: no number-domain tool, so the pad setpoint is out of reach
61:### 1.5 Possible future path — UNVERIFIED, not to be implemented
65:### 1.6 Write coordination: a fourth writer
77:### 1.7 Troubleshooting notes (kept because each one cost time)
84:## 2. Referenced material that is not in this repo

$ grep -c -E "eyJ|[A-Za-z0-9_-]{40,}" docs/SPEC-home-integration.md   # token-shaped strings
0
exit 1

$ python3 -c "import json,sys; ..."   # the JSON block in 1.2 parses when wrapped in braces
json ok
exit 0
```

## 4. `git diff --stat` and commit SHA

Commit `abac7a1b232d1665cac898b3e74e0be434d2d244` on `main`, parent `bcd50c4`. Added with `git add docs/SPEC-home-integration.md` (single path), committed alone. Not pushed. Raw:

```
 docs/SPEC-home-integration.md | 93 +++++++++++++++++++++++++++++++++++++++++++
 1 file changed, 93 insertions(+)

$ git rev-parse HEAD
abac7a1b232d1665cac898b3e74e0be434d2d244

$ git show --stat --format="%H %s" HEAD
abac7a1b232d1665cac898b3e74e0be434d2d244 docs: SPEC-home-integration - Home Assistant MCP server (Assist) via Claude Desktop + mcp-remote, 2026-09-14

 docs/SPEC-home-integration.md | 93 +++++++++++++++++++++++++++++++++++++++++++
 1 file changed, 93 insertions(+)

$ git status --short
```

`git status --short` after the commit was empty; this report and the `docs/REPORTS.md` line added for it are the only untracked/modified files now, left for the next docs commit per the repo's report convention (`docs/REPORTS.md` preamble).

## 5. Section numbers and cross-references actually written

`docs/SPEC-home-integration.md` as committed (headings verbatim from `grep -n '^#'`, line numbers as of `abac7a1`):

| Line | Heading | Task bullet it covers |
|---|---|---|
| 1 | `# Spec: Home integration — Home Assistant MCP server (Assist)` | file title; Status line explains the file is new |
| 7 | `## 1. Home Assistant MCP server, reached from Claude Desktop (2026-09-14)` | the requested new section |
| 9 | `### 1.1 Server side` | built-in MCP Server integration, Assist, endpoint, Streamable HTTP stateless, home-assistant 1.26.0, protocol 2025-06-18, long-lived token |
| 16 | `### 1.2 Client side: Claude Desktop on the Mac, bridged by mcp-remote` | mcp-remote@0.14.2, config path, absolute `/opt/homebrew/bin/npx` and the minimal-PATH reason, `--allow-http --transport http-only --header Authorization:${HA_TOKEN}`, `HA_TOKEN="Bearer <token>"` in env, mcp-remote does the `${}` substitution |
| 45 | `### 1.3 LAN-only by design` | no Nabu Casa, no public exposure, no claude.ai remote connector, standing local-control goal |
| 49 | `### 1.4 Constraint: no number-domain tool, so the pad setpoint is out of reach` | 23 tools, no number tool, no Assist set-value intent for number, template number setpoint unsettable; reachable: `intent__HassTurnOn` / `intent__HassTurnOff` on the template switch, `homeassistant__GetLiveContext` for reads |
| 61 | `### 1.5 Possible future path — UNVERIFIED, not to be implemented` | climate entity → `climate__HassClimateSetTemperature`, marked unverified |
| 65 | `### 1.6 Write coordination: a fourth writer` | MCP client alongside dial, Bedknob for Mac, HA; manual and on-demand, not scheduled |
| 77 | `### 1.7 Troubleshooting notes (kept because each one cost time)` | (a) stale `--help`, verify via `npm pack` + grep `package/dist`; (b) OAuth "dynamic client registration" error = 401, HA is IndieAuth not RFC 7591; (c) first-launch "Server disconnected" from the npx download vs startup timeout; (d) `notifications/initialized` trace and `Token result: Not found` benign |
| 84 | `## 2. Referenced material that is not in this repo` | the missing prior file, Alexa finding, §7.5, and where the v1 rule actually lives |

Cross-references as written, concretely:

- §1.4, Alexa: "This is the same class of limitation as the **Alexa RangeController** finding … That finding is **not in this repo** (see §2); when it is reachable from here, cross-reference it from this paragraph rather than restating its analysis." — The task asked to cross-reference the finding "already recorded in this spec". It is not recorded in this spec or anywhere in the repo, so the sentence names it and points at §2 instead of inventing a section number.
- §1.6, write coordination: "The task that produced this section asked for this to be linked to 'the existing open write-coordination question in section 7.5' of this spec. **There is no §7.5 in this repo** … treat this subsection as the in-repo pointer." — The question is stated as open and is not answered.
- §1.6 → `docs/SPEC-standby-poll.md` (300 s STANDBY poll) for how long an MCP write takes to reach the dial. That file and figure exist.
- §2 → `docs/SPEC-voice.md` §0.2 for the v1 scope rule. That section exists (line 15 of that file).
- Status line and §2 → the "not in this repo" note pattern used by `SPEC-standby-poll.md` and `SPEC-repo-consolidation.md` for Claude-Project-only files.

## 6. Deviations from the instructions

1. **"Update docs/SPEC-home-integration.md" became "create".** The file is absent from the working tree, from `git log --all -- docs/SPEC-home-integration.md` (no output), from every branch (`origin/*`, `somnus/*`, `gh-pages`, `firmware/dial-product`) and from `git log --all --name-only` (no path containing `home` or `alexa` ever committed). Creating it was the only way to deliver the content; the Status line says so in the file.
2. **The Alexa RangeController cross-reference could not be made.** `grep -ril 'alexa\|RangeController'` over the repo (excluding `.git`, `build`, `managed_components`) returns nothing. Per the instruction, the spec says the finding is not present rather than guessing; the analysis is not restated.
3. **The §7.5 link could not be made.** No such section exists. The spec says so in §1.6 and states the question as open in its own words.
4. **The "no-features-before-v1 rule" is not recorded under that name anywhere in the repo.** The nearest in-repo text is `docs/SPEC-voice.md` §0.2. The gate check in §2 above cites that and the fact that 1.0.0 has shipped; if the rule the task means lives in a Claude-Project-only file (`V1-scope.md`), it could not be opened here.
5. **`-y` appears in the documented `args` although the task's flag list omitted it.** It is what the on-disk `claude_desktop_config.json` contains today; the spec shows the real block and says so.
6. **Added an uncommitted line for this report to `docs/REPORTS.md`** (repo convention from its preamble: every report gets an index line; reports ride the next docs commit). Not requested by the task; left uncommitted, together with this report, for the owner's next docs commit.
7. **Committed.** The task asked for "the commit SHA", so a commit was made, of the spec alone. No push, no tag; the task did not ask for either.

No other deviations. Every bullet of the requested content is present in §1.1–§1.7 as tabulated in section 5.

## 7. Not verified without hardware or network access to 192.168.1.171

Nothing on the HA host was contacted during this task. Verified locally on the Mac, read-only:

- `~/Library/Application Support/Claude/claude_desktop_config.json` exists and its `home-assistant` entry matches the spec's §1.2 block exactly (command, all seven args in that order, `HA_TOKEN` in `env` beginning `Bearer `). Token value not copied anywhere.
- `/opt/homebrew/bin/npx` exists (symlink into `../Cellar/node/26.7.0/bin/npx`).
- `mcp-remote` version `0.14.2` is present in the `~/.npm/_npx` cache, consistent with note (c).

Taken from the task text and **not** re-verified here, because each needs the HA host or a live Claude Desktop session:

- that `http://192.168.1.171/api/mcp` answers, is Streamable HTTP and stateless, and reports `home-assistant 1.26.0` / protocol `2025-06-18`;
- that `tools/list` returns exactly 23 tools and none for the `number` domain; the three tool names quoted (`intent__HassTurnOn`, `intent__HassTurnOff`, `homeassistant__GetLiveContext`) and the climate tool name;
- that the pad is exposed to HA as a template number plus a template switch;
- the four troubleshooting behaviours in §1.7, including that the OAuth error is HA answering 401 and that HA's discovery metadata advertises IndieAuth;
- that Claude Desktop's MCP launch `PATH` omits `/opt/homebrew/bin` (the config uses the absolute path, consistent with the claim, but the minimal `PATH` itself was not inspected);
- the claim that the Alexa RangeController limitation is the "same class" — accepted from the task; the finding itself could not be read.
