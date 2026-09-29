// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Input system implementation

#include "input.h"

#include <stddef.h>
#include <stdio.h>

// Current active backend
InputBackend *current_input_backend = NULL;

// Initialize input with specified backend
bool input_init(InputBackend *backend) {
    if (backend == NULL) {
        fprintf(stderr, "input_init: NULL backend\n");
        return false;
    }

    if (backend->init == NULL) {
        fprintf(stderr, "input_init: backend '%s' has no init function\n",
                backend->name ? backend->name : "unknown");
        return false;
    }

    // Initialize the backend
    if (!backend->init()) {
        fprintf(stderr, "input_init: backend '%s' initialization failed\n",
                backend->name ? backend->name : "unknown");
        return false;
    }

    current_input_backend = backend;
    return true;
}

// Shutdown input system
void input_shutdown(void) {
    if (current_input_backend && current_input_backend->shutdown) {
        current_input_backend->shutdown();
    }
    current_input_backend = NULL;
}

// Get a key
int input_get_key(void) {
    if (current_input_backend && current_input_backend->get_key) {
        return current_input_backend->get_key();
    }
    return INPUT_KEY_NONE;
}

// Check if input is available
bool input_check_available(int microsec) {
    if (current_input_backend && current_input_backend->check_available) {
        return current_input_backend->check_available(microsec);
    }
    return false;
}

// Flush input buffer
void input_flush(void) {
    if (current_input_backend && current_input_backend->flush) {
        current_input_backend->flush();
    }
}
