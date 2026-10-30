// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The interfaces a display backend (ncurses today, a GUI later) implements.
// ui/render.c and ui/input.c call through them.

#ifndef BACKEND_H
#define BACKEND_H

#include <stdbool.h>

// Input key codes
// These provide a normalized representation of input across different devices
// Note: Prefixed with INPUT_ to avoid conflicts with ncurses KEY_* macros
typedef enum {
    // Special keys
    INPUT_KEY_NONE = 0,
    INPUT_KEY_ESCAPE = 27,

    // Movement keys (for devices that provide directional input)
    INPUT_KEY_UP = 256,
    INPUT_KEY_DOWN,
    INPUT_KEY_LEFT,
    INPUT_KEY_RIGHT,

    // Function keys
    INPUT_KEY_F1,
    INPUT_KEY_F2,
    INPUT_KEY_F3,
    INPUT_KEY_F4,
    INPUT_KEY_F5,
    INPUT_KEY_F6,
    INPUT_KEY_F7,
    INPUT_KEY_F8,
    INPUT_KEY_F9,
    INPUT_KEY_F10,
    INPUT_KEY_F11,
    INPUT_KEY_F12,

    // Control keys
    INPUT_KEY_ENTER = 13,
    INPUT_KEY_BACKSPACE = 8,
    INPUT_KEY_TAB = 9,
    INPUT_KEY_DELETE,
    INPUT_KEY_INSERT,
    INPUT_KEY_HOME,
    INPUT_KEY_END,
    INPUT_KEY_PAGE_UP,
    INPUT_KEY_PAGE_DOWN,

    // Gamepad buttons (for future gamepad support)
    INPUT_KEY_GAMEPAD_A,
    INPUT_KEY_GAMEPAD_B,
    INPUT_KEY_GAMEPAD_X,
    INPUT_KEY_GAMEPAD_Y,
    INPUT_KEY_GAMEPAD_START,
    INPUT_KEY_GAMEPAD_SELECT,
    INPUT_KEY_GAMEPAD_L1,
    INPUT_KEY_GAMEPAD_R1,

} InputKeyCode;

// Color definitions
// These can be mapped to different color schemes by backends
// Note: Prefixed with RENDER_ to avoid conflicts with ncurses COLOR_* macros
typedef enum {
    RENDER_COLOR_DEFAULT = 0,
    RENDER_COLOR_BLACK = 1,
    RENDER_COLOR_RED = 2,
    RENDER_COLOR_GREEN = 3,
    RENDER_COLOR_YELLOW = 4,
    RENDER_COLOR_BLUE = 5,
    RENDER_COLOR_MAGENTA = 6,
    RENDER_COLOR_CYAN = 7,
    RENDER_COLOR_WHITE = 8,
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
    void (*erase_line)(int row, int col);  // Erase from col to end of line
    void (*clear_from)(int row);  // Clear from row to bottom of screen
    void (*refresh)(void);  // Force immediate screen update

    // Drawing primitives
    void (*draw_char)(int row, int col, char ch, RenderColor color);
    void (*draw_string)(int row, int col, const char *str, RenderColor color);

    // Cursor management
    void (*move_cursor)(int row, int col);
    void (*show_cursor)(bool show);

    // Audio feedback
    void (*bell)(void);  // Audio or visual bell

    // Screen save/restore (for menus, inventory, etc.)
    void (*save_screen)(void);
    void (*restore_screen)(void);

} RenderBackend;

// Input backend interface
// Different input sources (keyboard, gamepad, network) implement this
typedef struct InputBackend {
    const char *name;

    // Lifecycle
    bool (*init)(void);
    void (*shutdown)(void);

    // Input polling
    int (*get_key)(void);           // Get a key (blocking)
    bool (*check_available)(int microsec);  // Check if input is available
    void (*flush)(void);            // Flush input buffer

} InputBackend;

#endif // BACKEND_H
