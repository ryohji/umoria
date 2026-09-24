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

// The three still live in variable.c at this step (#18-11-7A); they move in here
// as statics in #18-11-7C. Declared rather than included, so that this module
// does not pull in the global header -- the same choice as stats.c,
// object_levels.c and running.c.
extern int command_count;
extern bool default_dir;
extern char last_command;

void begin_command_count(int count) {
    command_count = count;
}

bool command_is_repeating(void) {
    return command_count > 0;
}

int command_count_remaining(void) {
    return command_count;
}

int take_command_count(void) {
    int count = command_count;
    command_count = 0;
    return count;
}

void consume_command_count(void) {
    command_count--;
}

void cancel_command_count(void) {
    command_count = 0;
}

int hold_command_count(void) {
    return command_count;
}

void resume_command_count(int held) {
    command_count = held;
}

bool direction_is_remembered(void) {
    return default_dir;
}

void reuse_remembered_direction(void) {
    default_dir = true;
}

void ask_for_direction_again(void) {
    default_dir = false;
}

void note_command(char command) {
    last_command = command;
}

bool previous_command_was(char command) {
    return last_command == command;
}
