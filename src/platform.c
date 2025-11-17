// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Platform abstraction layer implementation

#include "platform.h"

#include "backend_ncurses.h"
#include "render.h"
#include "input.h"

#include <stdio.h>

// Initialize platform services (rendering + input)
bool platform_init(void) {
    // Initialize rendering system with ncurses backend
    if (!render_init(render_ncurses_backend())) {
        fprintf(stderr, "Failed to initialize rendering system\n");
        return false;
    }

    // Initialize input system with ncurses backend
    if (!input_init(input_ncurses_backend())) {
        fprintf(stderr, "Failed to initialize input system\n");
        render_shutdown();
        return false;
    }

    return true;
}

// Shutdown platform services
void platform_shutdown(void) {
    input_shutdown();
    render_shutdown();
}
