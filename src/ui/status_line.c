// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The status line: the stats, the numbers and the conditions down the left
// side and along the bottom of the screen

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "command_state.h"
#include "dungeon_level.h"
#include "player_bio.h"
#include "player_class.h"
#include "player_display_numbers.h"
#include "player_gold.h"
#include "player_hp.h"
#include "player_level.h"
#include "player_mana.h"
#include "player_race.h"
#include "player_resting.h"
#include "player_speed.h"
#include "player_spells_to_learn.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"
#include "progress.h"
#include "score_death.h"
#include "screen_fields.h"

// Converts stat num into string -RAK-
void cnv_stat(uint8_t stat, char *out_val) {
    if (stat > 18) {
        int part1 = 18;
        int part2 = stat - 18;

        if (part2 == 100) {
            (void)strcpy(out_val, "18/100");
        } else {
            (void)sprintf(out_val, " %2d/%02d", part1, part2);
        }
    } else {
        (void)sprintf(out_val, "%6d", stat);
    }
}

// Print character stat in given row, column -RAK-
void prt_stat(int stat) {
    stat_type out_val1;
    cnv_stat(py.stats.use_stat[stat], out_val1);
    prt_stat_name(stat, 6 + stat, STAT_COLUMN);
    put_buffer(out_val1, 6 + stat, STAT_COLUMN + 6);
}

// Print character info in given row, column -RAK-
// The longest title is 13 characters, so only pad to 13
static void prt_field(const char *info, int row, int column) {
    erase_field(13, row, column);
    put_buffer(info, row, column);
}

// Print long number with header at given row, column
static void prt_lnum(const char *header, int32_t num, int row, int column) {
    vtype out_val;
    (void)sprintf(out_val, "%s: %6d", header, num);
    put_buffer(out_val, row, column);
}

// Print number at given row, column -RAK-
static void prt_int(int num, int row, int column) {
    vtype out_val;
    (void)sprintf(out_val, "%6d", num);
    put_buffer(out_val, row, column);
}

const char *title_string(void) {
    const char *p;

    if (player_level() < 1) {
        p = "Babe in arms";
    } else if (player_level() <= MAX_PLAYER_LEVEL) {
        p = player_title[player_class()][player_level() - 1];
    } else if (player_is_male()) {
        p = "**KING**";
    } else {
        p = "**QUEEN**";
    }

    return p;
}

// Prints title of character -RAK-
void prt_title(void) {
    prt_field(title_string(), 4, STAT_COLUMN);
}

// Prints level -RAK-
void prt_level(void) {
    prt_int((int)player_level(), 13, STAT_COLUMN + 6);
}

// Prints players current mana points. -RAK-
void prt_cmana(void) {
    prt_int(player_mana(), 15, STAT_COLUMN + 6);
}

// Prints Max hit points -RAK-
void prt_mhp(void) {
    prt_int(player_max_hp(), 16, STAT_COLUMN + 6);
}

// Prints players current hit points -RAK-
void prt_chp(void) {
    prt_int(player_hp(), 17, STAT_COLUMN + 6);
}

// prints current AC -RAK-
void prt_pac(void) {
    prt_int(player_display_ac(), 19, STAT_COLUMN + 6);
}

// Prints current gold -RAK-
void prt_gold(void) {
    prt_long(player_gold(), 20, STAT_COLUMN + 6);
}

// Prints depth in stat area -RAK-
void prt_depth(void) {
    vtype depths;

    // Fifty feet a level. The score multiplies the same depth by the same
    // fifty and calls the answer points -- see dungeon_level.h.
    int depth = dungeon_level() * 50;

    if (depth == 0) {
        (void)strcpy(depths, "Town level");
    } else {
        (void)sprintf(depths, "%d feet", depth);
    }

    prt(depths, 23, 65);
}

// Prints status of hunger -RAK-
void prt_hunger(void) {
    if (player_effect_in_force(PLAYER_EFFECT_WEAK)) {
        put_buffer("Weak  ", 23, 0);
    } else if (player_effect_in_force(PLAYER_EFFECT_HUNGRY)) {
        put_buffer("Hungry", 23, 0);
    } else {
        erase_field(6, 23, 0);
    }
}

// Prints Blind status -RAK-
void prt_blind(void) {
    if (player_effect_in_force(PLAYER_EFFECT_BLIND)) {
        put_buffer("Blind", 23, 7);
    } else {
        erase_field(5, 23, 7);
    }
}

// Prints Confusion status -RAK-
void prt_confused(void) {
    if (player_effect_in_force(PLAYER_EFFECT_CONFUSED)) {
        put_buffer("Confused", 23, 13);
    } else {
        erase_field(8, 23, 13);
    }
}

// Prints Fear status -RAK-
void prt_afraid(void) {
    if (player_effect_in_force(PLAYER_EFFECT_AFRAID)) {
        put_buffer("Afraid", 23, 22);
    } else {
        erase_field(6, 23, 22);
    }
}

// Prints Poisoned status -RAK-
void prt_poisoned(void) {
    if (player_effect_in_force(PLAYER_EFFECT_POISONED)) {
        put_buffer("Poisoned", 23, 29);
    } else {
        erase_field(8, 23, 29);
    }
}

// Prints Searching, Resting, Paralysis, or 'count' status -RAK-
void prt_state(void) {
    player_set_status_line_shows_repeat(false);

    if (player_timed_turns(PLAYER_TIMED_PARALYSIS) > 1) {
        put_buffer("Paralysed", 23, 38);
    } else if (player_is_resting()) {
        char tmp[16];

        if (player_rest_is_until_healed()) {
            (void)strcpy(tmp, "Rest *");
        } else if (display_counts) {
            (void)sprintf(tmp, "Rest %-5d", player_rest_turns());
        } else {
            (void)strcpy(tmp, "Rest");
        }
        put_buffer(tmp, 23, 38);
    } else if (command_is_repeating()) {
        // Big enough for "Repeat ", an int in decimal (at most 11 characters with
        // the sign) and the terminator. The count left is an int, so the size
        // follows from the type, not from the range of values. 16 was too small.
        char tmp[sizeof("Repeat ") + 11];

        if (display_counts) {
            (void)sprintf(tmp, "Repeat %-3d", command_count_remaining());
        } else {
            (void)strcpy(tmp, "Repeat");
        }

        player_set_status_line_shows_repeat(true);

        put_buffer(tmp, 23, 38);
        if (player_is_searching()) {
            put_buffer("Search", 23, 38);
        }
    } else if (player_is_searching()) {
        put_buffer("Searching", 23, 38);
    } else {
        // "repeat 999" is 10 characters
        erase_field(10, 23, 38);
    }
}

// Prints the speed of a character. -CJS-
void prt_speed(void) {
    int i = player_speed();

    // Search mode.
    if (player_is_searching()) {
        i--;
    }

    if (i > 1) {
        put_buffer("Very Slow", 23, 49);
    } else if (i == 1) {
        put_buffer("Slow     ", 23, 49);
    } else if (i == 0) {
        erase_field(9, 23, 49);
    } else if (i == -1) {
        put_buffer("Fast     ", 23, 49);
    } else {
        put_buffer("Very Fast", 23, 49);
    }
}

void prt_study(void) {
    player_clear_study_redraw_request();

    if (player_spells_to_learn() == 0) {
        erase_field(5, 23, 59);
    } else {
        put_buffer("Study", 23, 59);
    }
}

// Prints winner status on display -RAK-
void prt_winner(void) {
    if (score_disqualifications() & SCORE_DISQUALIFY_WIZARD) {
        if (progress_wizard_mode()) {
            put_buffer("Is wizard  ", 22, 0);
        } else {
            put_buffer("Was wizard ", 22, 0);
        }
    } else if (score_disqualifications() & SCORE_DISQUALIFY_RESURRECTED) {
        put_buffer("Resurrected", 22, 0);
    } else if (score_disqualifications() & SCORE_DISQUALIFY_DUPLICATE) {
        put_buffer("Duplicate", 22, 0);
    } else if (player_has_won()) {
        put_buffer("*Winner*   ", 22, 0);
    }
}

// Prints character-screen info -RAK-
void prt_stat_block(void) {
    prt_field(player_race_name(), 2, STAT_COLUMN);
    prt_field(player_class_title(), 3, STAT_COLUMN);
    prt_field(title_string(), 4, STAT_COLUMN);

    for (int i = 0; i < 6; i++) {
        prt_stat(i);
    }

    prt_num("LEV ", (int)player_level(), 13, STAT_COLUMN);
    prt_lnum("EXP ", player_experience(), 14, STAT_COLUMN);
    prt_num("MANA", player_mana(), 15, STAT_COLUMN);
    prt_num("MHP ", player_max_hp(), 16, STAT_COLUMN);
    prt_num("CHP ", player_hp(), 17, STAT_COLUMN);
    prt_num("AC  ", player_display_ac(), 19, STAT_COLUMN);
    prt_lnum("GOLD", player_gold(), 20, STAT_COLUMN);
    prt_winner();

    // Each window is asked in turn. The only calls below that change
    // any state are prt_state() (what it remembers of Repeat) and prt_study()
    // (the Study request), and neither touches the bits looked at here.
    if (player_effect_in_force(PLAYER_EFFECT_HUNGRY) || player_effect_in_force(PLAYER_EFFECT_WEAK)) {
        prt_hunger();
    }
    if (player_effect_in_force(PLAYER_EFFECT_BLIND)) {
        prt_blind();
    }
    if (player_effect_in_force(PLAYER_EFFECT_CONFUSED)) {
        prt_confused();
    }
    if (player_effect_in_force(PLAYER_EFFECT_AFRAID)) {
        prt_afraid();
    }
    if (player_effect_in_force(PLAYER_EFFECT_POISONED)) {
        prt_poisoned();
    }
    if (player_is_searching() || player_is_resting()) {
        prt_state();
    }

    // if speed non zero, print it, modify speed if Searching
    // One less while searching, the same as the i-- in prt_speed(). THE SAME
    // RULE IS WRITTEN IN TWO PLACES, AND IT IS NOT FOLDED -- searching and speed
    // are separate questions, and neither is half of the other
    // (src/player/player_speed.h).
    if (player_speed() - (player_is_searching() ? 1 : 0) != 0) {
        prt_speed();
    }

    // display the study field
    prt_study();
}

// Draws entire screen -RAK-
void draw_cave(void) {
    clear_screen();
    prt_stat_block();
    prt_map();
    prt_depth();
}
