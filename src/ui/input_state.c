// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#include "config.h"
#include "constant.h"
#include "types.h"
#include "ui_input.h"
#include "ui_screen.h"
#include "ui_internal.h"

// Input state: the repeat count, the pending inventory command, and how many
// times input has ended.

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

// Pending inventory command character. 0 means none pending.
static char pending_command = 0;

char pending_inven_command(void) {
    return pending_command;
}

void suspend_inven_command(char command) {
    pending_command = command;

    // Both halves of suspending, in one place: see the header for what goes
    // wrong when only one of them happens.
    forget_screen_flushed();
}

void finish_inven_command(void) {
    pending_command = 0;
}

// EOF counter. Starts at 0.
static int eof_count = 0;

// The number of EOFs io.c has put up with before it panic-saves and dies. Stated
// once, here, and nowhere else (see ui_internal.h for why it is not at the call site).
#define EOF_TRIES_ALLOWED 100

void note_input_ended(void) {
    eof_count++;
}

bool input_has_ended(void) {
    return eof_count != 0;
}

bool input_end_is_hopeless(void) {
    return eof_count > EOF_TRIES_ALLOWED;
}

int input_end_count(void) {
    return eof_count;
}
