// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Signal handling in main loop context
// This handles signals received by the async-safe handlers in platform/signals.c

#ifndef SIGNAL_HANDLING_H
#define SIGNAL_HANDLING_H

// Handle pending signals in a safe context (called from main loop)
// This performs all the I/O operations that were deferred from the signal handler
void handle_pending_signals(void);

#endif // SIGNAL_HANDLING_H
