// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* char_screen_fixture.c -- src/ui/char_screen.c を試すテストの足場
 * （put_misc3_test）
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
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_bio.h"
#include "player_body_weight.h"
#include "player_class.h"
#include "player_disarm.h"
#include "player_level.h"
#include "player_race.h"
#include "player_saving_throw.h"
#include "player_search_skill.h"
#include "player_speed.h"
#include "player_spells_to_learn.h"
#include "player_status_flags.h"
#include "player_stealth.h"
#include "player_timed_effects.h"
#include "stats_reset.h"

/* 各テストの前に呼ぶ。 */
void fixture_reset(void)
{
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
    player_speed_set(0);
    player_spells_to_learn_set(0);
    player_base_to_hit_set(0, 0);
    player_disarm_set(0);
    player_saving_throw_set(0);
    player_race_set(0);
    player_body_weight_set(0);
    player_attack_bonuses_set(0, 0);
    player_search_chance_set(0);
    player_search_frequency_set(0);
    player_name_set("");
    player_set_male(false);
    player_age_set(0);
    player_height_set(0);
    player_social_class_set(0);
    player_history_clear();
    player_stealth_set(0);
    player_class_set(0);
    shared_stubs_reset();
}
