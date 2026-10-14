// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Rendering abstraction layer
// Provides a backend-independent interface for all rendering operations

#ifndef RENDER_H
#define RENDER_H

#include <stdbool.h>

#include "backend.h"

// Global rendering state
extern RenderBackend *current_backend;

// Initialize rendering system with a specific backend
bool render_init(RenderBackend *backend);

// Shutdown rendering system
void render_shutdown(void);

// High-level rendering functions (used by game code)
// These queue rendering commands to be executed at end_frame

void render_clear(void);
void render_erase_line(int row, int col);
void render_clear_from(int row);
void render_refresh(void);  // Force immediate screen update
void render_char(int row, int col, char ch, RenderColor color);
void render_string(int row, int col, const char *str, RenderColor color);
void render_move_cursor(int row, int col);
void render_present(void);  // Flush all queued commands and present frame
void render_bell(void);  // Audio or visual bell

// Screen save/restore
void render_save_screen(void);
void render_restore_screen(void);

// Utility functions
void render_get_size(int *rows, int *cols);

#endif // RENDER_H
