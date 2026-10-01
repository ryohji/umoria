// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Code for potions

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "inventory.h"
#include "item_learn.h"
#include "player_abilities.h"
#include "player_food.h"
#include "player_level.h"
#include "player_mana.h"
#include "player_timed_effects.h"

// Potions for the quaffing -RAK-
void quaff(void) {
    free_turn_flag = true;

    int j, k, item_val;
    if (inventory_count() == 0) {
        msg_print("But you are not carrying anything.");
    } else if (!find_range(TV_POTION1, TV_POTION2, &j, &k)) {
        msg_print("You are not carrying any potions.");
    } else if (get_item(&item_val, "Quaff which potion?", j, k, CNIL, CNIL)) {
        inven_type *i_ptr = inventory_at(item_val);

        uint32_t i = i_ptr->flags;
        free_turn_flag = false;
        bool ident = false;
        if (i == 0) {
            msg_print("You feel less thirsty.");
            ident = true;
        } else {
            while (i != 0) {
                j = bit_pos(&i) + 1;
                if (i_ptr->tval == TV_POTION2) {
                    j += 32;
                }

                // Potions
                switch (j) {
                case 1:
                    if (inc_stat(A_STR)) {
                        msg_print("Wow!  What bulging muscles!");
                        ident = true;
                    }
                    break;
                case 2:
                    ident = true;
                    lose_str();
                    break;
                case 3:
                    if (res_stat(A_STR)) {
                        msg_print("You feel warm all over.");
                        ident = true;
                    }
                    break;
                case 4:
                    if (inc_stat(A_INT)) {
                        msg_print("Aren't you brilliant!");
                        ident = true;
                    }
                    break;
                case 5:
                    ident = true;
                    lose_int();
                    break;
                case 6:
                    if (res_stat(A_INT)) {
                        msg_print("You have have a warm feeling.");
                        ident = true;
                    }
                    break;
                case 7:
                    if (inc_stat(A_WIS)) {
                        msg_print("You suddenly have a profound thought!");
                        ident = true;
                    }
                    break;
                case 8:
                    ident = true;
                    lose_wis();
                    break;
                case 9:
                    if (res_stat(A_WIS)) {
                        msg_print("You feel your wisdom returning.");
                        ident = true;
                    }
                    break;
                case 10:
                    if (inc_stat(A_CHR)) {
                        msg_print("Gee, ain't you cute!");
                        ident = true;
                    }
                    break;
                case 11:
                    ident = true;
                    lose_chr();
                    break;
                case 12:
                    if (res_stat(A_CHR)) {
                        msg_print("You feel your looks returning.");
                        ident = true;
                    }
                    break;
                case 13:
                    ident = hp_player(damroll(2, 7));
                    break;
                case 14:
                    ident = hp_player(damroll(4, 7));
                    break;
                case 15:
                    ident = hp_player(damroll(6, 7));
                    break;
                case 16:
                    ident = hp_player(1000);
                    break;
                case 17:
                    if (inc_stat(A_CON)) {
                        msg_print("You feel tingly for a moment.");
                        ident = true;
                    }
                    break;
                case 18:
                    if (player_experience() < MAX_EXP) {
                        uint32_t l = (uint32_t)(player_experience() / 2) + 10;

                        if (l > 100000L) {
                            l = 100000L;
                        }

                        player_gain_experience((int32_t)l);
                        msg_print("You feel more experienced.");
                        prt_experience();
                        ident = true;
                    }
                    break;
                case 19:
                    if (!player_never_paralyzed()) {
                        // paralysis must == 0, otherwise could not drink potion
                        msg_print("You fall asleep.");
                        player_timed_add(PLAYER_TIMED_PARALYSIS, randint(4) + 4);
                        ident = true;
                    }
                    break;
                case 20:
                    if (!player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                        msg_print("You are covered by a veil of darkness.");
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_BLINDNESS, randint(100) + 100);
                    break;
                case 21:
                    if (!player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
                        msg_print("Hey!  This is good stuff!  * Hick! *");
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_CONFUSION, randint(20) + 12);
                    break;
                case 22:
                    if (!player_timed_in_force(PLAYER_TIMED_POISON)) {
                        msg_print("You feel very sick.");
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_POISON, randint(15) + 10);
                    break;
                case 23:
                    if (!player_timed_in_force(PLAYER_TIMED_HASTE)) {
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_HASTE, randint(25) + 15);
                    break;
                case 24:
                    if (!player_timed_in_force(PLAYER_TIMED_SLOWNESS)) {
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_SLOWNESS, randint(25) + 15);
                    break;
                case 26:
                    if (inc_stat(A_DEX)) {
                        msg_print("You feel more limber!");
                        ident = true;
                    }
                    break;
                case 27:
                    if (res_stat(A_DEX)) {
                        msg_print("You feel less clumsy.");
                        ident = true;
                    }
                    break;
                case 28:
                    if (res_stat(A_CON)) {
                        msg_print("You feel your health returning!");
                        ident = true;
                    }
                    break;
                case 29:
                    ident = cure_blindness();
                    break;
                case 30:
                    ident = cure_confusion();
                    break;
                case 31:
                    ident = cure_poison();
                    break;
                case 34:
                    if (player_experience() > 0) {
                        int32_t m, scale;
                        msg_print("You feel your memories fade.");
                        // Lose between 1/5 and 2/5 of your experience
                        m = player_experience() / 5;
                        if (player_experience() > MAX_SHORT) {
                            scale = MAX_LONG / player_experience();
                            m += (randint((int)scale) * player_experience()) /
                                 (scale * 5);
                        } else {
                            m += randint((int)player_experience()) / 5;
                        }
                        lose_exp(m);
                        ident = true;
                    }
                    break;
                case 35:
                    (void)cure_poison();
                    if (player_food() > 150) {
                        player_set_food(150);
                    }
                    player_timed_set(PLAYER_TIMED_PARALYSIS, 4);
                    msg_print("The potion makes you vomit!");
                    ident = true;
                    break;
                case 36:
                    if (!player_timed_in_force(PLAYER_TIMED_INVULNERABILITY)) {
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_INVULNERABILITY, randint(10) + 10);
                    break;
                case 37:
                    if (!player_timed_in_force(PLAYER_TIMED_HEROISM)) {
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_HEROISM, randint(25) + 25);
                    break;
                case 38:
                    if (!player_timed_in_force(PLAYER_TIMED_SUPER_HEROISM)) {
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_SUPER_HEROISM, randint(25) + 25);
                    break;
                case 39:
                    ident = remove_fear();
                    break;
                case 40:
                    ident = restore_level();
                    break;
                case 41:
                    if (!player_timed_in_force(PLAYER_TIMED_HEAT_RESISTANCE)) {
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_HEAT_RESISTANCE, randint(10) + 10);
                    break;
                case 42:
                    if (!player_timed_in_force(PLAYER_TIMED_COLD_RESISTANCE)) {
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_COLD_RESISTANCE, randint(10) + 10);
                    break;
                case 43:
                    if (!player_timed_in_force(PLAYER_TIMED_SEEING_INVISIBLE)) {
                        ident = true;
                    }
                    detect_inv2(randint(12) + 12);
                    break;
                case 44:
                    ident = slow_poison();
                    break;
                case 45:
                    ident = cure_poison();
                    break;
                case 46:
                    if (player_restore_mana()) {
                        ident = true;
                        msg_print("Your feel your head clear.");
                        prt_cmana();
                    }
                    break;
                case 47:
                    if (!player_timed_in_force(PLAYER_TIMED_INFRA_VISION)) {
                        msg_print("Your eyes begin to tingle.");
                        ident = true;
                    }
                    player_timed_add(PLAYER_TIMED_INFRA_VISION, 100 + randint(100));
                    break;
                default:
                    msg_print("Internal error in potion()");
                    break;
                }
                // End of Potions.
            }
        }

        i_ptr = learn_item_effect(ident, &item_val);

        add_food(i_ptr->p1);
        desc_remain(item_val);
        inven_destroy(item_val);
    }
}
