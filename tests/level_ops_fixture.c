// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* level_ops_fixture.c -- src/player/level_ops.c を試すテストの足場
 * （calc_hitpoints_test）
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
#include "player_bio.h"
#include "player_class.h"
#include "player_level.h"
#include "player_race.h"
#include "player_speed.h"
#include "player_spells_to_learn.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"

/* 各テストの前に呼ぶ。 */
void fixture_reset(void)
{
    extern player_type py;

    memset(inventory_and_equipment_at(0), 0,
           sizeof(inven_type) * (size_t)inventory_and_equipment_slot_count());
    memset(&py, 0, sizeof py);
    player_set_level(0);
    player_set_experience(0);
    player_set_max_experience(0);
    player_set_experience_fraction(0);
    player_set_experience_factor(0);
    player_set_status_word(0);
    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        player_timed_clear((player_timed_effect)effect);
    }
    player_speed_set(0);
    player_spells_to_learn_set(0);
    player_race_set(0);
    player_name_set("");
    player_set_male(false);
    player_age_set(0);
    player_height_set(0);
    player_social_class_set(0);
    player_history_clear();
    player_class_set(0);
    inventory_set_count(0);
    inventory_set_weight(0);
    shared_stubs_reset();
}
