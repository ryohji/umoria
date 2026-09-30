// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Moving the player about the map: for now the teleport alone
//
// Moved out of misc3.c unchanged (#42); its prototype stays in externs.h.
// The walking (move_char() and carry() in moria3.c) is to join it later.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "dungeon_map.h"
#include "dungeon_size.h"
#include "pending_teleport.h"
#include "player_pos.h"

// Teleport the player to a new location -RAK-
void teleport(int dis) {
    int y, x;

    do {
        y = randint(dungeon_height()) - 1;
        x = randint(dungeon_width()) - 1;
        while (distance(y, x, player_row(), player_col()) > dis) {
            y += ((player_row() - y) / 2);
            x += ((player_col() - x) / 2);
        }
    } while ((square_at(y, x)->fval >= MIN_CLOSED_SPACE) || (square_at(y, x)->cptr >= 2));

    move_rec(player_row(), player_col(), y, x);

    for (int i = player_row() - 1; i <= player_row() + 1; i++) {
        for (int j = player_col() - 1; j <= player_col() + 1; j++) {
            square_at(i, j)->tl = false;
            lite_spot(i, j);
        }
    }

    lite_spot(player_row(), player_col());
    player_place(y, x);
    check_view();
    creatures(false);
    teleport_done();
}
