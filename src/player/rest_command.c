// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Stopping what the player is doing: disturbances, search mode and resting

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "command_state.h"
#include "player_food.h"
#include "player_resting.h"
#include "player_status_flags.h"
#include "running.h"

// Something happens to disturb the player. -CJS-
// The first arg indicates a major disturbance, which affects search.
// The second arg indicates a light change.
void disturb(int s, int l) {
    cancel_command_count();
    if (s && player_is_searching()) {
        search_off();
    }
    if (player_resting()) {
        rest_off();
    }
    if (l || player_is_running()) {
        stop_running();
        check_view();
    }
    flush();
}

// Search Mode enhancement -RAK-
void search_on(void) {
    change_speed(1);
    player_start_searching();
    prt_state();
    prt_speed();
    player_adjust_digestion(1);
}

void search_off(void) {
    check_view();
    change_speed(-1);

    player_stop_searching();

    prt_state();
    prt_speed();
    player_adjust_digestion(-1);
}

// Resting allows a player to safely restore his hp -RAK-
void rest(void) {
    int rest_num;

    if (command_is_repeating()) {
        rest_num = take_command_count();
    } else {
        prt("Rest for how long? ", 0, 0);
        rest_num = 0;

        vtype rest_str;
        if (get_string(rest_str, 0, 19, 5)) {
            if (rest_str[0] == '*') {
                rest_num = -MAX_SHORT;
            } else {
                rest_num = atoi(rest_str);
            }
        }
    }
    // check for reasonable value, must be positive number
    // in range of a short, or must be -MAX_SHORT
    if ( (rest_num == -MAX_SHORT) || ((rest_num > 0) && (rest_num < MAX_SHORT)) ) {
        if (player_is_searching()) {
            search_off();
        }
        player_rest_set(rest_num);
        player_start_resting();
        prt_state();
        player_adjust_digestion(-1);
        prt("Press any key to stop resting...", 0, 0);
        put_qio();
    } else {
        if (rest_num != 0) {
            msg_print("Invalid rest count.");
        }
        erase_line(MSG_LINE, 0);
        free_turn_flag = true;
    }
}

void rest_off(void) {
    player_rest_stop();
    player_stop_resting();

    prt_state();

    // flush last message, or delete "press any key" message
    msg_print(CNIL);

    player_adjust_digestion(1);
}
