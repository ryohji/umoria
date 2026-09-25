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

// The four still live in py.misc (player.c:17). They come in here at
// #18-12-3C, where these become statics of their own and the four fields leave
// the struct. Declared here rather than taken from externs.h so that what this
// module touches is one line long and visible.
extern player_type py;

int16_t player_display_to_hit(void) {
    return py.misc.dis_th;
}

int16_t player_display_to_dam(void) {
    return py.misc.dis_td;
}

int16_t player_display_to_ac(void) {
    return py.misc.dis_tac;
}

int16_t player_display_ac(void) {
    return py.misc.dis_ac;
}

void player_display_start_from_real(int16_t to_hit, int16_t to_dam, int16_t to_ac) {
    // The sheet begins as a copy of the real plusses, with none of the armour
    // visible yet: what calc_bonuses() and create.c both spelled out.
    py.misc.dis_th = to_hit;
    py.misc.dis_td = to_dam;
    py.misc.dis_tac = to_ac;
    py.misc.dis_ac = 0;
}

void player_display_add_to_hit(int amount) {
    py.misc.dis_th = (int16_t)(py.misc.dis_th + amount);
}

void player_display_add_to_dam(int amount) {
    py.misc.dis_td = (int16_t)(py.misc.dis_td + amount);
}

void player_display_add_to_ac(int amount) {
    py.misc.dis_tac = (int16_t)(py.misc.dis_tac + amount);
}

void player_display_add_ac(int amount) {
    py.misc.dis_ac = (int16_t)(py.misc.dis_ac + amount);
}

void player_display_fold_to_ac(void) {
    // The bonus the sheet shows is part of the total the sheet shows.
    py.misc.dis_ac = (int16_t)(py.misc.dis_ac + py.misc.dis_tac);
}

void player_display_set_to_hit(int16_t value) {
    py.misc.dis_th = value;
}

void player_display_set_to_dam(int16_t value) {
    py.misc.dis_td = value;
}

void player_display_set_to_ac(int16_t value) {
    py.misc.dis_tac = value;
}

void player_display_set_ac(int16_t value) {
    py.misc.dis_ac = value;
}
