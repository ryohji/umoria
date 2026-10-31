// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Every creature of the monster table through roff_recall() with many monster
// memories: the movement, spell and defence flags known (none, the creature's
// own, half of them, all, only the attack-only and magic-only ones, its own with
// one or two pieces of treasure seen), kills and deaths, how often it was seen
// to wake or not, attacks seen (none, some, many), characters of 9 levels (the
// ordinals 1st, 2nd, 3rd, 11th... and "an 8th"), wizard mode off and on. What roff_recall() draws (every string with its row and
// column, every erased line) goes into one hash per creature, printed with a
// total; make -f makefile.test golden compares them with
// tests/golden/recall_all.hashes. Run with any argument to also print the
// screen of the first memory of each creature.
#include "config.h"
#include "constant.h"
#include "types.h"
#include "backend.h"
#include "command_internal.h"
#include "game_status.h"
#include "monsters.h"
#include "monster_state.h"
#include "player_progress.h"
#include "ui_input.h"
#include "ui_render.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t h;
static void mix_byte(uint8_t b) { h = (h ^ b) * 16777619u; }
static void mix_int(int v) { for (int n = 0; n < 4; n++) mix_byte((uint8_t)(v >> (8 * n))); }
static void mix_string(const char *s) { for (; *s; s++) mix_byte((uint8_t)*s); mix_byte('\n'); }

static bool verbose;
static char screen[24][81];

static bool r_init(void) { return true; }
static void r_nothing(void) {}
static void r_erase_line(int row, int col) {
    mix_byte('E'); mix_int(row); mix_int(col);
    if (row >= 0 && row < 24 && col >= 0 && col < 80) memset(&screen[row][col], ' ', (size_t)(80 - col));
}
static void r_clear_from(int row) { mix_byte('C'); mix_int(row); }
static void r_clear(void) { mix_byte('K'); }
static void r_draw_char(int row, int col, char ch, RenderColor color) {
    (void)color; mix_byte('c'); mix_int(row); mix_int(col); mix_byte((uint8_t)ch);
}
static void r_draw_string(int row, int col, const char *str, RenderColor color) {
    (void)color; mix_byte('s'); mix_int(row); mix_int(col); mix_string(str);
    if (row >= 0 && row < 24) {
        for (int n = 0; str[n] && col + n < 80; n++) screen[row][col + n] = str[n];
    }
}
static void r_move_cursor(int row, int col) { (void)row; (void)col; }
static void r_show_cursor(bool show) { (void)show; }

static RenderBackend render_backend = {
    .name = "recall_all", .init = r_init, .shutdown = r_nothing,
    .clear = r_clear, .erase_line = r_erase_line,
    .clear_from = r_clear_from, .refresh = r_nothing, .draw_char = r_draw_char,
    .draw_string = r_draw_string, .move_cursor = r_move_cursor, .show_cursor = r_show_cursor,
    .bell = r_nothing,
    .save_screen = r_nothing, .restore_screen = r_nothing,
};

static int i_get_key(void) { return ESCAPE; }
static bool i_check_available(int microsec) { (void)microsec; return false; }
static InputBackend input_backend = {
    .name = "recall_all", .init = r_init, .shutdown = r_nothing, .get_key = i_get_key,
    .check_available = i_check_available, .flush = r_nothing,
};

int main(int argc, char **argv) {
    verbose = argc > 1;
    if (!render_init(&render_backend) || !input_init(&input_backend)) {
        return 1;
    }
    static const struct { uint16_t kills, deaths; } fates[] = {{0, 0}, {1, 0}, {0, 1}, {7, 3}, {30000, 2}};
    static const struct { uint8_t wake, ignore; } sleeps[] = {{0, 0}, {255, 255}, {7, 200}};
    static const uint8_t attacks_seen[] = {0, 10, 255};
    static const uint16_t levels[] = {1, 2, 3, 8, 11, 12, 18, 21, 40};
    uint32_t total = 2166136261u;
    long count = 0;
    int index = 0;
    for (creature_iterator it = monster_creature_begin(); !monster_creature_same(it, monster_creature_end());
         it = monster_creature_next(it), index++) {
        const creature_type *cp = monster_creature_get(it);
        h = 2166136261u;
        bool shown = false;
        for (int flags = 0; flags < 7; flags++)
        for (size_t f = 0; f < sizeof fates / sizeof fates[0]; f++)
        for (size_t s = 0; s < sizeof sleeps / sizeof sleeps[0]; s++)
        for (size_t a = 0; a < sizeof attacks_seen; a++)
        for (size_t l = 0; l < sizeof levels / sizeof levels[0]; l++)
        for (int wizard = 0; wizard < 2; wizard++) {
            recall_type *mp = recall_get(it);
            static const uint32_t half = 0x55555555u;
            uint32_t own = cp->cmove & ~(uint32_t)CM_TREASURE;
            uint32_t cmove[7] = {0, cp->cmove, cp->cmove & half, 0xFFFFFFFFu,
                                 cp->cmove & (CM_ATTACK_ONLY | CM_ONLY_MAGIC),
                                 own | (1u << CM_TR_SHIFT), own | (2u << CM_TR_SHIFT)};
            uint32_t spells[7] = {0, cp->spells, cp->spells & half, 0xFFFFFFFFu, 0, cp->spells, cp->spells};
            uint16_t cdefense[7] = {0, cp->cdefense, (uint16_t)(cp->cdefense & half), 0xFFFFu, 0,
                                    cp->cdefense, cp->cdefense};
            mp->r_cmove = cmove[flags];
            mp->r_spells = spells[flags];
            mp->r_cdefense = cdefense[flags];
            mp->r_kills = fates[f].kills;
            mp->r_deaths = fates[f].deaths;
            mp->r_wake = sleeps[s].wake;
            mp->r_ignore = sleeps[s].ignore;
            for (int n = 0; n < MAX_MON_NATTACK; n++) {
                mp->r_attacks[n] = attacks_seen[a];
            }
            player_set_level(levels[l]);
            progress_set_wizard_mode(wizard != 0);
            memset(screen, ' ', sizeof screen);
            int key = roff_recall(it);
            mix_int(key);
            count++;
            if (verbose && !shown) {
                shown = true;
                printf("--- %d %s\n", index, cp->name);
                for (int r = 0; r < 24; r++) {
                    screen[r][80] = '\0';
                    int end = 80;
                    while (end > 0 && screen[r][end - 1] == ' ') end--;
                    if (end > 0) printf("%.*s\n", end, screen[r]);
                }
            }
        }
        printf("%3d %08x %s\n", index, h, cp->name);
        total = (total ^ h) * 16777619u;
    }
    printf("total %08x (%ld calls)\n", total, count);
    return 0;
}
