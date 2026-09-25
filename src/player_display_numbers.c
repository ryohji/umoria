// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The four combat numbers the character sheet shows

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_display_numbers.h"

// No externs.h here, the same as player_gold.c, player_food.c and the rest:
// four numbers that are only ever shown need nothing from the game around them.
// In particular this module does not know what the equipment is, whether an item
// is identified or cursed, or how heavy a weapon may be -- the caller decides
// all of that and hands over an amount. It prints nothing, and it does not raise
// PY_ARMOR when the AC moves (moria1.c still watches for that, because it cannot
// print inside a store).

// Owned here and static: the only way in is through the windows below. The four
// came over from the four dis_* fields of py.misc (#18-12-3C) with
// their initial values -- the fields were inside a struct with no initializer,
// so a sheet that says nothing is what a program starts with, and create.c puts
// the real numbers in once the character exists (or save.c puts back the ones
// the file remembers).
//
// The names follow the windows rather than the old fields: "dis_tac" was the
// bonus the sheet shows and "dis_ac" the total it shows, which the old pair of
// names does not say.
static int16_t shown_to_hit = 0;
static int16_t shown_to_dam = 0;
static int16_t shown_to_ac = 0;
static int16_t shown_ac = 0;

int16_t player_display_to_hit(void) {
    return shown_to_hit;
}

int16_t player_display_to_dam(void) {
    return shown_to_dam;
}

int16_t player_display_to_ac(void) {
    return shown_to_ac;
}

int16_t player_display_ac(void) {
    return shown_ac;
}

void player_display_start_from_real(int16_t to_hit, int16_t to_dam, int16_t to_ac) {
    // The sheet begins as a copy of the real plusses, with none of the armour
    // visible yet: what calc_bonuses() and create.c both spelled out.
    shown_to_hit = to_hit;
    shown_to_dam = to_dam;
    shown_to_ac = to_ac;
    shown_ac = 0;
}

void player_display_add_to_hit(int amount) {
    shown_to_hit = (int16_t)(shown_to_hit + amount);
}

void player_display_add_to_dam(int amount) {
    shown_to_dam = (int16_t)(shown_to_dam + amount);
}

void player_display_add_to_ac(int amount) {
    shown_to_ac = (int16_t)(shown_to_ac + amount);
}

void player_display_add_ac(int amount) {
    shown_ac = (int16_t)(shown_ac + amount);
}

void player_display_fold_to_ac(void) {
    // The bonus the sheet shows is part of the total the sheet shows.
    shown_ac = (int16_t)(shown_ac + shown_to_ac);
}

void player_display_set_to_hit(int16_t value) {
    shown_to_hit = value;
}

void player_display_set_to_dam(int16_t value) {
    shown_to_dam = value;
}

void player_display_set_to_ac(int16_t value) {
    shown_to_ac = value;
}

void player_display_set_ac(int16_t value) {
    shown_ac = value;
}
