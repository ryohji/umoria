// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// A display backend without a terminal, for running the whole game in tests.
//
// It stands in for app/platform_init.c: platform_init() installs a render
// backend that draws into a 24 x 80 array and an input backend that returns
// keys from a script file. Each key read appends one line to the transcript
// (the key and a hash of the screen as it was when the game asked for it);
// when the script runs out the process exits, and at exit the whole screen
// goes to the transcript.
//
//   HEADLESS_KEYS  the key script: escapes \n \r \e \t \\ \xNN; a line that
//                  starts with # is a comment; line breaks are not keys
//   HEADLESS_OUT   the transcript to write
//   HEADLESS_TRACE set to write the whole screen at every key, not only at exit
//
// time() is fixed (below), so dates on the screen do not change between runs.

#include "backend.h"
#include "ui_input.h"
#include "ui_render.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

bool platform_init(void);
void platform_shutdown(void);

enum { ROWS = 24, COLS = 80 };

static char screen[ROWS][COLS];
static char saved[ROWS][COLS];
static int cursor_row, cursor_col;
static int bells;

static char *keys;
static size_t key_count, key_next;
static FILE *out;
static bool trace;

static void fill_blank(char (*s)[COLS], int from_row) {
    for (int r = from_row; r < ROWS; r++) {
        memset(s[r], ' ', COLS);
    }
}

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            h = (h ^ (uint8_t)screen[r][c]) * 16777619u;
        }
    }
    return h;
}

static void dump_screen(void) {
    fprintf(out, "screen (cursor %d,%d, bells %d):\n", cursor_row, cursor_col, bells);
    for (int r = 0; r < ROWS; r++) {
        int end = COLS;
        while (end > 0 && screen[r][end - 1] == ' ') {
            end--;
        }
        fprintf(out, "|%.*s\n", end, screen[r]);
    }
}

// --- render ----------------------------------------------------------------

static bool r_init(void) {
    fill_blank(screen, 0);
    fill_blank(saved, 0);
    return true;
}

static void r_nothing(void) {}

static void r_clear(void) {
    fill_blank(screen, 0);
}

static void r_get_size(int *rows, int *cols) {
    *rows = ROWS;
    *cols = COLS;
}

static void r_erase_line(int row, int col) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        memset(&screen[row][col], ' ', (size_t)(COLS - col));
    }
}

static void r_clear_from(int row) {
    if (row >= 0 && row < ROWS) {
        fill_blank(screen, row);
    }
}

static void r_draw_char(int row, int col, char ch, RenderColor color) {
    (void)color;
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        screen[row][col] = ch;
    }
}

// A line break clears the rest of the line and goes on at the start of the
// next one, as on a terminal.
static void r_draw_string(int row, int col, const char *str, RenderColor color) {
    for (int c = col; *str != '\0'; str++, c++) {
        if (*str == '\n') {
            r_erase_line(row, c);
            row++;
            c = -1;
            continue;
        }
        r_draw_char(row, c, *str, color);
    }
}

static void r_move_cursor(int row, int col) {
    cursor_row = row;
    cursor_col = col;
}

static void r_show_cursor(bool show) {
    (void)show;
}

static void r_bell(void) {
    bells++;
}

static int r_get_char(void) {
    return -1;
}

static bool r_check_input(int microsec) {
    (void)microsec;
    return false;
}

static void r_save_screen(void) {
    memcpy(saved, screen, sizeof screen);
}

static void r_restore_screen(void) {
    memcpy(screen, saved, sizeof screen);
}

static RenderBackend render_backend = {
    .name = "headless",
    .init = r_init,
    .shutdown = r_nothing,
    .begin_frame = r_nothing,
    .end_frame = r_nothing,
    .clear = r_clear,
    .get_size = r_get_size,
    .erase_line = r_erase_line,
    .clear_from = r_clear_from,
    .refresh = r_nothing,
    .draw_char = r_draw_char,
    .draw_string = r_draw_string,
    .move_cursor = r_move_cursor,
    .show_cursor = r_show_cursor,
    .bell = r_bell,
    .get_char = r_get_char,
    .check_input = r_check_input,
    .save_screen = r_save_screen,
    .restore_screen = r_restore_screen,
};

// --- input -----------------------------------------------------------------

static int hex(int ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

// Reads the script into keys[]: escapes decoded, comments and line breaks dropped.
static bool load_keys(const char *path) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        return false;
    }
    size_t cap = 256;
    keys = malloc(cap);
    char line[1024];
    while (fgets(line, sizeof line, f) != NULL) {
        if (line[0] == '#') {
            continue;
        }
        for (const char *p = line; *p != '\0' && *p != '\n'; p++) {
            int ch = (uint8_t)*p;
            if (ch == '\\' && p[1] != '\0') {
                p++;
                switch (*p) {
                case 'n': ch = '\n'; break;
                case 'r': ch = '\r'; break;
                case 'e': ch = 27; break;
                case 't': ch = '\t'; break;
                case 'x':
                    if (hex(p[1]) >= 0 && hex(p[2]) >= 0) {
                        ch = hex(p[1]) * 16 + hex(p[2]);
                        p += 2;
                    }
                    break;
                default: ch = (uint8_t)*p; break;
                }
            }
            if (key_count == cap) {
                cap *= 2;
                keys = realloc(keys, cap);
            }
            keys[key_count++] = (char)ch;
        }
    }
    fclose(f);
    return true;
}

static bool i_init(void) {
    return true;
}

static void i_nothing(void) {}

// Runs once, at exit: whether the script ran out or the game quit by itself.
static bool script_ran_out;

static void finish(void) {
    fprintf(out, "%s after %zu of %zu keys\n", script_ran_out ? "end of script" : "game exited", key_next, key_count);
    dump_screen();
    fclose(out);
}

static int i_get_key(void) {
    if (key_next == key_count) {
        script_ran_out = true;
        exit(0);
    }
    const uint8_t key = (uint8_t)keys[key_next++];
    fprintf(out, "key %3zu 0x%02x %08x\n", key_next, key, screen_hash());
    if (trace) {
        dump_screen();
    }
    return key;
}

static bool i_check_available(int microsec) {
    (void)microsec;
    return false;
}

static InputBackend input_backend = {
    .name = "headless",
    .init = i_init,
    .shutdown = i_nothing,
    .get_key = i_get_key,
    .check_available = i_check_available,
    .flush = i_nothing,
};

// --- the clock ---------------------------------------------------------------

// The game reads the clock for the birth date, the save file and the tombstone.
// A fixed time keeps the transcript the same from run to run (this definition
// takes the place of the C library's for the whole executable).
time_t time(time_t *t) {
    const time_t fixed = 1700000000;
    if (t != NULL) {
        *t = fixed;
    }
    return fixed;
}

// --- the platform -----------------------------------------------------------

bool platform_init(void) {
    const char *keys_path = getenv("HEADLESS_KEYS");
    const char *out_path = getenv("HEADLESS_OUT");
    if (keys_path == NULL || out_path == NULL || !load_keys(keys_path)) {
        fprintf(stderr, "headless: set HEADLESS_KEYS and HEADLESS_OUT\n");
        return false;
    }
    out = fopen(out_path, "w");
    if (out == NULL) {
        fprintf(stderr, "headless: cannot write %s\n", out_path);
        return false;
    }
    const char *t = getenv("HEADLESS_TRACE");
    trace = t != NULL && t[0] != '\0';
    atexit(finish);
    return render_init(&render_backend) && input_init(&input_backend);
}

void platform_shutdown(void) {
    input_shutdown();
    render_shutdown();
}
