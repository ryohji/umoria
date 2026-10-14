/* source/signal_handling.c: signal handling in main loop context
 *
 * Copyright (C) 1989-2008 James E. Wilson, Robert A. Koeneke,
 *                         David J. Grabiner
 * Copyright (c) 2021-2026 Umoria Contributors
 *
 * This file is part of Umoria.
 *
 * Umoria is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Umoria is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Umoria.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Handles signals in a safe context (main loop), performing I/O operations
 * that were deferred from the async-signal-safe handlers in platform/signals.c */

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "signal_handling.h"
#include "messages.h"
#include "progress.h"
#include "save_state.h"
#include "score_death.h"
#include "signal_flags.h"
#include "platform.h"

#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// Handle pending signals in a safe context (called from main loop)
// This performs all the I/O operations that were deferred from the signal handler
void handle_pending_signals(void) {
    SignalType type;
    int signum;

    // Check if a signal is pending
    if (!signal_flags_check(&type, &signum)) {
        return; // No pending signals
    }

    // Clear the flag immediately to prevent reprocessing
    signal_flags_clear();

    // Reset error state
    signal_state_reset();

    // Handle based on signal type
    if (type == SIGNAL_INTERRUPT) {
        // User interrupt (SIGINT or SIGQUIT)
        if (player_is_dead()) {
            // Can't quit after death - ignore
            (void)signal(signum, SIG_IGN);
            return;
        }

        if (save_state_has_unsaved_character()) {
            // Ask user for confirmation
            if (!get_check("Really commit *Suicide*?")) {
                // User canceled - restore state and continue
                if (progress_turn() > 0) {
                    disturb(1, 0);
                }
                erase_line(0, 0);
                put_qio();

                // Restore -more- prompt if needed
                if (msg_at_more_prompt()) {
                    put_buffer(" -more-", MSG_LINE, 0);
                }
                put_qio();
                return;
            }

            // User confirmed suicide
            (void)strcpy(death_cause(), "Interrupting");
        } else {
            (void)strcpy(death_cause(), "Abortion");
        }

        prt("Interrupt!", 0, 0);
        set_player_dead(true);
        exit_game();
    } else if (type == SIGNAL_ERROR) {
        // Fatal error signal (SIGSEGV, SIGBUS, etc.)
        prt("OH NO!!!!!!  A gruesome software bug LEAPS out at you. There is NO "
            "defense!",
            23, 0);

        if (save_state_has_live_character()) {
            // Try panic save
            set_panic_save(true);
            prt("Your guardian angel is trying to save you.", 0, 0);
            (void)sprintf(death_cause(), "(panic save %d)", signum);

            if (!save_char()) {
                (void)strcpy(death_cause(), "software bug");
                set_player_dead(true);
                progress_set_turn(-1);
            }
        } else {
            set_player_dead(true);
            // Quietly save anyway
            (void)_save_char(save_file_path());
        }

        platform_shutdown();

        // Generate core dump for debugging
        (void)signal(signum, SIG_DFL);
        (void)kill(getpid(), signum);
        (void)sleep(5);
        exit(1);
    }
}
