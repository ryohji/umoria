// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How hard this character is to hit -- the real armour class

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_armour_class.h"

// No externs.h here, the same as the sixteen questions before this one.

// THE STORE IS STILL py.misc FOR NOW (#18-12-18A). Two halves, so there are two
// entrances instead of one, and C has exactly two lines to change.
extern player_type py;

static int16_t *the_armour_half(void) {
    return &py.misc.pac;
}

static int16_t *the_magical_half(void) {
    return &py.misc.ptoac;
}

int player_armour_class(void) {
    // The whole point of the module: twenty-six callers used to write this sum
    // out by hand. It is an int, as `p_ptr->pac + p_ptr->ptoac` always was.
    return *the_armour_half() + *the_magical_half();
}

int player_armour_class_magical(void) {
    return *the_magical_half();
}

int player_armour_class_armour(void) {
    return *the_armour_half();
}

void player_armour_class_reset(int magical) {
    // "Nothing is worn yet": the armour half goes to zero and the magical half
    // becomes the dexterity bonus. calc_bonuses() spelled out both assignments
    // before every rebuild.
    *the_armour_half() = 0;
    *the_magical_half() = (int16_t)magical;
}

void player_armour_class_set_parts(int armour, int magical) {
    // The saved file's two shorts, and create.c's race table. Not _reset(),
    // because the armour half is given here rather than wiped.
    *the_armour_half() = (int16_t)armour;
    *the_magical_half() = (int16_t)magical;
}

void player_armour_class_add_item(int armour, int magical) {
    // One worn slot: `i_ptr->ac` and `i_ptr->toac` in one window, so the store
    // is touched once per slot instead of twice.
    *the_armour_half() = (int16_t)(*the_armour_half() + armour);
    *the_magical_half() = (int16_t)(*the_magical_half() + magical);
}

void player_armour_class_adjust(int armour) {
    // A spell's worth, added to the armour half -- +100 or -100 for
    // invulnerability, +2 or -2 for a blessing. Both signs come through here,
    // which is why one window does the work of the old `+=` and `-=` pair.
    *the_armour_half() = (int16_t)(*the_armour_half() + armour);
}
