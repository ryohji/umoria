// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Signal flags for async-signal-safe communication
// Signal handlers should ONLY set these flags, never perform I/O

#ifndef SIGNAL_FLAGS_H
#define SIGNAL_FLAGS_H

#include <signal.h>
#include <stdbool.h>

// Signal types that can be received
typedef enum {
    SIGNAL_NONE = 0,
    SIGNAL_INTERRUPT,  // SIGINT, SIGQUIT - user interrupt
    SIGNAL_ERROR,      // SIGSEGV, SIGBUS, etc - fatal errors
} SignalType;

// Async-signal-safe flags
// These are the ONLY variables that signal handlers should modify
extern volatile sig_atomic_t signal_pending;
extern volatile sig_atomic_t signal_type;
extern volatile sig_atomic_t signal_number;

// Initialize signal flags
void signal_flags_init(void);

// Check if a signal is pending and get its information
// Returns true if a signal is pending
// This should be called from the main loop, NOT from signal handlers
bool signal_flags_check(SignalType *type, int *signum);

// Clear the pending signal flag
// Call this after handling the signal
void signal_flags_clear(void);

// Set signal flags (called from signal handler)
// This is the ONLY function that should be called from signal handlers
void signal_flags_set(SignalType type, int signum);

#endif // SIGNAL_FLAGS_H
