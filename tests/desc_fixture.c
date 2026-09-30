// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* desc_fixture.c -- src/item/desc.c を試すテストの足場
 * （objdes_test）
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
#include "inventory.h"
#include "item_ident.h"

/* 各テストの前に呼ぶ。 */
void fixture_reset(void)
{
    memset(item_kind_record_bytes(), 0, (size_t)item_kind_record_count());
    memset(inventory_and_equipment_at(0), 0,
           sizeof(inven_type) * (size_t)inventory_and_equipment_slot_count());
    inventory_set_count(0);
    inventory_set_weight(0);
    shared_stubs_reset();
}
