// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Signal handler registration (signals.c).

#ifndef OS_SIGNALS_H
#define OS_SIGNALS_H

#include <signal.h>

#ifdef SIGTSTP
// Installs the terminal backend's suspend handler for SIGTSTP.
void signals_set_suspend_handler(void (*handler)(int));
#endif

#endif // OS_SIGNALS_H
