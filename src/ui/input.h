// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Input abstraction layer
// Provides a device-independent interface for user input

#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#include "backend.h"

// Global input state
extern InputBackend *current_input_backend;

// Initialize input system with a specific backend
bool input_init(InputBackend *backend);

// Shutdown input system
void input_shutdown(void);

// High-level input functions

// Get a key (blocking)
// Returns the key code, or KEY_NONE if no input
int input_get_key(void);

// Check if input is available
// microsec: timeout in microseconds (0 = immediate, -1 = wait forever)
bool input_check_available(int microsec);

// Flush input buffer
void input_flush(void);

#endif // INPUT_H
