// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The rolls of a blow: how many blows a weapon gives, the extra damage of a
// slaying weapon, and the critical hits
//
// Moved out of misc3.c unchanged (#42); their prototypes stay in externs.h.
// Not all of them are free of side effects: tot_dam() records what the player
// has learned about the monster, and critical_blow() prints a message.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "player_class.h"
#include "player_level.h"

// Weapon weight VS strength and dexterity -RAK-
int attack_blows(int weight, int *wtohit) {
    int s = py.stats.use_stat[A_STR];
    int d = py.stats.use_stat[A_DEX];

    if (s * 15 < weight) {
        *wtohit = s * 15 - weight;
        return 1;
    } else {
        int str_index, dex_index;

        *wtohit = 0;
        if (d < 10) {
            dex_index = 0;
        } else if (d < 19) {
            dex_index = 1;
        } else if (d < 68) {
            dex_index = 2;
        } else if (d < 108) {
            dex_index = 3;
        } else if (d < 118) {
            dex_index = 4;
        } else {
            dex_index = 5;
        }

        int adj_weight = (s * 10 / weight);
        if (adj_weight < 2) {
            str_index = 0;
        } else if (adj_weight < 3) {
            str_index = 1;
        } else if (adj_weight < 4) {
            str_index = 2;
        } else if (adj_weight < 5) {
            str_index = 3;
        } else if (adj_weight < 7) {
            str_index = 4;
        } else if (adj_weight < 9) {
            str_index = 5;
        } else {
            str_index = 6;
        }

        return (int)blows_table[str_index][dex_index];
    }
}

// Special damage due to magical abilities of object -RAK-
int tot_dam(inven_type *i_ptr, int tdam, creature_handle h) {
    if ((i_ptr->flags & TR_EGO_WEAPON) && (((i_ptr->tval >= TV_SLING_AMMO) && (i_ptr->tval <= TV_ARROW)) || ((i_ptr->tval >= TV_HAFTED) && (i_ptr->tval <= TV_SWORD)) || (i_ptr->tval == TV_FLASK))) {
        creature_type *const creature = monster_get_creature(h);

        if ((creature->cdefense & CD_DRAGON) && (i_ptr->flags & TR_SLAY_DRAGON)) {
            // Slay Dragon
            tdam = tdam * 4;
            recall_update_characteristics(h, CD_DRAGON);
        } else if ((creature->cdefense & CD_UNDEAD) && (i_ptr->flags & TR_SLAY_UNDEAD)) {
            // Slay Undead
            tdam = tdam * 3;
            recall_update_characteristics(h, CD_UNDEAD);
        } else if ((creature->cdefense & CD_ANIMAL) && (i_ptr->flags & TR_SLAY_ANIMAL)) {
            // Slay Animal
            tdam = tdam * 2;
            recall_update_characteristics(h, CD_ANIMAL);
        } else if ((creature->cdefense & CD_EVIL) && (i_ptr->flags & TR_SLAY_EVIL)) {
            // Slay Evil
            tdam = tdam * 2;
            recall_update_characteristics(h, CD_EVIL);
        } else if ((creature->cdefense & CD_FROST) && (i_ptr->flags & TR_FROST_BRAND)) {
            // Frost
            tdam = tdam * 3 / 2;
            recall_update_characteristics(h, CD_FROST);
        } else if ((creature->cdefense & CD_FIRE) && (i_ptr->flags & TR_FLAME_TONGUE)) {
            // Fire
            tdam = tdam * 3 / 2;
            recall_update_characteristics(h, CD_FIRE);
        }
    }
    return tdam;
}

// Critical hits, Nasty way to die. -RAK-
int critical_blow(int weight, int plus, int dam, int attack_type) {
    int critical = dam;

    // Weight of weapon, plusses to hit, and character level all
    // contribute to the chance of a critical
    if (randint(5000) <= (weight + 5 * plus + (class_level_adj[player_class()][attack_type] * player_level()))) {
        weight += randint(650);

        if (weight < 400) {
            critical = 2 * dam + 5;
            msg_print("It was a good hit! (x2 damage)");
        } else if (weight < 700) {
            critical = 3 * dam + 10;
            msg_print("It was an excellent hit! (x3 damage)");
        } else if (weight < 900) {
            critical = 4 * dam + 15;
            msg_print("It was a superb hit! (x4 damage)");
        } else {
            critical = 5 * dam + 20;
            msg_print("It was a *GREAT* hit! (x5 damage)");
        }
    }

    return critical;
}
