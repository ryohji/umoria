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

// Color definitions
// These can be mapped to different color schemes by backends
typedef enum {
    COLOR_DEFAULT = 0,
    COLOR_BLACK = 1,
    COLOR_RED = 2,
    COLOR_GREEN = 3,
    COLOR_YELLOW = 4,
    COLOR_BLUE = 5,
    COLOR_MAGENTA = 6,
    COLOR_CYAN = 7,
    COLOR_WHITE = 8,
} RenderColor;

// Rendering backend interface
// Backends (ncurses, Pyxel, SDL, etc.) implement these functions
typedef struct RenderBackend {
    const char *name;

    // Lifecycle
    bool (*init)(void);
    void (*shutdown)(void);

    // Frame control
    void (*begin_frame)(void);
    void (*end_frame)(void);  // Present/flush the frame to screen

    // Screen management
    void (*clear)(void);
    void (*get_size)(int *rows, int *cols);

    // Drawing primitives
    void (*draw_char)(int row, int col, char ch, RenderColor color);
    void (*draw_string)(int row, int col, const char *str, RenderColor color);

    // Cursor management
    void (*move_cursor)(int row, int col);
    void (*show_cursor)(bool show);

    // Input (may be moved to separate input abstraction later)
    int (*get_char)(void);
    bool (*check_input)(int microsec);

    // Screen save/restore (for menus, inventory, etc.)
    void (*save_screen)(void);
    void (*restore_screen)(void);

} RenderBackend;

// Global rendering state
extern RenderBackend *current_backend;

// Initialize rendering system with a specific backend
bool render_init(RenderBackend *backend);

// Shutdown rendering system
void render_shutdown(void);

// High-level rendering functions (used by game code)
// These queue rendering commands to be executed at end_frame

void render_clear(void);
void render_char(int row, int col, char ch, RenderColor color);
void render_string(int row, int col, const char *str, RenderColor color);
void render_move_cursor(int row, int col);
void render_present(void);  // Flush all queued commands and present frame

// Screen save/restore
void render_save_screen(void);
void render_restore_screen(void);

// Utility functions
void render_get_size(int *rows, int *cols);

#endif // RENDER_H
