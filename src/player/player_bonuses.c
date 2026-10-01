// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The player's bonuses from worn equipment, and changes of speed

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "burden.h"
#include "equipment.h"
#include "monster_list.h"
#include "player_abilities.h"
#include "player_armour_class.h"
#include "player_attack_bonuses.h"
#include "player_display_numbers.h"
#include "player_food.h"
#include "player_infra_range.h"
#include "player_search_skill.h"
#include "player_speed.h"
#include "player_status_flags.h"
#include "player_stealth.h"
#include "player_timed_effects.h"
#include "stats.h"

// Changes speed of monsters relative to player -RAK-
// Note: When the player is sped up or slowed down, I simply change
// the speed of all the monsters. This greatly simplified the logic.
void change_speed(int num) {
    player_speed_adjust(num);
    player_request_speed_redraw();

    for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
        monster_list_at(i)->cspeed += num;
    }
}

// Player bonuses -RAK-
//
// When an item is worn or taken off, this re-adjusts the player bonuses.
//     Factor =  1 : wear
//     Factor = -1 : removed
//
// Only calculates properties with cumulative effect.  Properties that
// depend on everything being worn are recalculated by calc_bonuses() -CJS-
void py_bonuses(inven_type *t_ptr, int factor) {
    int amount = t_ptr->p1 * factor;
    if (t_ptr->flags & TR_STATS) {
        for (int i = 0; i < 6; i++) {
            if ((1 << i) & t_ptr->flags) {
                bst_stat(i, amount);
            }
        }
    }
    if (TR_SEARCH & t_ptr->flags) {
        // Opposite signs on purpose: search gear raises the skill and lowers
        // the frequency number (the n of 1/n), so the player finds more and
        // looks more often.
        // `amount` is already signed by the caller's factor, so this one line
        // covers putting the item on and taking it off again.
        player_search_skill_adjust(amount, -amount);
    }
    if (TR_STEALTH & t_ptr->flags) {
        // `amount` is already signed by the caller's factor, so this one line
        // covers putting the item on and taking it off again. No item has a
        // negative stealth p1; the noisy curse is a separate flag, TR_AGGRAVATE.
        player_stealth_adjust(amount);
    }
    if (TR_SPEED & t_ptr->flags) {
        change_speed(-amount);
    }
    if ((TR_BLIND & t_ptr->flags) && (factor > 0)) {
        player_timed_add(PLAYER_TIMED_BLINDNESS, 1000);
    }
    if ((TR_TIMID & t_ptr->flags) && (factor > 0)) {
        player_timed_add(PLAYER_TIMED_FEAR, 50);
    }
    if (TR_INFRA & t_ptr->flags) {
        // `amount` is already signed by the caller's factor, so this one line
        // covers putting the item on and taking it off again.
        player_infra_range_adjust(amount);
    }
}

// Recalculate the effect of all the stuff we use. -CJS-
void calc_bonuses(void) {
    // What the old answers were doing to the digestion has to be taken back
    // before they are forgotten -- that is why the asking comes first and why
    // forgetting and deriving are two steps with work in between.
    if (player_has_slow_digestion()) {
        player_adjust_digestion(1);
    }
    if (player_regenerates()) {
        player_adjust_digestion(-3);
    }

    player_abilities_forget_all();

    int old_dis_ac = player_display_ac();

    // Real To Hit / Real To Dam: reset to the base values first. The loop
    // below adds each item's plusses, so without this reset the previous
    // total would be counted twice.
    player_attack_bonuses_set(tohit_adj(), todam_adj());
    player_armour_class_reset(toac_adj()); // Real AC: nothing worn yet

    // What the sheet says starts out as a copy of the real plusses
    player_display_start_from_real((int16_t)player_to_hit_bonus(), (int16_t)player_to_damage_bonus(), (int16_t)player_armour_class_magical());

    for (int i = equipment_first_slot(); i < INVEN_LIGHT; i++) {
        inven_type *i_ptr = equipment_at(i);
        if (i_ptr->tval != TV_NOTHING) {
            // To-hit and to-dam are added separately only because of the bow
            // check below, which is about the item and so stays here.
            player_to_hit_bonus_adjust(i_ptr->tohit);

            // Bows can't damage. -CJS-
            if (i_ptr->tval != TV_BOW) {
                player_to_damage_bonus_adjust(i_ptr->todam);
            }

            player_armour_class_add_item(i_ptr->ac, i_ptr->toac);
            if (known2_p(i_ptr)) {
                player_display_add_to_hit(i_ptr->tohit);
                if (i_ptr->tval != TV_BOW) {
                    // Bows can't damage. -CJS-
                    player_display_add_to_dam(i_ptr->todam);
                }
                player_display_add_to_ac(i_ptr->toac);
                player_display_add_ac(i_ptr->ac);
            } else if (!(TR_CURSED & i_ptr->flags)) {
                // Base AC values should always be visible,
                // as long as the item is not cursed.
                player_display_add_ac(i_ptr->ac);
            }
        }
    }
    player_display_fold_to_ac();

    if (weapon_is_too_heavy()) {
        player_display_add_to_hit(py.stats.use_stat[A_STR] * WEAPON_WEIGHT_PER_STR - equipment_at(INVEN_WIELD)->weight);
    }

    // Add in temporary spell increases
    if (player_timed_in_force(PLAYER_TIMED_INVULNERABILITY)) {
        player_armour_class_adjust(100);
        player_display_add_ac(100);
    }
    if (player_timed_in_force(PLAYER_TIMED_BLESSING)) {
        player_armour_class_adjust(2);
        player_display_add_ac(2);
    }
    if (player_timed_in_force(PLAYER_TIMED_SEEING_INVISIBLE)) {
        player_grant_see_invisible();
    }

    // can't print AC here because might be in a store
    if (old_dis_ac != player_display_ac()) {
        player_request_armor_redraw();
    }

    inven_type *i_ptr;

    uint32_t item_flags = 0;
    i_ptr = equipment_at(equipment_first_slot());
    for (int i = equipment_first_slot(); i < INVEN_LIGHT; i++) {
        item_flags |= i_ptr->flags;
        i_ptr++;
    }

    // Which flag grants which ability is player_abilities.c's business; this
    // function only says what is being worn.
    player_abilities_note_item_flags(item_flags);

    // A sustain cannot be read off the flags of everything at once: WHICH stat
    // an item keeps is in that item's own p1, so the slots are walked again and
    // each item says its own. The numbering of the six stays in the module.
    i_ptr = equipment_at(equipment_first_slot());
    for (int i = equipment_first_slot(); i < INVEN_LIGHT; i++) {
        if (TR_SUST_STAT & i_ptr->flags) {
            player_abilities_note_sustain(i_ptr->p1);
        }
        i_ptr++;
    }

    if (player_has_slow_digestion()) {
        player_adjust_digestion(-1);
    }
    if (player_regenerates()) {
        player_adjust_digestion(3);
    }
}
