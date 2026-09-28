// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「罠と鍵をはずす腕はいくつか」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 19 つめの問いで、**`struct misc` から出る 5 つめ**
 * （どこまで潜ったか・体力の骰子・守りの点数・素の命中力につづく）。
 * 答えは short 1 本 —— もとは py.misc.disarm で、6 ファイルから
 * 10 か所が名ざしていた（10 行）。
 *
 * **1 フィールド 1 つの答えで、この道でいちばん素直な形**（16 つめ
 * 「体力の骰子は何面か」と同じ）。17 つめ（2 フィールド 1 問い）と
 * 18 つめ（2 フィールド 2 問い 1 module）のあとに来る「ふつう」。
 *
 * **大きいほど成功しやすい。** 読み手は罠の難しさを引いてから
 * randint(100) と比べる。遊びが認めている幅は 0〜200（wizard 画面が
 * 訊く範囲）だが、**負にもなる**（Half-Orc の種族の土台は -3、
 * Half-Troll は -5）。
 *
 * **創成時の DEX の下駄が 1 つ焼きこまれている。** create.c は
 * `race[i].b_dis + todis_adj()` を置き、読み手はさらに**そのときの**
 * 下駄を 2 つ足す。つまり 3 倍のうち 1 倍は創成時の DEX で凍っている。
 * **この module はその数を覚えるだけで、規則を直さない。**
 *
 * 罠と鍵の判定そのもの・暗がりと混乱と幻覚の 10 分の 1・人物画面の等級・
 * 4 か所に写してある合計の式は **この module の外**（player_disarm.h）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に 1 本を置きなおす。
 */
/* externs.h は要らない。窓口 3 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_disarm.h"

#include "minunit.h"

/* 種族を選んだところから始める（create.c:309 と同じ形）。
 * 引数は race[] の b_dis と、そのときの todis_adj()。 */
static void given_a_race_with_base(int b_dis, int dexterity_bonus_at_creation) { player_disarm_set(b_dis + dexterity_bonus_at_creation); }

/* ------------------------------------------------------------------
 * 腕そのもの -- 1 つの数、1 つの答え
 * ------------------------------------------------------------------ */

TEST(the_number_is_whatever_creation_put_there) {
    given_a_race_with_base(15, 0); /* Halfling、DEX 8〜12 */

    ASSERT_EQ_INT(15, player_disarm());
}

TEST(reading_the_number_twice_gives_the_same_answer) {
    given_a_race_with_base(0, 0); /* Human */
    player_disarm_adjust(25);

    ASSERT_EQ_INT(25, player_disarm());
    ASSERT_EQ_INT(25, player_disarm());
}

/* 0 は「まだ種族を選んでいない」でもあり、ありうる答えでもある ——
 * Human の土台は 0 で、DEX 8〜12 の創成時の下駄も 0。 */
TEST(zero_is_both_the_starting_state_and_a_real_answer) {
    given_a_race_with_base(0, 0);

    ASSERT_EQ_INT(0, player_disarm());
}

/* ------------------------------------------------------------------
 * 種族の土台 -- 8 行そのまま（負が 2 つある）
 * ------------------------------------------------------------------ */

TEST(the_small_folk_start_better_at_it) {
    given_a_race_with_base(15, 0); /* Halfling */
    const int halfling = player_disarm();

    given_a_race_with_base(0, 0); /* Human */

    ASSERT_EQ_INT(15, halfling);
    ASSERT_EQ_INT(0, player_disarm());
}

/* **負は本物の答え。** Half-Troll は -5 から始まる。 */
TEST(a_race_can_be_worse_than_nothing_at_it) {
    given_a_race_with_base(-5, 0); /* Half-Troll */

    ASSERT_EQ_INT(-5, player_disarm());
}

TEST(each_of_the_eight_racial_bases_lands_where_it_was_put) {
    /* race[].b_dis: Human 0・Half-Elf 2・Elf 5・Halfling 15・Gnome 10・
     * Dwarf 2・Half-Orc -3・Half-Troll -5。 */
    const int bases[] = {0, 2, 5, 15, 10, 2, -3, -5};

    for (int i = 0; i < 8; i++) {
        given_a_race_with_base(bases[i], 0);

        ASSERT_EQ_INT(bases[i], player_disarm());
    }
}

/* ------------------------------------------------------------------
 * 創成時の下駄 -- 1 度だけ焼きこまれる
 * ------------------------------------------------------------------ */

/* **create.c は土台と下駄を足した数を置く。** DEX 18 なら todis_adj() は 4
 * なので、Human でも 4 から始まる。 */
TEST(the_creation_time_dexterity_bonus_is_inside_the_number) {
    given_a_race_with_base(0, 4); /* Human、DEX 18 */

    ASSERT_EQ_INT(4, player_disarm());
}

/* **凍った下駄はあとで動かない。** 窓口は置きなおされるまで同じ数を返す ——
 * DEX が変わっても、読み手が足す 2 倍のほうだけが動く（それは呼び手の話）。 */
TEST(the_frozen_bonus_does_not_move_when_the_number_is_not_written) {
    given_a_race_with_base(15, 4); /* Halfling、DEX 18 */

    ASSERT_EQ_INT(19, player_disarm());
    ASSERT_EQ_INT(19, player_disarm());
}

/* ------------------------------------------------------------------
 * 階級ぶんを足す -- create.c の `+=` そのまま
 * ------------------------------------------------------------------ */

/* Human の Warrior は 0 ＋ 25。 */
TEST(the_class_adds_to_what_the_race_left_behind) {
    given_a_race_with_base(0, 0);

    player_disarm_adjust(25); /* Warrior */

    ASSERT_EQ_INT(25, player_disarm());
}

/* **遊びのなかでいちばん上手いのは Halfling の Rogue**（15 ＋ 45）。 */
TEST(the_best_pair_in_the_game_is_a_halfling_rogue) {
    given_a_race_with_base(15, 0);

    player_disarm_adjust(45); /* Rogue */

    ASSERT_EQ_INT(60, player_disarm());
}

/* **いちばん下手なのは Half-Troll の Paladin**（-5 ＋ 20）。 */
TEST(the_worst_pair_in_the_game_is_a_half_troll_paladin) {
    given_a_race_with_base(-5, 0);

    player_disarm_adjust(20); /* Paladin */

    ASSERT_EQ_INT(15, player_disarm());
}

TEST(each_of_the_six_class_amounts_is_added_as_it_stands) {
    /* class[].mdis: Warrior 25・Mage 30・Priest 25・Rogue 45・Ranger 30・
     * Paladin 20。 */
    const int amounts[] = {25, 30, 25, 45, 30, 20};

    for (int i = 0; i < 6; i++) {
        given_a_race_with_base(5, 0); /* Elf */

        player_disarm_adjust(amounts[i]);

        ASSERT_EQ_INT(5 + amounts[i], player_disarm());
    }
}

/* 足しは積む（遊びのなかで 2 度呼ぶ人はいないが、窓口は `+=` のまま）。 */
TEST(adjusting_twice_adds_twice) {
    given_a_race_with_base(0, 0);

    player_disarm_adjust(25);
    player_disarm_adjust(25);

    ASSERT_EQ_INT(50, player_disarm());
}

/* 負を足すこともできる —— もとのコードは検査していない（→ 所見 24）。 */
TEST(the_window_takes_a_negative_amount_because_the_field_did) {
    given_a_race_with_base(10, 0);

    player_disarm_adjust(-30);

    ASSERT_EQ_INT(-20, player_disarm());
}

/* ------------------------------------------------------------------
 * 置きなおし -- 種族・セーブの読みもどし・wizard 画面が同じ 1 本
 * ------------------------------------------------------------------ */

/* **セーブの読みもどしは種族の土台と同じ文**（15 つめに立てた問いへの
 * 4 度めの答えで、体力の骰子・素の命中力と同じ「同じ」。
 * 守りの点数だけが「違う」）。 */
TEST(loading_a_saved_game_uses_the_very_same_window) {
    given_a_race_with_base(0, 0);

    player_disarm_set(97);

    ASSERT_EQ_INT(97, player_disarm());
}

/* 置きなおしは積みではない —— 上手い人物のところへ下手なセーブを読んでも
 * 混ざらない。 */
TEST(a_saved_game_replaces_rather_than_adds) {
    given_a_race_with_base(15, 0);
    player_disarm_adjust(45);

    player_disarm_set(20);

    ASSERT_EQ_INT(20, player_disarm());
}

/* wizard 画面も同じ 1 本（片方だけという話が無いので、18 つめのように
 * 窓口が増えることはない）。 */
TEST(the_wizard_screen_uses_the_very_same_window) {
    given_a_race_with_base(0, 0);

    player_disarm_set(200);

    ASSERT_EQ_INT(200, player_disarm());
}

/* **0〜200 の留めは wizard 画面のもので、窓口は持たない** ——
 * もとのフィールドも留めていなかった（→ 所見 24）。 */
TEST(the_window_refuses_nothing_the_field_refused_nothing) {
    given_a_race_with_base(0, 0);

    player_disarm_set(999);
    ASSERT_EQ_INT(999, player_disarm());

    player_disarm_set(-999);
    ASSERT_EQ_INT(-999, player_disarm());
}

/* ------------------------------------------------------------------
 * セーブファイル -- short 1 本
 * ------------------------------------------------------------------ */

TEST(the_short_that_goes_out_is_the_one_that_came_in) {
    const int values[] = {0, 5, 25, 60, 200, -5, 32767, -32768};

    for (int i = 0; i < 8; i++) {
        player_disarm_set(values[i]);

        ASSERT_EQ_INT(values[i], player_disarm());
    }
}

int main(void) {
    RUN_TEST(the_number_is_whatever_creation_put_there);
    RUN_TEST(reading_the_number_twice_gives_the_same_answer);
    RUN_TEST(zero_is_both_the_starting_state_and_a_real_answer);

    RUN_TEST(the_small_folk_start_better_at_it);
    RUN_TEST(a_race_can_be_worse_than_nothing_at_it);
    RUN_TEST(each_of_the_eight_racial_bases_lands_where_it_was_put);

    RUN_TEST(the_creation_time_dexterity_bonus_is_inside_the_number);
    RUN_TEST(the_frozen_bonus_does_not_move_when_the_number_is_not_written);

    RUN_TEST(the_class_adds_to_what_the_race_left_behind);
    RUN_TEST(the_best_pair_in_the_game_is_a_halfling_rogue);
    RUN_TEST(the_worst_pair_in_the_game_is_a_half_troll_paladin);
    RUN_TEST(each_of_the_six_class_amounts_is_added_as_it_stands);
    RUN_TEST(adjusting_twice_adds_twice);
    RUN_TEST(the_window_takes_a_negative_amount_because_the_field_did);

    RUN_TEST(loading_a_saved_game_uses_the_very_same_window);
    RUN_TEST(a_saved_game_replaces_rather_than_adds);
    RUN_TEST(the_wizard_screen_uses_the_very_same_window);
    RUN_TEST(the_window_refuses_nothing_the_field_refused_nothing);

    RUN_TEST(the_short_that_goes_out_is_the_one_that_came_in);

    return TEST_SUMMARY();
}
