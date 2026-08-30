/*
 * Host unit test for the relative-temperature tables/helpers in
 * components/dial_state/dial_state.h. No ESP-IDF, no LVGL — dial_state.h's
 * relative section is self-contained (stdbool/stdint/stdio/math), so this
 * compiles with a plain host compiler:
 *
 *   cc -I components/dial_state -o /tmp/test_dial_rel test/test_dial_rel.c -lm && /tmp/test_dial_rel
 *
 * It pins the invariants the whole feature rests on:
 *  - every level's carrier (tenths of °C) maps back to that level
 *    (dial_rel_from_dc round-trip)
 *  - each carrier's wire °C lands strictly inside that level's Celsius
 *    bracket, so a device poll can never move the displayed level
 *  - dial_rel_step moves exactly one level in the turned direction (on-grid and
 *    off-grid), and pins at the rails
 *  - both tables are strictly increasing and correctly interleaved
 *
 * 2026-08-30 units fix: the carrier/boundary tables are now tenths of °C
 * (dc) directly — the canonical unit everywhere in the firmware — not whole
 * °F. There is no longer a store-carrier round-trip to test (the old
 * "dial_c_to_f(dial_f_to_c(f)) == f" check): dc IS the canonical unit, so
 * nothing downstream ever converts it through anything lossy. The table
 * values themselves are unchanged in substance — a straight unit conversion
 * of the old whole-°F table, not a re-derivation — so the Orion-bracket and
 * off-grid-stepping checks below still hold against the same real-world
 * temperatures the old test used (71°F ↔ 21.7°C = 217dc).
 *
 * The glyph-presence side (the spliced '+' in dial_font_num_88) is verified
 * visually by the simulator's dial-relative scenario, which renders "+2".
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dial_state.h"

// Orion's published relative table (temperature_scale.relative): level -> wire °C.
static const float ORION_REL_C[21] = {
    10.0f, 12.0f, 14.0f, 16.0f, 17.5f, 19.0f, 20.5f, 23.0f, 24.5f, 26.0f, 27.5f,
    29.0f, 30.5f, 32.0f, 33.5f, 35.0f, 37.0f, 39.0f, 41.0f, 43.0f, 45.0f
};

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); failures++; } } while (0)

int main(void)
{
    // Carriers self-consistent: each level's dc reads back as that level.
    for (int L = DIAL_REL_MIN; L <= DIAL_REL_MAX; L++) {
        int dc = dial_rel_to_dc(L);
        CHECK(dial_rel_from_dc(dc) == L, "level %d carrier %ddc reads back as %d", L, dc, dial_rel_from_dc(dc));
    }

    // Each carrier's wire °C lands strictly inside its level's Celsius bracket
    // (midpoints of Orion's own table) -> a poll can never move the level.
    for (int L = DIAL_REL_MIN; L <= DIAL_REL_MAX; L++) {
        float c  = dial_rel_to_dc(L) / 10.0f;
        float lo = (L > DIAL_REL_MIN) ? (ORION_REL_C[L - 1 - DIAL_REL_MIN] + ORION_REL_C[L - DIAL_REL_MIN]) / 2.0f : -1000.0f;
        float hi = (L < DIAL_REL_MAX) ? (ORION_REL_C[L - DIAL_REL_MIN] + ORION_REL_C[L + 1 - DIAL_REL_MIN]) / 2.0f :  1000.0f;
        CHECK(c > lo && c < hi, "level %d wire %.1f°C not strictly inside bracket %.2f..%.2f", L, c, lo, hi);
    }

    // Orion's own level->°C, bucketed by our nearest-level function, must agree.
    for (int i = 0; i < 21; i++) {
        int dc  = (int)lroundf(ORION_REL_C[i] * 10.0f);
        int got = dial_rel_from_dc(dc);
        CHECK(got == i + DIAL_REL_MIN, "Orion level %d (%.1f°C) buckets to our %d", i + DIAL_REL_MIN, ORION_REL_C[i], got);
    }

    // Tables strictly increasing.
    for (int i = 0; i < 20; i++)
        CHECK(DIAL_REL_DC[i] < DIAL_REL_DC[i + 1], "DIAL_REL_DC not strictly increasing at %d", i);
    for (int i = 0; i < 19; i++)
        CHECK(DIAL_REL_LO_DC[i] < DIAL_REL_LO_DC[i + 1], "DIAL_REL_LO_DC not strictly increasing at %d", i);

    // dial_rel_step: one detent = exactly one level in the turned direction.
    // On-grid neutral.
    CHECK(dial_rel_from_dc(dial_rel_step(dial_rel_to_dc(0),  1)) ==  1, "step +1 from level 0");
    CHECK(dial_rel_from_dc(dial_rel_step(dial_rel_to_dc(0), -1)) == -1, "step -1 from level 0");
    // Off-grid (217dc / 21.7°C, the same real temperature the pre-fix test's
    // 71°F exercised): must move to -3 warm / -5 cool, never stall.
    CHECK(dial_rel_from_dc(217) == -4, "217dc (21.7°C) should display as level -4");
    CHECK(dial_rel_from_dc(dial_rel_step(217,  1)) == -3, "step +1 from off-grid 217dc");
    CHECK(dial_rel_from_dc(dial_rel_step(217, -1)) == -5, "step -1 from off-grid 217dc");
    CHECK(dial_rel_step(217,  1) != 217, "a warm detent must change the carrier (no move-bed-without-number)");
    CHECK(dial_rel_step(217, -1) != 217, "a cool detent must change the carrier");
    // Rails pin: stepping past an end returns the same carrier (caller's range stop).
    CHECK(dial_rel_step(dial_rel_to_dc(DIAL_REL_MAX),  1) == dial_rel_to_dc(DIAL_REL_MAX), "warm past +10 pins");
    CHECK(dial_rel_step(dial_rel_to_dc(DIAL_REL_MIN), -1) == dial_rel_to_dc(DIAL_REL_MIN), "cool past -10 pins");
    // Multi-detent.
    CHECK(dial_rel_from_dc(dial_rel_step(dial_rel_to_dc(0), 3)) == 3, "step +3 from level 0");
    CHECK(dial_rel_from_dc(dial_rel_step(dial_rel_to_dc(0), -100)) == DIAL_REL_MIN, "big cool step clamps to -10");

    // Relative rails equal the device's advertised range endpoints.
    CHECK(dial_rel_to_dc(DIAL_REL_MIN) == DIAL_REL_MIN_DC, "level -10 carrier == DIAL_REL_MIN_DC");
    CHECK(dial_rel_to_dc(DIAL_REL_MAX) == DIAL_REL_MAX_DC, "level +10 carrier == DIAL_REL_MAX_DC");
    CHECK(DIAL_REL_MIN_DC == 100, "100dc rail == 10.0°C exactly");
    CHECK(DIAL_REL_MAX_DC == 450, "450dc rail == 45.0°C exactly");

    if (failures) { printf("\n%d assertion(s) FAILED\n", failures); return 1; }
    printf("all relative-scale table assertions passed\n");
    return 0;
}
