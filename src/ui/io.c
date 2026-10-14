// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Terminal I/O code, uses the curses package

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include "game_settings.h"
#include "ui_io.h"
#include "command_state.h"
#include "panel.h"
#include "input.h"
#include "input_ended.h"
#include "messages.h"
#include "render.h"
#include "screen_touched.h"

#define use_value2

static void wait_for_more_confirmation(void);
static void sleep_in_seconds(int);

// Handler for input-end game logic (registered by app layer)
static void (*input_end_handler)(void) = NULL;

// Handler for quit action (registered by app layer)
static void (*quit_handler)(void) = NULL;

// Register handler for input-end game logic
// Must be called before any inkey() can encounter EOF
void io_set_input_end_handler(void (*handler)(void)) {
    input_end_handler = handler;
}

// Register handler for quit action
// Must be called before any pause_exit() can be called
void io_set_quit_handler(void (*handler)(void)) {
    quit_handler = handler;
}

// Dump IO to buffer -RAK-
void put_buffer(const char *out_str, int row, int col) {
    vtype tmp_str;

    // truncate the string, to make sure that it won't go past right edge of screen.
    if (col > 79) {
        col = 79;
    }
    (void)strncpy(tmp_str, out_str, 79 - col);
    tmp_str[79 - col] = '\0';

    render_string(row, col, tmp_str, RENDER_COLOR_DEFAULT);
}

// Dump the IO buffer to terminal -RAK-
void put_qio(void) {
    // Let inven_command know something has changed.
    note_screen_flushed();

    render_refresh();
}

void shell_out(void) {
    put_buffer("[Opening new shells is not currently supported]\n", 0, 0);
}

// Returns a single character input from the terminal. -CJS-
//
// This silently consumes ^R to redraw the screen and reset the
// terminal, so that this operation can always be performed at
// any input prompt. inkey() never returns ^R.
char inkey(void) {
    put_qio();              // Dump IO buffer
    cancel_command_count(); // Just to be safe -CJS-

    while (true) {
        int i = input_get_key();

        // some machines may not sign extend.
        if (i == EOF) {
            // avoid infinite loops while trying to call inkey() for a -more- prompt.
            msg_set_pending(false);

            note_input_ended();

            render_refresh();

            // Call registered handler for game-level input-end logic
            // This handles: exit if no unsaved character, disturb, panic save if hopeless
            if (input_end_handler != NULL) {
                input_end_handler();
            }

            return ESCAPE;
        }

        if (i != CTRL_KEY('R')) {
            return (char)i;
        }

        // TODO: Implement screen refresh through rendering abstraction
        // (void)wrefresh(curscr);
        // moriaterm();
    }
}

// Flush the buffer -RAK-
void flush(void) {
    if (!input_has_ended()) {
        input_flush();
    }
}

// Clears given line of text -RAK-
void erase_line(int row, int col) {
    if (row == MSG_LINE && msg_pending()) {
        msg_print(CNIL);
    }

    render_erase_line(row, col);
}

// Clears screen
void clear_screen(void) {
    if (msg_pending()) {
        msg_print(CNIL);
    }
    render_clear();
}

void clear_from(int row) {
    render_clear_from(row);
}

// Outputs a char to a given interpolated y, x position -RAK-
// sign bit of a character used to indicate standout mode. -CJS
void print(char ch, int row, int col) {

    // Real co-ords convert to screen positions
    render_char(panel_screen_row(row), panel_screen_col(col), ch, RENDER_COLOR_DEFAULT);
}

// Moves the cursor to a given interpolated y, x position -RAK-
void move_cursor_relative(int row, int col) {
    // Real co-ords convert to screen positions
    render_move_cursor(panel_screen_row(row), panel_screen_col(col));
}

// Print a message so as not to interrupt a counted command. -CJS-
void count_msg_print(const char *p) {
    int i = hold_command_count();
    msg_print(p);
    resume_command_count(i);
}

// Outputs a line to a given y, x position -RAK-
void prt(const char *str_buff, int row, int col) {
    erase_line(row, col);
    put_buffer(str_buff, row, col);
}

// move cursor to a given y, x position
void move_cursor(int row, int col) {
    render_move_cursor(row, col);
}

// Outputs message to top line of screen
// These messages are kept for later reference.
void msg_print(const char *str_buff) {
    const int old_len = msg_pending() ? strlen(msg_history_recent(0)) : 0;
    const bool prev_msg_exists = msg_pending();
    const bool combine_messages = prev_msg_exists && str_buff && old_len + 2 + strlen(str_buff) < 73;

    msg_set_pending(str_buff != CNIL);

    if (!combine_messages) {
        // Make the null string a special case. -CJS-

        if (prev_msg_exists) {
            // ensure that the complete -more- message is visible.
            put_buffer("-more-", MSG_LINE, MIN(73, old_len + 2));

            wait_for_more_confirmation();
        }

        render_erase_line(MSG_LINE, 0);

        if (msg_pending()) {
            cancel_command_count();

            put_buffer(str_buff, MSG_LINE, 0);
            msg_history_push(str_buff);
        }
    } else {
        cancel_command_count();

        // If the new message and the old message are short enough,
        // display them together on the same line.
        // So we don't flush the old message in this case.

        put_buffer(str_buff, MSG_LINE, old_len + 2);
        msg_history_append(str_buff);
    }
}

static inline void wait_for_more_confirmation(void) {
    // let sigint handler know that we are waiting for a space
    msg_set_at_more_prompt(true);

retry:
    switch (inkey()) {
    case ' ':
    case ESCAPE:
    case '\n':
    case '\r':
        break;
    default:
        // Anything else dismisses nothing; keep waiting.
        goto retry;
    }

    msg_set_at_more_prompt(false);
}

// Used to verify a choice - user gets the chance to abort choice. -CJS-
bool get_check(const char *prompt) {
    prt(prompt, 0, 0);

    // Calculate cursor position after printing prompt
    int x = strlen(prompt);
    int col = (x > 73) ? 73 : x;

    render_string(0, col, " [y/n]", RENDER_COLOR_DEFAULT);

    int res;
    do {
        res = inkey();
    } while (res == ' ');

    erase_line(0, 0);

    if (res == 'Y' || res == 'y') {
        return true;
    } else {
        return false;
    }
}

// Prompts (optional) and returns ord value of input char
// Function returns false if <ESCAPE> is input
int get_com(const char *prompt, char *command) {
    if (prompt) {
        prt(prompt, 0, 0);
    }

    *command = inkey();

    bool res;
    if (*command == ESCAPE) {
        res = false;
    } else {
        res = true;
    }

    erase_line(MSG_LINE, 0);

    return res;
}

// Gets a string terminated by <RETURN>
// Function returns false if <ESCAPE> is input
bool get_string(char *in_str, int row, int column, int slen) {
    bool aborted = false;
    bool flag = false;

    // Clear the input area
    for (int i = 0; i < slen; i++) {
        render_char(row, column + i, ' ', RENDER_COLOR_DEFAULT);
    }

    render_move_cursor(row, column);
    int start_col = column;
    int end_col = column + slen - 1;

    if (end_col > 79) {
        // NOTE: `slen` is never read after this point, so commenting out. -MRC-
        // slen = 80 - column;
        end_col = 79;
    }

    char *p = in_str;

    do {
        int i = inkey();

        switch (i) {
        case ESCAPE:
            aborted = true;
            break;
        case CTRL_KEY('J'):
        case CTRL_KEY('M'):
            flag = true;
            break;
        case DELETE:
        case CTRL_KEY('H'):
            if (column > start_col) {
                column--;
                put_buffer(" ", row, column);
                move_cursor(row, column);
                *--p = '\0';
            }
            break;
        default:
            if (!isprint(i) || column > end_col) {
                bell();
            } else {
                render_char(row, column, (char)i, RENDER_COLOR_DEFAULT);
                *p++ = i;
                column++;
            }
            break;
        }
    } while ((!flag) && (!aborted));

    if (aborted) {
        return false;
    }

    // Remove trailing blanks
    while (p > in_str && p[-1] == ' ') {
        p--;
    }
    *p = '\0';

    return true;
}

// Pauses for user response before returning -RAK-
void pause_line(int prt_line) {
    prt("[Press any key to continue.]", prt_line, 23);
    (void)inkey();
    erase_line(prt_line, 0);
}

// Pauses for user response before returning -RAK-
// NOTE: Delay is for players trying to roll up "perfect"
// characters.  Make them wait a bit.
void pause_exit(int prt_line, int delay) {
    prt("[Press any key to continue, or Q to exit.]", prt_line, 10);

    char dummy = inkey();
    if (dummy == 'Q') {
        erase_line(prt_line, 0);

        if (delay > 0) {
            sleep_in_seconds(delay);
        }

        // Call registered quit handler
        if (quit_handler != NULL) {
            quit_handler();
        }
    }
    erase_line(prt_line, 0);
}

void save_screen(void) {
    render_save_screen();
}

void restore_screen(void) {
    render_restore_screen();
}

void bell(void) {
    put_qio();

    // The player can turn off beeps if he/she finds them annoying.
    if (!sound_beep_flag) {
        return;
    }

    render_bell();
}

static void sleep_in_seconds(int seconds) {
#ifdef _WIN32
    Sleep(seconds * 1000);
#else
    sleep(seconds);
#endif
}

// Provides for a timeout on input. Does a non-blocking read, consuming the data if
// any, and then returns 1 if data was read, zero otherwise.
bool check_input(int microsec) {
    if (input_check_available(microsec)) {
        int ch = input_get_key();
        // check for EOF errors here
        if (ch == -1) {
            note_input_ended();
            return false;
        }
        return true;
    } else {
        return false;
    }
}

// Find a default user name from the system.
void user_name(char *buf) {
#ifdef _WIN32
    unsigned long bufCharCount = PLAYER_NAME_SIZE;

    if (!GetUserName(buf, &bufCharCount)) {
        (void)strcpy(buf, "X"); // Gotta have some name
    }
#else
    // getlogin() declared in <unistd.h> (via headers.h).
    char *p = getlogin();

    if (p && p[0]) {
        (void)strcpy(buf, p);
    } else {
        struct passwd *pwline = getpwuid((int)getuid());
        if (pwline) {
            (void)strcpy(buf, pwline->pw_name);
        }
    }
    if (!buf[0]) {
        (void)strcpy(buf, "X"); // Gotta have some name
    }
#endif
}

#ifndef _WIN32
// On unix based systems we should expand `~` to the users home path,
// otherwise on Windows we can ignore all of this. -MRC-

// undefine these so that tfopen and topen will work
#undef fopen
#undef open

static int tilde(const char *, char *);

// open a file just as does fopen, but allow a leading ~ to specify a home directory
FILE *tfopen(const char *file, const char *mode) {
    // extern int errno;

    char buf[1024];
    if (tilde(file, buf)) {
        return (fopen(buf, mode));
    }
    errno = ENOENT;
    return NULL;
}

// open a file just as does open, but expand a leading ~ into a home directory name
int topen(char *file, int flags, int mode) {
    // extern int errno;

    char buf[1024];
    if (tilde(file, buf)) {
        return (open(buf, flags, mode));
    }
    errno = ENOENT;
    return -1;
}

// expands a tilde at the beginning of a file name to a users home directory
static int tilde(const char *file, char *exp) {
    *exp = '\0';
    if (file) {
        if (*file == '~') {
            char user[128];
            struct passwd *pw = NULL;
            int i = 0;

            user[0] = '\0';
            file++;
            // Limit is sizeof(user) - 1 to leave room for null terminator.
            // Cast sizeof to int to match i's type and avoid sign conversion.
            while (*file != '/' && i < (int)sizeof(user) - 1) {
                user[i++] = *file++;
            }
            user[i] = '\0';
            if (i == 0) {
                char *login = getlogin();

                if (login != NULL) {
                    (void)strcpy(user, login);
                } else if ((pw = getpwuid(getuid())) == NULL) {
                    return 0;
                }
            }
            if (pw == NULL && (pw = getpwnam(user)) == NULL) {
                return 0;
            }
            (void)strcpy(exp, pw->pw_dir);
        }
        (void)strcat(exp, file);
        return 1;
    }
    return 0;
}
#endif

