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
typedef enum {
    // Special keys
    KEY_NONE = 0,
    KEY_ESCAPE = 27,

    // Movement keys (for devices that provide directional input)
    KEY_UP = 256,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    // Function keys
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    // Control keys
    KEY_ENTER = 13,
    KEY_BACKSPACE = 8,
    KEY_TAB = 9,
    KEY_DELETE,
    KEY_INSERT,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,

    // Gamepad buttons (for future gamepad support)
    KEY_GAMEPAD_A,
    KEY_GAMEPAD_B,
    KEY_GAMEPAD_X,
    KEY_GAMEPAD_Y,
    KEY_GAMEPAD_START,
    KEY_GAMEPAD_SELECT,
    KEY_GAMEPAD_L1,
    KEY_GAMEPAD_R1,

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
