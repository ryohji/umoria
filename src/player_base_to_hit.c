// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How well this character swings and shoots before anything is added

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_base_to_hit.h"

// No externs.h here, the same as the seventeen questions before this one.

// THE TWO NUMBERS THEMSELVES, still py.misc.bth and py.misc.bthb for one commit
// more. Only this file names them; #18-12-19C moves them into two statics here and
// deletes the fields, leaving `struct misc` with sixteen.
extern player_type py;

static int16_t *the_melee_number(void) {
    return &py.misc.bth;
}

static int16_t *the_bows_number(void) {
    return &py.misc.bthb;
}

int player_base_to_hit(void) {
    return *the_melee_number();
}

int player_base_to_hit_with_bows(void) {
    return *the_bows_number();
}

void player_base_to_hit_set(int melee, int with_bows) {
    // The race's two bases, and the saved file's two shorts put back. One window
    // for both, because both are a plain replacement of the pair (the hit die
    // needed only one door here too; the armour class needed two).
    *the_melee_number() = (int16_t)melee;
    *the_bows_number() = (int16_t)with_bows;
}

void player_base_to_hit_set_melee(int melee) {
    // The wizard screen, which asks for one number at a time.
    *the_melee_number() = (int16_t)melee;
}

void player_base_to_hit_set_with_bows(int with_bows) {
    *the_bows_number() = (int16_t)with_bows;
}

void player_base_to_hit_adjust(int melee, int with_bows) {
    // The class's mbth and mbthb, added to what the race left. TWO AMOUNTS,
    // because a class is not equally good at both.
    *the_melee_number() = (int16_t)(*the_melee_number() + melee);
    *the_bows_number() = (int16_t)(*the_bows_number() + with_bows);
}

void player_base_to_hit_adjust_both(int amount) {
    // A spell's worth: heroism 12, super heroism 24, a blessing 5, and the same
    // number taken away again when the spell runs out. ONE AMOUNT FOR BOTH
    // NUMBERS, which is what twelve lines in dungeon.c were saying six times.
    *the_melee_number() = (int16_t)(*the_melee_number() + amount);
    *the_bows_number() = (int16_t)(*the_bows_number() + amount);
}
