// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of ui/: ui_render.

#ifndef UI_RENDER_H
#define UI_RENDER_H

#include "config.h"
#include "constant.h"
#include "types.h"
#include <stdbool.h>
#include "backend.h"

// --- render.c ---
// Rendering abstraction layer
// Provides a backend-independent interface for all rendering operations
// Initialize rendering system with a specific backend
bool render_init(RenderBackend *backend);

// Shutdown rendering system
void render_shutdown(void);
void render_char(int row, int col, char ch, RenderColor color);
void render_string(int row, int col, const char *str, RenderColor color);
void render_move_cursor(int row, int col);

#endif // UI_RENDER_H
