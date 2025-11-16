// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Signal flags implementation
// Provides async-signal-safe communication between signal handlers and main code

#include "signal_flags.h"

// These flags are modified by signal handlers
// volatile sig_atomic_t is the ONLY safe type for signal handler communication
volatile sig_atomic_t signal_pending = 0;
volatile sig_atomic_t signal_type = SIGNAL_NONE;
volatile sig_atomic_t signal_number = 0;

void signal_flags_init(void) {
    signal_pending = 0;
    signal_type = SIGNAL_NONE;
    signal_number = 0;
}

bool signal_flags_check(SignalType *type, int *signum) {
    if (!signal_pending) {
        return false;
    }

    // Signal is pending, retrieve information
    if (type != NULL) {
        *type = (SignalType)signal_type;
    }
    if (signum != NULL) {
        *signum = (int)signal_number;
    }

    return true;
}

void signal_flags_clear(void) {
    signal_pending = 0;
    signal_type = SIGNAL_NONE;
    signal_number = 0;
}

void signal_flags_set(SignalType type, int signum) {
    // This function is called from signal handlers
    // It must ONLY set atomic flags - no I/O, no function calls
    signal_type = (sig_atomic_t)type;
    signal_number = (sig_atomic_t)signum;
    signal_pending = 1;  // Set this last to ensure atomicity
}
