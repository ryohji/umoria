/* source/signals.c: signal handlers
 *
 * Copyright (C) 1989-2008 James E. Wilson, Robert A. Koeneke,
 *                         David J. Grabiner
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

/* This signal package was brought to you by  -JEW- */
/* Completely rewritten by                    -CJS- */

/* To find out what system we're on. */

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include <signal.h>

static int error_sig = -1;
static int signal_count = 0;
static int mask;

static void signal_handler(int sig) {
    int smask = sigsetmask(0) | (1 << sig);

    /* Ignore all second signals. */
    if (error_sig >= 0) {
        /* Be safe. We will die if persistent enough. */
        if (++signal_count > 10) {
            (void)signal(sig, SIG_DFL);
        }
        return;
    }
    error_sig = sig;

    /* Allow player to think twice. Wizard may force a core dump. */
    if (sig == SIGINT || sig == SIGQUIT) {
        if (death) {
            /* Can't quit after death. */
            (void)signal(sig, SIG_IGN);
        } else if (!character_saved && character_generated) {
            if (!get_check("Really commit *Suicide*?")) {
                if (turn > 0) {
                    disturb(1, 0);
                }
                erase_line(0, 0);
                put_qio();
                error_sig = -1;
                (void)sigsetmask(smask);

                /* in case control-c typed during msg_print */
                if (wait_for_more) {
                    put_buffer(" -more-", MSG_LINE, 0);
                }
                put_qio();

                /* OK. We don't quit. */
                return;
            }
            (void)strcpy(died_from, "Interrupting");
        } else {
            (void)strcpy(died_from, "Abortion");
        }
        prt("Interrupt!", 0, 0);
        death = true;
        exit_game();
    }

    /* Die. */
    prt("OH NO!!!!!!  A gruesome software bug LEAPS out at you. There is NO "
        "defense!",
        23, 0);
    if (!death && !character_saved && character_generated) {
        panic_save = 1;
        prt("Your guardian angel is trying to save you.", 0, 0);
        (void)sprintf(died_from, "(panic save %d)", sig);
        if (!save_char()) {
            (void)strcpy(died_from, "software bug");
            death = true;
            turn = -1;
        }
    } else {
        death = true;

        /* Quietly save the memory anyway. */
        (void)_save_char(savefile);
    }
    restore_term();

    /* always generate a core dump */
    (void)signal(sig, SIG_DFL);
    (void)kill(getpid(), sig);
    (void)sleep(5);
    exit(1);
}

void nosignals() {
    (void)signal(SIGTSTP, SIG_IGN);
    mask = sigsetmask(0);

    if (error_sig < 0) {
        error_sig = 0;
    }
}

void signals() {
    (void)signal(SIGTSTP, suspend);
    (void)sigsetmask(mask);

    if (error_sig == 0) {
        error_sig = -1;
    }
}

void init_signals() {
    (void)signal(SIGINT, signal_handler);
    (void)signal(SIGINT, signal_handler);
    (void)signal(SIGFPE, signal_handler);

    /* Ignore HANGUP, and let the EOF code take care of this case. */
    (void)signal(SIGHUP, SIG_IGN);
    (void)signal(SIGQUIT, signal_handler);
    (void)signal(SIGILL, signal_handler);
    (void)signal(SIGTRAP, signal_handler);
    (void)signal(SIGIOT, signal_handler);
    (void)signal(SIGEMT, signal_handler);
    (void)signal(SIGKILL, signal_handler);
    (void)signal(SIGBUS, signal_handler);
    (void)signal(SIGSEGV, signal_handler);
    (void)signal(SIGSYS, signal_handler);
    (void)signal(SIGTERM, signal_handler);
    (void)signal(SIGPIPE, signal_handler);
    (void)signal(SIGXCPU, signal_handler);
}

void ignore_signals() {
    (void)signal(SIGINT, SIG_IGN);
    (void)signal(SIGQUIT, SIG_IGN);
}

void default_signals() {
    (void)signal(SIGINT, SIG_DFL);
    (void)signal(SIGQUIT, SIG_DFL);
}

void restore_signals() {
    (void)signal(SIGINT, signal_handler);
    (void)signal(SIGQUIT, signal_handler);
}
