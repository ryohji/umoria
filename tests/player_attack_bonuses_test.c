// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「命中と打撃の下駄」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 23 つめの問いで、**`struct misc` から出る 9 つめ**
 * （どこまで潜ったか・体力の骰子・守りの点数・素の命中力・罠と鍵をはずす腕・
 * 抵抗・どの種族か・体の重さにつづく）。答えは **2 つの short** —— もとは
 * py.misc.ptohit と py.misc.ptodam で、6 ファイルから 21 か所が名ざしていた
 * （19 行）。**#18-12-24C から src/player/player_attack_bonuses.c の static 2 つ**で、
 * このテストは足場を 1 つも持たない（人物の器が要らないので
 * player_attack_bonuses_fixture.c は C で消えた）。
 *
 * **2 つのフィールドだが答えも 2 つ**（18 つめの `pac` ＋ `ptoac` は 26 か所
 * ぜんぶが和だったので答えは 1 つ）。**和を使う読み手が 1 人もいない** ——
 * 読み手はいつも「狙っているのか」「もう当たったのか」を知っている。
 * それでも **module は 1 つ** —— 置く側が 4 か所とも 2 行ずつで対になって
 * いるから（19 つめの `bth` ＋ `bthb` と同じ形 → 台帳の所見 40）。
 *
 * **数は負がふつう** —— もとは src/player/stats.c の補正表（DEX と STR）で、
 * 弱い人物は −3 から始まり、呪われた武器はさらに引く。
 *
 * **窓口は 5 本**（下調べは 6 本と見こんだ）——
 * 読み 2・**対で置く 1**・足す 2。**置くのが 1 本になったのは、
 * 4 か所の書き手がどれも 2 行で対になっているから**（19 つめと同じ判断）。
 * **足すのが 2 本なのは、装備の輪が弓のときだけ打撃を飛ばすから** ——
 * 対では呼べない。
 *
 * 外に残すものは 4 つ（player_attack_bonuses.h に書いてある）——
 * 数のもと（補正表と装備）・人物画面の等級（BTH_PLUS_ADJ が掛かる）・
 * 画面に見せる双子（`player_display_numbers.c`）・作りなおす時機
 * （`calc_bonuses()`）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に対を置きなおす。
 */
/* externs.h は要らない。窓口 5 本と、型と BTH_PLUS_ADJ のための 2 つだけ。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_attack_bonuses.h"

#include "minunit.h"

/* 創成で下駄が決まったところから始める（create.c:397 と同じ形。
 * 引数は命中・打撃の順で、セーブの並びも、消えたフィールドの並びもこの順）。 */
static void given_bonuses_of(int to_hit, int to_damage) { player_attack_bonuses_set(to_hit, to_damage); }

/* ------------------------------------------------------------------
 * 2 つの答え -- 別々に読め、互いに移らない
 * ------------------------------------------------------------------ */

TEST(the_aim_is_whatever_creation_worked_out) {
    given_bonuses_of(2, 3); /* DEX と STR が 18 あたりの人物 */

    ASSERT_EQ_INT(2, player_to_hit_bonus());
}

TEST(the_force_is_a_different_number_from_the_aim) {
    given_bonuses_of(2, 3);

    ASSERT_EQ_INT(3, player_to_damage_bonus());
}

TEST(reading_either_number_twice_gives_the_same_answer) {
    given_bonuses_of(5, 6);

    ASSERT_EQ_INT(5, player_to_hit_bonus());
    ASSERT_EQ_INT(5, player_to_hit_bonus());
    ASSERT_EQ_INT(6, player_to_damage_bonus());
    ASSERT_EQ_INT(6, player_to_damage_bonus());
}

/* **負がふつう** —— DEX 3 は命中に −3、STR 3 は打撃に −2（src/player/stats.c:99・
 * :136 の表の 1 段め）。ここを留めると遊びが変わる（→ 所見 24）。 */
TEST(a_weak_and_clumsy_character_has_negative_bonuses) {
    given_bonuses_of(-6, -2); /* DEX 3 と STR 3 で命中は −3 ＋ −3 */

    ASSERT_EQ_INT(-6, player_to_hit_bonus());
    ASSERT_EQ_INT(-2, player_to_damage_bonus());
}

/* 0 は「まだ決まっていない」でも「能力値がふつう」でもある ——
 * DEX 8〜15 と STR 5〜15 はどちらも 0 段。 */
TEST(zero_is_both_the_starting_state_and_an_ordinary_answer) {
    given_bonuses_of(0, 0);

    ASSERT_EQ_INT(0, player_to_hit_bonus());
    ASSERT_EQ_INT(0, player_to_damage_bonus());
}

/* ------------------------------------------------------------------
 * 対で置く 1 本 -- 4 か所の書き手はどれも 2 行で対になっている
 * ------------------------------------------------------------------ */

/* create.c:122（仮）→ :397（本物）は同じ 1 本で、**あとに置いたほうが残る**。
 * 足すのではない。 */
TEST(setting_the_pair_again_replaces_rather_than_adds) {
    given_bonuses_of(2, 3);
    given_bonuses_of(5, 1);

    ASSERT_EQ_INT(5, player_to_hit_bonus());
    ASSERT_EQ_INT(1, player_to_damage_bonus());
}

/* 片方だけ動かす窓口は無い（書き手が 1 つも無いから）。
 * **対で置くと、もう片方も必ず置きかわる** —— moria1.c:117 が
 * 装備を数えなおす前に両方を素の値へ戻すのがこの形。 */
TEST(the_pair_moves_together_because_no_caller_ever_set_one_alone) {
    given_bonuses_of(9, 9);
    given_bonuses_of(0, 0); /* 装備を数えなおす前の素の値 */

    ASSERT_EQ_INT(0, player_to_hit_bonus());
    ASSERT_EQ_INT(0, player_to_damage_bonus());
}

/* 引数の順は「命中・打撃」で、**セーブファイルの 2 つの short の順と同じ**
 * （save.c:177 が命中、:178 が打撃）。入れちがえたらここで落ちる。 */
TEST(the_aim_comes_first_the_way_the_saved_file_holds_it) {
    given_bonuses_of(1, 7);

    ASSERT_EQ_INT(1, player_to_hit_bonus());
    ASSERT_EQ_INT(7, player_to_damage_bonus());
}

/* 読みもどしは器の番地に読んでいた（`rd_short((uint16_t *)&m_ptr->…)`）。
 * B で局所の `uint16_t` 2 つに受けてから対で置く形になった
 * （save.c:672〜:676）。**置きなおす窓口は創成と同じ 1 本** —— 15 つめが
 * 立てた問いへの 8 度めの答えで、守りの点数を除く 6 つと同じ側。 */
TEST(loading_a_saved_game_uses_the_very_same_window) {
    given_bonuses_of(0, 0);

    int16_t to_hit_from_the_file = 4;
    int16_t to_damage_from_the_file = -1;
    player_attack_bonuses_set(to_hit_from_the_file, to_damage_from_the_file);

    ASSERT_EQ_INT(4, player_to_hit_bonus());
    ASSERT_EQ_INT(-1, player_to_damage_bonus());
}

/* ------------------------------------------------------------------
 * 足す 2 本 -- 装備の輪が 1 つずつ積む
 * ------------------------------------------------------------------ */

/* moria1.c:128 —— 身につけているものの `tohit` を 1 つずつ足す。 */
TEST(one_piece_of_equipment_adds_to_the_aim) {
    given_bonuses_of(2, 3);

    player_to_hit_bonus_adjust(4); /* +4 の武器 */

    ASSERT_EQ_INT(6, player_to_hit_bonus());
}

TEST(the_equipment_loop_keeps_adding_because_it_walks_every_slot) {
    given_bonuses_of(0, 0);

    player_to_hit_bonus_adjust(1);
    player_to_hit_bonus_adjust(2);
    player_to_hit_bonus_adjust(3);

    ASSERT_EQ_INT(6, player_to_hit_bonus());
}

/* 呪われた武器は引く。**留めは無いので負に落ちる**。 */
TEST(a_cursed_item_takes_away_from_the_aim) {
    given_bonuses_of(1, 1);

    player_to_hit_bonus_adjust(-5);

    ASSERT_EQ_INT(-4, player_to_hit_bonus());
}

TEST(one_piece_of_equipment_adds_to_the_force) {
    given_bonuses_of(2, 3);

    player_to_damage_bonus_adjust(2);

    ASSERT_EQ_INT(5, player_to_damage_bonus());
}

/* **足すのが 2 本に分かれている理由をここで固定する。** 弓は打撃に足さない
 * （"Bows can't damage. -CJS-" moria1.c:131）—— その `if` は呼び手に残るので、
 * **命中だけ足しても打撃は動かない**。 */
TEST(a_bow_adds_to_the_aim_only_and_the_force_stays_where_it_was) {
    given_bonuses_of(0, 0);

    player_to_hit_bonus_adjust(3); /* 弓の tohit */
    /* 打撃の窓口は呼ばない（呼び手の if が飛ばす） */

    ASSERT_EQ_INT(3, player_to_hit_bonus());
    ASSERT_EQ_INT(0, player_to_damage_bonus());
}

TEST(adjusting_the_force_leaves_the_aim_alone_as_well) {
    given_bonuses_of(0, 0);

    player_to_damage_bonus_adjust(2);

    ASSERT_EQ_INT(0, player_to_hit_bonus());
    ASSERT_EQ_INT(2, player_to_damage_bonus());
}

/* 足したあとで対を置くと、積んだものは消える —— moria1.c が装備を
 * 数えなおすたびにこれをしている（:117 で置き、:128 から積む）。 */
TEST(setting_the_pair_wipes_whatever_the_equipment_had_added) {
    given_bonuses_of(0, 0);
    player_to_hit_bonus_adjust(7);
    player_to_damage_bonus_adjust(7);

    given_bonuses_of(2, 3); /* 素の値へ戻す */

    ASSERT_EQ_INT(2, player_to_hit_bonus());
    ASSERT_EQ_INT(3, player_to_damage_bonus());
}

/* ------------------------------------------------------------------
 * 呼び手の計算 -- 窓口は掛けも割りもしない
 * ------------------------------------------------------------------ */

/* 人物画面だけが命中の下駄を **BTH_PLUS_ADJ（3）倍**する
 * （abilities.c:47・:48。B で入口の 1 度読みに畳んだ）。**掛けるのは呼び手の仕事** —— 殴りと投げは
 * そのまま足す（moria3.c:598・throw.c）。 */
TEST(the_character_sheet_multiplies_the_aim_by_three_but_the_window_does_not) {
    given_bonuses_of(4, 0);

    ASSERT_EQ_INT(4, player_to_hit_bonus());
    ASSERT_EQ_INT(12, player_to_hit_bonus() * BTH_PLUS_ADJ); /* 画面の側 */
    ASSERT_EQ_INT(24 + 12, 24 + player_to_hit_bonus() * BTH_PLUS_ADJ);
}

/* 殴ったときの打撃は `k += 打撃の下駄`（moria3.c:628）—— そのまま足す。
 * 0 未満に落ちたら 0 に上げるのは呼び手の規則（:629）。 */
TEST(a_blow_adds_the_force_whole_and_the_floor_is_the_callers_rule) {
    given_bonuses_of(0, -3);

    int damage = 2 + player_to_damage_bonus();
    ASSERT_EQ_INT(-1, damage); /* 窓口は何も直さない */
    if (damage < 0) {
        damage = 0; /* moria3.c:629 の側 */
    }
    ASSERT_EQ_INT(0, damage);
}

/* ------------------------------------------------------------------
 * 幅と留め -- 窓口は何も断らない（所見 24）
 * ------------------------------------------------------------------ */

/* もとのフィールドは何も留めていなかったので、窓口も留めない。
 * 人の下駄として有りえない数でも通る。 */
TEST(the_windows_refuse_nothing_the_fields_refused_nothing) {
    given_bonuses_of(1000, -1000);

    ASSERT_EQ_INT(1000, player_to_hit_bonus());
    ASSERT_EQ_INT(-1000, player_to_damage_bonus());
}

/* 置き場は 2 バイトの**符号つき**のまま。`p_ptr->misc.ptohit = tohit_adj()`
 * が切り落としていたのと同じところで折りかえす。 */
TEST(the_store_is_a_signed_short_so_thirty_two_thousand_seven_hundred_sixty_eight_wraps) {
    given_bonuses_of(32768, -32769);

    ASSERT_EQ_INT(-32768, player_to_hit_bonus());
    ASSERT_EQ_INT(32767, player_to_damage_bonus());
}

TEST(the_widest_numbers_a_signed_short_holds_are_kept) {
    given_bonuses_of(32767, -32768);

    ASSERT_EQ_INT(32767, player_to_hit_bonus());
    ASSERT_EQ_INT(-32768, player_to_damage_bonus());
}

/* 足す窓口も同じ幅で折りかえす（`+=` が int16_t に書きもどしていたのと同じ）。 */
TEST(adjusting_past_the_top_wraps_the_way_the_field_always_did) {
    given_bonuses_of(32767, 0);

    player_to_hit_bonus_adjust(1);

    ASSERT_EQ_INT(-32768, player_to_hit_bonus());
}

int main(void) {
    RUN_TEST(the_aim_is_whatever_creation_worked_out);
    RUN_TEST(the_force_is_a_different_number_from_the_aim);
    RUN_TEST(reading_either_number_twice_gives_the_same_answer);
    RUN_TEST(a_weak_and_clumsy_character_has_negative_bonuses);
    RUN_TEST(zero_is_both_the_starting_state_and_an_ordinary_answer);

    RUN_TEST(setting_the_pair_again_replaces_rather_than_adds);
    RUN_TEST(the_pair_moves_together_because_no_caller_ever_set_one_alone);
    RUN_TEST(the_aim_comes_first_the_way_the_saved_file_holds_it);
    RUN_TEST(loading_a_saved_game_uses_the_very_same_window);

    RUN_TEST(one_piece_of_equipment_adds_to_the_aim);
    RUN_TEST(the_equipment_loop_keeps_adding_because_it_walks_every_slot);
    RUN_TEST(a_cursed_item_takes_away_from_the_aim);
    RUN_TEST(one_piece_of_equipment_adds_to_the_force);
    RUN_TEST(a_bow_adds_to_the_aim_only_and_the_force_stays_where_it_was);
    RUN_TEST(adjusting_the_force_leaves_the_aim_alone_as_well);
    RUN_TEST(setting_the_pair_wipes_whatever_the_equipment_had_added);

    RUN_TEST(the_character_sheet_multiplies_the_aim_by_three_but_the_window_does_not);
    RUN_TEST(a_blow_adds_the_force_whole_and_the_floor_is_the_callers_rule);

    RUN_TEST(the_windows_refuse_nothing_the_fields_refused_nothing);
    RUN_TEST(the_store_is_a_signed_short_so_thirty_two_thousand_seven_hundred_sixty_eight_wraps);
    RUN_TEST(the_widest_numbers_a_signed_short_holds_are_kept);
    RUN_TEST(adjusting_past_the_top_wraps_the_way_the_field_always_did);

    return TEST_SUMMARY();
}
