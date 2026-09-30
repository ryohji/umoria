// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Asking the player for a direction

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "command_state.h"

// map rogue_like direction commands into numbers
static char map_roguedir(char comval) {
    switch (comval) {
    case 'h':
        comval = '4';
        break;
    case 'y':
        comval = '7';
        break;
    case 'k':
        comval = '8';
        break;
    case 'u':
        comval = '9';
        break;
    case 'l':
        comval = '6';
        break;
    case 'n':
        comval = '3';
        break;
    case 'j':
        comval = '2';
        break;
    case 'b':
        comval = '1';
        break;
    case '.':
        comval = '5';
        break;
    }
    return comval;
}

// Prompts for a direction -RAK-
// Direction memory added, for repeated commands.  -CJS
bool get_dir(const char *prompt, int *dir) {
    static char prev_dir; // Direction memory. -CJS-

    // used in counted commands. -CJS-
    if (direction_is_remembered()) {
        *dir = prev_dir;
        return true;
    }

    if (prompt == CNIL) {
        prompt = "Which direction?";
    }

    for (;;) {
        char command;

        // Don't end a counted command. -CJS-
        int save = hold_command_count();

        if (!get_com(prompt, &command)) {
            free_turn_flag = true;
            return false;
        }

        resume_command_count(save);

        if (rogue_like_commands) {
            command = map_roguedir(command);
        }

        if (command >= '1' && command <= '9' && command != '5') {
            prev_dir = command - '0';
            *dir = prev_dir;
            return true;
        }
        bell();
    }
}

// Similar to get_dir, except that no memory exists, and it is -CJS-
// allowed to enter the null direction.
bool get_alldir(const char *prompt, int *dir) {
    char command;

    for (;;) {
        if (!get_com(prompt, &command)) {
            free_turn_flag = true;
            return false;
        }

        if (rogue_like_commands) {
            command = map_roguedir(command);
        }

        if (command >= '1' && command <= '9') {
            *dir = command - '0';
            return true;
        }

        bell();
    }
}
