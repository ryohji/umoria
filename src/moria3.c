// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Misc code, mainly to handle player commands

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "floor_items.h"
#include "command_state.h"
#include "dungeon_level.h"
#include "dungeon_map.h"
#include "score_death.h"
#include "equipment.h"
#include "inventory.h"
#include "level_exit.h"
#include "monster_breeding.h"
#include "monster_list.h"
#include "monster_turn.h"
#include "panel.h"
#include "pending_teleport.h"
#include "player_abilities.h"
#include "player_armour_class.h"
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_class.h"
#include "player_disarm.h"
#include "player_glowing_hands.h"
#include "player_gold.h"
#include "player_level.h"
#include "player_mana.h"
#include "player_pos.h"
#include "player_search_skill.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"
#include "running.h"
#include "spells_known.h"
#include "stats.h"

// Deletes a monster entry from the level -RAK-
void delete_monster(int j) {
    fix1_delete_monster(j);
    fix2_delete_monster(j);
}

// The following two procedures implement the same function as delete monster.
// However, they are used within creatures(), because deleting a monster
// while scanning the monster list causes two problems, monsters might get two
// turns, and m_ptr/monptr might be invalid after the delete_monster.
// Hence the delete is done in two steps.
//
// fix1_delete_monster does everything delete_monster does except delete
// the monster record and take the mark back, this is called in breathe, and
// a couple of places in creatures.c
void fix1_delete_monster(int j) {
    monster_type *const m_ptr = monster_list_at(j);

    // force the hp negative to ensure that the monster is dead, for example,
    // if the monster was just eaten by another, it will still have positive
    // hit points
    m_ptr->hp = -1;
    square_at(m_ptr->fy, m_ptr->fx)->cptr = 0;
    if (m_ptr->ml) {
        lite_spot(m_ptr->fy, m_ptr->fx);
    }
    monster_breeding_note_death();
}

// fix2_delete_monster does everything in delete_monster that wasn't done
// by fix1_monster_delete above, this is only called in creatures()
void fix2_delete_monster(int j) {
    monster_type *const m_ptr = monster_list_at(j);
    monster_type *const the_last = monster_list_at(monster_list_used() - 1);

    if (m_ptr != the_last) {
        square_at(the_last->fy, the_last->fx)->cptr = j;
        *m_ptr = *the_last;
    }
    // Blank the row that was just copied away and take the mark back one
    monster_list_drop_last();
}

// Creates objects nearby the coordinates given -RAK-
static int summon_object(int y, int x, int num, int typ) {
    int real_typ;
    if ((typ == 1) || (typ == 5)) {
        real_typ = 1; // typ == 1 -> objects
    } else {
        real_typ = 256; // typ == 2 -> gold
    }

    int res = 0;

    do {
        int i = 0;

        do {
            int j = y - 3 + randint(5);
            int k = x - 3 + randint(5);

            if (in_bounds(j, k) && los(y, x, j, k)) {
                cave_type *c_ptr = square_at(j, k);

                if (c_ptr->fval <= MAX_OPEN_SPACE && (c_ptr->tptr == 0)) {
                    // typ == 3 -> 50% objects, 50% gold
                    if ((typ == 3) || (typ == 7)) {
                        if (randint(100) < 50) {
                            real_typ = 1;
                        } else {
                            real_typ = 256;
                        }
                    }
                    if (real_typ == 1) {
                        place_object(j, k, (typ >= 4));
                    } else {
                        place_gold(j, k);
                    }
                    lite_spot(j, k);
                    if (test_light(j, k)) {
                        res += real_typ;
                    }
                    i = 20;
                }
            }
            i++;
        } while (i <= 20);

        num--;
    } while (num != 0);

    return res;
}

// Deletes object from given location -RAK-
int delete_object(int y, int x) {
    cave_type *c_ptr = square_at(y, x);

    if (c_ptr->fval == BLOCKED_FLOOR) {
        c_ptr->fval = CORR_FLOOR;
    }
    pusht(c_ptr->tptr);
    c_ptr->tptr = 0;
    c_ptr->fm = false;
    lite_spot(y, x);

    bool delete;

    if (test_light(y, x)) {
        delete = true;
    } else {
        delete = false;
    }

    return delete;
}

// Allocates objects upon a creatures death -RAK-
// Oh well,  another creature bites the dust. Reward the
// victor based on flags set in the main creature record.
//
// Returns a mask of bits from the given flags which indicates what the
// monster is seen to have dropped.  This may be added to monster memory.
uint32_t monster_death(int y, int x, uint32_t flags) {
    int i;
    if (flags & CM_CARRY_OBJ) {
        i = 1;
    } else {
        i = 0;
    }
    if (flags & CM_CARRY_GOLD) {
        i += 2;
    }
    if (flags & CM_SMALL_OBJ) {
        i += 4;
    }

    int number = 0;
    if ((flags & CM_60_RANDOM) && (randint(100) < 60)) {
        number++;
    }
    if ((flags & CM_90_RANDOM) && (randint(100) < 90)) {
        number++;
    }
    if (flags & CM_1D2_OBJ) {
        number += randint(2);
    }
    if (flags & CM_2D2_OBJ) {
        number += damroll(2, 2);
    }
    if (flags & CM_4D2_OBJ) {
        number += damroll(4, 2);
    }

    uint32_t dump;
    if (number > 0) {
        dump = (uint32_t)summon_object(y, x, number, i);
    } else {
        dump = 0;
    }

    if (flags & CM_WIN) {
        // maybe the player died in mid-turn
        if (!player_is_dead()) {
            set_player_has_won(true);
            prt_winner();
            msg_print("*** CONGRATULATIONS *** You have won the game.");
            msg_print("You cannot save this game, but you may retire when ready.");
        }
    }

    uint32_t res;
    if (dump) {
        res = 0;
        if (dump & 255) {
            res |= CM_CARRY_OBJ;
            if (i & 0x04) {
                res |= CM_SMALL_OBJ;
            }
        }

        if (dump >= 256) {
            res |= CM_CARRY_GOLD;
        }

        dump = (dump % 256) + (dump / 256); // number of items
        res |= dump << CM_TR_SHIFT;
    } else {
        res = 0;
    }

    return res;
}
