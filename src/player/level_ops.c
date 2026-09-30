// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Going up a level: the level itself with its hit points, the experience
// display that climbs to it, and the hit points worked out for the level
//
// Moved out of misc3.c unchanged (#42), where they were the last of it; their
// prototypes stay in externs.h.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "hp_table.h"
#include "player_class.h"
#include "player_hp.h"
#include "player_level.h"
#include "player_status_flags.h"
#include "screen_fields.h"
#include "stats.h"

// Increases hit points and level -RAK-
static void gain_level(void) {
    player_advance_level();

    vtype out_val;
    (void)sprintf(out_val, "Welcome to level %d.", (int)player_level());
    msg_print(out_val);
    calc_hitpoints();

    // lose some of the 'extra' exp when gaining several levels at once
    player_trim_surplus_experience();

    prt_level();
    prt_title();

    if (player_class_spell_type() == MAGE) {
        calc_spells(A_INT);
        calc_mana(A_INT);
    } else if (player_class_spell_type() == PRIEST) {
        calc_spells(A_WIS);
        calc_mana(A_WIS);
    }
}

// Prints experience -RAK-
void prt_experience(void) {
    (void)player_cap_experience();

    while (player_deserves_next_level()) {
        gain_level();
    }

    (void)player_record_max_experience();

    prt_long(player_experience(), 14, STAT_COLUMN + 6);
}

// Calculate the players hit points
void calc_hitpoints(void) {
    int hitpoints = hp_total_at_level(player_level()) + (con_adj() * player_level());

    // always give at least one point per level + 1
    if (hitpoints < (player_level() + 1)) {
        hitpoints = player_level() + 1;
    }

    if (player_effect_in_force(PLAYER_EFFECT_HERO)) {
        hitpoints += 10;
    }

    if (player_effect_in_force(PLAYER_EFFECT_SUPER_HERO)) {
        hitpoints += 20;
    }

    // The window carries what is left across in proportion and says whether
    // the maximum moved. A maximum of zero means the character is still being
    // created, and the window leaves it completely alone -- so the flag is not
    // raised then either.
    if (player_change_max_hp((int16_t)hitpoints)) {
        // can't print hit points here, may be in store or inventory mode
        player_request_hp_redraw();
    }
}
