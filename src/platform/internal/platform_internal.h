// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Declarations shared inside platform/ only.

#ifndef PLATFORM_INTERNAL_H
#define PLATFORM_INTERNAL_H

#include "platform_signals.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include <signal.h>
#include <stdbool.h>

// --- signal_flags.c ---
// Initialize signal flags
void signal_flags_init(void);

// Set signal flags (called from signal handler)
// This is the ONLY function that should be called from signal handlers
void signal_flags_set(SignalType type, int signum);

#endif // PLATFORM_INTERNAL_H
