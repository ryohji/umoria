// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Global variables

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

const char *copyright[17] = {
    "Copyright (C) 1989-2008 James E. Wilson, Robert A. Koeneke, ",
    "                        David J. Grabiner",
    "",
    "This file is part of Umoria.",
    "",
    "Umoria is free software; you can redistribute it and/or modify ",
    "it under the terms of the GNU General Public License as published by",
    "the Free Software Foundation, either version 3 of the License, or",
    "(at your option) any later version.",
    "",
    "Umoria is distributed in the hope that it will be useful,",
    "but WITHOUT ANY WARRANTY; without even the implied warranty of ",
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
    "GNU General Public License for more details.",
    "",
    "You should have received a copy of the GNU General Public License ",
    "along with Umoria.  If not, see <http://www.gnu.org/licenses/>."};

// hack_monptr（いま creatures() が誰の手番を処理しているか）はここに無い。
// #18-14-1C で src/monster_turn.c の static になった。窓口は
// src/monster_turn.h（monster_turn_begin / monster_turn_end /
//  monster_delete_may_shift / monster_turn_index）。読み手 2 つが
// `hack_monptr < i` と手で書いていた比較には名前が付いた ——
// 「その席を詰めなおしてよいか」。上流の「horrible hack」の註は
// 呼び手（misc1.c と moria3.c）と窓口の記録に残っている。

// weapon_heavy と pack_heavy（重さに負けているか）はここに無い。#18-7-4C1 で
// src/burden.c の static になった。窓口は src/burden.h
// （weapon_is_too_heavy / set_weapon_too_heavy /
//  pack_speed_penalty / set_pack_speed_penalty）。

// Fifteen records that used to live here now live beside the code that uses
// them, private to their own file and reached through a window:
//
//   progress.c    turn, randes_seed, town_seed, wizard, to_be_wizard
//   score_death.c death, died_from, birth_date, noscore, total_winner,
//                 max_score
//   save_state.c  savefile, character_generated, character_saved, panic_save
//
// See src/progress.h, src/score_death.h and src/save_state.h.
//
// highscore_fp is the fourteenth of that batch and stays here. The two
// functions that read and write the score file (display_scores() and
// highscores(), both in death.c) use a local now, so the only writer left is
// init_scorefile() (files.c), which opens the file while the setuid privileges
// are still there. That handle is never read and never closed -- recorded as
// bug candidate B19, not changed here.
FILE *highscore_fp;               // File pointer to high score file
// How tall and how wide this level is moved to dungeon_size.c (#18-14-5C).
// The windows are in dungeon_size.h. It was the only line here that said
// anything about the shape of a level, and it was two names for one thing: a
// size. Both writers set both halves one after the other and the readers took
// them in pairs, so the window that sets them takes both -- there is no longer
// a way to change half of a size. The pair only ever holds one of two values,
// the town's 22 by 66 or a dungeon level's 66 by 198.
// Which level the game is on now moved to dungeon_level.c (#18-14-6C). The
// windows are in dungeon_level.h. It was one of the few lines here with an
// initializer of its own, and the zero was not an empty container but a place:
// a new game starts in the town. Six of its thirty readers were asking the same
// question -- am I in the town? -- in three different spellings, and they ask
// one window now. **Nothing here says which level the game is on, or how big it
// is, any more.**
// The serial number that tells one batch of missiles from another moved to
// missile_serial.c (#18-11-2C). The window is in missile_serial.h
//
// msg_flag, old_msg and last_msg (the top line: whether its message has been
// seen, and the history ring) moved to messages.c, next to the code that uses
// them
// Whether the player is running moved to running.c (find_flag, #18-11-6C).
// It was a count as well as a flag -- zero meant "not running" -- and the
// hundred-step cut-off went in with it, so the number never leaves the module

bool free_turn_flag; // Used in MORIA, do not move creatures
// What the game remembers about the command being typed moved to
// command_state.c (command_count and default_dir, #18-11-7C; last_command went
// with them from further down this file). All three exist because a command can
// be given a repeat count, so they went into one module

// options set via the '=' command
//
// The starting values live in options.c, beside the prompt and the save file
// bit of each option; game_options_reset() puts them here at startup. They
// used to be split: ten initialized right here, and rogue_like_commands in
// main.c from config.h.
bool rogue_like_commands;
bool find_cut;
bool find_examine;
bool find_bound;
bool find_prself;
bool prompt_carry_flag;
bool show_weight_flag;
bool highlight_seams;
bool find_ignore_doors;
bool sound_beep_flag;
bool display_counts;

// doing_inven and screen_change have moved out (#18-11-3C): the command waiting
// to be resumed lives in inven_command_state.c and the "has the screen been
// flushed" flag in screen_touched.c, both static.
//
// The command before this one moved to command_state.c as well (last_command,
// #18-11-7C), next to the repeat count it belongs with

// these used to be in dungeon.c
// The two flags about the character's light moved to player_light.c: whether a
// light is burning (player_light, #18-7-3C1) and whether its glow is currently
// drawn on the map (light_flag, #18-11-1C). The windows are in player_light.h
// Whether the input has run out moved to input_ended.c (eof_flag, #18-11-5C).
// It counted EOFs, and the count mattered in one place -- the 100 tries io.c
// puts up with before it panic-saves -- so the counting went in with it

// wait_for_more moved to messages.c as well; it is the -more- prompt's own
// state, read by the interrupt handler
int closing_flag = 0; // Used for closing

// The panel (which part of the dungeon the screen shows, and the coordinates
// that follow from it) moved to panel.c, next to the arithmetic that derives
// one from the other

cave_type cave[MAX_HEIGHT][MAX_WIDTH];

static recall_type c_recall[MAX_CREATURES]; // Monster memories

recall_type *recall_get(creature_handle h) {
    return c_recall + h.place;
}

void recall_update_characteristics(creature_handle h, int defence) {
    recall_get(h)->r_cdefense |= defence;
}

void recall_update_move(creature_handle h, int move) {
    recall_get(h)->r_cmove |= move;
}

static inline uint8_t get_carry(uint32_t cmove) {
    return (cmove & CM_TREASURE) >> CM_TR_SHIFT;
}

void recall_update_carry(creature_handle h, uint8_t number) {
    recall_type *const recall = recall_get(h);
    uint32_t current = get_carry(recall->r_cmove);
    recall->r_cmove &= ~CM_TREASURE;
    recall->r_cmove |= MAX(current, number) << CM_TR_SHIFT;
}

void recall_update_spell(creature_handle h, uint32_t type) {
    recall_type *const recall = recall_get(h);
    recall->r_spells |= type;
}

void recall_increment_spell_chance(creature_handle h) {
    recall_type *const recall = recall_get(h);
    if ((recall->r_spells & CS_FREQ) != CS_FREQ) {
        recall->r_spells += 1;
    }
}

void recall_increment_kill(creature_handle h) {
    recall_type *const recall = recall_get(h);
    if (recall->r_kills < MAX_SHORT) {
        recall->r_kills += 1;
    }
}

void recall_increment_death(creature_handle h) {
    recall_type *const recall = recall_get(h);
    if (recall->r_deaths < MAX_SHORT) {
        recall->r_deaths += 1;
    }
}
