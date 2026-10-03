// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Raising, lowering, restoring and boosting the player's stats, and working
// out the value in use from them

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "player_class.h"
#include "player_status_flags.h"
#include "stats.h"

static uint8_t modify_stat(int stat, int16_t amount) {
    uint8_t tmp_stat = player_stat_cur(stat);
    int loop = (amount < 0 ? -amount : amount);

    for (int i = 0; i < loop; i++) {
        if (amount > 0) {
            if (tmp_stat < 18) {
                tmp_stat++;
            } else if (tmp_stat < 108) {
                tmp_stat += 10;
            } else {
                tmp_stat = 118;
            }
        } else {
            if (tmp_stat > 27) {
                tmp_stat -= 10;
            } else if (tmp_stat > 18) {
                tmp_stat = 18;
            } else if (tmp_stat > 3) {
                tmp_stat--;
            }
        }
    }

    return tmp_stat;
}

// Set the value of the stat which is actually used. -CJS-
void set_use_stat(int stat) {
    player_stat_set_use(stat, modify_stat(stat, player_stat_mod(stat)));

    if (stat == A_STR) {
        player_request_strength_check();
        calc_bonuses();
    } else if (stat == A_DEX) {
        calc_bonuses();
    } else if (stat == A_INT && player_class_spell_type() == MAGE) {
        calc_spells(A_INT);
        calc_mana(A_INT);
    } else if (stat == A_WIS && player_class_spell_type() == PRIEST) {
        calc_spells(A_WIS);
        calc_mana(A_WIS);
    } else if (stat == A_CON) {
        calc_hitpoints();
    }
}

// Increases a stat by one randomized level -RAK-
bool inc_stat(int stat) {
    int tmp_stat = player_stat_cur(stat);
    if (tmp_stat < 118) {
        if (tmp_stat < 18) {
            tmp_stat++;
        } else if (tmp_stat < 116) {
            // stat increases by 1/6 to 1/3 of difference from max
            int gain = ((118 - tmp_stat) / 3 + 1) >> 1;
            tmp_stat += randint(gain) + gain;
        } else {
            tmp_stat++;
        }

        player_stat_set_cur(stat, tmp_stat);

        if (tmp_stat > player_stat_max(stat)) {
            player_stat_set_max(stat, tmp_stat);
        }
        set_use_stat(stat);
        prt_stat(stat);
        return true;
    } else {
        return false;
    }
}

// Decreases a stat by one randomized level -RAK-
bool dec_stat(int stat) {
    int tmp_stat = player_stat_cur(stat);
    if (tmp_stat > 3) {
        if (tmp_stat < 19) {
            tmp_stat--;
        } else if (tmp_stat < 117) {
            int loss = (((118 - tmp_stat) >> 1) + 1) >> 1;
            tmp_stat += -randint(loss) - loss;
            if (tmp_stat < 18) {
                tmp_stat = 18;
            }
        } else {
            tmp_stat--;
        }

        player_stat_set_cur(stat, tmp_stat);
        set_use_stat(stat);
        prt_stat(stat);
        return true;
    } else {
        return false;
    }
}

// Restore a stat.  Return true only if this actually makes a difference.
bool res_stat(int stat) {
    int i = player_stat_max(stat) - player_stat_cur(stat);

    if (i) {
        player_stat_set_cur(stat, player_stat_cur(stat) + i);
        set_use_stat(stat);
        prt_stat(stat);
        return true;
    }

    return false;
}

// Boost a stat artificially (by wearing something). If the display
// argument is true, then increase is shown on the screen.
void bst_stat(int stat, int amount) {
    player_stat_add_mod(stat, amount);

    set_use_stat(stat);

    // can not call prt_stat() here, may be in store, may be in inven_command
    player_request_stat_redraw(stat);
}
