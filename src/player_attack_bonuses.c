// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What this character adds to a blow's aim and to its force

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_attack_bonuses.h"

// No externs.h here, the same as the twenty-three questions before this one. The
// two numbers come from the strength and dexterity tables, but this file never
// reads those tables -- the callers look them up and hand the answers in.

// THE PLACE THE TWO NUMBERS LIVE, for the length of step A only: still the fields
// of the character's record, reached through one door each so that #18-12-24C can
// move them by changing these two functions and nothing else.
extern player_type py;

static int16_t *the_aim(void) {
    return &py.misc.ptohit;
}

static int16_t *the_force(void) {
    return &py.misc.ptodam;
}

int player_to_hit_bonus(void) {
    return *the_aim();
}

int player_to_damage_bonus(void) {
    return *the_force();
}

void player_attack_bonuses_set(int to_hit, int to_damage) {
    // Creation's first guess and then its real values, the recalculation that runs
    // when equipment changes, and a saved file's two shorts put back. One sentence
    // for all four, because all four replace the pair outright.
    //
    // The casts are the fields' own width, not a rule this window adds: both were
    // int16_t and `p_ptr->misc.ptohit = tohit_adj()` truncated exactly like this.
    *the_aim() = (int16_t)to_hit;
    *the_force() = (int16_t)to_damage;
}

void player_to_hit_bonus_adjust(int amount) {
    // One worn or wielded item's tohit. NEGATIVE IS ORDINARY: a cursed weapon
    // takes away.
    *the_aim() = (int16_t)(*the_aim() + amount);
}

void player_to_damage_bonus_adjust(int amount) {
    // One worn or wielded item's todam. The caller skips bows before calling ("Bows
    // can't damage. -CJS-"); that condition is about bows, not about this character.
    *the_force() = (int16_t)(*the_force() + amount);
}
