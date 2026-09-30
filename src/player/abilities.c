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

    // 素の命中力は窓口へ（#18-12-19B）。**この 2 行が税 9 本の出どころ** ——
    // このファイルを 9 つの recipe がリンクしている。足している残りは
    // この画面の問いのほう（階級の段ごとの列）。
    //
    // 武器の下駄も窓口へ（#18-12-24B）。**2 行が同じ数を読むので入口で 1 度に
    // 畳んだ** —— あいだに下駄を動かすものは無い（→ 所見 35 の 1 つめの形）。
    // **BTH_PLUS_ADJ（3）を掛けるのはこの画面の規則**で、窓口には入れない ——
    // 殴りと投げは下駄をそのまま足す（moria3.c:595・moria4.c:676）。
    const int to_hit_bonus = player_to_hit_bonus();
    a.bth = player_base_to_hit() + to_hit_bonus * BTH_PLUS_ADJ + (class_level_adj[player_class()][CLA_BTH] * player_level());
    a.bthb = player_base_to_hit_with_bows() + to_hit_bonus * BTH_PLUS_ADJ + (class_level_adj[player_class()][CLA_BTHB] * player_level());

    // 探索の腕と頻度も窓口へ（#18-12-25B）。**逆さにするのと 0 で留めるのは
    // この画面の規則**で、窓口には入れない —— 自動探索は頻度をそのまま
    // randint() に渡す（moria3.c:729）。0 when the frequency is >= 40; exceeds 29
    // when it is < 11 (search gear lowers it, player_bonuses.c)
    a.fos = 40 - player_search_frequency();
    if (a.fos < 0) {
        a.fos = 0;
    }

    // 腕のほうは補正が 1 つも乗らない（下駄に 3 を掛ける命中力とは違う）。
    a.srh = player_search_chance();

    // 足音の静かさも窓口へ（#18-12-27B）。**+1 はこの画面の規則**で、窓口には
    // 入れない —— likert() の除数が 1 なので、この 1 つで語が 1 段動く
    // （stl + 1, so the minimum is 1 (not 0)）。
    a.stl = player_stealth() + 1;

    a.dis = player_disarm() + 2 * todis_adj() + stat_adj(A_INT) + (class_level_adj[player_class()][CLA_DISARM] * player_level() / 3);
    // 抵抗は窓口へ（#18-12-21B）。**同じ 1 本が下の道具の腕にも答える** ——
    // 足すものが違うだけ（A_WIS と CLA_SAVE ／ A_INT と CLA_DEVICE）。
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
