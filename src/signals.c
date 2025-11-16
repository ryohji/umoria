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
/* Made async-signal-safe                     -2025- */

/* To find out what system we're on. */

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "signal_flags.h"

#include <signal.h>

static int error_sig = -1;
static int signal_count = 0;
static int mask;

static void signal_handler(int sig) {
    /* ASYNC-SIGNAL-SAFE HANDLER
     *
     * This handler ONLY sets atomic flags. All I/O operations (ncurses,
     * file I/O, printf, etc.) are deferred to the main loop where they
     * can be executed safely.
     *
     * According to POSIX, the only safe operations in signal handlers are:
     * - Setting sig_atomic_t variables
     * - Calling async-signal-safe functions (very limited list)
     *
     * We must NOT call: get_check(), prt(), msg_print(), save_char(),
     * or any ncurses functions from here.
     */

    /* Ignore repeated signals to prevent signal storms */
    if (error_sig >= 0) {
        /* After many signals, restore default handler to allow termination */
        if (++signal_count > 10) {
            (void)signal(sig, SIG_DFL);
        }
        return;
    }
    error_sig = sig;

    /* Classify signal type and set flags for main loop to handle */
    SignalType type;
    if (sig == SIGINT || sig == SIGQUIT) {
        type = SIGNAL_INTERRUPT;
    } else {
        type = SIGNAL_ERROR;
    }

    /* This is the ONLY communication with main code - set atomic flags */
    signal_flags_set(type, sig);

    /* For fatal errors (SIGSEGV, etc.), restore default handler
     * so if we can't handle it gracefully, we still crash properly */
    if (type == SIGNAL_ERROR) {
        (void)signal(sig, SIG_DFL);
    }
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
    /* Initialize signal flags */
    signal_flags_init();

    /* Install signal handlers */
    (void)signal(SIGINT, signal_handler);
    (void)signal(SIGFPE, signal_handler);

    /* Ignore HANGUP, and let the EOF code take care of this case. */
    (void)signal(SIGHUP, SIG_IGN);
    (void)signal(SIGQUIT, signal_handler);
    (void)signal(SIGILL, signal_handler);
    (void)signal(SIGTRAP, signal_handler);
#ifdef SIGIOT
    (void)signal(SIGIOT, signal_handler);
#endif
#ifdef SIGEMT
    (void)signal(SIGEMT, signal_handler);
#endif
    /* SIGKILL cannot be caught */
    (void)signal(SIGBUS, signal_handler);
    (void)signal(SIGSEGV, signal_handler);
#ifdef SIGSYS
    (void)signal(SIGSYS, signal_handler);
#endif
    (void)signal(SIGTERM, signal_handler);
    (void)signal(SIGPIPE, signal_handler);
#ifdef SIGXCPU
    (void)signal(SIGXCPU, signal_handler);
#endif
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
