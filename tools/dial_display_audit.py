#!/usr/bin/env python3
"""
dial_display_audit.py -- READ-ONLY live logger for the °F setpoint display
audit (docs/REPORT-dial-display-audit.md).

Polls GET http://<host>:8080/api/state at 1 Hz and prints a line whenever
side0's target_t changes, plus a 30s heartbeat. NEVER writes to the pad:
this script issues GET requests only, nothing else. No POST /api/power,
no POST /api/target_t, anywhere in this file.

Pad host comes ONLY from --host or the PAD_HOST env var -- deliberately no
hardcoded default, even though the bench pad's current address (192.168.1.169)
is visible in this repo's bench-logs/*.log. Pass it explicitly:

    PAD_HOST=192.168.1.169 python3 tools/dial_display_audit.py
    python3 tools/dial_display_audit.py --host 192.168.1.169

Every printed line is also appended, verbatim and timestamped, to
docs/dial-audit-run.log. Ctrl-C exits cleanly and prints a summary of the
distinct side0 target_t values observed, in the order first seen.
"""
import argparse
import json
import os
import sys
import time
import urllib.error
import urllib.request
from datetime import datetime, timezone

POLL_PERIOD_S = 1.0
HEARTBEAT_PERIOD_S = 30.0
PORT = 8080

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOG_PATH = os.path.join(REPO_ROOT, "docs", "dial-audit-run.log")


def now_hms():
    return datetime.now().strftime("%H:%M:%S")


def app_should_show_f(c):
    # round(C * 9/5 + 32), Python's round() is banker's-rounding at exact
    # .5 (matches the report's own arithmetic, not lroundf's round-half-up;
    # every value in this audit's actual range lands clear of that edge
    # case anyway since target_t is on a 0.1C grid, not exactly *.x5 after
    # the *9/5 scale in the cases this script ever sees).
    return round(c * 9.0 / 5.0 + 32.0)


def fetch_state(host):
    url = f"http://{host}:{PORT}/api/state"
    req = urllib.request.Request(url, method="GET")
    with urllib.request.urlopen(req, timeout=5) as resp:
        body = resp.read()
    return json.loads(body)


def log_line(line):
    print(line)
    os.makedirs(os.path.dirname(LOG_PATH), exist_ok=True)
    with open(LOG_PATH, "a") as f:
        f.write(line + "\n")


def format_sample(target_t, current_t, on):
    level = target_t - 27.0
    f = app_should_show_f(target_t)
    return (f"{now_hms()}  target_t={target_t:g}C  level={level:g}  "
            f"app_should_show={f}F  current_t={current_t if current_t is not None else 'null'}C  "
            f"on={on}")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--host", default=None,
                     help="Pad IP/hostname (no port). Falls back to PAD_HOST env var. "
                          "No default -- must be supplied.")
    ap.add_argument("--once", action="store_true",
                     help="Fetch one sample, print it, and exit (self-test / verification mode).")
    args = ap.parse_args()

    host = args.host or os.environ.get("PAD_HOST")
    if not host:
        print("ERROR: pad host not given. Use --host <ip> or set PAD_HOST.", file=sys.stderr)
        print("(bench-logs/*.log shows this dial's pad at 192.168.1.169 as of "
              "2026-09-06 -- verify before reuse, pad addresses change with DHCP.)",
              file=sys.stderr)
        sys.exit(2)

    prev_target = None
    observed_order = []
    last_heartbeat = 0.0

    def handle_sample():
        nonlocal prev_target, last_heartbeat
        try:
            state = fetch_state(host)
        except urllib.error.URLError as e:
            log_line(f"{now_hms()}  FAIL  http error: {e}")
            return
        except (ValueError, json.JSONDecodeError) as e:
            log_line(f"{now_hms()}  FAIL  parse error: {e}")
            return
        except Exception as e:  # keep polling no matter what
            log_line(f"{now_hms()}  FAIL  unexpected error: {e}")
            return

        try:
            side0 = state["side0"]
            target_t = float(side0["target_t"])
            current_t = side0.get("current_t")
            current_t = float(current_t) if current_t is not None else None
            on = bool(side0["is_on"])
        except (KeyError, TypeError, ValueError) as e:
            log_line(f"{now_hms()}  FAIL  unexpected /api/state shape: {e}")
            return

        now = time.monotonic()
        if prev_target is None or target_t != prev_target:
            log_line(format_sample(target_t, current_t, on))
            if target_t not in observed_order:
                observed_order.append(target_t)
            prev_target = target_t
            last_heartbeat = now
        elif now - last_heartbeat >= HEARTBEAT_PERIOD_S:
            log_line("(heartbeat) " + format_sample(target_t, current_t, on))
            last_heartbeat = now

    if args.once:
        handle_sample()
        return

    print(f"Polling GET http://{host}:{PORT}/api/state at 1 Hz. "
          f"Logging to {LOG_PATH}. Ctrl-C to stop.")
    try:
        while True:
            start = time.monotonic()
            handle_sample()
            elapsed = time.monotonic() - start
            time.sleep(max(0.0, POLL_PERIOD_S - elapsed))
    except KeyboardInterrupt:
        print()
        print(f"Stopped. {len(observed_order)} distinct side0 target_t value(s) observed, "
              f"in order: {observed_order}")


if __name__ == "__main__":
    main()
