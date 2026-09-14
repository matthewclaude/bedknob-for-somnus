# Spec: Home integration — Home Assistant MCP server (Assist)

Status: **Created 2026-09-14 as a new file. Documents the Home Assistant Model Context Protocol server integration completed that day. Documentation only; no firmware, app or HA configuration change is specified here.** The task that produced this file asked for a new section to be *added* to an existing `docs/SPEC-home-integration.md` and for cross-references to an Alexa RangeController finding and to an open write-coordination question in a "§7.5". No file by this name exists in this repo or anywhere in its git history (all refs checked 2026-09-14), and no other file in this repo records the Alexa finding or that section. The referenced material is therefore listed in §2 as *not present here*, not paraphrased. If the earlier spec lives in the Claude Project (as `V1-scope.md` and the other project-only files do), §1 is written so it can be moved under that file's numbering unchanged.

> **Note for an on-disk reader:** `V1-scope.md`, `START-HERE.md`, `HARDWARE-bringup-log.md`, `LICENSING.md` and `somnus-dial-project-summary.md` are **not in this repo** — they live only in the Claude Project. Everything this spec needs is restated here.

## 1. Home Assistant MCP server, reached from Claude Desktop (2026-09-14)

### 1.1 Server side

- Integration: Home Assistant's built-in **"Model Context Protocol Server"** integration, service = **Assist**. The tools it exposes are Assist's intents, not the HA REST or WebSocket API.
- Endpoint: `http://192.168.1.171/api/mcp`. Transport: **Streamable HTTP**, **stateless**.
- Server identifies as `home-assistant` **1.26.0**; MCP protocol version **2025-06-18**.
- Auth: an HA **long-lived access token**, sent as `Authorization: Bearer <token>`.

### 1.2 Client side: Claude Desktop on the Mac, bridged by mcp-remote

Claude Desktop speaks **stdio only** to MCP servers, so the HTTP endpoint is bridged with **`mcp-remote@0.14.2`**. The config lives at `~/Library/Application Support/Claude/claude_desktop_config.json`:

```json
"home-assistant": {
  "command": "/opt/homebrew/bin/npx",
  "args": [
    "-y",
    "mcp-remote@0.14.2",
    "http://192.168.1.171/api/mcp",
    "--allow-http",
    "--transport", "http-only",
    "--header", "Authorization:${HA_TOKEN}"
  ],
  "env": {
    "HA_TOKEN": "Bearer <token>"
  }
}
```

Three things in that block are load-bearing:

1. **`command` must be the absolute path `/opt/homebrew/bin/npx`.** Claude Desktop launches MCP servers with a minimal `PATH` that omits `/opt/homebrew/bin`, so a bare `npx` is not found. (`/opt/homebrew/bin/npx` is a symlink into the Homebrew `node` cellar; checked 2026-09-14.)
2. **Flags:** `--allow-http` (the endpoint is plain HTTP on the LAN), `--transport http-only` (skip the SSE fallback; HA is Streamable HTTP), `--header Authorization:${HA_TOKEN}`.
3. **The token rides in `env` as `HA_TOKEN="Bearer <token>"`**, whole header value included. mcp-remote performs the `${HA_TOKEN}` substitution in its own arguments itself; it is not shell expansion, and the config file never holds the bare token in `args`.

The `-y` in `args` is what is on disk on 2026-09-14; it lets the first-run `npx` download proceed without a prompt.

### 1.3 LAN-only by design

The server is reachable only on the home LAN. There is **no Nabu Casa** remote access, **no public exposure** of the endpoint, and **no claude.ai remote connector** — the only client is Claude Desktop on a Mac on the same network. This follows the standing local-control goal for the whole Somnus setup: the dial, the companions and now the MCP client all talk to the pad and to HA without leaving the house.

### 1.4 Constraint: no number-domain tool, so the pad setpoint is out of reach

`tools/list` against this server returns **23 tools** and contains **no tool for the `number` domain**. Assist has no set-value intent for `number` entities. The Somnus pad setpoint is exposed to HA as a **template number entity**, so **the setpoint cannot be set over MCP**. Reads and on/off still work:

| Want | Over MCP | Tool |
|---|---|---|
| Read pad state (setpoint, water, on/off) | yes | `homeassistant__GetLiveContext` |
| Turn the pad side on / off (template switch) | yes | `intent__HassTurnOn` / `intent__HassTurnOff` |
| Set the pad setpoint (template number) | **no** | none — no `number` tool |

This is the same class of limitation as the **Alexa RangeController** finding: a voice/assistant front end whose intent set has no verb for a numeric setpoint on this entity type. That finding is **not in this repo** (see §2); when it is reachable from here, cross-reference it from this paragraph rather than restating its analysis.

### 1.5 Possible future path — UNVERIFIED, not to be implemented

If the pad were exposed to HA as a **`climate` entity** instead of (or in addition to) the template number, Assist's `climate__HassClimateSetTemperature` tool would presumably apply, and the setpoint would become settable over MCP. **This is unverified.** Nothing has been tried: no climate entity has been defined, and it is not known whether a template climate entity with only a target temperature (no HVAC mode, no current-temperature sensor beyond water) satisfies that intent's expectations. Recorded as an option only. Do not implement it on the strength of this note.

### 1.6 Write coordination: a fourth writer

An MCP client (Claude Desktop, and anything else that may bridge to this server later) is now a **potential fourth writer** to the pad, alongside:

1. the dial (`dial_somnus`, `POST /api/...` on knob and touch);
2. Bedknob for Mac;
3. Home Assistant itself (automations and the HA UI acting on the template switch and template number).

The MCP writes differ in shape from the other three: they are **manual and on-demand** — a person asks Claude Desktop to turn the bed on or off — not scheduled and not polled, so they will not collide with anything on a timer. They still change pad state that the dial only learns about on its next poll (`docs/SPEC-standby-poll.md`: 300 s in STANDBY), exactly like a Bedknob for Mac or HA write.

The task that produced this section asked for this to be linked to "the existing open write-coordination question in section 7.5" of this spec. **There is no §7.5 in this repo** and no in-repo file records that question (see §2). Until it is reachable from here, treat this subsection as the in-repo pointer. The question, as far as this repo can state it, is how the writers are meant to coordinate when more than one acts on the pad inside one poll interval; it is **open**, and adding a fourth writer widens it rather than answers it.

### 1.7 Troubleshooting notes (kept because each one cost time)

- **(a) `npx mcp-remote --help` lies.** Its usage line is stale and omits `--header`, `--transport` and `--allow-http`. The flags exist. Verify them by fetching the package with `npm pack mcp-remote@0.14.2` and grepping `package/dist` for the flag names, not by trusting `--help`.
- **(b) "Incompatible auth server: does not support dynamic client registration" means 401, not an OAuth fault.** With an invalid or missing token, HA answers 401; mcp-remote then falls back to OAuth, reads HA's published OAuth discovery metadata, and fails because HA implements **IndieAuth**, not RFC 7591 dynamic client registration. Fix the token (and the `Bearer ` prefix in `HA_TOKEN`); do not chase the OAuth message.
- **(c) First launch may report "Server disconnected".** The first-run `npx` download of `mcp-remote` can overrun Claude Desktop's startup timeout. A second restart succeeds once the package is cached under `~/.npm/_npx`.
- **(d) Benign noise at connect time:** a `notifications/initialized` stack trace, and `Token result: Not found`. Both appear on a working connection and can be ignored.

## 2. Referenced material that is not in this repo

Checked on 2026-09-14 against the working tree, every local and remote ref (`origin/*`, `somnus/*`, `gh-pages`) and the full path history (`git log --all --name-only`):

- **A prior `docs/SPEC-home-integration.md`** — does not exist and never has in this repo. This file is new.
- **An "Alexa RangeController finding"** — no file in this repo mentions Alexa or RangeController. §1.4 names it as the analogous limitation and stops there.
- **"Section 7.5", an open write-coordination question** — no such section exists here. §1.6 carries the in-repo statement of the question instead.
- **The "no features before v1" rule** — the only in-repo record of it is `docs/SPEC-voice.md` §0.2, quoting the Sep 1 rule as "no new features until v1 is functional and a new user can easily add their pad." `somnus-v1.0.0` shipped on 2026-09-09 and stable is `somnus-v1.0.1`; this spec adds no feature to the firmware, the companions or HA, so the rule is not implicated on either count.

If the missing spec is in the Claude Project, the right move is to paste §1 into it under its own numbering and replace the two "not in this repo" sentences with real section references; nothing in §1 depends on the numbering used here.
