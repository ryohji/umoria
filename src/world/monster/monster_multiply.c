// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// A monster that breeds puts its young on a square next to it. Called by the
// monsters' turns and by the clone spell.

#include "config.h"
#include "constant.h"
#include "types.h"
#include "dungeon_state.h"
#include "monster_state.h"
#include "geometry.h"
#include "monsters.h"
#include "monster_turn.h"
#include "rnd.h"
#include "world_monster.h"
#include "world_internal.h"

// Makes sure a new creature gets lit up. -CJS-
static bool check_mon_lite(int y, int x) {
    int monptr = square_at(y, x)->cptr;

    if (monptr <= 1) {
        return false;
    } else {
        update_mon(monptr);
        return monster_list_at(monptr)->ml;
    }
}

// Places creature adjacent to given location -RAK-
// Rats and Flys are fun!
bool multiply_monster(int y, int x, creature_iterator creature, int monptr) {
    int j, k, result;
    cave_type *c_ptr;

    int i = 0;
    do {
        j = y - 2 + randint(3);
        k = x - 2 + randint(3);

        // don't create a new creature on top of the old one, that
        // causes invincible/invisible creatures to appear.
        if (in_bounds(j, k) && (j != y || k != x)) {
            c_ptr = square_at(j, k);
            if ((c_ptr->fval <= MAX_OPEN_SPACE) && (c_ptr->tptr == 0) && (c_ptr->cptr != 1)) {
                // Creature there already?
                if (c_ptr->cptr > 1) {
                    // Some critters are cannibalistic!
                    if ((monster_creature_get(creature)->cmove & CM_EATS_OTHER)
                        // Check the experience level -CJS-
                        && monster_creature_get(creature)->mexp >= monster_creature_get(monster_list_at(c_ptr->cptr)->creature)->mexp) {
                        // It ate an already processed monster.Handle * normally.
                        if (monptr < c_ptr->cptr) {
                            delete_monster((int)c_ptr->cptr);
                        } else {
                            // If it eats this monster, an already processed
                            // mosnter will take its place, causing all kinds
                            // of havoc. Delay the kill a bit.
                            fix1_delete_monster((int)c_ptr->cptr);
                        }

                        // in case compact_monster() is called,it needs monptr
                        monster_turn_begin(monptr);

                        // Place_monster() may fail if monster list full.
                        result = place_monster(j, k, creature, false);
                        monster_turn_end();
                        if (!result) {
                            return false;
                        }
                        monster_breeding_note_birth();
                        return check_mon_lite(j, k);
                    }
                } else {
                    // All clear,  place a monster

                    // in case compact_monster() is called,it needs monptr
                    monster_turn_begin(monptr);

                    // Place_monster() may fail if monster list full.
                    result = place_monster(j, k, creature, false);
                    monster_turn_end();
                    if (!result) {
                        return false;
                    }
                    monster_breeding_note_birth();
                    return check_mon_lite(j, k);
                }
            }
        }
        i++;
    } while (i <= 18);
    return false;
}
