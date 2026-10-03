// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Create a player character

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "hp_table.h"
#include "player_armour_class.h"
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_bio.h"
#include "player_body_weight.h"
#include "player_class.h"
#include "player_disarm.h"
#include "player_display_numbers.h"
#include "player_gold.h"
#include "player_hit_die.h"
#include "player_hp.h"
#include "player_infra_range.h"
#include "player_level.h"
#include "player_race.h"
#include "player_saving_throw.h"
#include "player_search_skill.h"
#include "player_stealth.h"
#include "stats.h"

// Generates character's stats -JWT-
static void get_stats(void) {
    int tot;
    int dice[18];

    do {
        tot = 0;
        for (int i = 0; i < 18; i++) {
            // Roll 3,4,5 sided dice once each
            dice[i] = randint(3 + i % 3);
            tot += dice[i];
        }
    } while (tot <= 42 || tot >= 54);

    for (int i = 0; i < 6; i++) {
        player_stat_set_max(i, 5 + dice[3 * i] + dice[3 * i + 1] + dice[3 * i + 2]);
    }
}

// Changes stats by given amount -JWT-
static void change_stat(int stat, int16_t amount) {
    int tmp_stat = player_stat_max(stat);

    if (amount < 0) {
        for (int i = 0; i > amount; i--) {
            if (tmp_stat > 108) {
                tmp_stat--;
            } else if (tmp_stat > 88) {
                tmp_stat += -randint(6) - 2;
            } else if (tmp_stat > 18) {
                tmp_stat += -randint(15) - 5;
                if (tmp_stat < 18) {
                    tmp_stat = 18;
                }
            } else if (tmp_stat > 3) {
                tmp_stat--;
            }
        }
    } else {
        for (int i = 0; i < amount; i++) {
            if (tmp_stat < 18) {
                tmp_stat++;
            } else if (tmp_stat < 88) {
                tmp_stat += randint(15) + 5;
            } else if (tmp_stat < 108) {
                tmp_stat += randint(6) + 2;
            } else if (tmp_stat < 118) {
                tmp_stat++;
            }
        }
    }
    player_stat_set_max(stat, tmp_stat);
}

// generate all stats and modify for race. needed in a separate
// module so looping of character selection would be allowed -RGM-
static void get_all_stats(void) {
    race_type *r_ptr = &race[player_race()];

    get_stats();
    change_stat(A_STR, r_ptr->str_adj);
    change_stat(A_INT, r_ptr->int_adj);
    change_stat(A_WIS, r_ptr->wis_adj);
    change_stat(A_DEX, r_ptr->dex_adj);
    change_stat(A_CON, r_ptr->con_adj);
    change_stat(A_CHR, r_ptr->chr_adj);

    player_set_level(1);

    for (int j = 0; j < 6; j++) {
        player_stat_set_cur(j, player_stat_max(j));
        set_use_stat(j);
    }

    // **Two windows for search, unlike base to-hit** — wizard.c replaces only the
    // chance, so each needs its own window. That is why the two lines are
    // separated (other questions in between).
    player_search_chance_set(r_ptr->srh);
    // **Base to-hit uses one window for both.** Race always sets both.
    player_base_to_hit_set(r_ptr->bth, r_ptr->bthb);
    player_search_frequency_set(r_ptr->fos);
    // **All race table assignments go through windows** — this function does not
    // name the player record (the stat side writes `py.stats` directly).
    player_stealth_set(r_ptr->stl);
    // **Saving throw from race table directly, no bonuses mixed in** — disarm
    // bakes in the DEX bonus here, but this one does not.
    player_saving_throw_set(r_ptr->bsav);
    player_hit_die_set(r_ptr->bhitdie);
    // **Attack bonuses from one window, race table not involved** — both come from
    // the DEX and STR adjustment tables, temporary values that :397 replaces.
    // **Arguments are to-hit, to-damage** (the original two lines had damage
    // first, but order does not matter since both `_adj()` only read stats).
    player_attack_bonuses_set(tohit_adj(), todam_adj());
    // The one place that puts the dexterity bonus in the ARMOUR half rather
    // than the magical one -- the class table below does it the other way
    // round. Nothing can tell the two spellings apart, because every reader
    // only ever asks for the sum (player_armour_class.h).
    player_armour_class_set_parts(toac_adj(), 0);
    player_set_experience_factor(r_ptr->b_exp);
    // The only question so far whose starting value is not zero: Human 0,
    // Dwarf 5, and five more in between. Deciding, not adding -- a character
    // made twice must not see twice as far (player_infra_range.h).
    player_infra_range_set(r_ptr->infra);
}

// Allows player to select a race -JWT-
static void choose_race(void) {
    int j = 0;
    int k = 0;
    int l = 2;
    int m = 21;

    clear_from(20);
    put_buffer("Choose a race (? for Help):", 20, 2);

    char tmp_str[80];
    do {
        (void)sprintf(tmp_str, "%c) %s", k + 'a', race[j].trace);
        put_buffer(tmp_str, m, l);
        k++;
        l += 15;
        if (l > 70) {
            l = 2;
            m++;
        }
        j++;
    } while (j < MAX_RACES);

    bool exit_flag = false;

    char s;
    do {
        move_cursor(20, 30);
        s = inkey();
        j = s - 'a';
        if ((j < MAX_RACES) && (j >= 0)) {
            exit_flag = true;
        } else if (s == '?') {
            helpfile(MORIA_WELCOME);
        } else {
            bell();
        }
    } while (!exit_flag);

    race_type *r_ptr = &race[j];
    player_race_set(j);
    put_buffer(r_ptr->trace, 3, 15);
}

// Will print the history of a character -JWT-
static void print_history(void) {
    put_buffer("Character Background", 14, 27);

    for (int i = 0; i < PLAYER_HISTORY_LINES; i++) {
        prt(player_history_line(i), i + 15, 10);
    }
}

// Get the racial history, determines social class -RAK-
//
// Assumptions:
//   - Each race has init history beginning at (race-1)*3+1
//   - All history parts are in ascending order
static void get_history(void) {
    char history_block[240];
    background_type *b_ptr;
    int test_roll;
    bool flag;

    // Start of the history table. **Row number arithmetic stays here** — this is
    // knowledge of background[] ordering (the Assumptions above say so), not a
    // property of the race (player_race.h).
    int hist_ptr = player_race() * 3 + 1;
    int social_class = randint(4);
    int cur_ptr = 0;
    history_block[0] = '\0';

    // Get a block of history text
    do {
        flag = false;
        do {
            if (background[cur_ptr].chart == hist_ptr) {
                test_roll = randint(100);
                while (test_roll > background[cur_ptr].roll) {
                    cur_ptr++;
                }
                b_ptr = &background[cur_ptr];
                (void)strcat(history_block, b_ptr->info);
                social_class += b_ptr->bonus - 50;
                if (hist_ptr > b_ptr->next) {
                    cur_ptr = 0;
                }
                hist_ptr = b_ptr->next;
                flag = true;
            } else {
                cur_ptr++;
            }
        } while (!flag);
    } while (hist_ptr >= 1);

    // Clear the previous history strings
    player_history_clear();

    // Process block of history text for pretty output
    int end_pos = (int)strlen(history_block) - 1;
    while (history_block[end_pos] == ' ') {
        end_pos--;
    }

    int cur_len;
    int new_start = 0;

    int start_pos = 0;
    int line_ctr = 0;

    flag = false;
    do {
        while (history_block[start_pos] == ' ') {
            start_pos++;
        }

        cur_len = end_pos - start_pos + 1;
        if (cur_len > 60) {
            cur_len = 60;
            while (history_block[start_pos + cur_len - 1] != ' ') {
                cur_len--;
            }
            new_start = start_pos + cur_len;
            while (history_block[start_pos + cur_len - 1] == ' ') {
                cur_len--;
            }
        } else {
            flag = true;
        }

        // Build one wrapped line locally before passing it. cur_len is capped at 60
        // above, so with the terminator it fits exactly in PLAYER_HISTORY_LINE_SIZE
        // (when writing directly to the field, the terminator of a 60-char line
        // fell into **the start of the next line** — bug candidate B21; see
        // types.h).
        char line[PLAYER_HISTORY_LINE_SIZE];
        (void)strncpy(line, &history_block[start_pos], (size_t)cur_len);
        line[cur_len] = '\0';
        player_history_line_set(line_ctr, line);
        line_ctr++;
        start_pos = new_start;
    } while (!flag);

    // Compute social class for player
    if (social_class > 100) {
        social_class = 100;
    } else if (social_class < 1) {
        social_class = 1;
    }
    player_social_class_set(social_class);
}

// Gets the character's sex -JWT-
static void get_sex(void) {
    char c;
    bool exit_flag = false;

    clear_from(20);
    put_buffer("Choose a sex (? for Help):", 20, 2);
    put_buffer("m) Male       f) Female", 21, 2);
    do {
        move_cursor(20, 29);
        // speed not important here
        c = inkey();
        if (c == 'f' || c == 'F') {
            player_set_male(false);
            put_buffer("Female", 4, 15);
            exit_flag = true;
        } else if (c == 'm' || c == 'M') {
            player_set_male(true);
            put_buffer("Male", 4, 15);
            exit_flag = true;
        } else if (c == '?') {
            helpfile(MORIA_WELCOME);
        } else {
            bell();
        }
    } while (!exit_flag);
}

// Computes character's age, height, and weight -JWT-
static void get_ahw(void) {
    int i = player_race();
    player_age_set(race[i].b_age + randint((int)race[i].m_age));
    if (player_is_male()) {
        player_height_set(randnor((int)race[i].m_b_ht, (int)race[i].m_m_ht));
        // **Male and female pull different race[] columns**, so two lines, but the
        // window sees the same single assignment.
        player_body_weight_set(randnor((int)race[i].m_b_wt, (int)race[i].m_m_wt));
    } else {
        player_height_set(randnor((int)race[i].f_b_ht, (int)race[i].f_m_ht));
        player_body_weight_set(randnor((int)race[i].f_b_wt, (int)race[i].f_m_wt));
    }
    player_disarm_set(race[i].b_dis + todis_adj());
}

// Gets a character class -JWT-
static void get_class(void) {
    char tmp_str[80];

    int cl[MAX_CLASS];
    for (int j = 0; j < MAX_CLASS; j++) {
        cl[j] = 0;
    }

    int i = player_race();
    int j = 0;
    int k = 0;
    int l = 2;
    int m = 21;
    uint32_t mask = 0x1;

    clear_from(20);
    put_buffer("Choose a class (? for Help):", 20, 2);
    do {
        if (race[i].rtclass & mask) {
            (void)sprintf(tmp_str, "%c) %s", k + 'a', class[j].title);
            put_buffer(tmp_str, m, l);
            cl[k] = j;
            l += 15;
            if (l > 70) {
                l = 2;
                m++;
            }
            k++;
        }
        j++;
        mask <<= 1;
    } while (j < MAX_CLASS);

    // **The 0 before the menu does not become the player's answer** — this loop
    // cannot exit without answering, so `player_class_set(cl[j])` below always
    // overwrites it (player_class.h, "twice in a lifetime").
    player_class_set(0);

    int min_value, max_value;
    player_type *p_ptr;
    class_type *c_ptr;
    char s;

    bool exit_flag = false;
    do {
        move_cursor(20, 31);
        s = inkey();
        j = s - 'a';
        if ((j < k) && (j >= 0)) {
            player_class_set(cl[j]);
            // **What the class gives is outside the window** (player_class.h, third
            // point) — the window returns only the row number; from that one line
            // below come the six adjustments, hit die, stealth, exp factor, and
            // title, all from the table columns.
            c_ptr = &class[player_class()];
            exit_flag = true;
            clear_from(20);
            put_buffer(c_ptr->title, 5, 15);

            // Adjust the stats for the class adjustment -RAK-
            p_ptr = &py;
            change_stat(A_STR, c_ptr->madj_str);
            change_stat(A_INT, c_ptr->madj_int);
            change_stat(A_WIS, c_ptr->madj_wis);
            change_stat(A_DEX, c_ptr->madj_dex);
            change_stat(A_CON, c_ptr->madj_con);
            change_stat(A_CHR, c_ptr->madj_chr);
            for (i = 0; i < 6; i++) {
                p_ptr->stats.cur_stat[i] = p_ptr->stats.max_stat[i];
                set_use_stat(i);
            }

            // Real values. Discard the temporary values from :122 and replace them —
            // the class's madj_str / madj_dex have moved the stats by this point.
            player_attack_bonuses_set(tohit_adj(), todam_adj());
            player_armour_class_reset(toac_adj());
            // Displayed values: a copy of the real plusses, with the visible
            // bonus folded into the visible total. Nothing is worn yet, so the
            // armour the sheet shows is only what the bonus is worth (the line
            // above put the armour half at zero).
            player_display_start_from_real((int16_t)player_to_hit_bonus(), (int16_t)player_to_damage_bonus(), (int16_t)player_armour_class_magical());
            player_display_fold_to_ac();

            // now set misc stats, do this after setting stats because of con_adj() for hitpoints
            player_hit_die_adjust(c_ptr->adj_hd);
            player_reset_hp((int16_t)(con_adj() + player_hit_die()));

            // Initialize hit_points array.
            // Put bounds on total possible hp, only succeed
            // if it is within 1/8 of average value.
            min_value = (MAX_PLAYER_LEVEL * 3 / 8 * (player_hit_die() - 1)) + MAX_PLAYER_LEVEL;
            max_value = (MAX_PLAYER_LEVEL * 5 / 8 * (player_hit_die() - 1)) + MAX_PLAYER_LEVEL;
            set_hp_total_at_level(1, (uint16_t)player_hit_die());
            do {
                // i is not an index but stays as level - 1 throughout the loop (to
                // keep the number of rolls and their order unchanged). The total at
                // level i + 1 is the roll for that level plus the total up to one
                // level below.
                for (i = 1; i < MAX_PLAYER_LEVEL; i++) {
                    set_hp_total_at_level(i + 1, (uint16_t)(randint(player_hit_die()) + hp_total_at_level(i)));
                }
            } while ((hp_total_at_level(MAX_PLAYER_LEVEL) < min_value) ||
                     (hp_total_at_level(MAX_PLAYER_LEVEL) > max_value));

            // The class portion keeps the two numbers separate (melee and archery are
            // different skills). -RAK-
            player_base_to_hit_adjust(c_ptr->mbth, c_ptr->mbthb);
            // **Both arguments are positive** — equipment calls with (amount,
            // -amount), but the sign is the caller's choice.
            player_search_skill_adjust(c_ptr->msrh, c_ptr->mfos);
            player_disarm_adjust(c_ptr->mdis);
            // **Argument is positive** — all classes add stealth, so the lowest
            // creation can set is -1 (Half-Troll Warrior). **This line made `struct
            // misc *m_ptr` unnecessary.**
            player_stealth_adjust(c_ptr->mstl);
            player_saving_throw_adjust(c_ptr->msav);
            player_set_experience_factor((uint8_t)(player_experience_factor() + c_ptr->m_exp));
        } else if (s == '?') {
            helpfile(MORIA_WELCOME);
        } else {
            bell();
        }
    } while (!exit_flag);
}

// Given a stat value, return a monetary value,
// which affects the amount of gold a player has.
static int monval(uint8_t i) {
    return 5 * ((int)i - 10);
}

static void get_money(void) {
    int tmp = monval(player_stat_max(A_STR)) +
              monval(player_stat_max(A_INT)) +
              monval(player_stat_max(A_WIS)) +
              monval(player_stat_max(A_CON)) +
              monval(player_stat_max(A_DEX));

    int gold = player_social_class() * 6 + randint(25) + 325; // Social Class adj
    gold -= tmp;                                   // Stat adj
    gold += monval(player_stat_max(A_CHR));                  // Charisma adj

    // She charmed the banker into it! -CJS-
    if (!player_is_male()) {
        gold += 50;
    }

    // Minimum
    if (gold < 80) {
        gold = 80;
    }

    player_set_gold(gold);
}

// -----------------------------------------------------
//     M A I N  for Character Creation Routine -JWT-
// -----------------------------------------------------
void create_character(void) {
    put_character();
    choose_race();
    get_sex();

roll:
    // here we start a loop giving a player a choice of characters -RGM-
    get_all_stats();
    get_history();
    get_ahw();
    print_history();
    put_misc1();
    put_stats();

    clear_from(20);
    put_buffer("Hit space to reroll or ESC to accept characteristics: ", 20, 2);

inkey:
    move_cursor(20, 56);
    switch (inkey()) {
    default:
        bell();
        goto inkey;
    case ' ':
        goto roll; // reroll stats
    case ESCAPE:
        break; // done with stat generation
    }

    get_class();
    get_money();
    put_stats();
    put_misc2();
    put_misc3();
    get_name();

    // This delay may be reduced, but is recommended to keep players from
    // continuously rolling up characters, which can be VERY expensive CPU wise.
    pause_exit(23, PLAYER_EXIT_PAUSE);
}
