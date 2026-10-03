// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Map direction numbers to command keys.

#ifndef DIRECTION_KEYS_H
#define DIRECTION_KEYS_H

typedef enum {
    DIR_KEY_WALK,
    DIR_KEY_RUN,
    DIR_KEY_TUNNEL
} direction_key_style;

// Returns the command key for the given direction (1-4, 6-9) and style.
// Returns 0 for any other direction (including 5).
char direction_command_key(int dir, direction_key_style style);

#endif // DIRECTION_KEYS_H
