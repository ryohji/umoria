// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Version history and info, and wizard mode debugging aids.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include "dungeon_state.h"
#include "game_status.h"
#include "world_monster.h"
#include "world_item.h"
#include "world_player.h"
#include "world_view.h"
#include "world_dungeon.h"
#include "commands.h"
#include "command_internal.h"
#include "game_settings.h"
#include "ui_screen.h"
#include "ui_input.h"
#include "player_stats.h"
#include "player_skills.h"
#include "player_progress.h"
#include "player_condition.h"

// lets anyone enter wizard mode after a disclaimer... -JEW-
bool enter_wiz_mode(void) {
    bool answer = false;

    if (!score_disqualifications()) {
        msg_print("Wizard mode is for debugging and experimenting.");
        answer = get_check("The game will not be scored if you enter wizard mode. Are you sure?");
    }

    if (score_disqualifications() || answer) {
        set_score_disqualifications((int16_t)(score_disqualifications() | SCORE_DISQUALIFY_WIZARD));
        progress_set_wizard_mode(true);
        return true;
    }

    return false;
}

// Light up the dungeon -RAK-
static void wizard_light(void) {
    bool flag;

    if (square_at(player_row(), player_col())->pl) {
        flag = false;
    } else {
        flag = true;
    }

    for (int i = 0; i < dungeon_height(); i++) {
        for (int j = 0; j < dungeon_width(); j++) {
            if (square_at(i, j)->fval <= MAX_CAVE_FLOOR) {
                for (int k = i - 1; k <= i + 1; k++) {
                    for (int l = j - 1; l <= j + 1; l++) {
                        cave_type *c_ptr = square_at(k, l);
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
static void change_character(void) {
    int tmp_val;
    vtype tmp_str;

    prt("(3 - 118) Strength     = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            player_stat_set_max(A_STR, tmp_val);
            (void)res_stat(A_STR);
        }
    } else {
        return;
    }

    prt("(3 - 118) Intelligence = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            player_stat_set_max(A_INT, tmp_val);
            (void)res_stat(A_INT);
        }
    } else {
        return;
    }

    prt("(3 - 118) Wisdom       = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            player_stat_set_max(A_WIS, tmp_val);
            (void)res_stat(A_WIS);
        }
    } else {
        return;
    }

    prt("(3 - 118) Dexterity    = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            player_stat_set_max(A_DEX, tmp_val);
            (void)res_stat(A_DEX);
        }
    } else {
        return;
    }

    prt("(3 - 118) Constitution = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            player_stat_set_max(A_CON, tmp_val);
            (void)res_stat(A_CON);
        }
    } else {
        return;
    }

    prt("(3 - 118) Charisma     = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 2) && (tmp_val < 119)) {
            player_stat_set_max(A_CHR, tmp_val);
            (void)res_stat(A_CHR);
        }
    } else {
        return;
    }

    prt("(1 - 32767) Hit points = ", 0, 0);
    if (get_string(tmp_str, 0, 25, 5)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > 0) && (tmp_val <= MAX_SHORT)) {
            player_reset_hp((int16_t)tmp_val);
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
            player_set_max_mana((int16_t)tmp_val);
            player_set_mana((int16_t)tmp_val);
            player_set_mana_fraction(0);
            prt_cmana();
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  Gold = ", player_gold());
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 7)) {
        int32_t tmp_lval = (int32_t)atol(tmp_str);
        if (tmp_lval > -1 && (*tmp_str != '\0')) {
            player_set_gold(tmp_lval);
            prt_gold();
        }
    } else {
        return;
    }

    // Set search chance (frequency not asked; uses separate accessor).
    (void)sprintf(tmp_str, "Current=%d  (0-200) Searching = ", player_search_chance());
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
            // Range validated here (0-200); accessor does not validate.
            player_search_chance_set(tmp_val);
        }
    } else {
        return;
    }

    // Set stealth. Range -1 to 18 covers the full gameplay range.
    (void)sprintf(tmp_str, "Current=%d  (-1-18) Stealth = ", player_stealth());
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -2) && (tmp_val < 19) && (*tmp_str != '\0')) {
            // Range validated here; accessor does not validate.
            player_stealth_set(tmp_val);
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-200) Disarming = ", player_disarm());
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
            player_disarm_set(tmp_val);
        }
    } else {
        return;
    }

    // The prompt says (0-100) but the check accepts 0-200. The range belongs to this
    // screen; the window clamps nothing (findings.md 24).
    (void)sprintf(tmp_str, "Current=%d  (0-100) Save = ", player_saving_throw());
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
            player_saving_throw_set(tmp_val);
        }
    } else {
        return;
    }

    // Set melee and ranged to-hit separately (0-200 range validated here).
    (void)sprintf(tmp_str, "Current=%d  (0-200) Base to hit = ", player_base_to_hit());
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
            player_base_to_hit_set_melee(tmp_val);
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  (0-200) Bows/Throwing = ", player_base_to_hit_with_bows());
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if ((tmp_val > -1) && (tmp_val < 201) && (*tmp_str != '\0')) {
            player_base_to_hit_set_with_bows(tmp_val);
        }
    } else {
        return;
    }

    (void)sprintf(tmp_str, "Current=%d  Weight = ", player_body_weight());
    tmp_val = (int)strlen(tmp_str);
    prt(tmp_str, 0, 0);
    if (get_string(tmp_str, 0, tmp_val, 3)) {
        tmp_val = atoi(tmp_str);
        if (tmp_val > -1 && (*tmp_str != '\0')) {
            // Refusing bad input is the prompt's rule, so it stays here (findings.md 24).
            player_body_weight_set(tmp_val);
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
static void wizard_create(void) {
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

    // Format string is now a literal (not a variable) so the compiler can check
    // argument types (-Wformat-nonliteral). Use SCNx32 for uint32_t to avoid size
    // mismatches on platforms where long is 64-bit.
    uint32_t item_flags = 0;
    (void)sscanf(tmp_str, "%" SCNx32, &item_flags);
    i_ptr->flags = item_flags;

    int32_t tmp_lval;

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
        cave_type *c_ptr = square_at(player_row(), player_col());
        if (c_ptr->tptr != 0) {
            (void)delete_object(player_row(), player_col());
        }

        tmp_val = popt();
        *floor_item_at(tmp_val) = forge;
        c_ptr->tptr = tmp_val;
        msg_print("Allocated.");
    } else {
        msg_print("Aborted.");
    }
}

// Runs a wizard mode command. The caller has already checked the mode and
// made the command a free move.
void wizard_command(char com_val) {
    int y, x, i;
    vtype tmp_str;

    switch (com_val) {
    case CTRL_KEY('A'): // ^A = Cure all
        (void)remove_curse();
        (void)cure_blindness();
        (void)cure_confusion();
        (void)cure_poison();
        (void)remove_fear();
        (void)res_stat(A_STR);
        (void)res_stat(A_INT);
        (void)res_stat(A_WIS);
        (void)res_stat(A_CON);
        (void)res_stat(A_DEX);
        (void)res_stat(A_CHR);
        player_timed_shorten_to(PLAYER_TIMED_SLOWNESS, 1);
        player_timed_shorten_to(PLAYER_TIMED_HALLUCINATION, 1);
        break;
    case CTRL_KEY('E'): // ^E = wizchar
        change_character();
        erase_line(MSG_LINE, 0);
        break;
    case CTRL_KEY('F'): // ^F = genocide
        (void)mass_genocide();
        break;
    case CTRL_KEY('G'): // ^G = treasure
        if (command_is_repeating()) {
            i = take_command_count();
        } else {
            i = 1;
        }
        random_object(player_row(), player_col(), i);
        prt_map();
        break;
    case CTRL_KEY('D'): // ^D = up/down
        if (command_is_repeating()) {
            i = take_command_count();
            if (i > 99) {
                i = 0;
            }
        } else {
            prt("Go to which level (0-99) ? ", 0, 0);
            i = -1;
            if (get_string(tmp_str, 0, 27, 10)) {
                i = atoi(tmp_str);
            }
        }
        if (i > -1) {
            // 0-99 because that is what the prompt above says, not
            // because leaving a level has a limit of its own.
            if (i > 99) {
                i = 99;
            }
            leave_for_level(i);
        } else {
            erase_line(MSG_LINE, 0);
        }
        break;
    case CTRL_KEY('O'): // ^O = objects
        print_objects();
        break;
    case '\\': // \ wizard help
        if (rogue_like_commands) {
            helpfile(MORIA_WIZ_HELP);
        } else {
            helpfile(MORIA_OWIZ_HELP);
        }
        break;
    case CTRL_KEY('I'): // ^I = identify
        (void)ident_spell();
        break;
    case '*':
        wizard_light();
        break;
    case ':':
        map_area();
        break;
    case CTRL_KEY('T'): // ^T = teleport
        teleport(100);
        break;
    case '+':
        // Deliberately sets experience without updating level. Level recalculated
        // by prt_experience().
        if (command_is_repeating()) {
            player_set_experience(take_command_count());
        } else if (player_experience() == 0) {
            player_set_experience(1);
        } else {
            player_set_experience(player_experience() * 2);
        }
        prt_experience();
        break;
    case '&': // & = summon
        y = player_row();
        x = player_col();
        (void)summon_monster(&y, &x, true);
        creatures(false);
        break;
    case '@':
        wizard_create();
        break;
    default:
        if (rogue_like_commands) {
            prt("Type '?' or '\\' for help.", 0, 0);
        } else {
            prt("Type '?' or ^H for help.", 0, 0);
        }
    }
}
