// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Using a charged device (a staff or a wand)

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "device.h"
#include "device_use.h"
#include "inventory.h"
#include "item_learn.h"
#include "player_class.h"
#include "player_level.h"
#include "player_skills.h"
#include "player_condition.h"
#include "stats.h"

void use_charged_device(const device_kind *kind) {
    int j, k, item_val;

    free_turn_flag = true;

    if (inventory_count() == 0) {
        msg_print("But you are not carrying anything.");
    } else if (!find_range(kind->tval, TV_NEVER, &j, &k)) {
        msg_print(kind->none_carried);
    } else if (get_item(&item_val, kind->prompt, j, k, CNIL, CNIL)) {
        free_turn_flag = false;

        inven_type *i_ptr = inventory_at(item_val);

        int dir = 0;
        if (kind->choose_direction != NULL && !kind->choose_direction(&dir)) {
            return;
        }

        // Device skill base is the same as saving throw (adds A_INT).
        int chance = device_use_chance(player_saving_throw(), stat_adj(A_INT), (int)i_ptr->level, kind->penalty, class_level_adj[player_class()][CLA_DEVICE], player_level(), player_timed_turns(PLAYER_TIMED_CONFUSION));

        if (!device_use_succeeds(chance)) {
            msg_print(kind->failed);
        } else if (i_ptr->p1 > 0) {
            bool ident = false;

            uint32_t flags = i_ptr->flags;
            (i_ptr->p1)--;

            while (flags != 0) {
                int pos = bit_pos(&flags) + 1;
                kind->effect(pos, dir, &flags, &ident);
            }

            // NOTE: the returned i_ptr is never read after this, so it is
            // discarded. -MRC-
            (void)learn_item_effect(ident, &item_val);

            desc_charges(item_val);
        } else {
            msg_print(kind->empty);
            if (!known2_p(i_ptr)) {
                add_inscribe(i_ptr, ID_EMPTY);
            }
        }
    }
}
