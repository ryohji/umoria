// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* spellbook_fixture.c -- src/item/spellbook.c を試すテストの足場
 * （calc_spells_test・gain_spells_test）
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
#include "player_class.h"
#include "player_level.h"
#include "player_spells_to_learn.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"
#include "stats_reset.h"

/* 各テストの前に呼ぶ。 */
void fixture_reset(void)
{
    memset(inventory_and_equipment_at(0), 0,
           sizeof(inven_type) * (size_t)inventory_and_equipment_slot_count());
    stats_reset();
    player_set_level(0);
    player_set_experience(0);
    player_set_max_experience(0);
    player_set_experience_fraction(0);
    player_set_experience_factor(0);
    player_set_status_word(0);
    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        player_timed_clear((player_timed_effect)effect);
    }
    player_spells_to_learn_set(0);
    player_class_set(0);
    inventory_set_count(0);
    inventory_set_weight(0);
    shared_stubs_reset();
}
