// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What happens to the player when hurt: losing hit points, damage by element,
// worn armour, and the saving throw

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "equipment.h"
#include "level_exit.h"
#include "player_abilities.h"
#include "player_class.h"
#include "player_hp.h"
#include "player_level.h"
#include "player_saving_throw.h"
#include "player_timed_effects.h"
#include "score_death.h"
#include "stats.h"

// Saving throws for player character. -RAK-
bool player_saves(void) {
    // MPW C couldn't handle the expression, so split it into two parts
    int16_t temp = class_level_adj[player_class()][CLA_SAVE];

    // The number comes from the window, the roll is made here (#18-12-21B).
    // The window answers "how good are they at resisting?", and this function
    // answers "did they resist this time?". The roll needs both randint() and
    // temp above (the table by class and level), so it stays out of the window.
    if (randint(100) <= (player_saving_throw() + stat_adj(A_WIS) + (temp * player_level() / 3))) {
        return true;
    } else {
        return false;
    }
}

// Decreases players hit points and sets death flag if necessary -RAK-
void take_hit(int damage, const char *hit_from) {
    if (player_timed_in_force(PLAYER_TIMED_INVULNERABILITY)) {
        damage = 0;
    }
    // Nothing clamps the number at zero: a fatal wound leaves it negative on
    // purpose, because that is the only record of the death anyone keeps.
    if (player_take_hp_damage(damage)) {
        if (!player_is_dead()) {
            set_player_dead(true);
            (void)strcpy(death_cause(), hit_from);
            set_player_has_won(false);
        }
        end_level();
    } else {
        prt_chp();
    }
}

// AC gets worse -RAK-
// Note: This routine affects magical AC bonuses so
// that stores can detect the damage.
int minus_ac(uint32_t typ_dam) {
    int tmp[6];
    int i = 0;
    if (equipment_at(INVEN_BODY)->tval != TV_NOTHING) {
        tmp[i] = INVEN_BODY;
        i++;
    }
    if (equipment_at(INVEN_ARM)->tval != TV_NOTHING) {
        tmp[i] = INVEN_ARM;
        i++;
    }
    if (equipment_at(INVEN_OUTER)->tval != TV_NOTHING) {
        tmp[i] = INVEN_OUTER;
        i++;
    }
    if (equipment_at(INVEN_HANDS)->tval != TV_NOTHING) {
        tmp[i] = INVEN_HANDS;
        i++;
    }
    if (equipment_at(INVEN_HEAD)->tval != TV_NOTHING) {
        tmp[i] = INVEN_HEAD;
        i++;
    }
    // also affect boots
    if (equipment_at(INVEN_FEET)->tval != TV_NOTHING) {
        tmp[i] = INVEN_FEET;
        i++;
    }

    bool minus = false;

    if (i > 0) {
        int j = tmp[randint(i) - 1];

        inven_type *i_ptr = equipment_at(j);

        msgtype out_val;
        bigvtype tmp_str;
        if (i_ptr->flags & typ_dam) {
            objdes(tmp_str, equipment_at(j), false);
            (void)snprintf(out_val, sizeof(out_val), "Your %s resists damage!", tmp_str);
            msg_print(out_val);
            minus = true;
        } else if ((i_ptr->ac + i_ptr->toac) > 0) {
            objdes(tmp_str, equipment_at(j), false);
            (void)snprintf(out_val, sizeof(out_val), "Your %s is damaged!", tmp_str);
            msg_print(out_val);
            i_ptr->toac--;
            calc_bonuses();
            minus = true;
        }
    }
    return minus;
}

// Corrode the unsuspecting person's armor -RAK-
void corrode_gas(const char *kb_str) {
    if (!minus_ac((uint32_t)TR_RES_ACID)) {
        take_hit(randint(8), kb_str);
    }

    if (inven_damage(set_corrodes, 5) > 0) {
        msg_print("There is an acrid smell coming from your pack.");
    }
}

// Poison gas the idiot. -RAK-
void poison_gas(int dam, const char *kb_str) {
    take_hit(dam, kb_str);
    player_timed_add(PLAYER_TIMED_POISON, 12 + randint(dam));
}

// Burn the fool up. -RAK-
void fire_dam(int dam, const char *kb_str) {
    if (player_resists_fire()) {
        dam = dam / 3;
    }
    if (player_timed_in_force(PLAYER_TIMED_HEAT_RESISTANCE)) {
        dam = dam / 3;
    }
    take_hit(dam, kb_str);
    if (inven_damage(set_flammable, 3) > 0) {
        msg_print("There is smoke coming from your pack!");
    }
}

// Freeze him to death. -RAK-
void cold_dam(int dam, char *kb_str) {
    if (player_resists_cold()) {
        dam = dam / 3;
    }
    if (player_timed_in_force(PLAYER_TIMED_COLD_RESISTANCE)) {
        dam = dam / 3;
    }
    take_hit(dam, kb_str);
    if (inven_damage(set_frost_destroy, 5) > 0) {
        msg_print("Something shatters inside your pack!");
    }
}

// Lightning bolt the sucker away. -RAK-
void light_dam(int dam, char *kb_str) {
    if (player_resists_light()) {
        take_hit((dam / 3), kb_str);
    } else {
        take_hit(dam, kb_str);
    }
    if (inven_damage(set_lightning_destroy, 3) > 0) {
        msg_print("There are sparks coming from your pack!");
    }
}

// Throw acid on the hapless victim -RAK-
void acid_dam(int dam, const char *kb_str) {
    int flag = 0;
    if (minus_ac((uint32_t)TR_RES_ACID)) {
        flag = 1;
    }
    if (player_resists_acid()) {
        flag += 2;
    }
    take_hit(dam / (flag + 1), kb_str);
    if (inven_damage(set_acid_affect, 3) > 0) {
        msg_print("There is an acrid smell coming from your pack!");
    }
}
