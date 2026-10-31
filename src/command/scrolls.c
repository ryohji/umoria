// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Scroll code

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include "dungeon_state.h"
#include "bits.h"
#include "world_item.h"
#include "world_dungeon.h"
#include "world_monster.h"
#include "world_player.h"
#include "rnd.h"
#include "commands.h"
#include "ui_screen.h"

#include "armor_selection.h"
#include "inventory.h"
#include "player_condition.h"

// Enchant Weapon To-Hit, on the wielded weapon if any. Returns whether the
// scroll is known now (ident as it was, or true).
static bool enchant_weapon_to_hit(inven_type *i_ptr, bool ident) {
    bigvtype tmp_str;
    msgtype out_val;

    if (i_ptr->tval != TV_NOTHING) {
        objdes(tmp_str, i_ptr, false);
        (void)snprintf(out_val, sizeof(out_val), "Your %s glows faintly!", tmp_str);
        msg_print(out_val);
        if (enchant(&i_ptr->tohit, 10)) {
            i_ptr->flags &= ~TR_CURSED;
            calc_bonuses();
        } else {
            msg_print("The enchantment fails.");
        }
        ident = true;
    }
    return ident;
}

// Enchant Weapon To-Dam, on the wielded weapon if any. Returns whether the
// scroll is known now (ident as it was, or true).
static bool enchant_weapon_to_dam(inven_type *i_ptr, bool ident) {
    bigvtype tmp_str;
    msgtype out_val;
    int j;

    if (i_ptr->tval != TV_NOTHING) {
        objdes(tmp_str, i_ptr, false);
        (void)snprintf(out_val, sizeof(out_val), "Your %s glows faintly!", tmp_str);
        msg_print(out_val);
        if ((i_ptr->tval >= TV_HAFTED) &&
            (i_ptr->tval <= TV_DIGGING)) {
            j = i_ptr->damage[0] * i_ptr->damage[1];
        } else {
            // Bows' and arrows' enchantments should not be
            // limited by their low base damages
            j = 10;
        }
        if (enchant(&i_ptr->todam, j)) {
            i_ptr->flags &= ~TR_CURSED;
            calc_bonuses();
        } else {
            msg_print("The enchantment fails.");
        }
        ident = true;
    }
    return ident;
}

// *Enchant Weapon*, on the wielded weapon if any. Returns whether the
// scroll is known now (ident as it was, or true).
static bool enchant_weapon_greatly(inven_type *i_ptr, bool ident) {
    bigvtype tmp_str;
    msgtype out_val;
    bool flag;
    int j, k;

    if (i_ptr->tval != TV_NOTHING) {
        objdes(tmp_str, i_ptr, false);
        (void)snprintf(out_val, sizeof(out_val), "Your %s glows brightly!", tmp_str);
        msg_print(out_val);
        flag = false;
        for (k = 0; k < randint(2); k++) {
            if (enchant(&i_ptr->tohit, 10)) {
                flag = true;
            }
        }
        if ((i_ptr->tval >= TV_HAFTED) && (i_ptr->tval <= TV_DIGGING)) {
            j = i_ptr->damage[0] * i_ptr->damage[1];
        } else {
          // Bows' and arrows' enchantments should not be limited
          // by their low base damages
          j = 10;
        }
        for (k = 0; k < randint(2); k++) {
            if (enchant(&i_ptr->todam, j)) {
                flag = true;
            }
        }
        if (flag) {
            i_ptr->flags &= ~TR_CURSED;
            calc_bonuses();
        } else {
            msg_print("The enchantment fails.");
        }
        ident = true;
    }
    return ident;
}

// Curse Weapon, on the wielded weapon if any. Returns whether the
// scroll is known now (ident as it was, or true).
static bool curse_weapon(inven_type *i_ptr, bool ident) {
    bigvtype tmp_str;
    msgtype out_val;

    if (i_ptr->tval != TV_NOTHING) {
        objdes(tmp_str, i_ptr, false);
        (void)snprintf(out_val, sizeof(out_val), "Your %s glows black, fades.", tmp_str);
        msg_print(out_val);
        unmagic_name(i_ptr);
        i_ptr->tohit = -randint(5) - randint(5);
        i_ptr->todam = -randint(5) - randint(5);
        i_ptr->toac = 0;
        // Must call py_bonuses() before set (clear) flags, and
        // must call calc_bonuses() after set (clear) flags, so that
        // all attributes will be properly turned off.
        py_bonuses(i_ptr, -1);
        i_ptr->flags = TR_CURSED;
        calc_bonuses();
        ident = true;
    }
    return ident;
}

// Enchant Armor, on the cursed or a random worn armour if any. Returns whether
// the scroll is known now (ident as it was, or true).
static bool enchant_armour(bool ident) {
    inven_type *i_ptr;
    bigvtype tmp_str;
    msgtype out_val;
    int k, l;

    l = pick_random_worn_armor();
    k = pick_first_cursed_armor();
    if (k != 0) {
        l = k;
    }

    if (l > 0) {
        i_ptr = equipment_at(l);
        objdes(tmp_str, i_ptr, false);
        (void)snprintf(out_val, sizeof(out_val), "Your %s glows faintly!", tmp_str);
        msg_print(out_val);
        if (enchant(&i_ptr->toac, 10)) {
            i_ptr->flags &= ~TR_CURSED;
            calc_bonuses();
        } else {
            msg_print("The enchantment fails.");
        }
        ident = true;
    }
    return ident;
}

// *Enchant Armor*, on the cursed or a random worn armour if any. Returns
// whether the scroll is known now (ident as it was, or true).
static bool enchant_armour_greatly(bool ident) {
    inven_type *i_ptr;
    bigvtype tmp_str;
    msgtype out_val;
    bool flag;
    int k, l;

    l = pick_random_worn_armor();
    k = pick_first_cursed_armor();
    if (k != 0) {
        l = k;
    }

    if (l > 0) {
        i_ptr = equipment_at(l);
        objdes(tmp_str, i_ptr, false);
        (void)snprintf(out_val, sizeof(out_val), "Your %s glows brightly!", tmp_str);
        msg_print(out_val);
        flag = false;
        for (k = 0; k < randint(2) + 1; k++) {
            if (enchant(&i_ptr->toac, 10)) {
                flag = true;
            }
        }
        if (flag) {
            i_ptr->flags &= ~TR_CURSED;
            calc_bonuses();
        } else {
            msg_print("The enchantment fails.");
        }
        ident = true;
    }
    return ident;
}

// Curse Armor, on a worn armour if any (the body armour most likely). Returns
// whether the scroll is known now (ident as it was, or true).
static bool curse_armour(bool ident) {
    inven_type *i_ptr;
    bigvtype tmp_str;
    msgtype out_val;
    int k;

    if ((equipment_at(INVEN_BODY)->tval != TV_NOTHING) && (randint(4) == 1)) {
        k = INVEN_BODY;
    } else if ((equipment_at(INVEN_ARM)->tval != TV_NOTHING) && (randint(3) == 1)) {
        k = INVEN_ARM;
    } else if ((equipment_at(INVEN_OUTER)->tval != TV_NOTHING) && (randint(3) == 1)) {
        k = INVEN_OUTER;
    } else if ((equipment_at(INVEN_HEAD)->tval != TV_NOTHING) && (randint(3) == 1)) {
        k = INVEN_HEAD;
    } else if ((equipment_at(INVEN_HANDS)->tval != TV_NOTHING) && (randint(3) == 1)) {
        k = INVEN_HANDS;
    } else if ((equipment_at(INVEN_FEET)->tval != TV_NOTHING) && (randint(3) == 1)) {
        k = INVEN_FEET;
    } else if (equipment_at(INVEN_BODY)->tval != TV_NOTHING) {
        k = INVEN_BODY;
    } else if (equipment_at(INVEN_ARM)->tval != TV_NOTHING) {
        k = INVEN_ARM;
    } else if (equipment_at(INVEN_OUTER)->tval != TV_NOTHING) {
        k = INVEN_OUTER;
    } else if (equipment_at(INVEN_HEAD)->tval != TV_NOTHING) {
        k = INVEN_HEAD;
    } else if (equipment_at(INVEN_HANDS)->tval != TV_NOTHING) {
        k = INVEN_HANDS;
    } else if (equipment_at(INVEN_FEET)->tval != TV_NOTHING) {
        k = INVEN_FEET;
    } else {
        k = 0;
    }

    if (k > 0) {
        i_ptr = equipment_at(k);
        objdes(tmp_str, i_ptr, false);
        (void)snprintf(out_val, sizeof(out_val), "Your %s glows black, fades.", tmp_str);
        msg_print(out_val);
        unmagic_name(i_ptr);
        i_ptr->flags = TR_CURSED;
        i_ptr->tohit = 0;
        i_ptr->todam = 0;
        i_ptr->toac = -randint(5) - randint(5);
        calc_bonuses();
        ident = true;
    }
    return ident;
}

// Whether item is the scroll that is being read: the same kind with the same
// flags.
static bool is_the_scroll(const inven_type *item, uint8_t tval, uint8_t subval, uint32_t flags) {
    return item->tval == tval && item->subval == subval && item->flags == flags;
}

// Scrolls for the reading -RAK-
// Returns whether it took a game turn.
bool read_scroll(void) {
    int j, k, y, x;
    int item_val;

    bool took_turn = false;

    if (player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
        msg_print("You can't see to read the scroll.");
    } else if (no_light()) {
        msg_print("You have no light to read by.");
    } else if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
        msg_print("You are too confused to read a scroll.");
    } else if (inventory_count() == 0) {
        msg_print("You are not carrying anything!");
    } else if (!find_range(TV_SCROLL1, TV_SCROLL2, &j, &k)) {
        msg_print("You are not carrying any scrolls!");
    } else if (get_item(&item_val, "Read which scroll?", j, k, CNIL, CNIL)) {
        took_turn = true;

        const inven_type *scroll = inventory_at(item_val);
        uint32_t i = scroll->flags;

        // What the scroll is, for finding it again after an identify has
        // moved it.
        const uint8_t scroll_tval = scroll->tval;
        const uint8_t scroll_subval = scroll->subval;
        const uint32_t scroll_flags = scroll->flags;

        bool used_up = true;
        bool ident = false;
        effect_outcome outcome;

        while (i != 0) {
            j = bit_pos(&i) + 1;
            if (scroll_tval == TV_SCROLL2) {
                j += 32;
            }

            // Scrolls.
            switch (j) {
            case 1:
                ident = enchant_weapon_to_hit(equipment_at(INVEN_WIELD), ident);
                break;
            case 2:
                ident = enchant_weapon_to_dam(equipment_at(INVEN_WIELD), ident);
                break;
            case 3:
                ident = enchant_armour(ident);
                break;
            case 4:
                msg_print("This is an identify scroll.");
                ident = true;
                outcome = ident_spell();
                used_up = outcome == EFFECT_DONE;
                if (outcome == EFFECT_CANCELLED) {
                    took_turn = false;
                }

                // The identify may merge objects, causing the identify scroll
                // to move to a different place.  Check for that here.  It can
                // move arbitrarily far if an identify scroll was used on
                // another identify scroll, but it always moves down.
                while (item_val > 0 && !is_the_scroll(inventory_at(item_val), scroll_tval, scroll_subval, scroll_flags)) {
                    item_val--;
                }
                break;
            case 5:
                if (remove_curse()) {
                    msg_print("You feel as if someone is watching over you.");
                    ident = true;
                }
                break;
            case 6:
                ident = light_area(player_row(), player_col());
                break;
            case 7:
                for (k = 0; k < randint(3); k++) {
                    y = player_row();
                    x = player_col();
                    ident |= summon_monster(&y, &x, false);
                }
                break;
            case 8:
                teleport(10);
                ident = true;
                break;
            case 9:
                teleport(100);
                ident = true;
                break;
            case 10: {
                int deeper = dungeon_level() + (-3) + 2 * randint(2);
                // The scroll may not push the player above the town; that is a
                // rule of this scroll, not of leaving a level.
                if (deeper < 1) {
                    deeper = 1;
                }
                leave_for_level(deeper);
                ident = true;
                break;
            }
            case 11:
                if (player_glowing_hands() == 0) {
                    msg_print("Your hands begin to glow.");
                    player_glowing_hands_begin();
                    ident = true;
                }
                break;
            case 12:
                ident = true;
                map_area();
                break;
            case 13:
                ident = sleep_monsters1(player_row(), player_col());
                break;
            case 14:
                ident = true;
                warding_glyph();
                break;
            case 15:
                ident = detect_treasure();
                break;
            case 16:
                ident = detect_object();
                break;
            case 17:
                ident = detect_trap();
                break;
            case 18:
                ident = detect_sdoor();
                break;
            case 19:
                msg_print("This is a mass genocide scroll.");
                (void)mass_genocide();
                ident = true;
                break;
            case 20:
                ident = detect_invisible();
                break;
            case 21:
                msg_print("There is a high pitched humming noise.");
                (void)aggravate_monster(20);
                ident = true;
                break;
            case 22:
                ident = trap_creation();
                break;
            case 23:
                ident = td_destroy();
                break;
            case 24:
                ident = door_creation();
                break;
            case 25:
                msg_print("This is a Recharge-Item scroll.");
                ident = true;
                outcome = recharge(60);
                used_up = outcome == EFFECT_DONE;
                if (outcome == EFFECT_CANCELLED) {
                    took_turn = false;
                }
                break;
            case 26:
                msg_print("This is a genocide scroll.");
                (void)genocide();
                ident = true;
                break;
            case 27:
                ident = unlight_area(player_row(), player_col());
                break;
            case 28:
                ident = protect_evil();
                break;
            case 29:
                ident = true;
                outcome = create_food();
                used_up = outcome == EFFECT_DONE;
                if (outcome == EFFECT_CANCELLED) {
                    took_turn = false;
                }
                break;
            case 30:
                ident = dispel_creature(CD_UNDEAD, 60);
                break;
            case 33:
                ident = enchant_weapon_greatly(equipment_at(INVEN_WIELD), ident);
                break;
            case 34:
                ident = curse_weapon(equipment_at(INVEN_WIELD), ident);
                break;
            case 35:
                ident = enchant_armour_greatly(ident);
                break;
            case 36:
                ident = curse_armour(ident);
                break;
            case 37:
                ident = false;
                for (k = 0; k < randint(3); k++) {
                    y = player_row();
                    x = player_col();
                    ident |= summon_undead(&y, &x);
                }
                break;
            case 38:
                ident = true;
                bless(randint(12) + 6);
                break;
            case 39:
                ident = true;
                bless(randint(24) + 12);
                break;
            case 40:
                ident = true;
                bless(randint(48) + 24);
                break;
            case 41:
                ident = true;
                if (!player_timed_in_force(PLAYER_TIMED_WORD_OF_RECALL)) {
                    player_timed_set(PLAYER_TIMED_WORD_OF_RECALL, 25 + randint(30));
                }
                msg_print("The air about you becomes charged.");
                break;
            case 42:
                destroy_area(player_row(), player_col());
                ident = true;
                break;
            default:
                msg_print("Internal error in scroll()");
                break;
            }
            // End of Scrolls.
        }

        // NOTE: the returned pointer is not used here, the scroll is not read
        // again after this. -MRC-
        (void)learn_item_effect(ident, &item_val);

        if (used_up) {
            desc_remain(item_val);
            inven_destroy(item_val);
        }
    }
    return took_turn;
}
