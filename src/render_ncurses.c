// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// ncurses rendering backend implementation

#include "render_ncurses.h"

#include "curses.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

// Terminal state saved for suspend/resume
static struct termios save_termios;

// Saved screen for save/restore operations
static WINDOW *savescr = NULL;

// Forward declarations
static bool ncurses_init(void);
static void ncurses_shutdown(void);
static void ncurses_begin_frame(void);
static void ncurses_end_frame(void);
static void ncurses_clear(void);
static void ncurses_get_size(int *rows, int *cols);
static void ncurses_draw_char(int row, int col, char ch, RenderColor color);
static void ncurses_draw_string(int row, int col, const char *str, RenderColor color);
static void ncurses_move_cursor(int row, int col);
static void ncurses_show_cursor(bool show);
static int ncurses_get_char(void);
static bool ncurses_check_input(int microsec);
static void ncurses_save_screen(void);
static void ncurses_restore_screen(void);

#ifdef SIGTSTP
// Suspend/resume handler
static void suspend(int signum);
#endif

// Backend instance
static RenderBackend ncurses_backend_instance = {
    .name = "ncurses",
    .init = ncurses_init,
    .shutdown = ncurses_shutdown,
    .begin_frame = ncurses_begin_frame,
    .end_frame = ncurses_end_frame,
    .clear = ncurses_clear,
    .get_size = ncurses_get_size,
    .draw_char = ncurses_draw_char,
    .draw_string = ncurses_draw_string,
    .move_cursor = ncurses_move_cursor,
    .show_cursor = ncurses_show_cursor,
    .get_char = ncurses_get_char,
    .check_input = ncurses_check_input,
    .save_screen = ncurses_save_screen,
    .restore_screen = ncurses_restore_screen,
};

RenderBackend *render_ncurses_backend(void) {
    return &ncurses_backend_instance;
}

// Initialize ncurses
static bool ncurses_init(void) {
    // Save original terminal state
    tcgetattr(0, &save_termios);

    // Initialize ncurses
    initscr();

    // Check minimum screen size
    if (LINES < 24 || COLS < 80) {
        endwin();
        fprintf(stderr, "Screen too small for moria (need 24x80, got %dx%d)\n",
                LINES, COLS);
        return false;
    }

    // Set up terminal mode
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, false);
    keypad(stdscr, false);

#ifdef __APPLE__
    // Reduce escape delay on macOS
    set_escdelay(50);
#endif

#ifdef SIGTSTP
    // Install suspend signal handler
    signal(SIGTSTP, suspend);
#endif

    // Create saved screen buffer
    savescr = newwin(0, 0, 0, 0);
    if (savescr == NULL) {
        endwin();
        fprintf(stderr, "Failed to create screen buffer\n");
        return false;
    }

    clear();
    refresh();

    return true;
}

// Shutdown ncurses
static void ncurses_shutdown(void) {
    // Move cursor to bottom right
    int y = 0, x = 0;
    getyx(stdscr, y, x);
    mvcur(y, x, LINES - 1, 0);

    // Exit ncurses
    endwin();
    fflush(stdout);

    // Restore original terminal state
    tcsetattr(0, TCSANOW, &save_termios);
}

// Begin a new frame (currently a no-op for ncurses)
static void ncurses_begin_frame(void) {
    // Nothing to do for ncurses immediate mode
}

// End frame and present to screen
static void ncurses_end_frame(void) {
    refresh();
}

// Clear screen
static void ncurses_clear(void) {
    clear();
}

// Get screen size
static void ncurses_get_size(int *rows, int *cols) {
    if (rows) *rows = LINES;
    if (cols) *cols = COLS;
}

// Draw a single character
static void ncurses_draw_char(int row, int col, char ch, RenderColor color) {
    // For now, ignore color (will add later)
    mvaddch(row, col, ch);
}

// Draw a string
static void ncurses_draw_string(int row, int col, const char *str, RenderColor color) {
    // For now, ignore color (will add later)
    // Truncate string if it would go past right edge
    if (col >= COLS) {
        return;
    }

    char buffer[256];
    int max_len = COLS - col;
    if (max_len > 255) max_len = 255;

    strncpy(buffer, str, max_len);
    buffer[max_len] = '\0';

    mvaddstr(row, col, buffer);
}

// Move cursor
static void ncurses_move_cursor(int row, int col) {
    move(row, col);
}

// Show/hide cursor
static void ncurses_show_cursor(bool show) {
    curs_set(show ? 1 : 0);
}

// Get a character from input
static int ncurses_get_char(void) {
    return getch();
}

// Check if input is available
static bool ncurses_check_input(int microsec) {
#ifdef TIMED_INPUT
    timeout(8);
    int result = getch();
    timeout(-1);

    if (result > 0) {
        ungetch(result);
        return true;
    }
    return false;
#else
    struct timeval tbuf;
    int smask;

    tbuf.tv_sec = 0;
    tbuf.tv_usec = microsec;

    smask = 1; // i.e. (1 << 0)
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

// Save screen for later restore
static void ncurses_save_screen(void) {
    if (savescr) {
        overwrite(stdscr, savescr);
    }
}

// Restore previously saved screen
static void ncurses_restore_screen(void) {
    if (savescr) {
        overwrite(savescr, stdscr);
        touchwin(stdscr);
    }
}

#ifdef SIGTSTP
// Handle suspend/resume signals (SIGTSTP)
// This ensures terminal is properly reset and restored
static void suspend(int signum) {
    struct termios tbuf;

    // Save current terminal state
    tcgetattr(0, &tbuf);

    // Clean up ncurses
    ncurses_shutdown();

    // Stop the process
    (void)kill(0, SIGSTOP);

    // After resume, reinitialize ncurses
    initscr();
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, false);
    keypad(stdscr, false);

    // Restore terminal state
    tcsetattr(0, TCSANOW, &tbuf);
    (void)wrefresh(curscr);
}
#endif
