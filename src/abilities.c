// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Ratings of the player's miscellaneous abilities
//
// Extracted from the identical blocks in put_misc3() (misc3.c, screen) and
// file_character() (files.c, dumped file). The two were the same down to the
// comments; only the output differed (put_buffer versus fprintf), so a change
// to one made the screen and the file disagree.
//
// The eight numeric expressions look alike but differ in which field they are
// based on, which stat_adj() argument they pass and which class_level_adj
// column they index, so they are kept together here where they can be compared
// side by side.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "abilities.h"
#include "player_base_to_hit.h"
#include "player_disarm.h"
#include "player_infra_range.h"
#include "player_level.h"
#include "stats.h"

struct player_abilities calc_player_abilities(void) {
    struct misc *p_ptr = &py.misc;
    struct player_abilities a;

    // 素の命中力は窓口へ（#18-12-19B）。**この 2 行が税 9 本の出どころ** ——
    // このファイルを 9 つの recipe がリンクしている。足している残りは
    // この画面の問いのほう（武器の下駄と階級の段ごとの列）。
    a.bth = player_base_to_hit() + p_ptr->ptohit * BTH_PLUS_ADJ + (class_level_adj[p_ptr->pclass][CLA_BTH] * player_level());
    a.bthb = player_base_to_hit_with_bows() + p_ptr->ptohit * BTH_PLUS_ADJ + (class_level_adj[p_ptr->pclass][CLA_BTHB] * player_level());

    // 0 when fos >= 40; exceeds 29 when fos < 11 (search gear lowers fos, moria1.c:48)
    a.fos = 40 - p_ptr->fos;
    if (a.fos < 0) {
        a.fos = 0;
    }

    a.srh = p_ptr->srh;

    // stl + 1, so the minimum is 1 (not 0)
    a.stl = p_ptr->stl + 1;

    a.dis = player_disarm() + 2 * todis_adj() + stat_adj(A_INT) + (class_level_adj[p_ptr->pclass][CLA_DISARM] * player_level() / 3);
    a.save = p_ptr->save + stat_adj(A_WIS) + (class_level_adj[p_ptr->pclass][CLA_SAVE] * player_level() / 3);

    // Based on `save`, not `disarm`. Preserved as it stands; the intent is
    // unclear but changing it would change the game's behaviour.
    a.dev = p_ptr->save + stat_adj(A_INT) + (class_level_adj[p_ptr->pclass][CLA_DEVICE] * player_level() / 3);

    // The window answers in SQUARES; the ten is this screen's own doing, one
    // square being ten feet (player_infra_range.h). The multiplication stays
    // here because it is how the number is shown, not what it is.
    (void)sprintf(a.infra, "%d feet", player_infra_range() * 10);

    return a;
}
