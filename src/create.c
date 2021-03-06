// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
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

// Generates character's stats -JWT-
static void get_stats() {
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
        py.stats.max_stat[i] =
            5 + dice[3 * i] + dice[3 * i + 1] + dice[3 * i + 2];
    }
}

// Changes stats by given amount -JWT-
static void change_stat(int stat, int16_t amount) {
    int tmp_stat = py.stats.max_stat[stat];

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
    py.stats.max_stat[stat] = tmp_stat;
}

// generate all stats and modify for race. needed in a separate
// module so looping of character selection would be allowed -RGM-
static void get_all_stats() {
    player_type *p_ptr = &py;
    race_type *r_ptr = &race[p_ptr->misc.prace];

    get_stats();
    change_stat(A_STR, r_ptr->str_adj);
    change_stat(A_INT, r_ptr->int_adj);
    change_stat(A_WIS, r_ptr->wis_adj);
    change_stat(A_DEX, r_ptr->dex_adj);
    change_stat(A_CON, r_ptr->con_adj);
    change_stat(A_CHR, r_ptr->chr_adj);

    p_ptr->misc.lev = 1;

    for (int j = 0; j < 6; j++) {
        py.stats.cur_stat[j] = py.stats.max_stat[j];
        set_use_stat(j);
    }

    p_ptr->misc.srh = r_ptr->srh;
    p_ptr->misc.bth = r_ptr->bth;
    p_ptr->misc.bthb = r_ptr->bthb;
    p_ptr->misc.fos = r_ptr->fos;
    p_ptr->misc.stl = r_ptr->stl;
    p_ptr->misc.save = r_ptr->bsav;
    p_ptr->misc.hitdie = r_ptr->bhitdie;
    p_ptr->misc.ptodam = todam_adj();
    p_ptr->misc.ptohit = tohit_adj();
    p_ptr->misc.ptoac = 0;
    p_ptr->misc.pac = toac_adj();
    p_ptr->misc.expfact = r_ptr->b_exp;
    p_ptr->flags.see_infra = r_ptr->infra;
}

// Allows player to select a race -JWT-
static void choose_race() {
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

    player_type *p_ptr = &py;
    race_type *r_ptr = &race[j];
    p_ptr->misc.prace = j;
    put_buffer(r_ptr->trace, 3, 15);
}

// Will print the history of a character -JWT-
static void print_history() {
    put_buffer("Character Background", 14, 27);

    for (int i = 0; i < 4; i++) {
        prt(py.misc.history[i], i + 15, 10);
    }
}

// Get the racial history, determines social class -RAK-
//
// Assumptions:
//   - Each race has init history beginning at (race-1)*3+1
//   - All history parts are in ascending order
static void get_history() {
    char history_block[240];
    background_type *b_ptr;
    int test_roll;
    bool flag;

    int hist_ptr = py.misc.prace * 3 + 1;
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
    for (hist_ptr = 0; hist_ptr < 4; hist_ptr++) {
        py.misc.history[hist_ptr][0] = '\0';
    }

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

        (void)strncpy(py.misc.history[line_ctr], &history_block[start_pos], cur_len);
        py.misc.history[line_ctr][cur_len] = '\0';
        line_ctr++;
        start_pos = new_start;
    } while (!flag);

    // Compute social class for player
    if (social_class > 100) {
        social_class = 100;
    } else if (social_class < 1) {
        social_class = 1;
    }
    py.misc.sc = social_class;
}

// Gets the character's sex -JWT-
static void get_sex() {
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
            py.misc.male = false;
            put_buffer("Female", 4, 15);
            exit_flag = true;
        } else if (c == 'm' || c == 'M') {
            py.misc.male = true;
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
static void get_ahw() {
    int i = py.misc.prace;
    py.misc.age = race[i].b_age + randint((int)race[i].m_age);
    if (py.misc.male) {
        py.misc.ht = randnor((int)race[i].m_b_ht, (int)race[i].m_m_ht);
        py.misc.wt = randnor((int)race[i].m_b_wt, (int)race[i].m_m_wt);
    } else {
        py.misc.ht = randnor((int)race[i].f_b_ht, (int)race[i].f_m_ht);
        py.misc.wt = randnor((int)race[i].f_b_wt, (int)race[i].f_m_wt);
    }
    py.misc.disarm = race[i].b_dis + todis_adj();
}

// Gets a character class -JWT-
static void get_class() {
    char tmp_str[80];

    int cl[MAX_CLASS];
    for (int j = 0; j < MAX_CLASS; j++) {
        cl[j] = 0;
    }

    int i = py.misc.prace;
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

    py.misc.pclass = 0;

    int min_value, max_value;
    struct misc *m_ptr;
    player_type *p_ptr;
    class_type *c_ptr;
    char s;

    bool exit_flag = false;
    do {
        move_cursor(20, 31);
        s = inkey();
        j = s - 'a';
        if ((j < k) && (j >= 0)) {
            py.misc.pclass = cl[j];
            c_ptr = &class[py.misc.pclass];
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

            p_ptr->misc.ptodam = todam_adj(); // Real values
            p_ptr->misc.ptohit = tohit_adj();
            p_ptr->misc.ptoac = toac_adj();
            p_ptr->misc.pac = 0;
            p_ptr->misc.dis_td = p_ptr->misc.ptodam; // Displayed values
            p_ptr->misc.dis_th = p_ptr->misc.ptohit;
            p_ptr->misc.dis_tac = p_ptr->misc.ptoac;
            p_ptr->misc.dis_ac = p_ptr->misc.pac + p_ptr->misc.dis_tac;

            // now set misc stats, do this after setting stats because of con_adj() for hitpoints
            m_ptr = &py.misc;
            m_ptr->hitdie += c_ptr->adj_hd;
            m_ptr->mhp = con_adj() + m_ptr->hitdie;
            m_ptr->chp = m_ptr->mhp;
            m_ptr->chp_frac = 0;

            // Initialize hit_points array.
            // Put bounds on total possible hp, only succeed
            // if it is within 1/8 of average value.
            min_value = (MAX_PLAYER_LEVEL * 3 / 8 * (m_ptr->hitdie - 1)) + MAX_PLAYER_LEVEL;
            max_value = (MAX_PLAYER_LEVEL * 5 / 8 * (m_ptr->hitdie - 1)) + MAX_PLAYER_LEVEL;
            player_hp[0] = m_ptr->hitdie;
            do {
                for (i = 1; i < MAX_PLAYER_LEVEL; i++) {
                    player_hp[i] = randint((int)m_ptr->hitdie);
                    player_hp[i] += player_hp[i - 1];
                }
            } while ((player_hp[MAX_PLAYER_LEVEL - 1] < min_value) ||
                     (player_hp[MAX_PLAYER_LEVEL - 1] > max_value));

            m_ptr->bth += c_ptr->mbth;
            m_ptr->bthb += c_ptr->mbthb; // RAK
            m_ptr->srh += c_ptr->msrh;
            m_ptr->disarm += c_ptr->mdis;
            m_ptr->fos += c_ptr->mfos;
            m_ptr->stl += c_ptr->mstl;
            m_ptr->save += c_ptr->msav;
            m_ptr->expfact += c_ptr->m_exp;
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

static void get_money() {
    uint8_t *a_ptr = py.stats.max_stat;
    int tmp = monval(a_ptr[A_STR]) +
              monval(a_ptr[A_INT]) +
              monval(a_ptr[A_WIS]) +
              monval(a_ptr[A_CON]) +
              monval(a_ptr[A_DEX]);

    int gold = py.misc.sc * 6 + randint(25) + 325; // Social Class adj
    gold -= tmp;                                   // Stat adj
    gold += monval(a_ptr[A_CHR]);                  // Charisma adj

    // She charmed the banker into it! -CJS-
    if (!py.misc.male) {
        gold += 50;
    }

    // Minimum
    if (gold < 80) {
        gold = 80;
    }

    py.misc.au = gold;
}

// -----------------------------------------------------
//     M A I N  for Character Creation Routine -JWT-
// -----------------------------------------------------
void create_character() {
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
