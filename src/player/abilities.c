// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Ratings of the player's miscellaneous abilities
//
// Extracted from the identical blocks in put_misc3() (char_screen.c, screen) and
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
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_class.h"
#include "player_disarm.h"
#include "player_infra_range.h"
#include "player_level.h"
#include "player_saving_throw.h"
#include "player_search_skill.h"
#include "player_stealth.h"
#include "stats.h"

struct player_abilities calc_player_abilities(void) {
    struct player_abilities a;

    // **Base to-hit is from the window.** Nine recipes link this file. What is
    // added here is the per-level column from the class table.
    //
    // **Equipment bonus folded once at entry** — nothing between the two lines
    // moves the bonus (observation 35, first form). **Multiplying by BTH_PLUS_ADJ
    // (3) is this screen's rule**, not the window's — melee and throwing add
    // the bonus as-is (player_melee.c, throw.c).
    const int to_hit_bonus = player_to_hit_bonus();
    a.bth = player_base_to_hit() + to_hit_bonus * BTH_PLUS_ADJ + (class_level_adj[player_class()][CLA_BTH] * player_level());
    a.bthb = player_base_to_hit_with_bows() + to_hit_bonus * BTH_PLUS_ADJ + (class_level_adj[player_class()][CLA_BTHB] * player_level());

    // **Inverting and clamping to 0 are this screen's rule**, not the window's —
    // auto-search passes frequency as-is to randint() (player_move.c). 0 when the
    // frequency is >= 40; exceeds 29 when it is < 11 (search gear lowers it,
    // player_bonuses.c)
    a.fos = 40 - player_search_frequency();
    if (a.fos < 0) {
        a.fos = 0;
    }

    // Search chance has no modifiers applied (unlike to-hit, which multiplies
    // the bonus by 3).
    a.srh = player_search_chance();

    // **The +1 is this screen's rule**, not the window's — likert() divides by 1,
    // so this single increment moves the word one step (stl + 1, so the minimum
    // is 1 (not 0)).
    a.stl = player_stealth() + 1;

    a.dis = player_disarm() + 2 * todis_adj() + stat_adj(A_INT) + (class_level_adj[player_class()][CLA_DISARM] * player_level() / 3);
    // **The same window answers both saving throw and device skill** — only what
    // is added differs (A_WIS and CLA_SAVE vs. A_INT and CLA_DEVICE).
    a.save = player_saving_throw() + stat_adj(A_WIS) + (class_level_adj[player_class()][CLA_SAVE] * player_level() / 3);

    // Based on the SAVING THROW, not `disarm`. Preserved as it stands; the
    // intent is unclear but changing it would change the game's behaviour.
    a.dev = player_saving_throw() + stat_adj(A_INT) + (class_level_adj[player_class()][CLA_DEVICE] * player_level() / 3);

    // The window answers in SQUARES; the ten is this screen's own doing, one
    // square being ten feet (player_infra_range.h). The multiplication stays
    // here because it is how the number is shown, not what it is.
    (void)sprintf(a.infra, "%d feet", player_infra_range() * 10);

    return a;
}
