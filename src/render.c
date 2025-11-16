// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Rendering system implementation

#include "render.h"

#include <stddef.h>
#include <stdio.h>

// Current active backend
RenderBackend *current_backend = NULL;

// Initialize rendering with specified backend
bool render_init(RenderBackend *backend) {
    if (backend == NULL) {
        fprintf(stderr, "render_init: NULL backend\n");
        return false;
    }

    if (backend->init == NULL) {
        fprintf(stderr, "render_init: backend '%s' has no init function\n",
                backend->name ? backend->name : "unknown");
        return false;
    }

    // Initialize the backend
    if (!backend->init()) {
        fprintf(stderr, "render_init: backend '%s' initialization failed\n",
                backend->name ? backend->name : "unknown");
        return false;
    }

    current_backend = backend;
    return true;
}

// Shutdown rendering system
void render_shutdown(void) {
    if (current_backend && current_backend->shutdown) {
        current_backend->shutdown();
    }
    current_backend = NULL;
}

// High-level rendering functions
// For now, these directly call backend functions
// Later, we'll add a rendering queue here

void render_clear(void) {
    if (current_backend && current_backend->clear) {
        current_backend->clear();
    }
}

void render_char(int row, int col, char ch, RenderColor color) {
    if (current_backend && current_backend->draw_char) {
        current_backend->draw_char(row, col, ch, color);
    }
}

void render_string(int row, int col, const char *str, RenderColor color) {
    if (current_backend && current_backend->draw_string) {
        current_backend->draw_string(row, col, str, color);
    }
}

void render_move_cursor(int row, int col) {
    if (current_backend && current_backend->move_cursor) {
        current_backend->move_cursor(row, col);
    }
}

void render_present(void) {
    if (current_backend && current_backend->end_frame) {
        current_backend->end_frame();
    }
}

void render_save_screen(void) {
    if (current_backend && current_backend->save_screen) {
        current_backend->save_screen();
    }
}

void render_restore_screen(void) {
    if (current_backend && current_backend->restore_screen) {
        current_backend->restore_screen();
    }
}

void render_get_size(int *rows, int *cols) {
    if (current_backend && current_backend->get_size) {
        current_backend->get_size(rows, cols);
    } else {
        // Default fallback
        if (rows) *rows = 24;
        if (cols) *cols = 80;
    }
}
