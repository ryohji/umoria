// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Staff code

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include "world_monster.h"
#include "world_player.h"
#include "world_item.h"
#include "rnd.h"
#include "staffs.h"
#include "ui_screen.h"

#include "device.h"
#include "player_condition.h"

static void staff_effect(int pos, int dir, uint32_t *flags, bool *ident) {
    int k, y, x;

    switch (pos) {
    case 1:
        *ident = light_area(player_row(), player_col());
        break;
    case 2:
        *ident = detect_sdoor();
        break;
    case 3:
        *ident = detect_trap();
        break;
    case 4:
        *ident = detect_treasure();
        break;
    case 5:
        *ident = detect_object();
        break;
    case 6:
        teleport(100);
        *ident = true;
        break;
    case 7:
        *ident = true;
        earthquake();
        break;
    case 8:
        *ident = false;
        for (k = 0; k < randint(4); k++) {
            y = player_row();
            x = player_col();
            *ident |= summon_monster(&y, &x, false);
        }
        break;
    case 10:
        *ident = true;
        destroy_area(player_row(), player_col());
        break;
    case 11:
        *ident = true;
        starlite(player_row(), player_col());
        break;
    case 12:
        *ident = speed_monsters(1);
        break;
    case 13:
        *ident = speed_monsters(-1);
        break;
    case 14:
        *ident = sleep_monsters2();
        break;
    case 15:
        *ident = hp_player(randint(8));
        break;
    case 16:
        *ident = detect_invisible();
        break;
    case 17:
        if (!player_timed_in_force(PLAYER_TIMED_HASTE)) {
            *ident = true;
        }
        player_timed_add(PLAYER_TIMED_HASTE, randint(30) + 15);
        break;
    case 18:
        if (!player_timed_in_force(PLAYER_TIMED_SLOWNESS)) {
            *ident = true;
        }
        player_timed_add(PLAYER_TIMED_SLOWNESS, randint(30) + 15);
        break;
    case 19:
        *ident = mass_poly();
        break;
    case 20:
        if (remove_curse()) {
            if (!player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                msg_print("The staff glows blue for a moment..");
            }
            *ident = true;
        }
        break;
    case 21:
        *ident = detect_evil();
        break;
    case 22:
        if ((cure_blindness()) || (cure_poison()) ||
            (cure_confusion())) {
            *ident = true;
        }
        break;
    case 23:
        *ident = dispel_creature(CD_EVIL, 60);
        break;
    case 25:
        *ident = unlight_area(player_row(), player_col());
        break;
    case 32:
        // store bought flag
        break;
    default:
        msg_print("Internal error in staffs()");
        break;
    }
}

static const device_kind staff = {
    TV_STAFF,
    DEVICE_PENALTY_STAFF,
    "You are not carrying any staffs.",
    "Use which staff?",
    "You failed to use the staff properly.",
    "The staff has no charges left.",
    NULL,
    staff_effect,
};

// Use a staff. -RAK-
void use(void) {
    use_charged_device(&staff);
}
