// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Every kind of the object table through magic_treasure() at many depths and
// with many seeds: each kind is put on the floor, given its magic at a depth,
// and every field of what came out (the special name, inscription, flags,
// values, plusses, damage, cost, number, weight, level, ident) goes into one
// hash per kind, printed with a total; make -f makefile.test golden compares
// them with tests/golden/magic_all.hashes.
#include "config.h"
#include "constant.h"
#include "types.h"
#include "dungeon_state.h"
#include "item_kinds.h"
#include "rnd.h"
#include "world_dungeon.h"
#include "world_item.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t h;
static void mix_bytes(const void *p, size_t n) {
    const uint8_t *b = p;
    for (size_t i = 0; i < n; i++) h = (h ^ b[i]) * 16777619u;
}
#define MIX(field) mix_bytes(&(field), sizeof(field))

int main(void) {
    magic_init();
    static const int levels[] = {0, 1, 5, 10, 15, 20, 30, 40, 50, 70, 100};
    enum { SEEDS = 40 };
    uint32_t total = 2166136261u;
    long count = 0;
    for (int o = 0; o < MAX_OBJECTS; o++) {
        h = 2166136261u;
        for (size_t l = 0; l < sizeof levels / sizeof levels[0]; l++)
        for (uint32_t seed = 1; seed <= SEEDS; seed++) {
            set_rnd_seed(seed * 104729u + (uint32_t)o);
            floor_items_reset();
            int x = popt();
            inven_type *t = floor_item_at(x);
            invcopy(t, o);
            magic_treasure(x, levels[l]);
            MIX(t->index); MIX(t->name2); mix_bytes(t->inscrip, strlen(t->inscrip));
            MIX(t->flags); MIX(t->tval); MIX(t->tchar); MIX(t->p1); MIX(t->cost);
            MIX(t->subval); MIX(t->number); MIX(t->weight); MIX(t->tohit); MIX(t->todam);
            MIX(t->ac); MIX(t->toac); MIX(t->damage); MIX(t->level); MIX(t->ident);
            count++;
        }
        printf("%3d %08x\n", o, h);
        total = (total ^ h) * 16777619u;
    }
    printf("total %08x (%ld calls)\n", total, count);
    return 0;
}
