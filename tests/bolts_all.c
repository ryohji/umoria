// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The bolts that fly from the player to the first creature in a line, through
// every case of a test floor: the spells of spells.c that go for a creature
// (hp_monster, drain_life, speed_monster, confuse_monster, sleep_monster,
// poly_monster, clone_monster, teleport_monster) and fire_bolt, in each of the
// 8 directions, with nobody in the line, a creature next to the player, a few
// squares away, behind a wall, two in a row, at the end of the range and past
// it, for five kinds of creature, a few strengths and three seeds. What the
// call returns, the state of every creature after it (where, hit points, speed,
// sleep, confusion, stun, kind), the player's experience and hit points, the
// floor items, and what was drawn on the screen go into one hash per spell,
// printed with a total; make -f makefile.test golden compares them with
// tests/golden/bolts_all.hashes.
#include "config.h"
#include "constant.h"
#include "types.h"
#include "backend.h"
#include "dungeon_state.h"
#include "item_kinds.h"
#include "monster_state.h"
#include "monsters.h"
#include "player_condition.h"
#include "player_progress.h"
#include "rnd.h"
#include "ui_input.h"
#include "ui_render.h"
#include "world_combat.h"
#include "world_item.h"
#include "world_monster.h"
#include "world_view.h"
#include "ui_screen.h"
#include "world_internal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t h;
static void mix_byte(uint8_t b) { h = (h ^ b) * 16777619u; }
static void mix_int(int v) { for (int n = 0; n < 4; n++) mix_byte((uint8_t)(v >> (8 * n))); }
static void mix_string(const char *s) { for (; *s; s++) mix_byte((uint8_t)*s); mix_byte('\n'); }

static bool r_init(void) { return true; }
static void r_nothing(void) {}
static void r_erase_line(int row, int col) { mix_byte('E'); mix_int(row); mix_int(col); }
static void r_clear_from(int row) { mix_byte('C'); mix_int(row); }
static void r_clear(void) { mix_byte('K'); }
static void r_draw_char(int row, int col, char ch, RenderColor color) {
    (void)color; mix_byte('c'); mix_int(row); mix_int(col); mix_byte((uint8_t)ch);
}
static void r_draw_string(int row, int col, const char *str, RenderColor color) {
    (void)color; mix_byte('s'); mix_int(row); mix_int(col); mix_string(str);
}
static void r_move_cursor(int row, int col) { (void)row; (void)col; }
static void r_show_cursor(bool show) { (void)show; }

static RenderBackend render_backend = {
    .name = "bolts_all", .init = r_init, .shutdown = r_nothing,
    .clear = r_clear, .erase_line = r_erase_line,
    .clear_from = r_clear_from, .refresh = r_nothing, .draw_char = r_draw_char,
    .draw_string = r_draw_string, .move_cursor = r_move_cursor, .show_cursor = r_show_cursor,
    .bell = r_nothing,
    .save_screen = r_nothing, .restore_screen = r_nothing,
};

static int i_get_key(void) { return ' '; }
static bool i_check_available(int microsec) { (void)microsec; return false; }
static InputBackend input_backend = {
    .name = "bolts_all", .init = r_init, .shutdown = r_nothing, .get_key = i_get_key,
    .check_available = i_check_available, .flush = r_nothing,
};

enum { PY = 11, PX = 33 };

// A floor the size of the town with granite all round, the player in the middle.
static void open_floor(void) {
    set_dungeon_size(SCREEN_HEIGHT, SCREEN_WIDTH);
    dungeon_map_reset();
    for (int y = 0; y < dungeon_height(); y++) {
        for (int x = 0; x < dungeon_width(); x++) {
            bool edge = y == 0 || x == 0 || y == dungeon_height() - 1 || x == dungeon_width() - 1;
            square_at(y, x)->fval = edge ? GRANITE_WALL : LIGHT_FLOOR;
            square_at(y, x)->pl = true;
        }
    }
    monster_list_reset();
    floor_items_reset();
    player_place(PY, PX);
    square_at(PY, PX)->cptr = 1;
    panel_set_dungeon_size(dungeon_height(), dungeon_width());
    panel_forget_position();
    (void)get_panel(PY, PX, true);
}

static const int dy[10] = {0, 1, 1, 1, 0, 0, 0, -1, -1, -1};
static const int dx[10] = {0, -1, 0, 1, -1, 0, 1, -1, 0, 1};

static bool in_floor(int y, int x) { return y > 0 && x > 0 && y < dungeon_height() - 1 && x < dungeon_width() - 1; }

static void put_creature(int dir, int d, creature_iterator kind) {
    int y = PY + dy[dir] * d;
    int x = PX + dx[dir] * d;
    if (in_floor(y, x)) {
        (void)place_monster(y, x, kind, false);
    }
}

static void put_wall(int dir, int d) {
    int y = PY + dy[dir] * d;
    int x = PX + dx[dir] * d;
    if (in_floor(y, x)) {
        square_at(y, x)->fval = GRANITE_WALL;
    }
}

// The cases of the line in front of the player.
enum { LAYOUTS = 7 };
static void lay_out(int layout, int dir, creature_iterator kind) {
    switch (layout) {
    case 0: break;                                       // nobody
    case 1: put_creature(dir, 1, kind); break;           // next to the player
    case 2: put_creature(dir, 3, kind); break;           // a few squares away
    case 3: put_wall(dir, 2); put_creature(dir, 4, kind); break; // behind a wall
    case 4: put_creature(dir, 2, kind); put_creature(dir, 5, kind); break; // two in a row
    case 5: put_creature(dir, OBJ_BOLT_RANGE, kind); break;     // at the end of the range
    case 6: put_creature(dir, OBJ_BOLT_RANGE + 1, kind); break; // past it
    }
}

static void mix_state(int result) {
    mix_byte('R'); mix_int(result);
    for (int n = 0; n < monster_list_used(); n++) {
        const monster_type *m = monster_list_at(n);
        mix_int(m->hp); mix_int(m->csleep); mix_int(m->cspeed); mix_int(m->creature.place);
        mix_int(m->fy); mix_int(m->fx); mix_int(m->confused); mix_int(m->stunned);
    }
    mix_int(player_experience()); mix_int(player_hp());
    mix_int(floor_items_used());
    for (int y = 0; y < dungeon_height(); y++) {
        for (int x = 0; x < dungeon_width(); x++) {
            mix_byte(square_at(y, x)->cptr);
        }
    }
}

typedef int (*bolt_fn)(int dir, int strength);
static int f_hp(int dir, int s) { return hp_monster(dir, PY, PX, s); }
static int f_drain(int dir, int s) { (void)s; return drain_life(dir, PY, PX); }
static int f_speed(int dir, int s) { return speed_monster(dir, PY, PX, s); }
static int f_confuse(int dir, int s) { (void)s; return confuse_monster(dir, PY, PX); }
static int f_sleep(int dir, int s) { (void)s; return sleep_monster(dir, PY, PX); }
static int f_poly(int dir, int s) { (void)s; return poly_monster(dir, PY, PX); }
static int f_clone(int dir, int s) { (void)s; return clone_monster(dir, PY, PX); }
static int f_teleport(int dir, int s) { (void)s; return teleport_monster(dir, PY, PX); }
static int f_bolt(int dir, int s) { fire_bolt(s < 100 ? GF_MAGIC_MISSILE : GF_FIRE, dir, PY, PX, s, "bolt"); return 0; }

static const struct {
    const char *name;
    bolt_fn fn;
    int strengths[3];
    int count;
} spells[] = {
    {"hp_monster", f_hp, {0, 3, 10000}, 3},
    {"drain_life", f_drain, {0}, 1},
    {"speed_monster", f_speed, {10, -10}, 2},
    {"confuse_monster", f_confuse, {0}, 1},
    {"sleep_monster", f_sleep, {0}, 1},
    {"poly_monster", f_poly, {0}, 1},
    {"clone_monster", f_clone, {0}, 1},
    {"teleport_monster", f_teleport, {0}, 1},
    {"fire_bolt", f_bolt, {10, 1000}, 2},
};

int main(void) {
    if (!render_init(&render_backend) || !input_init(&input_backend)) {
        return 1;
    }
    magic_init();
    monster_levels_init();
    object_levels_init();
    set_dungeon_level(5);

    // Five kinds: the first of level 1 or more that sleeps and is not undead,
    // the first that never sleeps, the first undead, the first of level 30 or
    // more, and the last (the Balrog).
    creature_iterator kinds[5];
    bool have[5] = {false};
    creature_iterator last = monster_creature_begin();
    for (creature_iterator it = monster_creature_begin(); !monster_creature_same(it, monster_creature_end());
         it = monster_creature_next(it)) {
        const creature_type *cp = monster_creature_get(it);
        bool picks[4] = {
            cp->level >= 1 && !(cp->cdefense & (CD_NO_SLEEP | CD_UNDEAD)),
            (cp->cdefense & CD_NO_SLEEP) != 0,
            (cp->cdefense & CD_UNDEAD) != 0,
            cp->level >= 30,
        };
        for (int k = 0; k < 4; k++) {
            if (picks[k] && !have[k]) {
                kinds[k] = it;
                have[k] = true;
            }
        }
        last = it;
    }
    kinds[4] = last;

    player_set_level(10);
    uint32_t total = 2166136261u;
    long count = 0;
    for (size_t s = 0; s < sizeof spells / sizeof spells[0]; s++) {
        h = 2166136261u;
        for (int st = 0; st < spells[s].count; st++)
        for (int dir = 1; dir <= 9; dir++) {
            if (dir == 5) continue;
            for (int layout = 0; layout < LAYOUTS; layout++)
            for (int k = 0; k < 5; k++)
            for (uint32_t seed = 1; seed <= 3; seed++) {
                set_rnd_seed(seed * 7919u + (uint32_t)k);
                open_floor();
                lay_out(layout, dir, kinds[k]);
                update_monsters(); // who can be seen, without moving them
                player_set_experience(0);
                int result = spells[s].fn(dir, spells[s].strengths[st]);
                mix_state(result);
                count++;
            }
        }
        printf("%-16s %08x\n", spells[s].name, h);
        total = (total ^ h) * 16777619u;
    }
    printf("total %08x (%ld calls)\n", total, count);
    return 0;
}
