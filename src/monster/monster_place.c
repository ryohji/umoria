// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Putting monsters on the level: taking a row of the monster list (making
// room when it is full), choosing a monster for the depth, and placing it

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "dungeon_level.h"
#include "dungeon_map.h"
#include "dungeon_size.h"
#include "monster_levels.h"
#include "monster_list.h"
#include "monster_turn.h"
#include "player_pos.h"
#include "player_speed.h"
#include "score_death.h"

// Moved out of misc1.c unchanged (#54); the prototypes of the global ones stay
// in externs.h. get_mons_num() and summon() are static and serve only the
// functions here, so they came along with their forward declarations.

static creature_handle get_mons_num(int level);
static bool summon(int *y, int *x, creature_handle h, int slp);

// Compact monsters -RAK-
// Return true if any monsters were deleted, false if could not delete any monsters.
bool compact_monsters(void) {
    msg_print("Compacting monsters...");

    int cur_dis = 66;
    bool delete_any = false;

    do {
        for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
            monster_type *mon_ptr = monster_list_at(i);
            if ((cur_dis < mon_ptr->cdis) && (randint(3) == 1)) {
                // Never compact away the Balrog!!
                if (monster_get_creature(mon_ptr->creature)->cmove & CM_WIN) {
                    ; // Do nothing
                } else if (monster_delete_may_shift(i)) {
                    // in case this is called from within creatures(), this is a horrible
                    // hack, the monster-list/creatures() code needs to be rewritten.
                    delete_monster(i);
                    delete_any = true;
                } else {
                    // fix1_delete_monster() does not take the mark back,
                    // so don't set delete_any if this was called.
                    fix1_delete_monster(i);
                }
            }
        }

        if (!delete_any) {
            cur_dis -= 6;
            // Can't delete any monsters, return failure.
            if (cur_dis < 0) {
                return false;
            }
        }
    } while (!delete_any);

    return true;
}

// Returns a pointer to next free space -RAK-
// Returns -1 if could not allocate a monster.
int popm(void) {
    if (monster_list_is_full()) {
        if (!compact_monsters()) {
            return -1;
        }
    }
    return monster_list_claim_slot();
}

// Places a monster at given location -RAK-
bool place_monster(int y, int x, creature_handle h, int slp) {
    const int cur_pos = popm();
    if (cur_pos == -1) {
        return false;
    } else {
        creature_type *const r_ptr = monster_get_creature(h);
        int (*const calc_hp)(const uint8_t *) = r_ptr->cdefense & CD_MAX_HP ? max_hp : pdamroll;
        monster_type *const mon_ptr = monster_list_at(cur_pos);
        mon_ptr->fy = y;
        mon_ptr->fx = x;
        mon_ptr->creature = h;
        mon_ptr->hp = calc_hp(r_ptr->hd);
        // the creature speed value is 10 greater, so that it can be a uint8_t
        mon_ptr->cspeed = r_ptr->speed - 10 + player_speed();
        mon_ptr->stunned = 0;
        mon_ptr->cdis = distance(player_row(), player_col(), y, x);
        mon_ptr->ml = false;
        mon_ptr->csleep = (slp = slp ? r_ptr->sleep : 0) ? slp * 2 + randint(slp * 10) : 0;
        square_at(y, x)->cptr = cur_pos;
        return true;
    }
}

// Places a monster at given location -RAK-
void place_win_monster(void) {
    if (!player_has_won()) {
        int x, y, z = randint(WIN_MON_TOT) - 1 + monsters_up_to_level(MAX_MONS_LEVEL);

        do {
            y = randint(dungeon_height() - 2);
            x = randint(dungeon_width() - 2);
        } while ((square_at(y, x)->fval >= MIN_CLOSED_SPACE) ||
                 (square_at(y, x)->cptr != 0) || (square_at(y, x)->tptr != 0) ||
                 (distance(y, x, player_row(), player_col()) <= MAX_SIGHT));

        // Check for case where could not allocate space for
        // the win monster, this should never happen.
        if (!place_monster(y, x, monster_make_creature_handle(z), 0)) {
            abort();
        }
    }
}

// Return a monster suitable to be placed at a given level. This
// makes high level monsters (up to the given level) slightly more
// common than low level monsters at any given level. -CJS-
static creature_handle get_mons_num(int level) {
    int i;

    if (level == 0) {
        i = randint(monsters_up_to_level(0)) - 1;
    } else {
        if (level > MAX_MONS_LEVEL) {
            level = MAX_MONS_LEVEL;
        }
        if (randint(MON_NASTY) == 1) {
            i = randnor(0, 4);
            level = level + abs(i) + 1;
            if (level > MAX_MONS_LEVEL) {
                level = MAX_MONS_LEVEL;
            }
        } else {
            // This code has been added to make it slightly more likely to get
            // the higher level monsters. Originally a uniform distribution over
            // all monsters of level less than or equal to the dungeon level.
            // This distribution makes a level n monster occur approx 2/n% of the
            // time on level n, and 1/n*n% are 1st level.
            const int num = monsters_up_to_level(level) - first_monster_at_level(1);
            i = randint(num) - 1;
            const int j = randint(num) - 1;
            const int k = MAX(i, j) + first_monster_at_level(1);
            level = monster_get_creature(monster_make_creature_handle(k))->level;
        }
        i = randint(monsters_at_level(level)) - 1 + first_monster_at_level(level);
    }

    return monster_make_creature_handle(i);
}

// Allocates a random monster -RAK-
void alloc_monster(int num, int dis, int slp) {
    int y, x;

    while (num--) {
        do {
            y = randint(dungeon_height() - 2);
            x = randint(dungeon_width() - 2);
        } while (square_at(y, x)->fval >= MIN_CLOSED_SPACE || (square_at(y, x)->cptr != 0) || (distance(y, x, player_row(), player_col()) <= dis));

        creature_handle h = get_mons_num(dungeon_level());
        const uint8_t cchar = monster_get_creature(h)->cchar;

        // Dragons ('d' or 'D') are always created sleeping here,
        // so as to give the player a sporting chance.

        // Place_monster() should always return true here.
        // It does not matter if it fails though.
        (void)place_monster(y, x, h, slp || cchar == 'd' || cchar == 'D');
    }
}

static bool summon(int *y, int *x, creature_handle h, int slp) {
    int i = 0;

    do {
        const int j = *y - 2 + randint(3);
        const int k = *x - 2 + randint(3);
        const cave_type *const cave_ptr = square_at(j, k);
        if (in_bounds(j, k) && cave_ptr->fval <= MAX_OPEN_SPACE && cave_ptr->cptr == 0) {
            // Place_monster() should always return true here.
            if (place_monster(j, k, h, slp)) {
                *y = j;
                *x = k;
                return true;
            }
            break;
        }
    } while (++i <= 9);

    return false;
}

// Places creature adjacent to given location -RAK-
bool summon_monster(int *y, int *x, int slp) {
    creature_handle h = get_mons_num(dungeon_level() + MON_SUMMON_ADJ);
    return summon(y, x, h, slp);
}

// Places undead adjacent to given location -RAK-
bool summon_undead(int *y, int *x) {
    int l = monsters_up_to_level(MAX_MONS_LEVEL);
    creature_handle h;

    do {
        int m = randint(l) - 1;
        int ctr = 0;
        do {
            h = monster_make_creature_handle(m);
            if (monster_get_creature(h)->cdefense & CD_UNDEAD) {
                l = 0;
                ctr = 20;
            } else {
                m += 1;
                ctr = m > l ? 20 : ctr + 1;
            }
        } while (ctr != 20);
    } while (l != 0);

    return summon(y, x, h, false);
}
