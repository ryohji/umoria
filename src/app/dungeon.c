// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The main command interpreter, updating player status

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include "dungeon_state.h"
#include "monster_state.h"
#include "game_status.h"
#include "bits.h"
#include "commands.h"
#include "world_monster.h"
#include "world_item.h"
#include "world_dungeon.h"
#include "world_view.h"
#include "world_player.h"
#include "world_combat.h"
#include "world_store.h"
#include "app_internal.h"
#include "ui_input.h"
#include "ui_screen.h"
#include "game_settings.h"
#include "geometry.h"
#include "monsters.h"
#include "player.h"
#include "rnd.h"
#include "inventory.h"
#include "item_knowledge.h"
#include "player_condition.h"
#include "player_stats.h"
#include "player_identity.h"
#include "player_progress.h"
#include "player_skills.h"

// Constants describing limits of certain objects
#define OBJ_LAMP_MAX          15000   // Maximum amount that lamp can be filled

// with MAX_MALLOC 101, it is possible to get compacting monsters messages
// while breeding/cloning monsters
#define MAX_MALLOC_CHANCE       160   // 1/x chance of new monster each round

// Player constants
#define PLAYER_FOOD_FAINT       300   // Character begins fainting
#define PLAYER_FOOD_WEAK       1000   // Warn player that he is getting very low
#define PLAYER_FOOD_ALERT      2000   // Warn player that he is getting low
#define PLAYER_REGEN_FAINT       33   // Regen factor*2^16 when fainting
#define PLAYER_REGEN_WEAK        98   // Regen factor*2^16 when weak

static char original_commands(char);
static bool do_command(char);
static bool valid_countcommand(char);
static void regenhp(int);
static void regenmana(int);
static bool enchanted(inven_type *);
static void examine_book(void);
static void go_up(void);
static void go_down(void);
static void jamdoor(void);
static void refill_lamp(void);

// Moria game module -RAK-
// The code in this section has gone through many revisions, and
// some of it could stand some more hard work. -RAK-

// It has had a bit more hard work. -CJS-

// Heroism (must precede anything that can damage player)
static void tick_heroism(void) {
    if (player_timed_in_force(PLAYER_TIMED_HEROISM)) {
        if (player_timed_beginning(PLAYER_TIMED_HEROISM)) {
            disturb(0, 0);
            player_gain_temporary_max_hp(10);
            // Adjust both melee and ranged to-hit by the same amount.
            player_base_to_hit_adjust_both(12);
            msg_print("You feel like a HERO!");
            prt_mhp();
            prt_chp();
        }
        if (player_timed_count_down(PLAYER_TIMED_HEROISM)) {
            disturb(0, 0);
            if (player_lose_temporary_max_hp(10)) {
                prt_chp();
            }
            player_base_to_hit_adjust_both(-12);
            msg_print("The heroism wears off.");
            prt_mhp();
        }
    }
}

// Super Heroism
static void tick_super_heroism(void) {
    if (player_timed_in_force(PLAYER_TIMED_SUPER_HEROISM)) {
        if (player_timed_beginning(PLAYER_TIMED_SUPER_HEROISM)) {
            disturb(0, 0);
            player_gain_temporary_max_hp(20);
            player_base_to_hit_adjust_both(24);
            msg_print("You feel like a SUPER HERO!");
            prt_mhp();
            prt_chp();
        }
        if (player_timed_count_down(PLAYER_TIMED_SUPER_HEROISM)) {
            disturb(0, 0);
            if (player_lose_temporary_max_hp(20)) {
                prt_chp();
            }
            player_base_to_hit_adjust_both(-24);
            msg_print("The super heroism wears off.");
            prt_mhp();
        }
    }
}

// Blindness
static void tick_blindness(void) {
    if (player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
        if (player_timed_beginning(PLAYER_TIMED_BLINDNESS)) {
            prt_map();
            prt_blind();
            disturb(0, 1);

            // unlight creatures
            creatures(false);
        }
        if (player_timed_count_down(PLAYER_TIMED_BLINDNESS)) {
            prt_blind();
            prt_map();

            // light creatures
            disturb(0, 1);
            creatures(false);
            msg_print("The veil of darkness lifts.");
        }
    }
}

// Confusion
static void tick_confusion(void) {
    if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
        if (player_timed_beginning(PLAYER_TIMED_CONFUSION)) {
            prt_confused();
        }
        if (player_timed_count_down(PLAYER_TIMED_CONFUSION)) {
            prt_confused();
            msg_print("You feel less confused now.");
            if (player_resting()) {
                rest_off();
            }
        }
    }
}

// Afraid
static void tick_fear(void) {
    if (player_timed_in_force(PLAYER_TIMED_FEAR)) {
        // The old test was (shero + hero) > 0. Neither counter is ever
        // negative, so the sum is above zero exactly when one of the two is.
        bool heroic = player_timed_in_force(PLAYER_TIMED_SUPER_HEROISM) ||
                      player_timed_in_force(PLAYER_TIMED_HEROISM);

        if (!player_effect_in_force(PLAYER_EFFECT_AFRAID)) {
            if (heroic) {
                // Cleared, not ended: the count-down below then takes the
                // counter to minus one and no "bolder" message is given,
                // which is what the original did.
                player_timed_clear(PLAYER_TIMED_FEAR);
            } else {
                (void)player_timed_beginning(PLAYER_TIMED_FEAR);
                prt_afraid();
            }
        } else if (heroic) {
            // One turn left, so the count-down below ends it properly.
            player_timed_set(PLAYER_TIMED_FEAR, 1);
        }
        if (player_timed_count_down(PLAYER_TIMED_FEAR)) {
            prt_afraid();
            msg_print("You feel bolder now.");
            disturb(0, 0);
        }
    }
}

// Poisoned
static void tick_poison(void) {
    int i;

    if (player_timed_in_force(PLAYER_TIMED_POISON)) {
        if (player_timed_beginning(PLAYER_TIMED_POISON)) {
            prt_poisoned();
        }
        if (player_timed_count_down(PLAYER_TIMED_POISON)) {
            prt_poisoned();
            msg_print("You feel better.");
            disturb(0, 0);
        } else {
            switch (con_adj()) {
            case -4:
                i = 4;
                break;
            case -3:
            case -2:
                i = 3;
                break;
            case -1:
                i = 2;
                break;
            case 0:
                i = 1;
                break;
            case 1:
            case 2:
            case 3:
                i = ((progress_turn() % 2) == 0);
                break;
            case 4:
            case 5:
                i = ((progress_turn() % 3) == 0);
                break;
            case 6:
                i = ((progress_turn() % 4) == 0);
                break;
            default:
                // An uninitialized warning if given further down,
                // so let's fix that here -MRC-
                i = 0;
            }

            take_hit(i, "poison");
            disturb(1, 0);
        }
    }
}

// Fast
static void tick_haste(void) {
    if (player_timed_in_force(PLAYER_TIMED_HASTE)) {
        if (player_timed_beginning(PLAYER_TIMED_HASTE)) {
            change_speed(-1);
            msg_print("You feel yourself moving faster.");
            disturb(0, 0);
        }
        if (player_timed_count_down(PLAYER_TIMED_HASTE)) {
            change_speed(1);
            msg_print("You feel yourself slow down.");
            disturb(0, 0);
        }
    }
}

// Slow
static void tick_slowness(void) {
    if (player_timed_in_force(PLAYER_TIMED_SLOWNESS)) {
        if (player_timed_beginning(PLAYER_TIMED_SLOWNESS)) {
            change_speed(1);
            msg_print("You feel yourself moving slower.");
            disturb(0, 0);
        }
        if (player_timed_count_down(PLAYER_TIMED_SLOWNESS)) {
            change_speed(-1);
            msg_print("You feel yourself speed up.");
            disturb(0, 0);
        }
    }
}

// Hallucinating?   (Random characters appear!)
static void tick_hallucination(void) {
    if (player_timed_in_force(PLAYER_TIMED_HALLUCINATION)) {
        end_find();
        if (player_timed_count_down(PLAYER_TIMED_HALLUCINATION)) {
            prt_map(); // Used to draw entire screen! -CJS-
        }
    }
}

// Paralysis
static void tick_paralysis(void) {
    if (player_timed_in_force(PLAYER_TIMED_PARALYSIS)) {
        // when paralysis true, you can not see any movement that occurs
        (void)player_timed_count_down(PLAYER_TIMED_PARALYSIS);
        disturb(1, 0);
    }
}

// Protection from evil counter
static void tick_protection_from_evil(void) {
    if (player_timed_in_force(PLAYER_TIMED_PROTECTION_FROM_EVIL)) {
        if (player_timed_count_down(PLAYER_TIMED_PROTECTION_FROM_EVIL)) {
            msg_print("You no longer feel safe from evil.");
        }
    }
}

// Invulnerability
static void tick_invulnerability(void) {
    if (player_timed_in_force(PLAYER_TIMED_INVULNERABILITY)) {
        if (player_timed_beginning(PLAYER_TIMED_INVULNERABILITY)) {
            disturb(0, 0);
            player_armour_class_adjust(100);
            player_display_add_ac(100);
            prt_pac();
            msg_print("Your skin turns into steel!");
        }
        if (player_timed_count_down(PLAYER_TIMED_INVULNERABILITY)) {
            disturb(0, 0);
            player_armour_class_adjust(-100);
            player_display_add_ac(-100);
            prt_pac();
            msg_print("Your skin returns to normal.");
        }
    }
}

// Blessed
static void tick_blessing(void) {
    if (player_timed_in_force(PLAYER_TIMED_BLESSING)) {
        if (player_timed_beginning(PLAYER_TIMED_BLESSING)) {
            disturb(0, 0);
            player_base_to_hit_adjust_both(5);
            player_armour_class_adjust(2);
            player_display_add_ac(2);
            msg_print("You feel righteous!");
            prt_pac();
        }
        if (player_timed_count_down(PLAYER_TIMED_BLESSING)) {
            disturb(0, 0);
            player_base_to_hit_adjust_both(-5);
            player_armour_class_adjust(-2);
            player_display_add_ac(-2);
            msg_print("The prayer has expired.");
            prt_pac();
        }
    }
}

// Resist Heat
static void tick_heat_resistance(void) {
    if (player_timed_in_force(PLAYER_TIMED_HEAT_RESISTANCE)) {
        if (player_timed_count_down(PLAYER_TIMED_HEAT_RESISTANCE)) {
            msg_print("You no longer feel safe from flame.");
        }
    }
}

// Resist Cold
static void tick_cold_resistance(void) {
    if (player_timed_in_force(PLAYER_TIMED_COLD_RESISTANCE)) {
        if (player_timed_count_down(PLAYER_TIMED_COLD_RESISTANCE)) {
            msg_print("You no longer feel safe from cold.");
        }
    }
}

// Detect Invisible
static void tick_seeing_invisible(void) {
    if (player_timed_in_force(PLAYER_TIMED_SEEING_INVISIBLE)) {
        if (player_timed_beginning(PLAYER_TIMED_SEEING_INVISIBLE)) {
            player_grant_see_invisible();

            // light but don't move creatures
            creatures(false);
        }
        if (player_timed_count_down(PLAYER_TIMED_SEEING_INVISIBLE)) {
            // may still be able to see_inv if wearing magic item
            calc_bonuses();

            // unlight but don't move creatures
            creatures(false);
        }
    }
}

// Timed infra-vision
static void tick_infra_vision(void) {
    if (player_timed_in_force(PLAYER_TIMED_INFRA_VISION)) {
        if (player_timed_beginning(PLAYER_TIMED_INFRA_VISION)) {
            // The clock counts TURNS and lives in player_timed_effects.c;
            // the one square it is worth is this file's to add, because this
            // is where the two ends meet (player_skills.h).
            player_infra_range_adjust(1);

            // light but don't move creatures
            creatures(false);
        }
        if (player_timed_count_down(PLAYER_TIMED_INFRA_VISION)) {
            player_infra_range_adjust(-1);

            // unlight but don't move creatures
            creatures(false);
        }
    }
}

// Word-of-Recall  Note: Word-of-Recall is a delayed action
static void tick_word_of_recall(void) {
    if (player_timed_in_force(PLAYER_TIMED_WORD_OF_RECALL)) {
        if (player_timed_turns(PLAYER_TIMED_WORD_OF_RECALL) == 1) {
            player_timed_add(PLAYER_TIMED_PARALYSIS, 1);
            player_timed_clear(PLAYER_TIMED_WORD_OF_RECALL);
            if (!player_is_in_town()) {
                leave_for_level(0);
                msg_print("You feel yourself yanked upwards!");
            } else if (player_max_depth() != 0) {
                leave_for_level(player_max_depth());
                msg_print("You feel yourself yanked downwards!");
            } else {
                // In town, and no depth recorded yet, so there is nowhere to
                // be yanked to. The level still ends, as it always did: the
                // town is built again.
                end_level();
            }
        } else {
            (void)player_timed_count_down(PLAYER_TIMED_WORD_OF_RECALL);
        }
    }
}

void dungeon(void) {
    int i;

    // Main procedure for dungeon. -RAK-
    // Note: There is a lot of preliminary magic going on here at first

    // Check light status for setup
    inven_type *i_ptr = equipment_at(INVEN_LIGHT);
    set_player_has_light(i_ptr->p1 > 0);

    // Record deepest level reached.
    player_note_depth_reached(dungeon_level());

    // Reset flags and initialize variables
    int find_count = 0;
    cancel_command_count();
    begin_level();
    forget_run();
    forget_pending_teleport();
    monster_breeding_reset();
    square_at(player_row(), player_col())->cptr = 1;

    // Ensure we display the panel.
    panel_forget_position();

    // Light up the area around character
    check_view();

    // must do this after panel_forget_position(), because search_off() will
    // call check_view(), and so the panel must know where it is before
    // search_off() is called
    if (player_is_searching()) {
        search_off();
    }

    // Light,  but do not move critters
    creatures(false);

    // Print the depth
    prt_depth();

    //
    // Loop until dead,  or new level
    //
    do {
        progress_advance_turn();

        // Check for pending signals (SIGINT, SIGSEGV, etc.)
        // This must be done in the main loop, not in signal handlers
        handle_pending_signals();

        // turn over the store contents every, say, 1000 turns
        if (!player_is_in_town() && ((progress_turn() % 1000) == 0)) {
            store_maint();
        }

        // Check for creature generation
        if (randint(MAX_MALLOC_CHANCE) == 1) {
            alloc_monster(1, MAX_SIGHT, false);
        }

        // Check light status
        i_ptr = equipment_at(INVEN_LIGHT);
        if (player_has_light()) {
            if (i_ptr->p1 > 0) {
                i_ptr->p1--;
                if (i_ptr->p1 == 0) {
                    set_player_has_light(false);
                    msg_print("Your light has gone out!");
                    disturb(0, 1);

                    // unlight creatures
                    creatures(false);
                } else if ((i_ptr->p1 < 40) && (randint(5) == 1) && !player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                    disturb(0, 0);
                    msg_print("Your light is growing faint.");
                }
            } else {
                set_player_has_light(false);
                disturb(0, 1);

                // unlight creatures
                creatures(false);
            }
        } else if (i_ptr->p1 > 0) {
            i_ptr->p1--;
            set_player_has_light(true);
            disturb(0, 1);

            // light creatures
            creatures(false);
        }

        //
        // Update counters and messages
        //

        tick_heroism();

        tick_super_heroism();

        // Check food status
        int regen_amount = PLAYER_REGEN_NORMAL; // Regenerate hp and mana
        if (player_food() < PLAYER_FOOD_ALERT) {
            if (player_food() < PLAYER_FOOD_WEAK) {
                if (player_food() < 0) {
                    regen_amount = 0;
                } else if (player_food() < PLAYER_FOOD_FAINT) {
                    regen_amount = PLAYER_REGEN_FAINT;
                } else {
                    regen_amount = PLAYER_REGEN_WEAK;
                }
                if (player_note_effect_started(PLAYER_EFFECT_WEAK)) {
                    msg_print("You are getting weak from hunger.");
                    disturb(0, 0);
                    prt_hunger();
                }
                if ((player_food() < PLAYER_FOOD_FAINT) && (randint(8) == 1)) {
                    player_timed_add(PLAYER_TIMED_PARALYSIS, randint(5));
                    msg_print("You faint from the lack of food.");
                    disturb(1, 0);
                }
            } else if (player_note_effect_started(PLAYER_EFFECT_HUNGRY)) {
                msg_print("You are getting hungry.");
                disturb(0, 0);
                prt_hunger();
            }
        }

        // Food consumption
        // Note: Speeded up characters really burn up the food!
        // Speed is squared here before converting to food consumption.
        const int speed_steps = player_speed();
        if (speed_steps < 0) {
            player_burn_food(speed_steps * speed_steps);
        }
        player_digest();
        if (player_food() < 0) {
            take_hit(-player_food() / 16, "starvation"); // -CJS-
            disturb(1, 0);
        }

        // Regenerate
        if (player_regenerates()) {
            regen_amount = regen_amount * 3 / 2;
        }
        if (player_is_searching() || player_resting()) {
            regen_amount = regen_amount * 2;
        }
        if (!player_timed_in_force(PLAYER_TIMED_POISON) && (player_hp() < player_max_hp())) {
            regenhp(regen_amount);
        }
        if (player_mana() < player_max_mana()) {
            regenmana(regen_amount);
        }

        tick_blindness();

        tick_confusion();

        tick_fear();

        tick_poison();

        tick_haste();

        tick_slowness();

        // Resting is over?
        if (player_resting()) {
            // Asked before the turn passes: a count of minus one becomes zero
            // below, and then it is no longer a "rest until healed".
            bool until_healed = player_rest_is_until_healed();

            bool ran_out = player_rest_count_down();

            // Two ways for a rest to end. The second one only applies to a rest
            // until reaching max mana and max hit points, and it reads two other
            // questions, so it stays here.
            if (ran_out ||
                (until_healed && player_hp() == player_max_hp() &&
                 player_mana() == player_max_mana())) {
                rest_off();
            }
        }

        // Check for interrupts to find or rest.
        if ((command_is_repeating() || player_is_running() || player_resting()) &&
            (check_input(player_is_running() ? 0 : 10000))) {
            disturb(0, 0);
        }

        tick_hallucination();

        tick_paralysis();

        tick_protection_from_evil();

        tick_invulnerability();

        tick_blessing();

        tick_heat_resistance();

        tick_cold_resistance();

        tick_seeing_invisible();

        tick_infra_vision();

        tick_word_of_recall();

        // Random teleportation
        if (player_teleports_randomly() && (randint(100) == 1)) {
            disturb(0, 0);
            teleport(40);
        }

        // See if we are too weak to handle the weapon or pack. -CJS-
        if (player_strength_check_requested()) {
            check_strength();
        }

        if (player_study_redraw_requested()) {
            prt_study();
        }

        if (player_take_speed_redraw_request()) {
            prt_speed();
        }

        if (player_status_line_shows_paralysis() && !player_timed_in_force(PLAYER_TIMED_PARALYSIS)) {
            prt_state();
            player_set_status_line_shows_paralysis(false);
        } else if (player_timed_in_force(PLAYER_TIMED_PARALYSIS)) {
            prt_state();
            player_set_status_line_shows_paralysis(true);
        } else if (player_resting()) {
            prt_state();
        }

        if (player_take_armor_redraw_request()) {
            prt_pac();
        }

        if (player_any_stat_redraw_requested()) {
            for (int n = 0; n < 6; n++) {
                if (player_stat_redraw_requested(n)) {
                    prt_stat(n);
                }
            }

            player_clear_stat_redraw_requests();
        }

        if (player_take_hp_redraw_request()) {
            prt_mhp();
            prt_chp();
        }

        if (player_take_mana_redraw_request()) {
            prt_cmana();
        }

        // Allow for a slim chance of detect enchantment -CJS-
        // for 1st level char, check once every 2160 turns
        // for 40th level char, check once every 416 turns
        if (((progress_turn() & 0xF) == 0) && !player_timed_in_force(PLAYER_TIMED_CONFUSION) &&
            (randint((10 + 750 / (5 + player_level()))) == 1)) {

            for (i = 0; i < inventory_and_equipment_slot_count(); i++) {
                if (i == inventory_count()) {
                    i = 22;
                }
                i_ptr = inventory_and_equipment_at(i);

                // if in inventory, succeed 1 out of 50 times,
                // if in equipment list, success 1 out of 10 times
                if ((i_ptr->tval != TV_NOTHING) && enchanted(i_ptr) &&
                    (randint(i < 22 ? 50 : 10) == 1)) {
                    vtype tmp_str;
                    (void)sprintf(tmp_str, "There's something about what you are %s...", describe_use(i));
                    disturb(0, 0);
                    msg_print(tmp_str);
                    add_inscribe(i_ptr, ID_MAGIK);
                }
            }
        }

        // Check the state of the monster list, and delete some monsters if
        // the monster list is nearly full.  This helps to avoid problems in
        // creature.c when monsters try to multiply.  Compact_monsters() is
        // much more likely to succeed if called from here, than if called
        // from within creatures().
        if (monster_list_free_slots() < 10) {
            (void)compact_monsters();
        }

        // Accept a command?
        if (!player_timed_in_force(PLAYER_TIMED_PARALYSIS) && !player_resting() && (!player_is_dead())) {
            char command; // Last command
            bool took_turn;

            // Accept a command and execute it
            do {
                if (player_status_line_shows_repeat()) {
                    prt_state();
                }

                ask_for_direction_again();
                free_turn_flag = false;

                if (player_is_running()) {
                    find_run();
                    find_count--;
                    if (find_count == 0) {
                        end_find();
                    }
                    put_qio();
                    took_turn = !free_turn_flag;
                } else if (pending_inven_command()) {
                    inven_command(pending_inven_command());
                    took_turn = !free_turn_flag;
                } else {
                    // move the cursor to the players character
                    move_cursor_relative(player_row(), player_col());

                    if (command_is_repeating()) {
                        msg_set_pending(false);
                        reuse_remembered_direction();
                    } else {
                        msg_set_pending(false);
                        command = inkey();

                        i = 0;

                        // Get a count for a command.
                        if ((rogue_like_commands && command >= '0' && command <= '9') || (!rogue_like_commands && command == '#')) {
                            // Buffer sized for decimal int with sign and null terminator.
                            char tmp[12];

                            prt("Repeat count:", 0, 0);
                            if (command == '#') {
                                command = '0';
                            }

                            i = 0;

                            while (true) {
                                if (command == DELETE || command == CTRL_KEY('H')) {
                                    i = i / 10;
                                    (void)sprintf(tmp, "%d", i);
                                    prt(tmp, 0, 14);
                                } else if (command >= '0' && command <= '9') {
                                    if (i > 99) {
                                        bell();
                                    } else {
                                        i = i * 10 + command - '0';
                                        (void)sprintf(tmp, "%d", i);
                                        prt(tmp, 0, 14);
                                    }
                                } else {
                                    break;
                                }
                                command = inkey();
                            }

                            if (i == 0) {
                                i = 99;
                                (void)sprintf(tmp, "%d", i);
                                prt(tmp, 0, 14);
                            }

                            // a special hack to allow numbers as commands
                            if (command == ' ') {
                                prt("Command:", 0, 20);
                                command = inkey();
                            }
                        }

                        // Another way of typing control codes -CJS-
                        if (command == '^') {
                            if (command_is_repeating()) {
                                prt_state();
                            }
                            if (get_com("Control-", &command)) {
                                if (command >= 'A' && command <= 'Z') {
                                    command -= 'A' - 1;
                                } else if (command >= 'a' && command <= 'z') {
                                    command -= 'a' - 1;
                                } else {
                                    msg_print(
                                        "Type ^ <letter> for a control char");
                                    command = ' ';
                                }
                            } else {
                                command = ' ';
                            }
                        }

                        // move cursor to player char again, in case it moved
                        move_cursor_relative(player_row(), player_col());

                        // Commands are always converted to rogue form. -CJS-
                        if (rogue_like_commands == false) {
                            command = original_commands(command);
                        }
                        if (i > 0) {
                            if (!valid_countcommand(command)) {
                                free_turn_flag = true;
                                msg_print("Invalid command with a count.");
                                command = ' ';
                            } else {
                                begin_command_count(i);
                                prt_state();
                            }
                        }
                    }

                    // Flash the message line.
                    erase_line(MSG_LINE, 0);
                    move_cursor_relative(player_row(), player_col());
                    put_qio();

                    took_turn = do_command(command);

                    // Find is counted differently, as the command changes.
                    if (player_is_running()) {
                        find_count = take_command_count() - 1;
                    } else if (!took_turn) {
                        cancel_command_count();
                    } else if (command_is_repeating()) {
                        consume_command_count();
                    }
                }
                // End of commands

            } while (!took_turn && !level_is_over() && !input_has_ended());
        } else {
            // if paralyzed, resting, or dead, flush output
            // but first move the cursor onto the player, for aesthetics
            move_cursor_relative(player_row(), player_col());
            put_qio();
        }

        // Teleport?
        if (teleport_is_pending()) {
            teleport(100);
        }

        // Move the creatures
        if (!level_is_over()) {
            creatures(true);
        }

        // Exit when this level is finished
    } while (!level_is_over() && !input_has_ended());
}

static char original_commands(char com_val) {
    int dir_val;

    switch (com_val) {
    case CTRL_KEY('K'): // ^K = exit
        com_val = 'Q';
        break;
    case CTRL_KEY('J'):
    case CTRL_KEY('M'):
        com_val = '+';
        break;
    case CTRL_KEY('P'): // ^P = repeat
    case CTRL_KEY('W'): // ^W = password
    case CTRL_KEY('X'): // ^X = save
    case CTRL_KEY('V'): // ^V = view license
    case ' ':
    case '!':
    case '$':
        break;
    case '.':
        if (get_dir(CNIL, &dir_val)) {
            com_val = direction_command_key(dir_val, DIR_KEY_RUN);
            if (com_val == 0) {
                com_val = ' ';
            }
        } else {
            com_val = ' ';
        }
        break;
    case '/':
    case '<':
    case '>':
    case '-':
    case '=':
    case '{':
    case '?':
    case 'A':
        break;
    case '1':
        com_val = 'b';
        break;
    case '2':
        com_val = 'j';
        break;
    case '3':
        com_val = 'n';
        break;
    case '4':
        com_val = 'h';
        break;
    case '5': // Rest one turn
        com_val = '.';
        break;
    case '6':
        com_val = 'l';
        break;
    case '7':
        com_val = 'y';
        break;
    case '8':
        com_val = 'k';
        break;
    case '9':
        com_val = 'u';
        break;
    case 'B':
        com_val = 'f';
        break;
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
        break;
    case 'L':
        com_val = 'W';
        break;
    case 'M':
        break;
    case 'R':
        break;
    case 'S':
        com_val = '#';
        break;
    case 'T':
        if (get_dir(CNIL, &dir_val)) {
            com_val = direction_command_key(dir_val, DIR_KEY_TUNNEL);
            if (com_val == 0) {
                com_val = ' ';
            }
        } else {
            com_val = ' ';
        }
        break;
    case 'V':
        break;
    case 'a':
        com_val = 'z';
        break;
    case 'b':
        com_val = 'P';
        break;
    case 'c':
    case 'd':
    case 'e':
        break;
    case 'f':
        com_val = 't';
        break;
    case 'h':
        com_val = '?';
        break;
    case 'i':
        break;
    case 'j':
        com_val = 'S';
        break;
    case 'l':
        com_val = 'x';
        break;
    case 'm':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
        break;
    case 't':
        com_val = 'T';
        break;
    case 'u':
        com_val = 'Z';
        break;
    case 'v':
    case 'w':
        break;
    case 'x':
        com_val = 'X';
        break;

    // wizard mode commands follow
    case CTRL_KEY('A'): // ^A = cure all
        break;
    case CTRL_KEY('B'): // ^B = objects
        com_val = CTRL_KEY('O');
        break;
    case CTRL_KEY('D'): // ^D = up/down
        break;
    case CTRL_KEY('H'): // ^H = wizhelp
        com_val = '\\';
        break;
    case CTRL_KEY('I'): // ^I = identify
        break;
    case CTRL_KEY('L'): // ^L = wizlight
        com_val = '*';
        break;
    case ':':
    case CTRL_KEY('T'): // ^T = teleport
    case CTRL_KEY('E'): // ^E = wizchar
    case CTRL_KEY('F'): // ^F = genocide
    case CTRL_KEY('G'): // ^G = treasure
    case '@':
    case '+':
        break;
    case CTRL_KEY('U'): // ^U = summon
        com_val = '&';
        break;
    default:
        com_val = '~'; // Anything illegal.
        break;
    }
    return com_val;
}

// Returns whether the command took a game turn.
static bool do_command(char com_val) {
    int dir_val;
    bool do_pickup, do_diplay_scores;
    int y, x, i;
    vtype out_val, tmp_str;

    // hack for move without pickup.  Map '-' to a movement command.
    if (com_val == '-') {
        do_pickup = false;
        i = hold_command_count();

        if (get_dir(CNIL, &dir_val)) {
            resume_command_count(i);
            com_val = direction_command_key(dir_val, DIR_KEY_WALK);
            if (com_val == 0) {
                com_val = '~';
            }
        } else {
            com_val = ' ';
        }
    } else {
        do_pickup = true;
    }

    switch (com_val) {
    case 'Q': // (Q)uit    (^K)ill
        flush();
        if (get_check("Do you really want to quit?")) {
            end_level();
            set_player_dead(true);
            (void)strcpy(death_cause(), "Quitting");
        }
        free_turn_flag = true;
        break;
    case CTRL_KEY('P'): // (^P)revious message.
        if (command_is_repeating()) {
            i = take_command_count();
            if (i > MAX_SAVE_MSG) {
                i = MAX_SAVE_MSG;
            }
        } else if (!previous_command_was(CTRL_KEY('P'))) {
            i = 1;
        } else {
            i = MAX_SAVE_MSG;
        }

        if (i > 1) {
            save_screen();
            x = i;

            // Oldest at the top, newest on the bottom row of the block: the
            // row number counts down as we walk back through the history.
            while (i > 0) {
                i--;
                prt(msg_history_recent(x - 1 - i), i, 0);
            }

            erase_line(x, 0);
            pause_line(x);
            restore_screen();
        } else {
            // Distinguish real and recovered messages with a '>'. -CJS-
            put_buffer(">", 0, 0);
            prt(msg_history_recent(0), 0, 1);
        }

        free_turn_flag = true;
        break;
    case CTRL_KEY('V'): // (^V)iew license
        helpfile(MORIA_GPL);
        free_turn_flag = true;
        break;
    case CTRL_KEY('W'): // (^W)izard mode
        if (progress_wizard_mode()) {
            progress_set_wizard_mode(false);
            msg_print("Wizard mode off.");
        } else if (enter_wiz_mode()) {
            msg_print("Wizard mode on.");
        }

        prt_winner();
        free_turn_flag = true;
        break;
    case CTRL_KEY('X'): // e(^X)it and save
        if (player_has_won()) {
            msg_print(
                "You are a Total Winner,  your character must be retired.");
            if (rogue_like_commands) {
                msg_print("Use 'Q' to when you are ready to quit.");
            } else {
                msg_print("Use <Control>-K when you are ready to quit.");
            }
        } else {
            (void)strcpy(death_cause(), "(saved)");
            msg_print("Saving game...");

            if (save_char()) {
                exit_game();
            }

            (void)strcpy(death_cause(), "(alive and well)");
        }

        free_turn_flag = true;
        break;
    case '=': // (=) set options
        save_screen();
        set_options();
        restore_screen();
        free_turn_flag = true;
        break;
    case '{': // ({) inscribe an object
        scribe_object();
        free_turn_flag = true;
        break;
    case '!': // (!) escape to the shell
    case '$':
        shell_out();
        free_turn_flag = true;
        break;
    case ESCAPE: // (ESC)   do nothing.
    case ' ':    // (space) do nothing.
        free_turn_flag = true;
        break;
    case 'b': // (b) down, left  (1)
        move_char(1, do_pickup);
        break;
    case 'j': // (j) down    (2)
        move_char(2, do_pickup);
        break;
    case 'n': // (n) down, right  (3)
        move_char(3, do_pickup);
        break;
    case 'h': // (h) left    (4)
        move_char(4, do_pickup);
        break;
    case 'l': // (l) right    (6)
        move_char(6, do_pickup);
        break;
    case 'y': // (y) up, left    (7)
        move_char(7, do_pickup);
        break;
    case 'k': // (k) up    (8)
        move_char(8, do_pickup);
        break;
    case 'u': // (u) up, right  (9)
        move_char(9, do_pickup);
        break;
    case 'B': // (B) run down, left  (. 1)
        find_init(1);
        break;
    case 'J': // (J) run down    (. 2)
        find_init(2);
        break;
    case 'N': // (N) run down, right  (. 3)
        find_init(3);
        break;
    case 'H': // (H) run left    (. 4)
        find_init(4);
        break;
    case 'L': // (L) run right  (. 6)
        find_init(6);
        break;
    case 'Y': // (Y) run up, left  (. 7)
        find_init(7);
        break;
    case 'K': // (K) run up    (. 8)
        find_init(8);
        break;
    case 'U': // (U) run up, right  (. 9)
        find_init(9);
        break;
    case '/': // (/) identify a symbol
        ident_char();
        free_turn_flag = true;
        break;
    case '.': // (.) stay in one place (5)
        move_char(5, do_pickup);
        if (command_count_remaining() > 1) {
            consume_command_count();
            if (!rest()) {
                free_turn_flag = true;
            }
        }
        break;
    case '<': // (<) go down a staircase
        go_up();
        break;
    case '>': // (>) go up a staircase
        go_down();
        break;
    case '?': // (?) help with commands
        if (rogue_like_commands) {
            helpfile(MORIA_HELP);
        } else {
            helpfile(MORIA_ORIG_HELP);
        }
        free_turn_flag = true;
        break;
    case 'f': // (f)orce    (B)ash
        bash();
        break;
    case 'C': // (C)haracter description
        save_screen();
        change_name();
        restore_screen();
        free_turn_flag = true;
        break;
    case 'D': // (D)isarm trap
        disarm_trap();
        break;
    case 'E': // (E)at food
        free_turn_flag = !eat();
        break;
    case 'F': // (F)ill lamp
        refill_lamp();
        break;
    case 'G': // (G)ain magic spells
        free_turn_flag = !gain_spells();
        break;
    case 'V': // (V)iew scores
        if (!previous_command_was('V')) {
            do_diplay_scores = true;
        } else {
            do_diplay_scores = false;
        }
        save_screen();
        display_scores(do_diplay_scores);
        restore_screen();
        free_turn_flag = true;
        break;
    case 'W': // (W)here are we on the map  (L)ocate on map
        if (player_timed_in_force(PLAYER_TIMED_BLINDNESS) || no_light()) {
            msg_print("You can't see your map.");
        } else {
            int cy, cx, p_y, p_x;

            y = player_row();
            x = player_col();
            if (get_panel(y, x, true)) {
                prt_map();
            }
            cy = panel_row_index();
            cx = panel_col_index();
            for (;;) {
                p_y = panel_row_index();
                p_x = panel_col_index();
                if (p_y == cy && p_x == cx) {
                    tmp_str[0] = '\0';
                } else {
                    (void)sprintf(tmp_str, "%s%s of", p_y < cy ? " North" : p_y > cy ? " South" : "", p_x < cx ? " West" : p_x > cx ? " East" : "");
                }
                (void)sprintf(out_val, "Map sector [%d,%d], which is%s your sector. Look which direction?", p_y, p_x, tmp_str);
                if (!get_dir(out_val, &dir_val)) {
                    break;
                }

                // -CJS-
                // Should really use the move function, but what the hell. This
                // is nicer, as it moves exactly to the same place in another
                // section. The direction calculation is not intuitive. Sorry.
                for (;;) {
                    x += ((dir_val - 1) % 3 - 1) * SCREEN_WIDTH / 2;
                    y -= ((dir_val - 1) / 3 - 1) * SCREEN_HEIGHT / 2;
                    // NOTE: y is compared against width. May be a bug (see dungeon_state.h).
                    if (x < 0 || y < 0 || x >= dungeon_width() || y >= dungeon_width()) {
                        msg_print("You've gone past the end of your map.");
                        x -= ((dir_val - 1) % 3 - 1) * SCREEN_WIDTH / 2;
                        y += ((dir_val - 1) / 3 - 1) * SCREEN_HEIGHT / 2;
                        break;
                    }
                    if (get_panel(y, x, true)) {
                        prt_map();
                        break;
                    }
                }
            }

            // Move to a new panel - but only if really necessary.
            if (get_panel(player_row(), player_col(), false)) {
                prt_map();
            }
        }
        free_turn_flag = true;
        break;
    case 'R': // (R)est a while
        free_turn_flag = !rest();
        break;
    case '#': // (#) search toggle  (S)earch toggle
        if (player_is_searching()) {
            search_off();
        } else {
            search_on();
        }
        free_turn_flag = true;
        break;
    case CTRL_KEY('B'): // (^B) tunnel down left  (T 1)
        free_turn_flag = !tunnel(1);
        break;
    case CTRL_KEY('M'): // cr must be treated same as lf.
    case CTRL_KEY('J'): // (^J) tunnel down    (T 2)
        free_turn_flag = !tunnel(2);
        break;
    case CTRL_KEY('N'): // (^N) tunnel down right  (T 3)
        free_turn_flag = !tunnel(3);
        break;
    case CTRL_KEY('H'): // (^H) tunnel left    (T 4)
        free_turn_flag = !tunnel(4);
        break;
    case CTRL_KEY('L'): // (^L) tunnel right    (T 6)
        free_turn_flag = !tunnel(6);
        break;
    case CTRL_KEY('Y'): // (^Y) tunnel up left    (T 7)
        free_turn_flag = !tunnel(7);
        break;
    case CTRL_KEY('K'): // (^K) tunnel up    (T 8)
        free_turn_flag = !tunnel(8);
        break;
    case CTRL_KEY('U'): // (^U) tunnel up right    (T 9)
        free_turn_flag = !tunnel(9);
        break;
    case 'z': // (z)ap a wand    (a)im a wand
        free_turn_flag = !aim();
        break;
    case 'M':
        screen_map();
        free_turn_flag = true;
        break;
    case 'P': // (P)eruse a book  (B)rowse in a book
        examine_book();
        free_turn_flag = true;
        break;
    case 'c': // (c)lose an object
        free_turn_flag = !closeobject();
        break;
    case 'd': // (d)rop something
        inven_command('d');
        break;
    case 'e': // (e)quipment list
        inven_command('e');
        break;
    case 't': // (t)hrow something  (f)ire something
        free_turn_flag = !throw_object();
        break;
    case 'i': // (i)nventory list
        inven_command('i');
        break;
    case 'S': // (S)pike a door  (j)am a door
        jamdoor();
        break;
    case 'x': // e(x)amine surrounds  (l)ook about
        look();
        free_turn_flag = true;
        break;
    case 'm': // (m)agic spells
        free_turn_flag = !cast();
        break;
    case 'o': // (o)pen something
        free_turn_flag = !openobject();
        break;
    case 'p': // (p)ray
        free_turn_flag = !pray();
        break;
    case 'q': // (q)uaff
        free_turn_flag = !quaff();
        break;
    case 'r': // (r)ead
        free_turn_flag = !read_scroll();
        break;
    case 's': // (s)earch for a turn
        // Search once with the player's skill; frequency is irrelevant for manual search.
        search(player_row(), player_col(), player_search_chance());
        break;
    case 'T': // (T)ake off something  (t)ake off
        inven_command('t');
        break;
    case 'Z': // (Z)ap a staff  (u)se a staff
        free_turn_flag = !use();
        break;
    case 'v': // (v)ersion of game
        helpfile(MORIA_VER);
        free_turn_flag = true;
        break;
    case 'w': // (w)ear or wield
        inven_command('w');
        break;
    case 'X': // e(X)change weapons  e(x)change
        inven_command('x');
        break;
    default:
        if (progress_wizard_mode()) {
            // Wizard commands are free moves
            free_turn_flag = true;
            wizard_command(com_val);
        } else {
            prt("Type '?' for help.", 0, 0);
            free_turn_flag = true;
        }
    }
    note_command(com_val);
    return !free_turn_flag;
}

// Check whether this command will accept a count. -CJS-
static bool valid_countcommand(char c) {
    switch (c) {
    case 'Q':
    case CTRL_KEY('W'):
    case CTRL_KEY('X'):
    case '=':
    case '{':
    case '/':
    case '<':
    case '>':
    case '?':
    case 'C':
    case 'E':
    case 'F':
    case 'G':
    case 'V':
    case '#':
    case 'z':
    case 'P':
    case 'c':
    case 'd':
    case 'e':
    case 't':
    case 'i':
    case 'x':
    case 'm':
    case 'p':
    case 'q':
    case 'r':
    case 'T':
    case 'Z':
    case 'v':
    case 'w':
    case 'W':
    case 'X':
    case CTRL_KEY('A'):
    case '\\':
    case CTRL_KEY('I'):
    case '*':
    case ':':
    case CTRL_KEY('T'):
    case CTRL_KEY('E'):
    case CTRL_KEY('F'):
    case CTRL_KEY('S'):
    case CTRL_KEY('Q'):
        return false;
    case CTRL_KEY('P'):
    case ESCAPE:
    case ' ':
    case '-':
    case 'b':
    case 'f':
    case 'j':
    case 'n':
    case 'h':
    case 'l':
    case 'y':
    case 'k':
    case 'u':
    case '.':
    case 'B':
    case 'J':
    case 'N':
    case 'H':
    case 'L':
    case 'Y':
    case 'K':
    case 'U':
    case 'D':
    case 'R':
    case CTRL_KEY('Y'):
    case CTRL_KEY('K'):
    case CTRL_KEY('U'):
    case CTRL_KEY('L'):
    case CTRL_KEY('N'):
    case CTRL_KEY('J'):
    case CTRL_KEY('B'):
    case CTRL_KEY('H'):
    case 'S':
    case 'o':
    case 's':
    case CTRL_KEY('D'):
    case CTRL_KEY('G'):
    case '+':
        return true;
    default:
        return false;
    }
}

// Regenerate hit points -RAK-
// All that is left here is working out the rate (above) and noticing that the
// whole-point part moved, so the status line needs redrawing. The fixed-point
// sum, the overflow guard, the carry and the stop at the top are all inside
// player_regenerate_hp().
static void regenhp(int percent) {
    int16_t old_chp = player_hp();

    player_regenerate_hp(percent);

    if (old_chp != player_hp()) {
        prt_chp();
    }
}

// Regenerate mana points -RAK-
// All that is left here is working out the rate (above) and noticing that the
// whole number moved, because the status line only shows that -- the sixteenths
// of a point live in player_mana.c.
static void regenmana(int percent) {
    int16_t old_cmana = player_mana();

    player_regenerate_mana(percent);

    if (old_cmana != player_mana()) {
        prt_cmana();
    }
}

// Is an item an enchanted weapon or armor and we don't know? -CJS-
// only returns true if it is a good enchantment
static bool enchanted(inven_type *t_ptr) {
    if (t_ptr->tval < TV_MIN_ENCHANT || t_ptr->tval > TV_MAX_ENCHANT || t_ptr->flags & TR_CURSED) {
        return false;
    }

    if (known2_p(t_ptr)) {
        return false;
    }
    if (item_noted_magical(t_ptr)) {
        return false;
    }
    if (t_ptr->tohit > 0 || t_ptr->todam > 0 || t_ptr->toac > 0) {
        return true;
    }
    if ((0x4000107fL & t_ptr->flags) && t_ptr->p1 > 0) {
        return true;
    }
    if (0x07ffe980L & t_ptr->flags) {
        return true;
    }

    return false;
}

// Examine a Book -RAK-
static void examine_book(void) {
    int i, k, item_val;

    if (!find_range(TV_MAGIC_BOOK, TV_PRAYER_BOOK, &i, &k)) {
        msg_print("You are not carrying any books.");
    } else if (player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
        msg_print("You can't see to read your spell book!");
    } else if (no_light()) {
        msg_print("You have no light to read by.");
    } else if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
        msg_print("You are too confused.");
    } else if (get_item(&item_val, "Which Book?", i, k, CNIL, CNIL)) {
        int spell_index[31];
        spell_type *s_ptr;

        bool flag = true;
        inven_type *i_ptr = inventory_at(item_val);

        if (player_class_spell_type() == MAGE) {
            if (i_ptr->tval != TV_MAGIC_BOOK) {
                flag = false;
            }
        } else if (player_class_spell_type() == PRIEST) {
            if (i_ptr->tval != TV_PRAYER_BOOK) {
                flag = false;
            }
        } else {
            flag = false;
        }

        if (!flag) {
            msg_print("You do not understand the language.");
        } else {
            i = 0;
            uint32_t j = inventory_at(item_val)->flags;

            while (j) {
                k = bit_pos(&j);
                s_ptr = &magic_spell[player_class() - 1][k];
                if (s_ptr->slevel < 99) {
                    spell_index[i] = k;
                    i++;
                }
            }

            save_screen();
            print_spells(spell_index, i, true, -1);
            pause_line(0);
            restore_screen();
        }
    }
}

// Go up one level -RAK-
static void go_up(void) {
    bool no_stairs = false;
    cave_type *c_ptr = square_at(player_row(), player_col());

    if (c_ptr->tptr != 0) {
        if (floor_item_at(c_ptr->tptr)->tval == TV_UP_STAIR) {
            leave_for_level(dungeon_level() - 1);
            msg_print("You enter a maze of up staircases.");
            msg_print("You pass through a one-way door.");
        } else {
            no_stairs = true;
        }
    } else {
        no_stairs = true;
    }

    if (no_stairs) {
        msg_print("I see no up staircase here.");
        free_turn_flag = true;
    }
}

// Go down one level -RAK-
static void go_down(void) {
    const uint8_t tptr = square_at(player_row(), player_col())->tptr;

    if (tptr != 0 && floor_item_at(tptr)->tval == TV_DOWN_STAIR) {
        leave_for_level(dungeon_level() + 1);
        msg_print("You enter a maze of down staircases.");
        msg_print("You pass through a one-way door.");
    } else {
        free_turn_flag = true;
        msg_print("I see no down staircase here.");
    }
}

// Jam a closed door -RAK-
static void jamdoor(void) {
    free_turn_flag = true;

    int y = player_row();
    int x = player_col();

    int dir;
    if (get_dir(CNIL, &dir)) {
        (void)mmove(dir, &y, &x);
        cave_type *c_ptr = square_at(y, x);

        if (c_ptr->tptr != 0) {
            inven_type *t_ptr = floor_item_at(c_ptr->tptr);
            if (t_ptr->tval == TV_CLOSED_DOOR) {
                if (c_ptr->cptr == 0) {
                    int i, j;

                    if (find_range(TV_SPIKE, TV_NEVER, &i, &j)) {
                        free_turn_flag = false;
                        count_msg_print("You jam the door with a spike.");

                        if (t_ptr->p1 > 0) {
                            // Make locked to stuck.
                            t_ptr->p1 = -t_ptr->p1;
                        }

                        // Successive spikes have a progressively smaller effect.
                        // Series is: 0 20 30 37 43 48 52 56 60 64 67 70 ...
                        t_ptr->p1 -= 1 + 190 / (10 - t_ptr->p1);

                        inven_type *i_ptr = inventory_at(i);
                        if (i_ptr->number > 1) {
                            i_ptr->number--;
                            inventory_set_weight(inventory_weight() - i_ptr->weight);
                        } else {
                            inven_destroy(i);
                        }
                    } else {
                        msg_print("But you have no spikes.");
                    }
                } else {
                    free_turn_flag = false;

                    char tmp_str[80];
                    (void)sprintf(tmp_str, "The %s is in your way!", monster_creature_get(monster_list_at(c_ptr->cptr)->creature)->name);
                    msg_print(tmp_str);
                }
            } else if (t_ptr->tval == TV_OPEN_DOOR) {
                msg_print("The door must be closed first.");
            } else {
                msg_print("That isn't a door!");
            }
        } else {
            msg_print("That isn't a door!");
        }
    }
}

// Refill the players lamp -RAK-
static void refill_lamp(void) {
    int i, j;

    free_turn_flag = true;

    int k = equipment_at(INVEN_LIGHT)->subval;

    if (k != 0) {
        msg_print("But you are not using a lamp.");
    } else if (!find_range(TV_FLASK, TV_NEVER, &i, &j)) {
        msg_print("You have no oil.");
    } else {
        free_turn_flag = false;

        inven_type *i_ptr = equipment_at(INVEN_LIGHT);
        i_ptr->p1 += inventory_at(i)->p1;
        if (i_ptr->p1 > OBJ_LAMP_MAX) {
            i_ptr->p1 = OBJ_LAMP_MAX;
            msg_print("Your lamp overflows, spilling oil on the ground.");
            msg_print("Your lamp is full.");
        } else if (i_ptr->p1 > OBJ_LAMP_MAX / 2) {
            msg_print("Your lamp is more than half full.");
        } else if (i_ptr->p1 == OBJ_LAMP_MAX / 2) {
            msg_print("Your lamp is half full.");
        } else {
            msg_print("Your lamp is less than half full.");
        }
        desc_remain(i);
        inven_destroy(i);
    }
}
