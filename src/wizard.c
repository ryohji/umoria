// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Version history and info, and wizard mode debugging aids.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

// Light up the dungeon -RAK-
void wizard_light() {
    bool flag;

    if (cave[char_row][char_col].pl) {
        flag = false;
    } else {
        flag = true;
    }

    for (int i = 0; i < cur_height; i++) {
        for (int j = 0; j < cur_width; j++) {
            if (cave[i][j].fval <= MAX_CAVE_FLOOR) {
                for (int k = i - 1; k <= i + 1; k++) {
                    for (int l = j - 1; l <= j + 1; l++) {
                        cave_type *c_ptr = &cave[k][l];
                        c_ptr->pl = flag;

                        if (!flag) {
                            c_ptr->fm = false;
                        }
                    }
                }
            }
        }
    }
    prt_map();
}

// Wizard routine for gaining on stats -RAK-
void change_character() {
    int tmp_val;
    vtype tmp_str;

    uint8_t *a_ptr = py.stats.max_stat;

    prt("(3 - 118) Strength     = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            a_ptr[A_STR] = tmp_val;
            (void)res_stat(A_STR);
        }
    } else {
        return;
    }

    prt("(3 - 118) Intelligence = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            a_ptr[A_INT] = tmp_val;
            (void)res_stat(A_INT);
        }
    } else {
        return;
    }

    prt("(3 - 118) Wisdom       = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            a_ptr[A_WIS] = tmp_val;
            (void)res_stat(A_WIS);
        }
    } else {
        return;
    }

    prt("(3 - 118) Dexterity    = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            a_ptr[A_DEX] = tmp_val;
            (void)res_stat(A_DEX);
        }
    } else {
        return;
    }

    prt("(3 - 118) Constitution = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            a_ptr[A_CON] = tmp_val;
            (void)res_stat(A_CON);
        }
    } else {
        return;
    }

    prt("(3 - 118) Charisma     = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            a_ptr[A_CHR] = tmp_val;
            (void)res_stat(A_CHR);
        }
    } else {
        return;
    }

    struct misc *m_ptr = &py.misc;

    prt("(1 - 32767) Hit points = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 5)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 0) && (tmp_val <= MAX_SHORT)) {
            m_ptr->mhp = tmp_val;
            m_ptr->chp = tmp_val;
            m_ptr->chp_frac = 0;
            prt_mhp();
            prt_chp();
        }
    } else {
        return;
    }

    prt("(0 - 32767) Mana       = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 5)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val <= MAX_SHORT) && (*tmp_str != '\0')) {
            m_ptr->mana = tmp_val;
            m_ptr->cmana = tmp_val;
            m_ptr->cmana_frac = 0;
            prt_cmana();
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  Gold = ", m_ptr->au);
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 7)) {
        int32_t tmp_lval = (int32_t)atol(tmp_str);
        if (tmp_lval > -1 && (*tmp_str != '\0')) {
            m_ptr->au = tmp_lval;
            prt_gold();
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-200) Searching = ", m_ptr->srh);
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
            m_ptr->srh = tmp_val;
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  (-1-18) Stealth = ", m_ptr->stl);
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -2) && (tmp_val < 19) && (*tmp_str != '\0')) {
            m_ptr->stl = tmp_val;
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-200) Disarming = ", m_ptr->disarm);
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
            m_ptr->disarm = tmp_val;
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-100) Save = ", m_ptr->save);
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
            m_ptr->save = tmp_val;
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-200) Base to hit = ", m_ptr->bth);
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
            m_ptr->bth = tmp_val;
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-200) Bows/Throwing = ", m_ptr->bthb);
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
            m_ptr->bthb = tmp_val;
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  Weight = ", m_ptr->wt);
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if (tmp_val > -1 && (*tmp_str != '\0')) {
            m_ptr->wt = tmp_val;
        }
    } else {
        return;
    }

    while (get_com("Alter speed? (+/-)", tmp_str)) {
        if (*tmp_str == '+') {
            change_speed(-1);
        } else if (*tmp_str == '-') {
            change_speed(1);
        } else {
            break;
        }
        prt_speed();
    }
}

// Wizard routine for creating objects -RAK-
void wizard_create() {
    int tmp_val;
    vtype tmp_str;

    msg_print("Warning: This routine can cause a fatal error.");

    inven_type forge;
    inven_type *i_ptr = &forge;

    i_ptr->index = OBJ_WIZARD;
    i_ptr->name2 = 0;
    inscribe(i_ptr, "wizard item");
    i_ptr->ident = ID_KNOWN2 | ID_STOREBOUGHT;

    prt("Tval   : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 3)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->tval = tmp_val;

    prt("Tchar  : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 1)) {
        return;
    }
    i_ptr->tchar = tmp_str[0];

    prt("Subval : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 5)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->subval = tmp_val;

    prt("Weight : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 5)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->weight = tmp_val;

    prt("Number : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 5)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->number = tmp_val;

    prt("Damage (dice): ", 0, 0);
    if (!get_string(tmp_str, 0, 15, 3)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->damage[0] = tmp_val;

    prt("Damage (sides): ", 0, 0);
    if (!get_string(tmp_str, 0, 16, 3)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->damage[1] = tmp_val;

    prt("+To hit: ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 3)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->tohit = tmp_val;

    prt("+To dam: ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 3)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->todam = tmp_val;

    prt("AC     : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 3)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->ac = tmp_val;

    prt("+To AC : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 3)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->toac = tmp_val;

    prt("P1     : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 5)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->p1 = tmp_val;

    prt("Flags (In HEX): ", 0, 0);
    if (!get_string(tmp_str, 0, 16, 8)) {
        return;
    }

    // can't be constant string, this causes problems with
    // the GCC compiler and some scanf routines.
    char pattern[4];

    (void)strcpy(pattern, "%lx");

    int32_t tmp_lval;
    (void)sscanf(tmp_str, pattern, &tmp_lval);
    i_ptr->flags = tmp_lval;

    prt("Cost : ", 0, 0);
    if (!get_string(tmp_str, 0, 9, 8)) {
        return;
    }
    tmp_lval = (int)atol(tmp_str);
    i_ptr->cost = tmp_lval;

    prt("Level : ", 0, 0);
    if (!get_string(tmp_str, 0, 10, 3)) {
        return;
    }
    tmp_val = atoi(tmp_str);
    i_ptr->level = tmp_val;

    if (get_check("Allocate?")) {
        // delete object first if any, before call popt
        cave_type *c_ptr = &cave[char_row][char_col];
        if (c_ptr->tptr != 0) {
            (void)delete_object(char_row, char_col);
        }

        tmp_val = popt();
        t_list[tmp_val] = forge;
        c_ptr->tptr = tmp_val;
        msg_print("Allocated.");
    } else {
        msg_print("Aborted.");
    }
}
