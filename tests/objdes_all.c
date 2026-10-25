// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Every kind of the object table through objdes() in many states: the kind
// known or not and tried or not, all 256 ident bytes, four sets of values (one
// is the kind's own), counts 0, 1 and 5, with and without a special name and
// an inscription, pref 0 and 1. Prints one hash per kind and a total; make -f
// makefile.test golden compares them with tests/golden/objdes_all.hashes.
// Run with any argument to also print a sample of the names.
#include "config.h"
#include "constant.h"
#include "types.h"
#include "world_item.h"
#include "item_kinds.h"
#include "item_knowledge.h"
#include "game_status.h"
#include "treasure.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static uint32_t h = 2166136261u;
static void mix(const char *s) { for (; *s; s++) h = (h ^ (uint8_t)*s) * 16777619u; h = (h ^ '\n') * 16777619u; }

int main(int argc, char **argv) {
    int verbose = argc > 1;
    magic_init();
    static const int vals[3][4] = {{-3, 2, -1, -2}, {0, 0, 0, 0}, {4, -5, 3, 7}};
    static const int numbers[3] = {0, 1, 5};
    static const int name2s[2] = {SN_NULL, 3};
    uint32_t total = 2166136261u;
    long count = 0;
    for (int o = 0; o < MAX_OBJECTS; o++) {
        h = 2166136261u;
        for (int known = 0; known < 2; known++)
        for (int tried = 0; tried < 2; tried++)
        for (int ident = 0; ident < 256; ident++)
        for (int v = 0; v < 4; v++)
        for (int n = 0; n < 3; n++)
        for (int s2 = 0; s2 < 2; s2++)
        for (int ins = 0; ins < 2; ins++)
        for (int pref = 0; pref < 2; pref++) {
            memset(item_kind_record_bytes(), 0, (size_t)item_kind_record_count());
            inven_type it;
            invcopy(&it, o);
            if (known) item_kind_mark_known(&it);
            if (tried) item_kind_mark_tried(&it);
            it.ident = (uint8_t)ident;
            if (v < 3) { // v == 3 keeps the values the kind is made with
                it.tohit = (int16_t)vals[v][0]; it.todam = (int16_t)vals[v][1];
                it.toac = (int16_t)vals[v][2]; it.p1 = (int16_t)vals[v][3];
            }
            it.number = (uint8_t)numbers[n];
            it.name2 = (uint8_t)name2s[s2];
            strcpy(it.inscrip, ins ? "abc" : "");
            bigvtype out;
            objdes(out, &it, pref);
            mix(out);
            if (verbose && ident == 0 && v == 2 && ins == 0) printf("%d %d %d %d %d %d: %s\n", o, known, tried, n, s2, pref, out);
            count++;
        }
        printf("obj %3d %08x\n", o, h);
        total = (total ^ h) * 16777619u;
    }
    printf("total %08x over %ld calls\n", total, count);
    return 0;
}
