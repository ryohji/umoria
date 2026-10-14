// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Game-level handling of input end (EOF from stdin)
// Called by ui/io.c inkey() when input has ended

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "input_ended.h"
#include "save_state.h"
#include "score_death.h"

#include <string.h>

// Handle game-level logic when input has ended
// Order of operations must match original inkey() implementation
void handle_input_end(void) {
    // If no unsaved character, just exit
    if (!save_state_has_unsaved_character()) {
        exit_game();
    }

    // Disturb any ongoing activity
    disturb(1, 0);

    // If too many EOFs, panic save and exit
    if (input_end_is_hopeless()) {
        // just in case, to make sure that the process eventually dies
        set_panic_save(true);

        (void)strcpy(death_cause(), "(end of input: panic saved)");
        if (!save_char()) {
            (void)strcpy(death_cause(), "panic: unexpected eof");
            set_player_dead(true);
        }
        exit_game();
    }
}
