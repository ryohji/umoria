// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Code for mage spells

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "inventory.h"
#include "commands.h"
#include "dice.h"
#include "ui_input.h"
#include "ui_screen.h"
#include "world_item.h"
#include "world_player.h"
#include "world_dungeon.h"
#include "world_combat.h"
#include "world_view.h"
#include "player.h"
#include "rnd.h"
#include "player_identity.h"
#include "player_progress.h"
#include "player_condition.h"
#include "player_spells.h"

// Throw a magic spell -RAK-
// Returns whether it took a game turn.
bool cast(void) {
    int i, j, item_val;

    bool took_turn = false;
    if (player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
        msg_print("You can't see to read your spell book!");
    } else if (no_light()) {
        msg_print("You have no light to read by.");
    } else if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
        msg_print("You are too confused.");
    } else if (player_class_spell_type() != MAGE) {
        msg_print("You can't cast spells!");
    } else if (!find_range(TV_MAGIC_BOOK, TV_NEVER, &i, &j)) {
        msg_print("But you are not carrying any spell-books!");
    } else if (get_item(&item_val, "Use which spell-book?", i, j, CNIL, CNIL)) {
        int choice, chance;
        int result = cast_spell("Cast which spell?", item_val, &choice, &chance);

        if (result < 0) {
            msg_print("You don't know any spells in that book.");
        } else if (result > 0) {
            took_turn = true;

            spell_type *m_ptr = &magic_spell[player_class() - 1][choice];

            if (randint(100) < chance) {
                msg_print("You failed to get the spell off!");
            } else {
                int dir;

                // Spells.
                switch (choice + 1) {
                case 1:
                    if (get_dir(CNIL, &dir)) {
                        fire_bolt(GF_MAGIC_MISSILE, dir, player_row(), player_col(), damroll(2, 6), spell_names[0]);
                    } else {
                        took_turn = false;
                    }
                    break;
                case 2:
                    (void)detect_monsters();
                    break;
                case 3:
                    teleport(10);
                    break;
                case 4:
                    (void)light_area(player_row(), player_col());
                    break;
                case 5:
                    (void)hp_player(damroll(4, 4));
                    break;
                case 6:
                    (void)detect_sdoor();
                    (void)detect_trap();
                    break;
                case 7:
                    if (get_dir(CNIL, &dir)) {
                        fire_ball(GF_POISON_GAS, dir, player_row(), player_col(), 12, spell_names[6]);
                    } else {
                        took_turn = false;
                    }
                    break;
                case 8:
                    if (get_dir(CNIL, &dir)) {
                        (void)confuse_monster(dir, player_row(), player_col());
                    } else {
                        took_turn = false;
                    }
                    break;
                case 9:
                    if (get_dir(CNIL, &dir)) {
                        fire_bolt(GF_LIGHTNING, dir, player_row(), player_col(), damroll(4, 8), spell_names[8]);
                    } else {
                        took_turn = false;
                    }
                    break;
                case 10:
                    (void)td_destroy();
                    break;
                case 11:
                    if (get_dir(CNIL, &dir)) {
                        (void)sleep_monster(dir, player_row(), player_col());
                    } else {
                        took_turn = false;
                    }
                    break;
                case 12:
                    (void)cure_poison();
                    break;
                case 13:
                    teleport((player_level() * 5));
                    break;
                case 14:
                    (void)remove_curse();
                    break;
                case 15:
                    if (get_dir(CNIL, &dir)) {
                        fire_bolt(GF_FROST, dir, player_row(), player_col(), damroll(6, 8), spell_names[14]);
                    } else {
                        took_turn = false;
                    }
                    break;
                case 16:
                    if (get_dir(CNIL, &dir)) {
                        (void)wall_to_mud(dir, player_row(), player_col());
                    } else {
                        took_turn = false;
                    }
                    break;
                case 17:
                    if (create_food() == EFFECT_CANCELLED) {
                        took_turn = false;
                    }
                    break;
                case 18:
                    if (recharge(20) == EFFECT_CANCELLED) {
                        took_turn = false;
                    }
                    break;
                case 19:
                    (void)sleep_monsters1(player_row(), player_col());
                    break;
                case 20:
                    if (get_dir(CNIL, &dir)) {
                        (void)poly_monster(dir, player_row(), player_col());
                    } else {
                        took_turn = false;
                    }
                    break;
                case 21:
                    if (ident_spell() == EFFECT_CANCELLED) {
                        took_turn = false;
                    }
                    break;
                case 22:
                    (void)sleep_monsters2();
                    break;
                case 23:
                    if (get_dir(CNIL, &dir)) {
                        fire_bolt(GF_FIRE, dir, player_row(), player_col(), damroll(9, 8), spell_names[22]);
                    } else {
                        took_turn = false;
                    }
                    break;
                case 24:
                    if (get_dir(CNIL, &dir)) {
                        (void)speed_monster(dir, player_row(), player_col(), -1);
                    } else {
                        took_turn = false;
                    }
                    break;
                case 25:
                    if (get_dir(CNIL, &dir)) {
                        fire_ball(GF_FROST, dir, player_row(), player_col(), 48, spell_names[24]);
                    } else {
                        took_turn = false;
                    }
                    break;
                case 26:
                    if (recharge(60) == EFFECT_CANCELLED) {
                        took_turn = false;
                    }
                    break;
                case 27:
                    if (get_dir(CNIL, &dir)) {
                        (void)teleport_monster(dir, player_row(), player_col());
                    } else {
                        took_turn = false;
                    }
                    break;
                case 28:
                    player_timed_add(PLAYER_TIMED_HASTE, randint(20) + player_level());
                    break;
                case 29:
                    if (get_dir(CNIL, &dir)) {
                        fire_ball(GF_FIRE, dir, player_row(), player_col(), 72, spell_names[28]);
                    } else {
                        took_turn = false;
                    }
                    break;
                case 30:
                    destroy_area(player_row(), player_col());
                    break;
                case 31:
                    (void)genocide();
                    break;
                default:
                    break;
                }
                // End of spells.

                if (took_turn) {
                    if (!spell_has_worked(choice)) {
                        player_gain_experience(m_ptr->sexp << 2);
                        spell_mark_worked(choice);
                        prt_experience();
                    }
                }
            }

            if (took_turn) {
                // Paying less than the spell costs is what makes the caster
                // faint -- five turns of it for every point that was missing.
                int spent = player_spend_mana(m_ptr->smana);
                if (spent < m_ptr->smana) {
                    msg_print("You faint from the effort!");
                    player_timed_set(PLAYER_TIMED_PARALYSIS,
                                     randint((int)(5 * (m_ptr->smana - spent))));
                    if (randint(3) == 1) {
                        msg_print("You have damaged your health!");
                        (void)dec_stat(A_CON);
                    }
                }
                prt_cmana();
            }
        }
    }
    return took_turn;
}
