#pragma once
#include <stdbool.h>
#include <stddef.h>

/*
 * Subnet scan for the Somnus pad's address (docs/SPEC-pad-discovery.md).
 * The pad advertises no mDNS/SSDP (confirmed by direct test against real
 * hardware), so a brute-force scan of the dial's own subnet is the only
 * discovery mechanism available.
 *
 * Own probe path, deliberately NOT dial_somnus's: dial_somnus.h's client is
 * a single instance, non-reentrant, worker-task-only by explicit contract
 * ("not reentrant -- the worker is already single-threaded for network
 * calls") -- fundamentally incompatible with N concurrent probes against N
 * different candidate addresses. This component owns its own short-lived
 * esp_http_client instances instead, one per probe, matching dial_somnus.c's
 * own init/perform/cleanup shape just run concurrently across worker tasks.
 *
 * Threading: dial_pad_discovery_scan() is a blocking call, meant to be
 * called from main.c's worker_task (the same task that already blocks in
 * dial_somnus_connect() and backoff_wait() during the pre-PH_READY connect
 * loop) -- it internally spawns and joins its own short-lived helper tasks
 * and returns once a pass (or both passes) complete. Not reentrant itself;
 * only one scan is expected to run at a time.
 */

// True if a scan should run now. First call after boot always returns true
// (the first failure of the persisted address triggers exactly one scan
// immediately, per the spec's "Scan trigger" section); after that, gated
// behind a cooldown so a still-unreachable pad doesn't get the subnet swept
// on every retry. Call once per connect-loop failure, before deciding
// whether to scan.
bool dial_pad_discovery_should_attempt(void);

// Record that a scan attempt just happened (found or not) -- (re)starts the
// cooldown clock dial_pad_discovery_should_attempt() reads. Call once after
// every dial_pad_discovery_scan() call, regardless of its result.
void dial_pad_discovery_mark_attempted(void);

// Run the two-pass scan: an ordered candidate list (the persisted address's
// neighborhood, the dial's own DHCP neighborhood, the conventional
// static/low and DHCP-pool ranges, then everything else ascending -- see
// the spec's "Refinement 1"), swept once at a short timeout (fast path for
// a live, responsive pad) and, only if that finds nothing, swept again at a
// longer one (for a pad that's slow to answer). Every hit is validated by
// decoding the response into the pad's known /api/state JSON shape, never
// by trusting a 200 alone.
//
// `failed_url` is the address that just failed to connect (the caller's
// current dial_state_get_pad_url() value) -- used only to seed the
// persisted-address-neighborhood tier; not otherwise touched or persisted
// by this call. On a match, writes "http://<ip>:8080" into `out_url`
// (must be at least DIAL_PAD_URL_MAX_LEN+1 bytes) and returns true. Blocks
// the calling task for the duration of the scan -- seconds in the common
// case, up to ~57.6s worst case if nothing is found by either pass.
bool dial_pad_discovery_scan(const char *failed_url, char *out_url, size_t out_sz);
