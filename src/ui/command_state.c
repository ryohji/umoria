// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the game remembers about the command being typed: where it is kept

#include "config.h"
#include "constant.h"
#include "types.h"

#include "command_state.h"

// This module includes no externs.h: it uses nothing outside itself.

// State: repeats left, remembered direction flag, and last command character.
// All start at 0 or false.
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
