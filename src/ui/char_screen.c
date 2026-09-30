// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The character screen: the page that shows who the player is, and the
// commands that show it and change the name on it
//
// Moved out of misc3.c unchanged (#42); their prototypes stay in externs.h.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "abilities.h"
#include "player_bio.h"
#include "player_body_weight.h"
#include "player_class.h"
#include "player_display_numbers.h"
#include "player_gold.h"
#include "player_hp.h"
#include "player_level.h"
#include "player_mana.h"
#include "player_race.h"
#include "save_state.h"
#include "screen_fields.h"

// Print long number (7 digits of space) with header at given row, column
static void prt_7lnum(const char *header, int32_t num, int row, int column) {
    vtype out_val;
    (void)sprintf(out_val, "%s: %7d", header, num);
    put_buffer(out_val, row, column);
}

// Prints the following information on the screen. -JWT-
void put_character(void) {
    clear_screen();

    put_buffer("Name        :", 2, 1);
    put_buffer("Race        :", 3, 1);
    put_buffer("Sex         :", 4, 1);
    put_buffer("Class       :", 5, 1);

    if (character_is_generated()) {
        put_buffer(player_name(), 2, 15);
        put_buffer(player_race_name(), 3, 15);
        put_buffer((player_is_male() ? "Male" : "Female"), 4, 15);
        put_buffer(player_class_title(), 5, 15);
    }
}

// Prints the following information on the screen. -JWT-
void put_stats(void) {
    for (int i = 0; i < 6; i++) {
        vtype buf;

        cnv_stat(py.stats.use_stat[i], buf);
        prt_stat_name(i, 2 + i, 61);
        put_buffer(buf, 2 + i, 66);
        if (py.stats.max_stat[i] > py.stats.cur_stat[i]) {
            cnv_stat(py.stats.max_stat[i], buf);
            put_buffer(buf, 2 + i, 73);
        }
    }

    prt_num("+ To Hit    ", player_display_to_hit(), 9, 1);
    prt_num("+ To Damage ", player_display_to_dam(), 10, 1);
    prt_num("+ To AC     ", player_display_to_ac(), 11, 1);
    prt_num("  Total AC  ", player_display_ac(), 12, 1);
}

// Returns a rating of x depending on y -JWT-
const char *likert(int x, int y) {
    switch ((x / y)) {
    case -3:
    case -2:
    case -1:
        return "Very Bad";
    case 0:
    case 1:
        return "Bad";
    case 2:
        return "Poor";
    case 3:
    case 4:
        return "Fair";
    case 5:
        return "Good";
    case 6:
        return "Very Good";
    case 7:
    case 8:
        return "Excellent";
    default:
        return "Superb";
    }
}

// Prints age, height, weight, and SC -JWT-
void put_misc1(void) {
    prt_num("Age          ", player_age(), 2, 38);
    prt_num("Height       ", player_height(), 3, 38);
    prt_num("Weight       ", player_body_weight(), 4, 38);
    prt_num("Social Class ", player_social_class(), 5, 38);
}

// Prints the following information on the screen. -JWT-
void put_misc2(void) {
    prt_7lnum("Level      ", (int32_t)player_level(), 9, 28);
    prt_7lnum("Experience ", player_experience(), 10, 28);
    prt_7lnum("Max Exp    ", player_max_experience(), 11, 28);

    if (player_level() >= MAX_PLAYER_LEVEL) {
        prt("Exp to Adv.: *******", 12, 28);
    } else {
        prt_7lnum("Exp to Adv.", player_experience_needed_to_advance(), 12, 28);
    }

    prt_7lnum("Gold       ", player_gold(), 13, 28);
    prt_num("Max Hit Points ", player_max_hp(), 9, 52);
    prt_num("Cur Hit Points ", player_hp(), 10, 52);
    prt_num("Max Mana       ", player_max_mana(), 11, 52);
    prt_num("Cur Mana       ", player_mana(), 12, 52);
}

// Prints ratings on certain abilities -RAK-
void put_misc3(void) {
    clear_from(14);

    struct player_abilities a = calc_player_abilities();

    put_buffer("(Miscellaneous Abilities)", 15, 25);
    put_buffer("Fighting    :", 16, 1);
    put_buffer(likert(a.bth, 12), 16, 15);
    put_buffer("Bows/Throw  :", 17, 1);
    put_buffer(likert(a.bthb, 12), 17, 15);
    put_buffer("Saving Throw:", 18, 1);
    put_buffer(likert(a.save, 6), 18, 15);

    put_buffer("Stealth     :", 16, 28);
    put_buffer(likert(a.stl, 1), 16, 42);
    put_buffer("Disarming   :", 17, 28);
    put_buffer(likert(a.dis, 8), 17, 42);
    put_buffer("Magic Device:", 18, 28);
    put_buffer(likert(a.dev, 6), 18, 42);

    put_buffer("Perception  :", 16, 55);
    put_buffer(likert(a.fos, 3), 16, 69);
    put_buffer("Searching   :", 17, 55);
    put_buffer(likert(a.srh, 6), 17, 69);
    put_buffer("Infra-Vision:", 18, 55);
    put_buffer(a.infra, 18, 69);
}

// Used to display the character on the screen. -RAK-
void display_char(void) {
    put_character();
    put_misc1();
    put_stats();
    put_misc2();
    put_misc3();
}

// Gets a name for the character -JWT-
void get_name(void) {
    prt("Enter your player's name  [press <RETURN> when finished]", 21, 2);
    erase_field(23, 2, 15);

    // 器ぜんぶを写してから打たせる。**終端までではなく 27 バイト全部**を
    // 写すのは、get_string() が ESC のとき終端を書かずに返すからで、すぐ下の
    // `name[0] == 0` は終端より先のバイトも読んでいる（player_bio.h の名前の
    // 項。窓口が「終端より先はぜんぶ 0」を約束しているので、写したものは
    // フィールドを直に渡していたころと同じ中身になる）。
    char name[PLAYER_NAME_SIZE];
    memcpy(name, player_name(), PLAYER_NAME_SIZE);

    if (!get_string(name, 2, 15, 23) || name[0] == 0) {
        user_name(name);
        put_buffer(name, 2, 15);
    }
    player_name_set(name);

    clear_from(20);
}

// Changes the name of the character -JWT-
void change_name(void) {
    vtype temp;

    display_char();

    bool flag = false;

    do {
        prt("<f>ile character description. <c>hange character name.", 21, 2);
        char c = inkey();
        switch (c) {
        case 'c':
            get_name();
            flag = true;
            break;
        case 'f':
            prt("File name:", 0, 0);
            if (get_string(temp, 0, 10, 60) && temp[0]) {
                if (file_character(temp)) {
                    flag = true;
                }
            }
            break;
        case ESCAPE:
        case ' ':
        case '\n':
        case '\r':
            flag = true;
            break;
        default:
            bell();
            break;
        }
    } while (!flag);
}
