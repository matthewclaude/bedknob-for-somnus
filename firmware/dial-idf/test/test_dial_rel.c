/*
 * Host unit test for the relative-temperature scale in
 * components/dial_state/dial_state.h. No ESP-IDF, no LVGL — dial_state.h's
 * relative section is self-contained (stdbool/stdint/stdio/math), so this
 * compiles with a plain host compiler:
 *
 *   cc -I components/dial_state -o /tmp/test_dial_rel test/test_dial_rel.c -lm && /tmp/test_dial_rel
 *
 * It pins the invariants the whole feature rests on:
 *  - every level's carrier (tenths of °C) maps back to that level
 *    (dial_rel_from_dc round-trip)
 *  - dial_rel_to_dc is strictly increasing in level
 *  - dial_rel_step moves exactly one level in the turned direction (on-grid and
 *    off-grid), and pins at the rails
 *  - every carrier stays inside the Somnus API's accepted range
 *
 * 2026-09-01 relative-scale fix (docs/SPEC-somnus-relative-scale.md): the
 * −10…+10 DIAL_REL_DC/DIAL_REL_LO_DC lookup tables this file used to check
 * are gone, replaced by uniform 1.0°C/level arithmetic over a −15…+15 range.
 * The old version of this test cross-checked the dial's levels against an
 * ORION_REL_C[] reference table — i.e. it asserted agreement with the
 * UPSTREAM Orion product's scale, not the Somnus app actually driving this
 * pad. That was precisely the wrong invariant, and it passing the whole time
 * is why the mismatch between this dial and the Somnus app survived
 * unnoticed. There is no reference table anymore: the scale IS the spec now,
 * pinned directly against the three real-pad measurements in
 * docs/SPEC-somnus-relative-scale.md instead of against Orion's table.
 */
#include <stdio.h>
#include <stdlib.h>
#include "dial_state.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); failures++; } } while (0)

int main(void)
{
    // Round trip: every level's carrier reads back as that level.
    for (int L = DIAL_REL_MIN; L <= DIAL_REL_MAX; L++) {
        int dc = dial_rel_to_dc(L);
        CHECK(dial_rel_from_dc(dc) == L, "level %d carrier %ddc reads back as %d", L, dc, dial_rel_from_dc(dc));
    }

    // dial_rel_to_dc strictly increasing in level.
    for (int L = DIAL_REL_MIN; L < DIAL_REL_MAX; L++)
        CHECK(dial_rel_to_dc(L) < dial_rel_to_dc(L + 1), "dial_rel_to_dc not strictly increasing at level %d", L);

    // Every level's carrier is within the Somnus API's accepted range.
    for (int L = DIAL_REL_MIN; L <= DIAL_REL_MAX; L++) {
        int dc = dial_rel_to_dc(L);
        CHECK(dc >= 120 && dc <= 423, "level %d carrier %ddc outside API range 120..423", L, dc);
    }

    // The three measured points (docs/SPEC-somnus-relative-scale.md), plus
    // the level-0 midpoint they imply.
    CHECK(dial_rel_to_dc(-15) == 120, "level -15 should be 120dc (12.0C)");
    CHECK(dial_rel_to_dc(-6)  == 210, "level -6 should be 210dc (21.0C)");
    CHECK(dial_rel_to_dc(15)  == 420, "level +15 should be 420dc (42.0C)");
    CHECK(dial_rel_to_dc(0)   == 270, "level 0 should be 270dc (27.0C)");

    // dial_rel_step: one detent = exactly one level in the turned direction.
    // On-grid neutral.
    CHECK(dial_rel_from_dc(dial_rel_step(dial_rel_to_dc(0),  1)) ==  1, "step +1 from level 0");
    CHECK(dial_rel_from_dc(dial_rel_step(dial_rel_to_dc(0), -1)) == -1, "step -1 from level 0");
    // Off-grid: 217dc is (217-270)/10 = -5.3, i.e. level -5. Stepping from
    // there must move to -4 warm / -6 cool, never stall, and must always
    // change the carrier (no move-bed-without-number, no move-number-without-bed).
    CHECK(dial_rel_from_dc(217) == -5, "217dc (21.7°C) should display as level -5");
    CHECK(dial_rel_from_dc(dial_rel_step(217,  1)) == -4, "step +1 from off-grid 217dc");
    CHECK(dial_rel_from_dc(dial_rel_step(217, -1)) == -6, "step -1 from off-grid 217dc");
    CHECK(dial_rel_step(217,  1) != 217, "a warm detent must change the carrier (no move-bed-without-number)");
    CHECK(dial_rel_step(217, -1) != 217, "a cool detent must change the carrier");
    // Rails pin: stepping past an end returns the same carrier (caller's range stop).
    CHECK(dial_rel_step(dial_rel_to_dc(DIAL_REL_MAX),  1) == dial_rel_to_dc(DIAL_REL_MAX), "warm past +15 pins");
    CHECK(dial_rel_step(dial_rel_to_dc(DIAL_REL_MIN), -1) == dial_rel_to_dc(DIAL_REL_MIN), "cool past -15 pins");
    // Multi-detent, and large steps clamp rather than wrap.
    CHECK(dial_rel_from_dc(dial_rel_step(dial_rel_to_dc(0), 3)) == 3, "step +3 from level 0");
    CHECK(dial_rel_from_dc(dial_rel_step(dial_rel_to_dc(0), -100)) == DIAL_REL_MIN, "big cool step clamps to -15, not wraps");
    CHECK(dial_rel_from_dc(dial_rel_step(dial_rel_to_dc(0), 100)) == DIAL_REL_MAX, "big warm step clamps to +15, not wraps");

    // Relative rails equal the device's advertised range endpoints.
    CHECK(dial_rel_to_dc(DIAL_REL_MIN) == DIAL_REL_MIN_DC, "level -15 carrier == DIAL_REL_MIN_DC");
    CHECK(dial_rel_to_dc(DIAL_REL_MAX) == DIAL_REL_MAX_DC, "level +15 carrier == DIAL_REL_MAX_DC");
    CHECK(DIAL_REL_MIN_DC == 120, "120dc rail == 12.0°C exactly");
    CHECK(DIAL_REL_MAX_DC == 420, "420dc rail == 42.0°C exactly");

    if (failures) { printf("\n%d assertion(s) FAILED\n", failures); return 1; }
    printf("all relative-scale table assertions passed\n");
    return 0;
}
