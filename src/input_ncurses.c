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
    switch (ch) {
        case ERR:
            return KEY_NONE;
        case KEY_BACKSPACE:
        case 127:  // DEL character
            return KEY_BACKSPACE;
        default:
            // For most keys, ncurses code matches ASCII or our codes
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
