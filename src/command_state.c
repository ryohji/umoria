// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the game remembers about the command being typed: where it is kept

#include "config.h"
#include "constant.h"
#include "types.h"

#include "command_state.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c, missile_serial.c, inven_command_state.c, screen_touched.c,
// level_exit.c, pending_teleport.c, input_ended.c and running.c: remembering a
// count, a "reuse the direction" answer and one character needs nothing from the
// rest of the game. In particular this module does not read the keyboard, does
// not know which commands accept a count (dungeon.c decides that), and does not
// know which direction is remembered (get_dir() keeps that itself).

// All three are owned here and are static: the only way in is through the
// thirteen windows below. They came over from variable.c (#18-11-7C) with their
// initial values -- the old globals were written "int command_count" with no
// initializer, "bool default_dir = false" and "char last_command = ' '", and a
// game begins with no count typed, no direction remembered and no command yet.
//
// The names follow the windows. The count is a number of repeats left, not a
// count of anything else; the direction answer is about remembering, not about a
// default; and the character is the command before this one.
static int repeats_left = 0;
static bool direction_remembered = false;
static char previous_command = ' ';

void begin_command_count(int count) {
    repeats_left = count;
}

bool command_is_repeating(void) {
    return repeats_left > 0;
}

int command_count_remaining(void) {
    return repeats_left;
}

int take_command_count(void) {
    int count = repeats_left;
    repeats_left = 0;
    return count;
}

void consume_command_count(void) {
    repeats_left--;
}

void cancel_command_count(void) {
    repeats_left = 0;
}

int hold_command_count(void) {
    return repeats_left;
}

void resume_command_count(int held) {
    repeats_left = held;
}

bool direction_is_remembered(void) {
    return direction_remembered;
}

void reuse_remembered_direction(void) {
    direction_remembered = true;
}

void ask_for_direction_again(void) {
    direction_remembered = false;
}

void note_command(char command) {
    previous_command = command;
}

bool previous_command_was(char command) {
    return previous_command == command;
}
