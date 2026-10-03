// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Map direction numbers to command keys.

#include "constant.h"
#include "direction_keys.h"

// Map direction 1-9 to walk/run/tunnel keys. Index by direction: 0-9.
// Walk uses lowercase, run uses uppercase, tunnel uses control keys.
static const char walk_keys[10] = {0, 'b', 'j', 'n', 'h', 0, 'l', 'y', 'k', 'u'};
static const char run_keys[10] = {0, 'B', 'J', 'N', 'H', 0, 'L', 'Y', 'K', 'U'};

char direction_command_key(int dir, direction_key_style style) {
    if (dir < 0 || dir > 9 || dir == 5) {
        return 0;
    }

    switch (style) {
    case DIR_KEY_WALK:
        return walk_keys[dir];
    case DIR_KEY_RUN:
        return run_keys[dir];
    case DIR_KEY_TUNNEL:
        // Tunnel keys are control versions of the uppercase letters.
        return run_keys[dir] ? CTRL_KEY(run_keys[dir]) : 0;
    default:
        return 0;
    }
}
