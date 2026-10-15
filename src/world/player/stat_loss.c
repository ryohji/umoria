// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Losing a stat point to poison, bad food or a potion

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include "world_player.h"
#include "ui_screen.h"

#include "player_stats.h"

// Indexed by A_STR to A_CHR.
static const struct {
    const char *lost;
    const char *sustained;
} stat_loss_messages[6] = {
    {"You feel very sick.", "You feel sick for a moment,  it passes."},
    {"You become very dizzy.", "You become dizzy for a moment,  it passes."},
    {"You feel very naive.", "You feel naive for a moment,  it passes."},
    {"You feel very sore.", "You feel sore for a moment,  it passes."},
    {"You feel very sick.", "You feel sick for a moment,  it passes."},
    {"Your skin starts to itch.", "Your skin starts to itch, but feels better now."},
};

// Lose a stat point. -RAK-
void lose_stat(int stat) {
    if (!player_stat_sustained(stat)) {
        (void)dec_stat(stat);
        msg_print(stat_loss_messages[stat].lost);
    } else {
        msg_print(stat_loss_messages[stat].sustained);
    }
}
