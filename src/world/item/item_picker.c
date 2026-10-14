// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Item picker and display: showing inventory/equipment and choosing an item

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "burden.h"
#include "dungeon_map.h"
#include "equipment.h"
#include "inventory.h"
#include "item_picker.h"
#include "player_pos.h"
#include "player_status_flags.h"
#include "stats.h"

// Displays inventory items from r1 to r2 -RAK-
// Designed to keep the display as far to the right as possible. -CJS-
// The parameter col gives a column at which to start, but if the display
// does not fit, it may be moved left.  The return value is the left edge
// used. If mask is non-zero, then only display those items which have a
// non-zero entry in the mask array.
int show_inven(int r1, int r2, bool weight, int col, const char *mask) {
    bigvtype tmp_val;
    vtype out_val[23];

    int len = 79 - col;

    int lim;
    if (weight) {
        lim = 68;
    } else {
        lim = 76;
    }

    // Print the items
    for (int i = r1; i <= r2; i++) {
        if (mask == CNIL || mask[i]) {
            objdes(tmp_val, inventory_at(i), true);

            // Truncate if too long.
            tmp_val[lim] = 0;

            (void)sprintf(out_val[i], "%c) %s", 'a' + i, tmp_val);
            int l = (int)strlen(out_val[i]) + 2;
            if (weight) {
                l += 9;
            }
            if (l > len) {
                len = l;
            }
        }
    }

    col = 79 - len;
    if (col < 0) {
        col = 0;
    }

    int current_line = 1;
    for (int i = r1; i <= r2; i++) {
        if (mask == CNIL || mask[i]) {
            // don't need first two spaces if in first column
            if (col == 0) {
                prt(out_val[i], current_line, col);
            } else {
                put_buffer("  ", current_line, col);
                prt(out_val[i], current_line, col + 2);
            }
            if (weight) {
                int total_weight = inventory_at(i)->weight * inventory_at(i)->number;
                (void)sprintf(tmp_val, "%3d.%d lb", (total_weight) / 10, (total_weight) % 10);
                prt(tmp_val, current_line, 71);
            }
            current_line++;
        }
    }
    return col;
}

// Return a string describing how a given equipment item is carried. -CJS-
const char *describe_use(int i) {
    const char *p;
    switch (i) {
    case INVEN_WIELD:
        p = "wielding";
        break;
    case INVEN_HEAD:
        p = "wearing on your head";
        break;
    case INVEN_NECK:
        p = "wearing around your neck";
        break;
    case INVEN_BODY:
        p = "wearing on your body";
        break;
    case INVEN_ARM:
        p = "wearing on your arm";
        break;
    case INVEN_HANDS:
        p = "wearing on your hands";
        break;
    case INVEN_RIGHT:
        p = "wearing on your right hand";
        break;
    case INVEN_LEFT:
        p = "wearing on your left hand";
        break;
    case INVEN_FEET:
        p = "wearing on your feet";
        break;
    case INVEN_OUTER:
        p = "wearing about your body";
        break;
    case INVEN_LIGHT:
        p = "using to light the way";
        break;
    case INVEN_AUX:
        p = "holding ready by your side";
        break;
    default:
        p = "carrying in your pack";
        break;
    }
    return p;
}

// Displays equipment items from r1 to end -RAK-
// Keep display as far right as possible. -CJS-
int show_equip(bool weight, int col) {
    bigvtype prt2;
    vtype out_val[INVEN_ARRAY_SIZE - INVEN_WIELD];

    inven_type *i_ptr;

    int line = 0;
    int len = 79 - col;

    int lim;
    if (weight) {
        lim = 52;
    } else {
        lim = 60;
    }

    // Range of equipment
    for (int i = equipment_first_slot(); i < equipment_end_slot(); i++) {
        i_ptr = equipment_at(i);
        if (i_ptr->tval != TV_NOTHING) {
            const char *prt1;

            // Get position
            switch (i) {
            case INVEN_WIELD:
                if (player_stat_use(A_STR) * WEAPON_WEIGHT_PER_STR < i_ptr->weight) {
                    prt1 = "Just lifting";
                } else {
                    prt1 = "Wielding";
                }
                break;
            case INVEN_HEAD:
                prt1 = "On head";
                break;
            case INVEN_NECK:
                prt1 = "Around neck";
                break;
            case INVEN_BODY:
                prt1 = "On body";
                break;
            case INVEN_ARM:
                prt1 = "On arm";
                break;
            case INVEN_HANDS:
                prt1 = "On hands";
                break;
            case INVEN_RIGHT:
                prt1 = "On right hand";
                break;
            case INVEN_LEFT:
                prt1 = "On left hand";
                break;
            case INVEN_FEET:
                prt1 = "On feet";
                break;
            case INVEN_OUTER:
                prt1 = "About body";
                break;
            case INVEN_LIGHT:
                prt1 = "Light source";
                break;
            case INVEN_AUX:
                prt1 = "Spare weapon";
                break;
            default:
                prt1 = "Unknown value";
                break;
            }
            objdes(prt2, equipment_at(i), true);

            // Truncate if necessary
            prt2[lim] = 0;

            (void)sprintf(out_val[line], "%c) %-14s: %s", line + 'a', prt1, prt2);
            int l = (int)strlen(out_val[line]) + 2;
            if (weight) {
                l += 9;
            }
            if (l > len) {
                len = l;
            }
            line++;
        }
    }
    col = 79 - len;
    if (col < 0) {
        col = 0;
    }

    line = 0;

    // Range of equipment
    for (int i = equipment_first_slot(); i < equipment_end_slot(); i++) {
        i_ptr = equipment_at(i);
        if (i_ptr->tval != TV_NOTHING) {
            // don't need first two spaces when using whole screen
            if (col == 0) {
                prt(out_val[line], line + 1, col);
            } else {
                put_buffer("  ", line + 1, col);
                prt(out_val[line], line + 1, col + 2);
            }
            if (weight) {
                int total_weight = i_ptr->weight * i_ptr->number;
                (void)sprintf(prt2, "%3d.%d lb", (total_weight) / 10, (total_weight) % 10);
                prt(prt2, line + 1, 71);
            }
            line++;
        }
    }
    erase_line(line + 1, col);
    return col;
}

// Remove item from equipment list -RAK-
void takeoff(int item_val, int posn) {
    equipment_set_count(equipment_count() - 1);

    inven_type *t_ptr = equipment_at(item_val);

    inventory_set_weight(inventory_weight() - t_ptr->weight * t_ptr->number);
    player_request_strength_check();

    const char *p;
    if (item_val == INVEN_WIELD || item_val == INVEN_AUX) {
        p = "Was wielding ";
    } else if (item_val == INVEN_LIGHT) {
        p = "Light source was ";
    } else {
        p = "Was wearing ";
    }

    msgtype out_val;
    bigvtype prt2;
    objdes(prt2, t_ptr, true);
    if (posn >= 0) {
        (void)snprintf(out_val, sizeof(out_val), "%s%s (%c)", p, prt2, 'a' + posn);
    } else {
        (void)snprintf(out_val, sizeof(out_val), "%s%s", p, prt2);
    }
    msg_print(out_val);

    // For secondary weapon
    if (item_val != INVEN_AUX) {
        py_bonuses(t_ptr, -1);
    }
    invcopy(t_ptr, OBJ_NOTHING);
}

// Used to verify if this really is the item we wish to -CJS-
// wear or read.
static int verify(const char *prompt, int item) {
    bigvtype out_str, object;

    // The index crosses both windows: of the five call sites, two hand in a
    // pack slot, two an equipment slot, and get_item hands in whichever screen
    // the player is looking at.
    objdes(object, inventory_and_equipment_at(item), true);

    // change the period to a question mark
    object[strlen(object) - 1] = '?';

    (void)sprintf(out_str, "%s %s", prompt, object);

    return get_check(out_str);
}

// Get the ID of an item and return the CTR value of it -RAK-
int get_item(int *com_val, const char *pmt, int i, int j, const char *mask, const char *message) {
    bool test_flag;
    bool full;
    bool item = false;
    bool redraw = false;
    int i_scr = 1;
    *com_val = 0;

    if (j > INVEN_WIELD) {
        full = true;
        if (inventory_count() == 0) {
            i_scr = 0;
            j = equipment_count() - 1;
        } else {
            j = inventory_count() - 1;
        }
    } else {
        full = false;
    }

    if (inventory_count() > 0 || (full && equipment_count() > 0)) {
        do {
            if (redraw) {
                if (i_scr > 0) {
                    (void)show_inven(i, j, false, 80, mask);
                } else {
                    (void)show_equip(false, 80);
                }
            }

            vtype out_val;

            if (full) {
                (void)sprintf(out_val, "(%s: %c-%c,%s%s / for %s, or ESC) %s",
                              (i_scr > 0 ? "Inven" : "Equip"), i + 'a', j + 'a',
                              (i_scr > 0 ? " 0-9," : ""),
                              (redraw ? "" : " * to see,"),
                              (i_scr > 0 ? "Equip" : "Inven"), pmt);
            } else {
                (void)sprintf(out_val, "(Items %c-%c,%s%s ESC to exit) %s",
                              i + 'a', j + 'a', (i_scr > 0 ? " 0-9," : ""),
                              (redraw ? "" : " * for inventory list,"), pmt);
            }

            test_flag = false;
            prt(out_val, 0, 0);

            do {
                char which = inkey();
                switch (which) {
                case ESCAPE:
                    test_flag = true;
                    free_turn_flag = true;
                    i_scr = -1;
                    break;
                case '/':
                    if (full) {
                        if (i_scr > 0) {
                            if (equipment_count() == 0) {
                                prt("But you're not using anything -more-", 0, 0);
                                (void)inkey();
                            } else {
                                i_scr = 0;
                                test_flag = true;
                                if (redraw) {
                                    j = equipment_count();
                                    while (j < inventory_count()) {
                                        j++;
                                        erase_line(j, 0);
                                    }
                                }
                                j = equipment_count() - 1;
                            }
                            prt(out_val, 0, 0);
                        } else {
                            if (inventory_count() == 0) {
                                prt("But you're not carrying anything -more-", 0, 0);
                                (void)inkey();
                            } else {
                                i_scr = 1;
                                test_flag = true;
                                if (redraw) {
                                    j = inventory_count();
                                    while (j < equipment_count()) {
                                        j++;
                                        erase_line(j, 0);
                                    }
                                }
                                j = inventory_count() - 1;
                            }
                        }
                    }
                    break;
                case '*':
                    if (!redraw) {
                        test_flag = true;
                        save_screen();
                        redraw = true;
                    }
                    break;
                default:
                    // look for item whose inscription matches "which"
                    if ((which >= '0') && (which <= '9') && (i_scr != 0)) {
                        int m;
                        for (m = i; (m < inventory_slot_count()) && ((inventory_at(m)->inscrip[0] != which) || (inventory_at(m)->inscrip[1] != '\0')); m++) {
                            ;
                        }
                        if (m < inventory_slot_count()) {
                            *com_val = m;
                        } else {
                            *com_val = -1;
                        }
                    } else if (isupper((int)which)) {
                        *com_val = which - 'A';
                    } else {
                        *com_val = which - 'a';
                    }

                    if ((*com_val >= i) && (*com_val <= j) &&
                        (mask == CNIL || mask[*com_val])) {
                        if (i_scr == 0) {
                            i = 21;
                            j = *com_val;
                            do {
                                while (equipment_at(++i)->tval == TV_NOTHING) {
                                    ;
                                }
                                j--;
                            } while (j >= 0);
                            *com_val = i;
                        }
                        if (isupper((int)which) && !verify("Try", *com_val)) {
                            test_flag = true;
                            free_turn_flag = true;
                            i_scr = -1;
                            break;
                        }
                        test_flag = true;
                        item = true;
                        i_scr = -1;
                    } else if (message) {
                        msg_print(message);

                        // Set test_flag to force redraw of the question.
                        test_flag = true;
                    } else {
                        bell();
                    }
                    break;
                }
            } while (!test_flag);
        } while (i_scr >= 0);

        if (redraw) {
            restore_screen();
        }

        erase_line(MSG_LINE, 0);
    } else {
        prt("You are not carrying anything.", 0, 0);
    }

    return item;
}
