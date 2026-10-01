// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* object_levels_fixture.c -- src/item/object_levels.c と
 * src/dungeon/object_alloc.c を試すテストの足場
 * （object_levels_test）
 *
 * fixture_reset() は、リンクされる窓口の状態と、代役（shared_stubs.c）の
 * 記録を 0 に戻す。
 */

#include <stddef.h>
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"
#include "shared_stubs.h"
#include "stub_unreached.h"
#include "inventory.h"
#include "item_ident.h"

/* set_large は訊かれた品物と回数を記録し、並べた答え（'y' で大きい）を順に
 * 返す。答えを使いきったら「大きくない」。 */
static const char *fixture_large_answers = "";
static const treasure_type *fixture_large_last;
static int fixture_large_calls;

bool set_large(treasure_type *t) {
    fixture_large_last = t;
    fixture_large_calls++;
    if (*fixture_large_answers == '\0') {
        return false;
    }
    return *fixture_large_answers++ == 'y';
}

void fixture_set_large_answers(const char *answers) {
    fixture_large_answers = answers == NULL ? "" : answers;
}

int fixture_set_large_call_count(void) { return fixture_large_calls; }
const treasure_type *fixture_set_large_last_item(void) { return fixture_large_last; }

/* set_large を代役にするので、同じ sets.o にある店の判定（tables.c の表が
 * 名前を持つだけで、呼ばれない）も代役にして、sets.o を引かない。 */
bool general_store(int t) { stub_unreached(__func__); }
bool armory(int t) { stub_unreached(__func__); }
bool weaponsmith(int t) { stub_unreached(__func__); }
bool temple(int t) { stub_unreached(__func__); }
bool alchemist(int t) { stub_unreached(__func__); }
bool magic_shop(int t) { stub_unreached(__func__); }

/* 各テストの前に呼ぶ。 */
void fixture_reset(void)
{
    memset(item_kind_record_bytes(), 0, (size_t)item_kind_record_count());
    memset(inventory_and_equipment_at(0), 0,
           sizeof(inven_type) * (size_t)inventory_and_equipment_slot_count());
    inventory_set_count(0);
    inventory_set_weight(0);
    fixture_large_answers = "";
    fixture_large_last = NULL;
    fixture_large_calls = 0;
    shared_stubs_reset();
}
