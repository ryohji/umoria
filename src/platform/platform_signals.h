// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of platform/: platform_signals.

#ifndef PLATFORM_SIGNALS_H
#define PLATFORM_SIGNALS_H

#include "config.h"
#include "constant.h"
#include "types.h"
#include <signal.h>
#include <stdbool.h>

// --- signals.c ---
// Signal handler registration (signals.c).
#ifdef SIGTSTP
// Installs the terminal backend's suspend handler for SIGTSTP.
void signals_set_suspend_handler(void (*handler)(int));
#endif

// --- signal_flags.c ---
// Signal flags for async-signal-safe communication
// Signal handlers should ONLY set these flags, never perform I/O
// Signal types that can be received
typedef enum {
    SIGNAL_NONE = 0,
    SIGNAL_INTERRUPT,  // SIGINT, SIGQUIT - user interrupt
    SIGNAL_ERROR,      // SIGSEGV, SIGBUS, etc - fatal errors
} SignalType;

// Check if a signal is pending and get its information
// Returns true if a signal is pending
// This should be called from the main loop, NOT from signal handlers
bool signal_flags_check(SignalType *type, int *signum);

// Clear the pending signal flag
// Call this after handling the signal
void signal_flags_clear(void);

// --- signals.c ---
void nosignals(void);
void signals(void);
void init_signals(void);
void signal_state_reset(void);

#endif // PLATFORM_SIGNALS_H
