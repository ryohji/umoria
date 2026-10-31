// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Every creature of the monster table takes four turns with creatures(),
// awake and three squares from the player, on a test floor: it casts spells
// (mon_cast_spell), moves (mon_move) and attacks. From a known state of the
// player: a human warrior of level
// 20 with 500 hit points, every stat at 16, gold, experience, food, a potion,
// a wand, and a dagger, soft leather armour and a torch on. Each creature
// takes its turns with each of eight seeds, in
// three states of the player (as above; with plusses on what is worn,
// paralysed and blind; with every ability), the first with protection from
// evil on. What was drawn on the screen and the
// state afterwards (the player's hit points, stats, timed effects, gold,
// experience, place, death, pack and equipment, and the creature) go into one
// hash per creature, printed with a total; make -f makefile.test golden
// compares them with tests/golden/turns_all.hashes.
#include "config.h"
#include "constant.h"
#include "types.h"
#include "backend.h"
#include "dungeon_state.h"
#include "game_status.h"
#include "inventory.h"
#include "item_kinds.h"
#include "monster_state.h"
#include "monsters.h"
#include "player_condition.h"
#include "player_identity.h"
#include "player_progress.h"
#include "player_stats.h"
#include "rnd.h"
#include "ui_input.h"
#include "ui_render.h"
#include "ui_screen.h"
#include "world_item.h"
#include "world_monster.h"
#include "world_view.h"
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
    .name = "turns_all", .init = r_init, .shutdown = r_nothing,
    .clear = r_clear, .erase_line = r_erase_line,
    .clear_from = r_clear_from, .refresh = r_nothing, .draw_char = r_draw_char,
    .draw_string = r_draw_string, .move_cursor = r_move_cursor, .show_cursor = r_show_cursor,
    .bell = r_nothing,
    .save_screen = r_nothing, .restore_screen = r_nothing,
};

static int i_get_key(void) { return ' '; }
static bool i_check_available(int microsec) { (void)microsec; return false; }
static InputBackend input_backend = {
    .name = "turns_all", .init = r_init, .shutdown = r_nothing, .get_key = i_get_key,
    .check_available = i_check_available, .flush = r_nothing,
};

enum { PY = 11, PX = 33 };

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

static void carry(int kind, int number, int p1) {
    inven_type item;
    invcopy(&item, kind);
    item.number = (uint8_t)number;
    if (p1 >= 0) {
        item.p1 = (int16_t)p1;
    }
    (void)inven_carry(&item);
}

static void wear(int slot, int kind) {
    invcopy(equipment_at(slot), kind);
}

// The player as every attack finds them.
static void known_player(void) {
    set_player_dead(false);
    player_class_set(0);
    player_race_set(0);
    player_set_level(20);
    player_set_experience_factor(100);
    player_set_max_experience(10000);
    player_set_experience(10000);
    player_set_max_hp(500);
    player_set_hp(500);
    player_set_gold(1000);
    for (int s = 0; s < 6; s++) {
        player_stat_set_max(s, 16);
        player_stat_set_cur(s, 16);
        player_stat_set_mod(s, 0);
        player_stat_set_use(s, 16);
    }
    for (int e = 0; e < PLAYER_TIMED_COUNT; e++) {
        player_timed_clear((player_timed_effect)e);
    }
    for (int n = 0; n < INVEN_WIELD; n++) {
        invcopy(inventory_at(n), OBJ_NOTHING);
    }
    inventory_set_count(0);
    inventory_set_weight(0);
    for (int slot = INVEN_WIELD; slot <= INVEN_AUX; slot++) {
        invcopy(equipment_at(slot), OBJ_NOTHING);
    }
    carry(21, 5, -1);  // rations of food
    carry(237, 3, -1); // potions of cure light wounds
    carry(282, 1, 10); // a wand of magic missile, ten charges
    wear(INVEN_WIELD, 28);  // a dagger
    wear(INVEN_BODY, 103);  // soft leather armour
    wear(INVEN_LIGHT, 86);  // a torch
    equipment_at(INVEN_LIGHT)->p1 = 4000;
    equipment_set_count(3);
    player_armour_class_set_parts(4, 0);
    player_abilities_forget_all();
}

// Variant 1: plusses on what is worn, paralysed and blind. Variant 2: every
// ability (resistances, sustains, free action...).
static void vary_player(int variant) {
    if (variant == 1) {
        equipment_at(INVEN_WIELD)->tohit = 3;
        equipment_at(INVEN_WIELD)->todam = 3;
        equipment_at(INVEN_BODY)->toac = 3;
        player_timed_set(PLAYER_TIMED_PARALYSIS, 5);
        player_timed_set(PLAYER_TIMED_BLINDNESS, 5);
    } else if (variant == 2) {
        for (int n = 0; n < PLAYER_ABILITIES_SAVED_BYTES; n++) {
            player_abilities_restore_byte(n, 1);
        }
    }
}

static void mix_item(const inven_type *t) {
    mix_int(t->index); mix_int(t->tval); mix_int(t->number); mix_int(t->p1);
    mix_int(t->tohit); mix_int(t->todam); mix_int(t->toac); mix_int((int)t->flags); mix_int(t->ident);
}

static void mix_state(void) {
    mix_byte('P');
    mix_int(player_is_dead()); mix_int(player_hp()); mix_int(player_max_hp());
    mix_int(player_gold()); mix_int(player_experience()); mix_int(player_row()); mix_int(player_col());
    for (int s = 0; s < 6; s++) {
        mix_int(player_stat_cur(s)); mix_int(player_stat_max(s));
    }
    for (int e = 0; e < PLAYER_TIMED_COUNT; e++) {
        mix_int(player_timed_turns((player_timed_effect)e));
    }
    mix_int(inventory_count());
    for (int n = 0; n < inventory_count(); n++) {
        mix_item(inventory_at(n));
    }
    for (int slot = INVEN_WIELD; slot <= INVEN_AUX; slot++) {
        mix_item(equipment_at(slot));
    }
    for (int n = 0; n < monster_list_used(); n++) {
        const monster_type *m = monster_list_at(n);
        mix_int(m->hp); mix_int(m->fy); mix_int(m->fx); mix_int(m->confused); mix_int(m->creature.place);
    }
}

int main(void) {
    if (!render_init(&render_backend) || !input_init(&input_backend)) {
        return 1;
    }
    magic_init();
    monster_levels_init();
    object_levels_init();
    set_dungeon_level(5);

    uint32_t total = 2166136261u;
    long count = 0;
    int index = 0;
    for (creature_iterator it = monster_creature_begin(); !monster_creature_same(it, monster_creature_end());
         it = monster_creature_next(it), index++) {
        h = 2166136261u;
        for (int variant = 0; variant < 3; variant++)
        for (uint32_t seed = 1; seed <= 8; seed++) {
            set_rnd_seed(seed * 15485863u + (uint32_t)index + 7919u * (uint32_t)variant);
            open_floor();
            known_player();
            vary_player(variant);
            if (variant == 0 && seed == 1) {
                player_timed_set(PLAYER_TIMED_PROTECTION_FROM_EVIL, 100);
            }
            (void)place_monster(PY, PX + 3, it, false);
            monster_list_at(monster_list_used() - 1)->csleep = 0;
            update_monsters();
            for (int turn = 0; turn < 4 && !player_is_dead(); turn++) {
                creatures();
            }
            mix_state();
            count++;
        }
        printf("%3d %08x %s\n", index, h, monster_creature_get(it)->name);
        total = (total ^ h) * 16777619u;
    }
    printf("total %08x (%ld calls)\n", total, count);
    return 0;
}
