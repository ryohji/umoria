// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The store screen: showing the stock and the player's gold, and the buy and
// sell commands that hand over to the haggling in store_haggle.c
//
// enter_store() is declared in externs.h; the rest is static.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "inven_command_state.h"
#include "inventory.h"
#include "stores.h"
#include "store_haggle.h"
#include "messages.h"
#include "player_gold.h"
#include "progress.h"
#include "stats.h"

static const char *comment1[14] = {
    "Done!",
    "Accepted!",
    "Fine.",
    "Agreed!",
    "Ok.",
    "Taken!",
    "You drive a hard bargain, but taken.",
    "You'll force me bankrupt, but it's a deal.",
    "Sigh.  I'll take it.",
    "My poor sick children may starve, but done!",
    "Finally!  I accept.",
    "Robbed again.",
    "A pleasure to do business with you!",
    "My spouse will skin me, but accepted.",
};

// Comment one : Finished haggling
static void prt_comment1(void) {
    msg_print(comment1[randint(14) - 1]);
}

// Displays the set of commands -RAK-
static void display_commands(void) {
    prt("You may:", 20, 0);
    prt(" p) Purchase an item.           b) Browse store's inventory.", 21, 0);
    prt(" s) Sell an item.               i/e/t/w/x) Inventory/Equipment Lists.", 22, 0);
    prt("ESC) Exit from Building.        ^R) Redraw the screen.", 23, 0);
}

// Displays a store's inventory -RAK-
static void display_inventory(int store_num, int start) {
    store_type *s_ptr = store_at(store_num);

    int i = (start % 12);

    int stop = ((start / 12) + 1) * 12;
    if (stop > s_ptr->store_ctr) {
        stop = s_ptr->store_ctr;
    }

    while (start < stop) {
        inven_type *i_ptr = &s_ptr->store_inven[start].sitem;

        int32_t x = i_ptr->number;
        if ((i_ptr->subval >= ITEM_SINGLE_STACK_MIN) && (i_ptr->subval <= ITEM_SINGLE_STACK_MAX)) {
            i_ptr->number = 1;
        }

        bigvtype out_val1;
        msgtype out_val2;

        objdes(out_val1, i_ptr, true);
        i_ptr->number = x;
        (void)snprintf(out_val2, sizeof(out_val2), "%c) %s", 'a' + i, out_val1);
        prt(out_val2, i + 5, 0);
        x = s_ptr->store_inven[start].scost;
        if (x <= 0) {
            int32_t value = -x;
            value = value * chr_adj() / 100;
            if (value <= 0) {
                value = 1;
            }
            (void)sprintf(out_val2, "%9d", value);
        } else {
            (void)sprintf(out_val2, "%9d [Fixed]", x);
        }
        prt(out_val2, i + 5, 59);
        i++;
        start++;
    }
    if (i < 12) {
        for (int j = 0; j < (11 - i + 1); j++) {
            // clear remaining lines
            erase_line(j + i + 5, 0);
        }
    }
    if (s_ptr->store_ctr > 12) {
        put_buffer("- cont. -", 17, 60);
    } else {
        erase_line(17, 60);
    }
}

// Re-displays only a single cost -RAK-
static void display_cost(int store_num, int pos) {
    store_type *s_ptr = store_at(store_num);

    int i = (pos % 12);

    vtype out_val;
    if (s_ptr->store_inven[pos].scost < 0) {
        int32_t j = -s_ptr->store_inven[pos].scost;
        j = j * chr_adj() / 100;
        (void)sprintf(out_val, "%d", j);
    } else {
        (void)sprintf(out_val, "%9d [Fixed]", s_ptr->store_inven[pos].scost);
    }
    prt(out_val, i + 5, 59);
}

// Displays players gold -RAK-
static void store_prt_gold(void) {
    vtype out_val;
    (void)sprintf(out_val, "Gold Remaining : %d", player_gold());
    prt(out_val, 18, 17);
}

// Displays store -RAK-
static void display_store(int store_num, int cur_top) {
    store_type *s_ptr = store_at(store_num);

    clear_screen();
    put_buffer(owners[s_ptr->owner].owner_name, 3, 9);
    put_buffer("Item", 4, 3);
    put_buffer("Asking Price", 4, 60);
    store_prt_gold();
    display_commands();
    display_inventory(store_num, cur_top);
}

// Get the ID of a store item and return it's value -RAK-
static bool get_store_item(int *com_val, const char *pmt, int i, int j) {
    bool flag = false;

    *com_val = -1;

    vtype out_val;
    (void)sprintf(out_val, "(Items %c-%c, ESC to exit) %s", i + 'a', j + 'a', pmt);

    char command;
    while (get_com(out_val, &command)) {
        command -= 'a';
        if (command >= i && command <= j) {
            flag = true;
            *com_val = command;
            break;
        }
        bell();
    }
    erase_line(MSG_LINE, 0);

    return flag;
}

// Buy an item from a store -RAK-
static bool store_purchase(int store_num, int *cur_top) {
    bool purchase = false;

    store_type *s_ptr = store_at(store_num);

    // i == number of objects shown on screen
    int i;
    if (*cur_top == 12) {
        i = s_ptr->store_ctr - 1 - 12;
    } else if (s_ptr->store_ctr > 11) {
        i = 11;
    } else {
        i = s_ptr->store_ctr - 1;
    }

    int item_val;

    if (s_ptr->store_ctr < 1) {
        msg_print("I am currently out of stock.");
    } else if (get_store_item(&item_val, "Which item are you interested in? ", 0, i)) {
        // Get the item number to be bought

        item_val = item_val + *cur_top; // true item_val

        inven_type sell_obj;
        take_one_item(&sell_obj, &s_ptr->store_inven[item_val].sitem);

        if (inven_check_num(&sell_obj)) {
            int choice;
            int32_t price;

            if (s_ptr->store_inven[item_val].scost > 0) {
                price = s_ptr->store_inven[item_val].scost;
                choice = 0;
            } else {
                choice = purchase_haggle(store_num, &price, &sell_obj);
            }

            if (choice == 0) {
                if (player_gold() >= price) {
                    prt_comment1();
                    decrease_insults(store_num);
                    player_pay_gold(price);

                    int item_new = inven_carry(&sell_obj);
                    i = s_ptr->store_ctr;
                    store_destroy(store_num, item_val, true);

                    msgtype out_val;
                    bigvtype tmp_str;
                    objdes(tmp_str, inventory_at(item_new), true);
                    (void)snprintf(out_val, sizeof(out_val), "You have %s (%c)", tmp_str, item_new + 'a');
                    prt(out_val, 0, 0);

                    check_strength();
                    if (*cur_top >= s_ptr->store_ctr) {
                        *cur_top = 0;
                        display_inventory(store_num, *cur_top);
                    } else {
                        inven_record *r_ptr = &s_ptr->store_inven[item_val];

                        if (i == s_ptr->store_ctr) {
                            if (r_ptr->scost < 0) {
                                r_ptr->scost = price;
                                display_cost(store_num, item_val);
                            }
                        } else {
                            display_inventory(store_num, item_val);
                        }
                    }
                    store_prt_gold();
                } else {
                    if (increase_insults(store_num)) {
                        purchase = true;
                    } else {
                        prt_comment1();
                        msg_print("Liar!  You have not the gold!");
                    }
                }
            } else if (choice == 2) {
                purchase = true;
            }

            // Less intuitive, but looks better here than in purchase_haggle.
            display_commands();
            erase_line(1, 0);
        } else {
            prt("You cannot carry that many different items.", 0, 0);
        }
    }

    return purchase;
}

// Sell an item to the store -RAK-
static bool store_sell(int store_num, int *cur_top) {
    bool sell = false;

    int first_item = inventory_count();
    int last_item = -1;

    char mask[INVEN_WIELD];

    for (int counter = 0; counter < inventory_count(); counter++) {
        int flag = (*store_buy[store_num])(inventory_at(counter)->tval);

        mask[counter] = flag;
        if (flag) {
            if (counter < first_item) {
                first_item = counter;
            }
            if (counter > last_item) {
                last_item = counter;
            }
        } // end of if (flag)
    } // end of for (counter)

    int item_val;

    if (last_item == -1) {
        msg_print("You have nothing to sell to this store!");
    } else if (get_item(&item_val, "Which one? ", first_item, last_item, mask, "I do not buy such items.")) {
        inven_type sold_obj;
        msgtype out_val;
        bigvtype tmp_str;

        take_one_item(&sold_obj, inventory_at(item_val));
        objdes(tmp_str, &sold_obj, true);

        (void)snprintf(out_val, sizeof(out_val), "Selling %s (%c)", tmp_str, item_val + 'a');
        msg_print(out_val);

        if (store_check_num(&sold_obj, store_num)) {
            int32_t price;

            int choice = sell_haggle(store_num, &price, &sold_obj);
            if (choice == 0) {
                prt_comment1();
                decrease_insults(store_num);
                player_gain_gold(price);

                // identify object in inventory to set the per-kind record
                identify(&item_val);

                // retake sold_obj so that it will be identified
                take_one_item(&sold_obj, inventory_at(item_val));

                // call known2 for store item, so charges/pluses are known
                known2(&sold_obj);
                inven_destroy(item_val);
                objdes(tmp_str, &sold_obj, true);
                (void)snprintf(out_val, sizeof(out_val), "You've sold %s", tmp_str);
                msg_print(out_val);

                int item_pos;
                store_carry(store_num, &item_pos, &sold_obj);

                check_strength();

                if (item_pos >= 0) {
                    if (item_pos < 12) {
                        if (*cur_top < 12) {
                            display_inventory(store_num, item_pos);
                        } else {
                            *cur_top = 0;
                            display_inventory(store_num, *cur_top);
                        }
                    } else if (*cur_top > 11) {
                        display_inventory(store_num, item_pos);
                    } else {
                        *cur_top = 12;
                        display_inventory(store_num, *cur_top);
                    }
                }
                store_prt_gold();
            } else if (choice == 2) {
                sell = true;
            } else if (choice == 3) {
                msg_print("How dare you!");
                msg_print("I will not buy that!");
                sell = increase_insults(store_num);
            }

            // Less intuitive, but looks better here than in sell_haggle.
            erase_line(1, 0);
            display_commands();
        } else {
            msg_print("I have not the room in my store to keep it.");
        }
    }
    return sell;
}

// Entering a store -RAK-
void enter_store(int store_num) {
    store_type *s_ptr = store_at(store_num);

    if (s_ptr->store_open < progress_turn()) {
        bool exit_flag = false;
        int cur_top = 0;
        display_store(store_num, cur_top);

        do {
            move_cursor(20, 9);

            // the player is about to be asked for a command, so the message
            // up there needs no -more- (the same as dungeon.c's command loop)
            msg_set_pending(false);

            char command;
            if (get_com(CNIL, &command)) {
                int tmp_chr;

                switch (command) {
                case 'b':
                    if (cur_top == 0) {
                        if (s_ptr->store_ctr > 12) {
                            cur_top = 12;
                            display_inventory(store_num, cur_top);
                        } else {
                            msg_print("Entire inventory is shown.");
                        }
                    } else {
                        cur_top = 0;
                        display_inventory(store_num, cur_top);
                    }
                    break;
                case 'E': case 'e': // Equipment List
                case 'I': case 'i': // Inventory
                case 'T': case 't': // Take off
                case 'W': case 'w': // Wear
                case 'X': case 'x': // Switch weapon
                    tmp_chr = py.stats.use_stat[A_CHR];

                    do {
                        inven_command(command);
                        command = pending_inven_command();
                    } while (command);

                    // redisplay store prices if charisma changes
                    if (tmp_chr != py.stats.use_stat[A_CHR]) {
                        display_inventory(store_num, cur_top);
                    }
                    free_turn_flag = false; // No free moves here. -CJS-
                    break;
                case 'p':
                    exit_flag = store_purchase(store_num, &cur_top);
                    break;
                case 's':
                    exit_flag = store_sell(store_num, &cur_top);
                    break;
                default:
                    bell();
                    break;
                }
            } else {
                exit_flag = true;
            }
        } while (!exit_flag);

        // Can't save and restore the screen because inven_command does that.
        draw_cave();
    } else {
        msg_print("The doors are locked.");
    }
}
