// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// ncurses input backend implementation

#include "input_ncurses.h"

#include "curses.h"

#include <stdio.h>
#include <sys/select.h>

// Forward declarations
static bool ncurses_input_init(void);
static void ncurses_input_shutdown(void);
static int ncurses_get_key(void);
static bool ncurses_check_available(int microsec);
static void ncurses_flush(void);

// Backend instance
static InputBackend ncurses_input_backend_instance = {
    .name = "ncurses",
    .init = ncurses_input_init,
    .shutdown = ncurses_input_shutdown,
    .get_key = ncurses_get_key,
    .check_available = ncurses_check_available,
    .flush = ncurses_flush,
};

InputBackend *input_ncurses_backend(void) {
    return &ncurses_input_backend_instance;
}

// Initialize ncurses input
static bool ncurses_input_init(void) {
    // ncurses is already initialized by the rendering system
    // Nothing specific to do for input
    return true;
}

// Shutdown ncurses input
static void ncurses_input_shutdown(void) {
    // ncurses shutdown is handled by the rendering system
    // Nothing specific to do for input
}

// Get a key from ncurses
static int ncurses_get_key(void) {
    int ch = getch();

    // Map ncurses key codes to our normalized key codes
    // ncurses uses different values for special keys
    // Note: ncurses KEY_* macros are different from our INPUT_KEY_* enum
    if (ch == ERR) {
        return INPUT_KEY_NONE;
    }

    // Map ncurses special keys to our key codes
    switch (ch) {
        case KEY_UP:    return INPUT_KEY_UP;
        case KEY_DOWN:  return INPUT_KEY_DOWN;
        case KEY_LEFT:  return INPUT_KEY_LEFT;
        case KEY_RIGHT: return INPUT_KEY_RIGHT;
        case KEY_BACKSPACE:
        case 127:       return INPUT_KEY_BACKSPACE;
        case KEY_DC:    return INPUT_KEY_DELETE;
        case KEY_IC:    return INPUT_KEY_INSERT;
        case KEY_HOME:  return INPUT_KEY_HOME;
        case KEY_END:   return INPUT_KEY_END;
        case KEY_PPAGE: return INPUT_KEY_PAGE_UP;
        case KEY_NPAGE: return INPUT_KEY_PAGE_DOWN;
        case KEY_F(1):  return INPUT_KEY_F1;
        case KEY_F(2):  return INPUT_KEY_F2;
        case KEY_F(3):  return INPUT_KEY_F3;
        case KEY_F(4):  return INPUT_KEY_F4;
        case KEY_F(5):  return INPUT_KEY_F5;
        case KEY_F(6):  return INPUT_KEY_F6;
        case KEY_F(7):  return INPUT_KEY_F7;
        case KEY_F(8):  return INPUT_KEY_F8;
        case KEY_F(9):  return INPUT_KEY_F9;
        case KEY_F(10): return INPUT_KEY_F10;
        case KEY_F(11): return INPUT_KEY_F11;
        case KEY_F(12): return INPUT_KEY_F12;
        default:
            // For regular ASCII keys, pass through as-is
            return ch;
    }
}

// Check if input is available
static bool ncurses_check_available(int microsec) {
#ifdef TIMED_INPUT
    // Use ncurses built-in timeout mechanism
    timeout(8);
    int result = getch();
    timeout(-1);

    if (result > 0) {
        ungetch(result);
        return true;
    }
    return false;
#else
    // Use select() for more precise timeout control
    struct timeval tbuf;
    int smask;

    tbuf.tv_sec = 0;
    tbuf.tv_usec = microsec;

    smask = 1; // i.e. (1 << 0) for stdin
    if (select(1, (fd_set *)&smask, (fd_set *)0, (fd_set *)0, &tbuf) == 1) {
        int ch = getch();
        if (ch == -1) {
            return false;
        }
        ungetch(ch);
        return true;
    }
    return false;
#endif
}

// Flush input buffer
static void ncurses_flush(void) {
    // Drain the input buffer by reading until no more input
    nodelay(stdscr, TRUE);  // Make getch() non-blocking
    while (getch() != ERR) {
        // Discard all pending input
    }
    nodelay(stdscr, FALSE);  // Restore blocking mode
}
