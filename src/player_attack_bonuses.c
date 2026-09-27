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

// No externs.h here, and after #18-12-24C NOT ONE LINE THAT REACHES ANYTHING. The
// two numbers come from the strength and dexterity tables, but this file never
// reads those tables -- the callers look them up and hand the answers in, the same
// way the body's weight (the question before this one) never reads race[].

// THE PLACE THE TWO NUMBERS LIVE. They were the ptohit and ptodam fields of the
// character's record until #18-12-24C; now these two shorts are the only place the
// answers live, and the five windows below are the only way to reach them.
// (`struct misc` is down to ten fields.) The `the_aim()` / `the_force()` doors that
// step A used to reach py.misc went with the fields -- there is no indirection left
// to keep.
//
// ZERO IS THE HONEST START. An ordinary character really does add nothing -- the
// dexterity table gives 0 for 8 through 15 and the strength table for 5 through 15
// -- so a character who has not been rolled yet is indistinguishable from an
// average one, and that is what the fields did too.
static int16_t the_aim;
static int16_t the_force;

int player_to_hit_bonus(void) {
    return the_aim;
}

int player_to_damage_bonus(void) {
    return the_force;
}

void player_attack_bonuses_set(int to_hit, int to_damage) {
    // Creation's first guess and then its real values, the recalculation that runs
    // when equipment changes, and a saved file's two shorts put back. One sentence
    // for all four, because all four replace the pair outright.
    //
    // The casts are the store's own width, not a rule this window adds: both fields
    // were int16_t and `p_ptr->misc.ptohit = tohit_adj()` truncated exactly like
    // this, so the two statics are int16_t as well.
    the_aim = (int16_t)to_hit;
    the_force = (int16_t)to_damage;
}

void player_to_hit_bonus_adjust(int amount) {
    // One worn or wielded item's tohit. NEGATIVE IS ORDINARY: a cursed weapon
    // takes away.
    the_aim = (int16_t)(the_aim + amount);
}

void player_to_damage_bonus_adjust(int amount) {
    // One worn or wielded item's todam. The caller skips bows before calling ("Bows
    // can't damage. -CJS-"); that condition is about bows, not about this character.
    the_force = (int16_t)(the_force + amount);
}
