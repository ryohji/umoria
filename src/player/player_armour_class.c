// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How hard this character is to hit -- the real armour class

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_armour_class.h"

// No externs.h here.

// THE TWO NUMBERS THEMSELVES. These two shorts are the only place the answer
// lives, and the windows below are the only way to reach it.
//
// ZERO IS A REAL ANSWER HERE: a character with nothing worn and a middling
// dexterity has no armour class at all, and the game means it.
static int16_t worn_armour;
static int16_t magical_plusses;

int player_armour_class(void) {
    // The whole point of the module: twenty-six callers used to write this sum
    // out by hand. It is an int, as `p_ptr->pac + p_ptr->ptoac` always was.
    return worn_armour + magical_plusses;
}

int player_armour_class_magical(void) {
    return magical_plusses;
}

int player_armour_class_armour(void) {
    return worn_armour;
}

void player_armour_class_reset(int magical) {
    // "Nothing is worn yet": the armour half goes to zero and the magical half
    // becomes the dexterity bonus. calc_bonuses() spelled out both assignments
    // before every rebuild.
    worn_armour = 0;
    magical_plusses = (int16_t)magical;
}

void player_armour_class_set_parts(int armour, int magical) {
    // The saved file's two shorts, and create.c's race table. Not _reset(),
    // because the armour half is given here rather than wiped.
    worn_armour = (int16_t)armour;
    magical_plusses = (int16_t)magical;
}

void player_armour_class_add_item(int armour, int magical) {
    // One worn slot: `i_ptr->ac` and `i_ptr->toac` in one window, so the store
    // is touched once per slot instead of twice.
    worn_armour = (int16_t)(worn_armour + armour);
    magical_plusses = (int16_t)(magical_plusses + magical);
}

void player_armour_class_adjust(int armour) {
    // A spell's worth, added to the armour half -- +100 or -100 for
    // invulnerability, +2 or -2 for a blessing. Both signs come through here,
    // which is why one window does the work of the old `+=` and `-=` pair.
    worn_armour = (int16_t)(worn_armour + armour);
}
