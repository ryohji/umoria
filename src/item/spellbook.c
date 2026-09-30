// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The spells the player can cast: the chance of failing one, the list to
// choose from, how many can be learned and learning them, and the mana
//
// Moved out of misc3.c unchanged (#42); their prototypes stay in externs.h.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "inventory.h"
#include "player_class.h"
#include "player_level.h"
#include "player_mana.h"
#include "player_spells_to_learn.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"
#include "spells_known.h"
#include "stats.h"

// Returns spell chance of failure for spell -RAK-
int spell_chance(int spell) {
    spell_type *s_ptr = &magic_spell[player_class() - 1][spell];

    int stat;
    int chance = s_ptr->sfail - 3 * (player_level() - s_ptr->slevel);

    if (player_class_spell_type() == MAGE) {
        stat = A_INT;
    } else {
        stat = A_WIS;
    }

    chance -= 3 * (stat_adj(stat) - 1);
    if (s_ptr->smana > player_mana()) {
        chance += 5 * (s_ptr->smana - player_mana());
    }

    if (chance > 95) {
        chance = 95;
    } else if (chance < 5) {
        chance = 5;
    }

    return chance;
}

// Print list of spells -RAK-
// if nonconsec is  -1: spells numbered consecutively from 'a' to 'a'+num
//                 >=0: spells numbered by offset from nonconsec
void print_spells(int *spell, int num, int comment, int nonconsec) {
    int col;
    if (comment) {
        col = 22;
    } else {
        col = 31;
    }

    int offset = (player_class_spell_type() == MAGE ? SPELL_OFFSET : PRAYER_OFFSET);

    erase_line(1, col);
    put_buffer("Name", 1, col + 5);
    put_buffer("Lv Mana Fail", 1, col + 35);

    // only show the first 22 choices
    if (num > 22) {
        num = 22;
    }

    for (int i = 0; i < num; i++) {
        int j = spell[i];
        spell_type *s_ptr = &magic_spell[player_class() - 1][j];

        const char *p;
        if (comment == false) {
            p = "";
        } else if (spell_is_forgotten(j)) {
            p = " forgotten";
        } else if (!spell_is_learned(j)) {
            p = " unknown";
        } else if (!spell_has_worked(j)) {
            p = " untried";
        } else {
            p = "";
        }

        // determine whether or not to leave holes in character choices, nonconsec -1
        // when learning spells, consec offset>=0 when asking which spell to cast.
        char spell_char;
        if (nonconsec == -1) {
            spell_char = 'a' + i;
        } else {
            spell_char = 'a' + j - nonconsec;
        }

        vtype out_val;
        (void)sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s", spell_char, spell_names[j + offset], s_ptr->slevel, s_ptr->smana, spell_chance(j), p);
        prt(out_val, 2 + i, col);
    }
}

// Returns spell pointer -RAK-
int get_spell(int *spell, int num, int *sn, int *sc, const char *prompt, int first_spell) {
    *sn = -1;

    vtype out_str;
    (void)sprintf(out_str, "(Spells %c-%c, *=List, <ESCAPE>=exit) %s", spell[0] + 'a' - first_spell, spell[num - 1] + 'a' - first_spell, prompt);

    bool flag = false;
    bool redraw = false;

    int offset = (player_class_spell_type() == MAGE ? SPELL_OFFSET : PRAYER_OFFSET);

    char choice;
    while (flag == false && get_com(out_str, &choice)) {
        if (isupper((int)choice)) {
            *sn = choice - 'A' + first_spell;

            // verify that this is in spell[], at most 22 entries in spell[]
            int i;
            for (i = 0; i < num; i++) {
                if (*sn == spell[i]) {
                    break;
                }
            }
            if (i == num) {
                *sn = -2;
            } else {
                spell_type *s_ptr = &magic_spell[player_class() - 1][*sn];

                vtype tmp_str;
                (void)sprintf(tmp_str, "Cast %s (%d mana, %d%% fail)?", spell_names[*sn + offset], s_ptr->smana, spell_chance(*sn));
                if (get_check(tmp_str)) {
                    flag = true;
                } else {
                    *sn = -1;
                }
            }
        } else if (islower((int)choice)) {
            *sn = choice - 'a' + first_spell;

            // verify that this is in spell[], at most 22 entries in spell[]
            int i;
            for (i = 0; i < num; i++) {
                if (*sn == spell[i]) {
                    break;
                }
            }
            if (i == num) {
                *sn = -2;
            } else {
                flag = true;
            }
        } else if (choice == '*') {
            // only do this drawing once
            if (!redraw) {
                save_screen();
                redraw = true;
                print_spells(spell, num, false, first_spell);
            }
        } else if (isalpha((int)choice)) {
            *sn = -2;
        } else {
            *sn = -1;
            bell();
        }
        if (*sn == -2) {
            vtype tmp_str;
            (void)sprintf(tmp_str, "You don't know that %s.", (offset == SPELL_OFFSET ? "spell" : "prayer"));
            msg_print(tmp_str);
        }
    }
    if (redraw) {
        restore_screen();
    }

    erase_line(MSG_LINE, 0);
    if (flag) {
        *sc = spell_chance(*sn);
    }

    return flag;
}

// calculate number of spells player should have, and
// learn forget spells until that number is met -JEW-
void calc_spells(int stat) {

    spell_type *msp_ptr = &magic_spell[player_class() - 1][0];

    const char *p;
    int offset;
    if (stat == A_INT) {
        p = "spell";
        offset = SPELL_OFFSET;
    } else {
        p = "prayer";
        offset = PRAYER_OFFSET;
    }

    // check to see if know any spells greater than level, eliminate them
    for (int i = 31; i >= 0; i--) {
        if (spell_is_learned(i)) {
            if (msp_ptr[i].slevel > player_level()) {
                spell_forget(i);

                vtype tmp_str;
                (void)sprintf(tmp_str, "You have forgotten the %s of %s.", p, spell_names[i + offset]);
                msg_print(tmp_str);
            } else {
                break;
            }
        }
    }

    // calc number of spells allowed
    int num_allowed = 0;
    // WHAT THE CLASS GIVES IS OUTSIDE THE WINDOW (item 3 in player_class.h) --
    // first_spell_lev is read in two places only, this line and calc_mana().
    int levels = player_level() - class[player_class()].first_spell_lev + 1;
    switch (stat_adj(stat)) {
    case 0:
        num_allowed = 0;
        break;
    case 1:
    case 2:
    case 3:
        num_allowed = 1 * levels;
        break;
    case 4:
    case 5:
        num_allowed = 3 * levels / 2;
        break;
    case 6:
        num_allowed = 2 * levels;
        break;
    case 7:
        num_allowed = 5 * levels / 2;
        break;
    }

    int new_spells = num_allowed - learned_spell_count();

    if (new_spells > 0) {
        // remember forgotten spells while forgotten spells exist of new_spells
        // positive, remember the spells in the order that they were learned
        for (int n = 0; (any_spell_forgotten() && new_spells && (n < num_allowed) && (n < 32)); n++) {
            // j is (i+1)th spell learned
            int j = spell_learned_nth(n);

            if (spell_is_forgotten(j)) {
                if (msp_ptr[j].slevel <= player_level()) {
                    new_spells--;
                    spell_remember(j);

                    vtype tmp_str;
                    (void)sprintf(tmp_str, "You have remembered the %s of %s.", p, spell_names[j + offset]);
                    msg_print(tmp_str);
                } else {
                    num_allowed++;
                }
            }
        }

        if (new_spells > 0) {
            // determine which spells player can learn must check all spells here,
            // in gain_spell() we actually check if the books are present
            uint32_t spell_flag = spells_not_learned_among(0x7FFFFFFFL);

            int j;
            int id = 0;
            uint32_t mask;
            for (j = 0, mask = 0x1; spell_flag; mask <<= 1, j++) {
                if (spell_flag & mask) {
                    spell_flag &= ~mask;
                    if (msp_ptr[j].slevel <= player_level()) {
                        id++;
                    }
                }
            }

            if (new_spells > id) {
                new_spells = id;
            }
        }
    } else if (new_spells < 0) {
        // forget spells until new_spells zero or no more spells know, spells
        // are forgotten in the opposite order that they were learned
        for (int i = 31; new_spells && any_spell_learned(); i--) {
            // j is the (i+1)th spell learned
            int j = spell_learned_nth(i);
            if (spell_is_learned(j)) {
                spell_forget(j);
                new_spells++;

                vtype tmp_str;
                (void)sprintf(tmp_str, "You have forgotten the %s of %s.", p, spell_names[j + offset]);
                msg_print(tmp_str);
            }
        }

        new_spells = 0;
    }

    // The local new_spells is the answer just recounted; the window holds the one set last
    // time. SET IT AGAIN ONLY WHEN THEY DIFFER (if they are equal, nothing is redrawn either).
    if (new_spells != player_spells_to_learn()) {
        if (new_spells > 0 && player_spells_to_learn() == 0) {
            vtype tmp_str;
            (void)sprintf(tmp_str, "You can learn some new %ss now.", p);
            msg_print(tmp_str);
        }

        player_spells_to_learn_set(new_spells);
        player_request_study_redraw();
    }
}

// gain spells when player wants to    - jw
void gain_spells(void) {
    uint32_t spell_flag;

    // Priests don't need light because they get spells from their god, so only
    // fail when can't see if player has MAGE spells. This check is done below.
    if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
        msg_print("You are too confused.");
        return;
    }

    // Taken from the window, counted down locally, and set back once at the end
    // (player_spells_to_learn_set() below). Not set part way: the answer is not settled
    // until the spells that could not be learnt for want of a book are added back.
    int new_spells = player_spells_to_learn();
    int diff_spells = 0;

    // THIS ADDRESS IS FORMED BEFORE THE SCHOOL IS ASKED (bug candidate B23) -- for
    // a warrior it is `magic_spell[-1]`, but it is never read, because the if below
    // goes into neither school. DO NOT REORDER IT -- a step B changes no behaviour.
    spell_type *msp_ptr = &magic_spell[player_class() - 1][0];

    int stat, offset;
    if (player_class_spell_type() == MAGE) {
        stat = A_INT;
        offset = SPELL_OFFSET;

        // People with MAGE spells can't learn spells if they can't read their books.
        if (player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
            msg_print("You can't see to read your spell book!");
            return;
        } else if (no_light()) {
            msg_print("You have no light to read by.");
            return;
        }
    } else {
        stat = A_WIS;
        offset = PRAYER_OFFSET;
    }

    if (!new_spells) {
        vtype tmp_str;
        (void)sprintf(tmp_str, "You can't learn any new %ss!", (stat == A_INT ? "spell" : "prayer"));
        msg_print(tmp_str);
        free_turn_flag = true;
    } else {
        // determine which spells player can learn
        // mages need the book to learn a spell, priests do not need the book
        if (stat == A_INT) {
            spell_flag = 0;
            for (int i = 0; i < inventory_count(); i++) {
                if (inventory_at(i)->tval == TV_MAGIC_BOOK) {
                    spell_flag |= inventory_at(i)->flags;
                }
            }
        } else {
            spell_flag = 0x7FFFFFFF;
        }

        // clear bits for spells already learned
        spell_flag = spells_not_learned_among(spell_flag);

        int i = 0;
        int spells[31];

        int j;
        uint32_t mask;
        for (j = 0, mask = 0x1; spell_flag; mask <<= 1, j++) {
            if (spell_flag & mask) {
                spell_flag &= ~mask;
                if (msp_ptr[j].slevel <= player_level()) {
                    spells[i] = j;
                    i++;
                }
            }
        }

        if (new_spells > i) {
            msg_print("You seem to be missing a book.");
            diff_spells = new_spells - i;
            new_spells = i;
        }
        if (new_spells == 0) {
            ;
        } else if (stat == A_INT) {
            // get to choose which mage spells will be learned
            save_screen();
            print_spells(spells, i, false, -1);

            char query;
            while (new_spells && get_com("Learn which spell?", &query)) {
                int c = query - 'a';

                // test j < 23 in case i is greater than 22, only 22 spells
                // are actually shown on the screen, so limit choice to those
                if (c >= 0 && c < i && c < 22) {
                    new_spells--;
                    spell_learn(spells[c]);
                    for (; c <= i - 1; c++) {
                        spells[c] = spells[c + 1];
                    }
                    i--;
                    erase_line(c + 1, 31);
                    print_spells(spells, i, false, -1);
                } else {
                    bell();
                }
            }
            restore_screen();
        } else {
            // pick a prayer at random
            while (new_spells) {
                int s = randint(i) - 1;
                spell_learn(spells[s]);

                vtype tmp_str;
                (void)sprintf(tmp_str, "You have learned the prayer of %s.", spell_names[spells[s] + offset]);
                msg_print(tmp_str);

                for (; s <= i - 1; s++) {
                    spells[s] = spells[s + 1];
                }
                i--;
                new_spells--;
            }
        }

        player_spells_to_learn_set(new_spells + diff_spells);
        if (player_spells_to_learn() == 0) {
            player_request_study_redraw();
        }

        // set the mana for first level characters when they learn their first spell.
        if (player_max_mana() == 0) {
            calc_mana(stat);
        }
    }
}

// Gain some mana if you know at least one spell -RAK-
void calc_mana(int stat) {
    if (any_spell_learned()) {
        int new_mana = 0;
        int levels = player_level() - class[player_class()].first_spell_lev + 1;
        switch (stat_adj(stat)) {
        case 0:
            new_mana = 0;
            break;
        case 1:
        case 2:
            new_mana = 1 * levels;
            break;
        case 3:
            new_mana = 3 * levels / 2;
            break;
        case 4:
            new_mana = 2 * levels;
            break;
        case 5:
            new_mana = 5 * levels / 2;
            break;
        case 6:
            new_mana = 3 * levels;
            break;
        case 7:
            new_mana = 4 * levels;
            break;
        }

        // increment mana by one, so that first level chars have 2 mana
        if (new_mana > 0) {
            new_mana++;
        }

        // mana can be zero when creating character
        if (player_change_max_mana((int16_t)new_mana)) {
            // can't print mana here, may be in store or inventory mode
            player_request_mana_redraw();
        }
    } else if (player_lose_all_mana()) {
        // can't print mana here, may be in store or inventory mode
        player_request_mana_redraw();
    }
}
