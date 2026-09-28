// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「素の命中力はいくつか」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 18 つめの問いで、**`struct misc` から出る 4 つめ**
 * （どこまで潜ったか・体力の骰子・守りの点数につづく）。答えは short 2 本 ——
 * もとは py.misc.bth（振るとき）と py.misc.bthb（射るとき・投げるとき）で、
 * 7 ファイルから 35 か所が名ざしていた（35 行）。
 *
 * **2 つのフィールドで、答えも 2 つ。** 17 つめ（守りの点数）のちょうど鏡像 ——
 * あちらは読み手がぜんぶ和を読んで片方を読む人がいなかったが、こちらは
 * **読み手がぜんぶ片方だけを読み、和を読む人が 1 人もいない**（振るのか
 * 射るのかは、読み手がすでに知っている）。
 *
 * **それでも 1 つの module なのは、書きかたが共有されているから。** 書く
 * 8 組はぜんぶ対で、うち 6 組は両方に同じ数を足す（英雄 12・大英雄 24・
 * 祝福 5）。種族も階級もセーブも両方を書き、片方だけを書くのは wizard 画面
 * だけ。
 *
 * **大きいほど当たりやすい。** 読み手は test_hit() に「攻める側の数」として
 * 渡す —— 守りの点数とは向きが逆で、同じ関数の別の引数に入る。
 * 遊びが認めている幅は 0〜200（wizard 画面が訊く範囲）だが、
 * **負にもなる**（Elf の種族の土台は近接 -5）。
 *
 * 当たり判定そのもの・人物画面の等級・素手と暗がりの直し・呪文の時計は
 * **この module の外**（player_base_to_hit.h）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に 2 本を置きなおす。
 */
/* externs.h は要らない。窓口 7 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_base_to_hit.h"

#include "minunit.h"

/* 種族を選んだところから始める（create.c:104 と同じ形）。
 * 引数は race[] の bth と bthb。 */
static void given_a_race_with_bases(int melee, int with_bows) { player_base_to_hit_set(melee, with_bows); }

/* ------------------------------------------------------------------
 * 2 つの答え -- 読み手はどちらが要るかを知っている
 * ------------------------------------------------------------------ */

TEST(the_two_numbers_are_not_the_same_number) {
    given_a_race_with_bases(-5, 15); /* Elf */

    ASSERT_EQ_INT(-5, player_base_to_hit());
    ASSERT_EQ_INT(15, player_base_to_hit_with_bows());
}

TEST(reading_either_number_twice_gives_the_same_answer) {
    given_a_race_with_bases(0, 0); /* Human */
    player_base_to_hit_adjust(70, 55);

    ASSERT_EQ_INT(70, player_base_to_hit());
    ASSERT_EQ_INT(70, player_base_to_hit());
    ASSERT_EQ_INT(55, player_base_to_hit_with_bows());
    ASSERT_EQ_INT(55, player_base_to_hit_with_bows());
}

/* **負は本物の答え。** Elf の Mage は近接 -5 ＋ 34 で 29 から始まる。 */
TEST(a_race_can_be_bad_at_swinging_and_good_at_shooting) {
    given_a_race_with_bases(-5, 15);

    player_base_to_hit_adjust(34, 20); /* Mage */

    ASSERT_EQ_INT(29, player_base_to_hit());
    ASSERT_EQ_INT(35, player_base_to_hit_with_bows());
}

/* 0 は「まだ決まっていない」でもあり、ありうる答えでもある ——
 * 種族を選ぶまでは 0 で、Human の土台もちょうど 0。 */
TEST(zero_is_both_the_starting_state_and_a_real_racial_base) {
    given_a_race_with_bases(0, 0);

    ASSERT_EQ_INT(0, player_base_to_hit());
    ASSERT_EQ_INT(0, player_base_to_hit_with_bows());
}

/* ------------------------------------------------------------------
 * 2 つとも置く -- 種族の土台とセーブの読みもどし（同じ 1 本）
 * ------------------------------------------------------------------ */

TEST(the_race_puts_both_bases_at_once) {
    given_a_race_with_bases(-1, 5); /* Half-Elf */

    ASSERT_EQ_INT(-1, player_base_to_hit());
    ASSERT_EQ_INT(5, player_base_to_hit_with_bows());
}

/* **セーブの読みもどしは種族の土台と同じ文**（15 つめに立てた問いへの
 * 3 度めの答えで、体力の骰子と同じ「同じ」。守りの点数だけが「違う」）。 */
TEST(loading_a_saved_game_uses_the_very_same_window) {
    given_a_race_with_bases(0, 0);

    player_base_to_hit_set(112, 97);

    ASSERT_EQ_INT(112, player_base_to_hit());
    ASSERT_EQ_INT(97, player_base_to_hit_with_bows());
}

/* 置きなおしは積みではない —— 強い人物のところへ弱いセーブを読んでも
 * 混ざらない。 */
TEST(a_saved_game_replaces_rather_than_adds) {
    given_a_race_with_bases(0, 0);
    player_base_to_hit_adjust(70, 55);

    player_base_to_hit_set(34, 20);

    ASSERT_EQ_INT(34, player_base_to_hit());
    ASSERT_EQ_INT(20, player_base_to_hit_with_bows());
}

/* ------------------------------------------------------------------
 * 階級ぶんを足す -- 2 つの数は別々（ここが「両方に同じだけ」と違う）
 * ------------------------------------------------------------------ */

/* Human の Warrior は 0 ＋ 70 と 0 ＋ 55。 */
TEST(a_class_is_not_equally_good_at_both) {
    given_a_race_with_bases(0, 0);

    player_base_to_hit_adjust(70, 55);

    ASSERT_EQ_INT(70, player_base_to_hit());
    ASSERT_EQ_INT(55, player_base_to_hit_with_bows());
}

/* Ranger は弓のほうが上手い（56 と 72）—— 大小が逆になる階級がある。 */
TEST(some_classes_shoot_better_than_they_swing) {
    given_a_race_with_bases(0, 0);

    player_base_to_hit_adjust(56, 72);

    ASSERT_EQ_INT(56, player_base_to_hit());
    ASSERT_EQ_INT(72, player_base_to_hit_with_bows());
}

/* 足しは種族のうえに積む（create.c は `+=` で書いていた）。 */
TEST(the_class_adds_to_what_the_race_left_behind) {
    given_a_race_with_bases(-5, 15); /* Elf */

    player_base_to_hit_adjust(60, 66); /* Rogue */

    ASSERT_EQ_INT(55, player_base_to_hit());
    ASSERT_EQ_INT(81, player_base_to_hit_with_bows());
}

/* ------------------------------------------------------------------
 * 呪文のぶん -- 12 行が 6 呼びに畳まれた窓口
 * ------------------------------------------------------------------ */

TEST(heroism_is_worth_twelve_to_both_numbers) {
    given_a_race_with_bases(0, 0);
    player_base_to_hit_adjust(70, 55);

    player_base_to_hit_adjust_both(12);

    ASSERT_EQ_INT(82, player_base_to_hit());
    ASSERT_EQ_INT(67, player_base_to_hit_with_bows());
}

TEST(super_heroism_is_worth_twenty_four_to_both_numbers) {
    given_a_race_with_bases(0, 0);

    player_base_to_hit_adjust_both(24);

    ASSERT_EQ_INT(24, player_base_to_hit());
    ASSERT_EQ_INT(24, player_base_to_hit_with_bows());
}

TEST(a_blessing_is_worth_five_to_both_numbers) {
    given_a_race_with_bases(0, 0);

    player_base_to_hit_adjust_both(5);

    ASSERT_EQ_INT(5, player_base_to_hit());
    ASSERT_EQ_INT(5, player_base_to_hit_with_bows());
}

/* 同じ窓口が足しも引きもする。dungeon.c は呪文が始まった瞬間に足して
 * 切れた瞬間に引くので、**行きと帰りで元に戻らなければならない**。 */
TEST(a_spell_that_runs_out_leaves_both_numbers_where_they_were) {
    given_a_race_with_bases(0, 0);
    player_base_to_hit_adjust(70, 55);

    player_base_to_hit_adjust_both(24);
    player_base_to_hit_adjust_both(-24);

    ASSERT_EQ_INT(70, player_base_to_hit());
    ASSERT_EQ_INT(55, player_base_to_hit_with_bows());
}

/* 3 つの呪文は重なる（英雄 12 ＋ 大英雄 24 ＋ 祝福 5 = 41）。 */
TEST(three_spells_at_once_all_count) {
    given_a_race_with_bases(0, 0);

    player_base_to_hit_adjust_both(12);
    player_base_to_hit_adjust_both(24);
    player_base_to_hit_adjust_both(5);

    ASSERT_EQ_INT(41, player_base_to_hit());
    ASSERT_EQ_INT(41, player_base_to_hit_with_bows());
}

/* **呪文は 2 つの数の差を変えない。** これがこの module を 1 つにしている
 * 規則で、遊びのなかのどの呪文も両方を同じだけ動かす。 */
TEST(a_spell_never_changes_the_gap_between_the_two_numbers) {
    given_a_race_with_bases(0, 0);
    player_base_to_hit_adjust(70, 55);
    const int gap = player_base_to_hit() - player_base_to_hit_with_bows();

    player_base_to_hit_adjust_both(12);

    ASSERT_EQ_INT(gap, player_base_to_hit() - player_base_to_hit_with_bows());
}

/* ------------------------------------------------------------------
 * wizard 画面 -- 遊びのなかで唯一、片方だけを置く
 * ------------------------------------------------------------------ */

TEST(setting_the_melee_number_alone_leaves_the_bows_number_alone) {
    given_a_race_with_bases(0, 0);
    player_base_to_hit_adjust(70, 55);

    player_base_to_hit_set_melee(200);

    ASSERT_EQ_INT(200, player_base_to_hit());
    ASSERT_EQ_INT(55, player_base_to_hit_with_bows());
}

TEST(setting_the_bows_number_alone_leaves_the_melee_number_alone) {
    given_a_race_with_bases(0, 0);
    player_base_to_hit_adjust(70, 55);

    player_base_to_hit_set_with_bows(200);

    ASSERT_EQ_INT(70, player_base_to_hit());
    ASSERT_EQ_INT(200, player_base_to_hit_with_bows());
}

/* **0〜200 の留めは wizard 画面のもので、窓口は持たない** ——
 * もとのフィールドも留めていなかった（→ 所見 24）。 */
TEST(the_windows_refuse_nothing_the_fields_refused_nothing) {
    given_a_race_with_bases(0, 0);

    player_base_to_hit_set_melee(999);
    player_base_to_hit_set_with_bows(-999);

    ASSERT_EQ_INT(999, player_base_to_hit());
    ASSERT_EQ_INT(-999, player_base_to_hit_with_bows());
}

/* ------------------------------------------------------------------
 * セーブファイル -- short 2 本（近接が先。並びは動かせない）
 * ------------------------------------------------------------------ */

TEST(the_two_shorts_that_go_out_are_the_ones_that_came_in) {
    const int values[] = {0, 5, 34, 70, 200, -5, 32767, -32768};

    for (int i = 0; i < 8; i++) {
        player_base_to_hit_set(values[i], values[7 - i]);

        ASSERT_EQ_INT(values[i], player_base_to_hit());
        ASSERT_EQ_INT(values[7 - i], player_base_to_hit_with_bows());
    }
}

/* **効いている呪文のぶんはファイルの数に入ったまま出ていく** ——
 * 守りの点数のように作りなおす道が無いので、英雄のまま保存すれば
 * 82 が書かれ、82 が読みもどされる（呪文の時計も一緒に保存される）。 */
TEST(a_spell_in_force_is_saved_inside_the_numbers) {
    given_a_race_with_bases(0, 0);
    player_base_to_hit_adjust(70, 55);
    player_base_to_hit_adjust_both(12);

    ASSERT_EQ_INT(82, player_base_to_hit());

    player_base_to_hit_set(player_base_to_hit(), player_base_to_hit_with_bows());

    ASSERT_EQ_INT(82, player_base_to_hit());
    ASSERT_EQ_INT(67, player_base_to_hit_with_bows());
}

int main(void) {
    RUN_TEST(the_two_numbers_are_not_the_same_number);
    RUN_TEST(reading_either_number_twice_gives_the_same_answer);
    RUN_TEST(a_race_can_be_bad_at_swinging_and_good_at_shooting);
    RUN_TEST(zero_is_both_the_starting_state_and_a_real_racial_base);

    RUN_TEST(the_race_puts_both_bases_at_once);
    RUN_TEST(loading_a_saved_game_uses_the_very_same_window);
    RUN_TEST(a_saved_game_replaces_rather_than_adds);

    RUN_TEST(a_class_is_not_equally_good_at_both);
    RUN_TEST(some_classes_shoot_better_than_they_swing);
    RUN_TEST(the_class_adds_to_what_the_race_left_behind);

    RUN_TEST(heroism_is_worth_twelve_to_both_numbers);
    RUN_TEST(super_heroism_is_worth_twenty_four_to_both_numbers);
    RUN_TEST(a_blessing_is_worth_five_to_both_numbers);
    RUN_TEST(a_spell_that_runs_out_leaves_both_numbers_where_they_were);
    RUN_TEST(three_spells_at_once_all_count);
    RUN_TEST(a_spell_never_changes_the_gap_between_the_two_numbers);

    RUN_TEST(setting_the_melee_number_alone_leaves_the_bows_number_alone);
    RUN_TEST(setting_the_bows_number_alone_leaves_the_melee_number_alone);
    RUN_TEST(the_windows_refuse_nothing_the_fields_refused_nothing);

    RUN_TEST(the_two_shorts_that_go_out_are_the_ones_that_came_in);
    RUN_TEST(a_spell_in_force_is_saved_inside_the_numbers);

    return TEST_SUMMARY();
}
