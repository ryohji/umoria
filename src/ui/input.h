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
