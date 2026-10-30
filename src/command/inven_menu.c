// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The inventory and equipment screens command handler

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include "dungeon_state.h"
#include "world_dungeon.h"
#include "world_item.h"
#include "world_player.h"
#include "game_settings.h"
#include "commands.h"
#include "ui_screen.h"
#include "ui_input.h"
#include "item_kinds.h"

#include "inventory.h"
#include "player_stats.h"
#include "player_condition.h"

#include "command_internal.h"

// All inventory commands (wear, exchange, take off, drop, inventory and
// equipment) are handled in an alternative command input mode, which accepts
// any of the inventory commands.
//
// It is intended that this function be called several times in succession,
// as some commands take up a turn, and the rest of moria must proceed in the
// interim. pending_inven_command() (ui_input.h) is normally zero;
// however if on return from inven_command it is expected that inven_command
// should be called *again*, (being still in inventory command input mode), then
// suspend_inven_command() has been given the inventory command character which
// should be used in the next call to inven_command.
//
// On return, the screen is restored, but not flushed. Provided no flush of
// the screen takes place before the next call to inven_command, the inventory
// command screen is silently redisplayed, and no actual output takes place at
// all. If the screen is flushed before a subsequent call, then the player is
// prompted to see if we should continue. This allows the player to see any
// changes that take place on the screen during inventory command input.
//
// screen_was_flushed() (ui_screen.h) tells whether the saved screen is still
// current. Suspending clears it, flushing sets it. suspend_inven_command() does both.
//
// The display of inventory items is kept to the right of the screen to
// minimize the work done to restore the screen afterwards. -CJS-

// Inventory command screen states.
#define BLANK_SCR 0
#define EQUIP_SCR 1
#define INVEN_SCR 2
#define WEAR_SCR 3
#define HELP_SCR 4
#define WRONG_SCR 5

// Keep track of the state of the inventory screen, between the calls of
// inven_command() as well: it is suspended after a turn and taken up again.
static struct {
    int state;               // which of the screens above is shown
    int left, bottom;        // the left column and the bottom line of what is drawn
    int wear_low, wear_high; // the items in the pack that can be worn
} screen;

// Draw the inventory screen.
static void inven_screen(int new_scr) {
    int line = 0;

    if (new_scr != screen.state) {
        screen.state = new_scr;
        switch (new_scr) {
        case BLANK_SCR:
            line = 0;
            break;
        case HELP_SCR:
            if (screen.left > 52) {
                screen.left = 52;
            }
            prt("  ESC: exit", 1, screen.left);
            prt("  w  : wear or wield object", 2, screen.left);
            prt("  t  : take off item", 3, screen.left);
            prt("  d  : drop object", 4, screen.left);
            prt("  x  : exchange weapons", 5, screen.left);
            prt("  i  : inventory of pack", 6, screen.left);
            prt("  e  : list used equipment", 7, screen.left);
            line = 7;
            break;
        case INVEN_SCR:
            screen.left = show_inven(0, inventory_count() - 1, show_weight_flag, screen.left, CNIL);
            line = inventory_count();
            break;
        case WEAR_SCR:
            screen.left = show_inven(screen.wear_low, screen.wear_high, show_weight_flag, screen.left, CNIL);
            line = screen.wear_high - screen.wear_low + 1;
            break;
        case EQUIP_SCR:
            screen.left = show_equip(show_weight_flag, screen.left);
            line = equipment_count();
            break;
        }
        if (line >= screen.bottom) {
            screen.bottom = line + 1;
            erase_line(screen.bottom, screen.left);
        } else {
            while (++line <= screen.bottom) {
                erase_line(line, screen.left);
            }
        }
    }
}

// Puts on the inventory item in the equipment slot, taking off what was there.
static void wear_item(int item, int slot) {
    msgtype prt1;
    bigvtype prt2;
    int tmp;
    inven_type tmp_obj;

    // first remove new item from inventory
    tmp_obj = *inventory_at(item);
    inven_type *i_ptr = &tmp_obj;

    screen.wear_high--;

    // Fix for torches
    if (i_ptr->number > 1 &&
        i_ptr->subval <= ITEM_SINGLE_STACK_MAX) {
        i_ptr->number = 1;
        screen.wear_high++;
    }
    inventory_set_weight(inventory_weight() + i_ptr->weight * i_ptr->number);

    // Subtracts weight
    inven_destroy(item);

    // Second, add old item to inv and remove
    // from equipment list, if necessary.
    i_ptr = equipment_at(slot);
    if (i_ptr->tval != TV_NOTHING) {
        int tmp2 = inventory_count();
        tmp = inven_carry(i_ptr);
        // If item removed did not stack with anything
        // in inventory, then increment screen.wear_high.
        if (inventory_count() != tmp2) {
            screen.wear_high++;
        }
        takeoff(slot, tmp);
    }

    // third, wear new item
    *i_ptr = tmp_obj;
    equipment_set_count(equipment_count() + 1);
    py_bonuses(i_ptr, 1);

    const char *string;
    if (slot == INVEN_WIELD) {
        string = "You are wielding";
    } else if (slot == INVEN_LIGHT) {
        string = "Your light source is";
    } else {
        string = "You are wearing";
    }
    objdes(prt2, i_ptr, true);
    // Get the right equipment letter.
    tmp = equipment_first_slot();
    item = 0;
    while (tmp != slot) {
        if (equipment_at(tmp++)->tval != TV_NOTHING) {
            item++;
        }
    }

    (void)snprintf(prt1, sizeof(prt1), "%s %s (%c)", string, prt2, 'a' + item);
    msg_print(prt1);
    // this is a new weapon, so clear heavy flag
    if (slot == INVEN_WIELD) {
        set_weapon_too_heavy(false);
    }
    check_strength();
    if (i_ptr->flags & TR_CURSED) {
        msg_print("Oops! It feels deathly cold!");
        add_inscribe(i_ptr, ID_DAMD);
        // To force a cost of 0, even if unidentified.
        i_ptr->cost = -1;
    }
}

// Put an appropriate header.
static void prt_inven_header(void) {
    msgtype prt1;

    if (screen.state == INVEN_SCR) {
        if (!show_weight_flag || inventory_count() == 0) {
            (void)sprintf(prt1, "You are carrying %d.%d pounds. In your pack there is %s",
                          inventory_weight() / 10, inventory_weight() % 10,
                          (inventory_count() == 0 ? "nothing." : "-"));
        } else {
            (void)sprintf(prt1, "You are carrying %d.%d pounds. Your capacity is %d.%d pounds. %s",
                          inventory_weight() / 10, inventory_weight() % 10,
                          weight_limit() / 10, weight_limit() % 10,
                          "In your pack is -");
        }
        prt(prt1, 0, 0);
    } else if (screen.state == WEAR_SCR) {
        if (screen.wear_high < screen.wear_low) {
            prt("You have nothing you could wield.", 0, 0);
        } else {
            prt("You could wield -", 0, 0);
        }
    } else if (screen.state == EQUIP_SCR) {
        if (equipment_count() == 0) {
            prt("You are not using anything.", 0, 0);
        } else {
            prt("You are using -", 0, 0);
        }
    } else {
        prt("Allowed commands:", 0, 0);
    }
}

// This does all the work.
// Returns whether it took a game turn.
bool inven_command(char command) {
    msgtype prt1;
    bigvtype prt2;
    int item, tmp;
    inven_type tmp_obj;
    int slot = 0;

    bool took_turn = false;

    save_screen();

    // Take up where we left off after a previous inventory command. -CJS-
    if (pending_inven_command()) {
        // If the screen has been flushed, we need to redraw. If the command
        // is a simple ' ' to recover the screen, just quit. Otherwise, check
        // and see what the user wants.
        if (screen_was_flushed()) {
            if (command == ' ' ||
                !get_check("Continuing with inventory command?")) {
                finish_inven_command();
                return false;
            }
            screen.left = 50;
            screen.bottom = 0;
        }
        tmp = screen.state;
        screen.state = WRONG_SCR;
        inven_screen(tmp);
    } else {
        screen.left = 50;
        screen.bottom = 0;

        // this forces exit of inven_command() if selecting is not set true
        screen.state = BLANK_SCR;
    }

    bool selecting;

    do {
        if (isupper((int)command)) {
            command = tolower((int)command);
        }

        // Simple command getting and screen selection.
        selecting = false;
        switch (command) {
        case 'i': // Inventory
            if (inventory_count() == 0) {
                msg_print("You are not carrying anything.");
            } else {
                inven_screen(INVEN_SCR);
            }
            break;
        case 'e': // Equipment
            if (equipment_count() == 0) {
                msg_print("You are not using any equipment.");
            } else {
                inven_screen(EQUIP_SCR);
            }
            break;
        case 't': // Take off
            if (equipment_count() == 0) {
                msg_print("You are not using any equipment.");
                // don't print message restarting inven command after taking off
                // something, it is confusing
            } else if (inventory_count() >= inventory_slot_count() &&
                       !pending_inven_command()) {
                msg_print("You will have to drop something first.");
            } else {
                if (screen.state != BLANK_SCR) {
                    inven_screen(EQUIP_SCR);
                }
                selecting = true;
            }
            break;
        case 'd': // Drop
            if (inventory_count() == 0 && equipment_count() == 0) {
                msg_print("But you're not carrying anything.");
            } else if (square_at(player_row(), player_col())->tptr != 0) {
                msg_print("There's no room to drop anything here.");
            } else {
                selecting = true;
                if ((screen.state == EQUIP_SCR && equipment_count() > 0) ||
                    inventory_count() == 0) {
                    if (screen.state != BLANK_SCR) {
                        inven_screen(EQUIP_SCR);
                    }
                    command = 'r'; // Remove - or take off and drop.
                } else if (screen.state != BLANK_SCR) {
                    inven_screen(INVEN_SCR);
                }
            }
            break;
        case 'w': // Wear/wield
            for (screen.wear_low = 0;
                 screen.wear_low < inventory_count() && inventory_at(screen.wear_low)->tval > TV_MAX_WEAR;
                 screen.wear_low++) {
                ;
            }
            for (screen.wear_high = screen.wear_low; screen.wear_high < inventory_count() &&
                                       inventory_at(screen.wear_high)->tval >= TV_MIN_WEAR;
                 screen.wear_high++) {
                ;
            }
            screen.wear_high--;
            if (screen.wear_low > screen.wear_high) {
                msg_print("You have nothing to wear or wield.");
            } else {
                if (screen.state != BLANK_SCR && screen.state != INVEN_SCR) {
                    inven_screen(WEAR_SCR);
                }
                selecting = true;
            }
            break;
        case 'x':
            if (equipment_at(INVEN_WIELD)->tval == TV_NOTHING &&
                equipment_at(INVEN_AUX)->tval == TV_NOTHING) {
                msg_print("But you are wielding no weapons.");
            } else if (TR_CURSED & equipment_at(INVEN_WIELD)->flags) {
                objdes(prt1, equipment_at(INVEN_WIELD), false);
                (void)snprintf(prt2, sizeof(prt2), "The %s you are wielding appears to be cursed.", prt1);
                msg_print(prt2);
            } else {
                took_turn = true;
                tmp_obj = *equipment_at(INVEN_AUX);
                *equipment_at(INVEN_AUX) = *equipment_at(INVEN_WIELD);
                *equipment_at(INVEN_WIELD) = tmp_obj;
                if (screen.state == EQUIP_SCR) {
                    screen.left = show_equip(show_weight_flag, screen.left);
                }

                py_bonuses(equipment_at(INVEN_AUX), -1);  // Subtract bonuses
                py_bonuses(equipment_at(INVEN_WIELD), 1); // Add bonuses

                if (equipment_at(INVEN_WIELD)->tval != TV_NOTHING) {
                    (void)strcpy(prt1, "Primary weapon   : ");
                    objdes(prt2, equipment_at(INVEN_WIELD), true);
                    msg_print(strcat(prt1, prt2));
                } else {
                    msg_print("No primary weapon.");
                }

                // this is a new weapon, so clear the heavy flag
                set_weapon_too_heavy(false);
                check_strength();
            }
            break;
        case ' ': // Dummy command to return again to main prompt.
            break;
        case '?':
            inven_screen(HELP_SCR);
            break;
        default:
            // Nonsense command
            bell();
            break;
        }

        // Say we are done here, instead of at the beginning, so that the
        // messages above can use "are we resuming?" to decide what to print.
        finish_inven_command();

        // Keep looking for objects to drop/wear/take off/throw off
        char which = 'z';
        while (selecting && !took_turn) {
            int from, to;
            const char *prompt;
            const char *swap = "";

            if (command == 'w') {
                from = screen.wear_low;
                to = screen.wear_high;
                prompt = "Wear/Wield";
            } else {
                from = 0;
                if (command == 'd') {
                    to = inventory_count() - 1;
                    prompt = "Drop";
                    if (equipment_count() > 0) {
                        swap = ", / for Equip";
                    }
                } else {
                    to = equipment_count() - 1;
                    if (command == 't') {
                        prompt = "Take off";
                    } else { // command == 'r'
                        prompt = "Throw off";
                        if (inventory_count() > 0) {
                            swap = ", / for Inven";
                        }
                    }
                }
            }
            if (from > to) {
                selecting = false;
            } else {
                const char *disp;
                if (screen.state == BLANK_SCR) {
                    disp = ", * to list";
                } else {
                    disp = "";
                }
                (void)sprintf(
                    prt1,
                    "(%c-%c%s%s%s, space to break, ESC to exit) %s which one?",
                    from + 'a', to + 'a', disp, swap,
                    ((command == 'w' || command == 'd') ? ", 0-9" : ""),
                    prompt);

                // Abort everything.
                if (!get_com(prt1, &which)) {
                    selecting = false;
                    which = ESCAPE;
                } else if (which == ' ' || which == '*') {
                    // Draw the screen and maybe exit to main prompt.

                    if (command == 't' || command == 'r') {
                        inven_screen(EQUIP_SCR);
                    } else if (command == 'w' && screen.state != INVEN_SCR) {
                        inven_screen(WEAR_SCR);
                    } else {
                        inven_screen(INVEN_SCR);
                    }
                    if (which == ' ') {
                        selecting = false;
                    }
                } else if (which == '/' && swap[0]) {
                   // Swap screens (for drop)

                    if (command == 'd') {
                        command = 'r';
                    } else {
                        command = 'd';
                    }
                    if (screen.state == EQUIP_SCR) {
                        inven_screen(INVEN_SCR);
                    } else if (screen.state == INVEN_SCR) {
                        inven_screen(EQUIP_SCR);
                    }
                } else {
                    if ((which >= '0') && (which <= '9') && (command != 'r') && (command != 't')) {
                        // look for item whose inscription matches "which"
                        int m;
                        for (m = from;
                             m <= to && ((inventory_at(m)->inscrip[0] != which) ||
                                         (inventory_at(m)->inscrip[1] != '\0'));
                             m++) {
                            ;
                        }
                        if (m <= to) {
                            item = m;
                        } else {
                            item = -1;
                        }
                    } else if ((which >= 'A') && (which <= 'Z')) {
                        item = which - 'A';
                    } else {
                        item = which - 'a';
                    }
                    if (item < from || item > to) {
                        bell();
                    } else { // Found an item!
                        if (command == 'r' || command == 't') {
                            // Get its place in the equipment list.
                            tmp = item;
                            item = 21;
                            do {
                                item++;
                                if (equipment_at(item)->tval != TV_NOTHING) {
                                    tmp--;
                                }
                            } while (tmp >= 0);
                            if (isupper((int)which) && !confirm_item(prompt, item)) {
                                item = -1;
                            } else if (TR_CURSED & equipment_at(item)->flags) {
                                msg_print("Hmmm, it seems to be cursed.");
                                item = -1;
                            } else if (command == 't' && !inven_check_num(equipment_at(item))) {
                                if (square_at(player_row(), player_col())->tptr != 0) {
                                    msg_print("You can't carry it.");
                                    item = -1;
                                } else if (get_check("You can't carry it.  Drop it?")) {
                                    command = 'r';
                                } else {
                                    item = -1;
                                }
                            }
                            if (item >= 0) {
                                if (command == 'r') {
                                    inven_drop(item, true);
                                    // As a safety measure, set the player's inven
                                    // weight to 0, when the last object is dropped.
                                    if (inventory_count() == 0 && equipment_count() == 0) {
                                        inventory_set_weight(0);
                                    }
                                } else {
                                    slot = inven_carry(equipment_at(item));
                                    takeoff(item, slot);
                                }
                                check_strength();
                                took_turn = true;
                                if (command == 'r') {
                                    selecting = false;
                                }
                            }
                        } else if (command == 'w') {
                            // Wearing. Go to a bit of trouble over replacing
                            // existing equipment.
                            if (isupper((int)which) && !confirm_item(prompt, item)) {
                                item = -1;
                            } else {
                                // Slot for equipment
                                switch (inventory_at(item)->tval) {
                                case TV_SLING_AMMO: case TV_BOLT: case TV_ARROW: case TV_BOW:
                                case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
                                case TV_DIGGING: case TV_SPIKE:
                                    slot = INVEN_WIELD;
                                    break;
                                case TV_LIGHT:
                                    slot = INVEN_LIGHT;
                                    break;
                                case TV_BOOTS:
                                    slot = INVEN_FEET;
                                    break;
                                case TV_GLOVES:
                                    slot = INVEN_HANDS;
                                    break;
                                case TV_CLOAK:
                                    slot = INVEN_OUTER;
                                    break;
                                case TV_HELM:
                                    slot = INVEN_HEAD;
                                    break;
                                case TV_SHIELD:
                                    slot = INVEN_ARM;
                                    break;
                                case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
                                    slot = INVEN_BODY;
                                    break;
                                case TV_AMULET:
                                    slot = INVEN_NECK;
                                    break;
                                case TV_RING:
                                    if (equipment_at(INVEN_RIGHT)->tval == TV_NOTHING) {
                                        slot = INVEN_RIGHT;
                                    } else if (equipment_at(INVEN_LEFT)->tval == TV_NOTHING) {
                                        slot = INVEN_LEFT;
                                    } else {
                                        slot = 0;
                                        // Rings. Give choice over where they go.
                                        do {
                                            char query;
                                            if (!get_com("Put ring on which hand (l/r/L/R)?", &query)) {
                                                item = -1;
                                                slot = -1;
                                            } else if (query == 'l') {
                                                slot = INVEN_LEFT;
                                            } else if (query == 'r') {
                                                slot = INVEN_RIGHT;
                                            } else {
                                                if (query == 'L') {
                                                    slot = INVEN_LEFT;
                                                } else if (query == 'R') {
                                                    slot = INVEN_RIGHT;
                                                } else {
                                                    bell();
                                                }
                                                if (slot &&
                                                    !confirm_item("Replace", slot)) {
                                                    slot = 0;
                                                }
                                            }
                                        } while (slot == 0);
                                    }
                                    break;
                                default:
                                    msg_print("IMPOSSIBLE: I don't see how you can use that.");
                                    item = -1;
                                    break;
                                }
                            }
                            if (item >= 0 && equipment_at(slot)->tval != TV_NOTHING) {
                                if (TR_CURSED & equipment_at(slot)->flags) {
                                    objdes(prt1, equipment_at(slot), false);
                                    (void)snprintf(prt2, sizeof(prt2), "The %s you are ", prt1);
                                    if (slot == INVEN_HEAD) {
                                        (void)strcat(prt2, "wielding ");
                                    } else {
                                        (void)strcat(prt2, "wearing ");
                                    }
                                    msg_print(strcat(prt2, "appears to be cursed."));
                                    item = -1;
                                } else if (inventory_at(item)->subval == ITEM_GROUP_MIN &&
                                           inventory_at(item)->number > 1 &&
                                           !inven_check_num(equipment_at(slot)))
                                {
                                    // this can happen if try to wield a torch,
                                    // and have more than one in inventory
                                    msg_print("You will have to drop something first.");
                                    item = -1;
                                }
                            }
                            if (item >= 0) {
                                // OK. Wear it.
                                took_turn = true;

                                wear_item(item, slot);
                            }
                        } else {
                            // command == 'd'

                            // NOTE: initializing to `ESCAPE` as warnings were being given. -MRC-
                            char query = ESCAPE;

                            if (inventory_at(item)->number > 1) {
                                objdes(prt1, inventory_at(item), true);
                                prt1[strlen(prt1) - 1] = '?';
                                (void)snprintf(prt2, sizeof(prt2), "Drop all %s [y/n]", prt1);
                                prt1[strlen(prt1) - 1] = '.';
                                prt(prt2, 0, 0);
                                query = inkey();
                                if (query != 'y' && query != 'n') {
                                    if (query != ESCAPE) {
                                        bell();
                                    }
                                    erase_line(MSG_LINE, 0);
                                    item = -1;
                                }
                            } else if (isupper((int)which) && !confirm_item(prompt, item)) {
                                item = -1;
                            } else {
                                query = 'y';
                            }

                            if (item >= 0) {
                                took_turn = true; // Player turn
                                inven_drop(item, query == 'y');
                                check_strength();
                            }

                            selecting = false;

                            // As a safety measure, set the player's inven weight
                            // to 0, when the last object is dropped.
                            if (inventory_count() == 0 && equipment_count() == 0) {
                                inventory_set_weight(0);
                            }
                        }
                        if (took_turn && screen.state == BLANK_SCR) {
                            selecting = false;
                        }
                    }
                }
            }
        }
        if (which == ESCAPE || screen.state == BLANK_SCR) {
            command = ESCAPE;
        } else if (took_turn) {
            // Save state for recovery if they want to call us again next turn.
            // A ' ' is a dummy command, just to recover the screen.
            char resume = selecting ? command : ' ';

            // Flush the last message before suspending: putting text on the
            // screen notes a flush of its own, and suspending is what starts
            // the count of "has the world changed since?" over again.
            msg_print(CNIL);
            suspend_inven_command(resume);
            command = ESCAPE;
        } else {
            prt_inven_header();

            erase_line(screen.bottom, screen.left);
            put_buffer("e/i/t/w/x/d/?/ESC:", screen.bottom, 60);
            command = inkey();
            erase_line(screen.bottom, screen.left);
        }
    } while (command != ESCAPE);

    if (screen.state != BLANK_SCR) {
        restore_screen();
    }

    calc_bonuses();
    return took_turn;
}
