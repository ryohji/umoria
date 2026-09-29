// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「体の重さ」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 22 つめの問いで、**`struct misc` から出る 8 つめ**
 * （どこまで潜ったか・体力の骰子・守りの点数・素の命中力・罠と鍵をはずす腕・
 * 抵抗・どの種族かにつづく）。答えは 1 つの short —— もとは py.misc.wt で、
 * 6 ファイルから 12 か所が名ざしていた（12 行）。
 *
 * **単位はポンド**（1/10 ポンドではない）。Halfling の男は 60 前後、
 * Human の男は 180 前後（src/data/player.c:107 の race[] の m_b_wt / f_b_wt）。
 *
 * **身なりではなく物理量。** 6 人の読み手のうち 4 人は規則のほう ——
 * 持てる重さの上限（misc3.c:942）・盾での打ちかかりの命中と打撃
 * （moria4.c:910・:920）・扉への体当たり（moria4.c:1000）。
 *
 * **窓口は 2 本だけで、この道で初めて 3 本を下回る。** 読みと置くの 2 本で、
 * **`_adjust` は無い** —— ただし理由は 21 つめ（どの種族か）と違う。
 * **あちらは数ではなかった。こちらは数なのに誰も足さない** ——
 * 食べても荷を負っても呪われても体重は動かない。**人物は太らない。**
 * それを 1 件で固定する（setting_the_weight_again_replaces_rather_than_adds）。
 *
 * **書かれるのは普通の一生で 1 回だけ** —— 創成のとき（create.c が男女で
 * 別の列を引く 2 行）。ほかに読みもどしと wizard の置きなおしがある。
 *
 * 外に残すものは 3 つ（player_body_weight.h に書いてある）——
 * 持てる重さの上限（主語は腕力）・打ちかかりと体当たりの 3 つの式
 * （割る数が `/10`・`/60`・`/2` と違う）・画面と持ち出しファイルの見せかた
 * （受け手が prt_num と fprintf で共通の式が無い）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に重さを置きなおす。
 */
/* externs.h は要らない。窓口 2 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_body_weight.h"

#include "minunit.h"

/* 創成で重さが決まったところから始める（create.c:313 と同じ形）。
 * 引数はポンド 1 つ。 */
static void given_a_body_weighing(int pounds) { player_body_weight_set(pounds); }

/* ------------------------------------------------------------------
 * 重さそのもの -- 1 つの数、足すものは無い
 * ------------------------------------------------------------------ */

TEST(the_weight_is_whatever_creation_rolled) {
    given_a_body_weighing(60); /* Halfling の男 */

    ASSERT_EQ_INT(60, player_body_weight());
}

TEST(reading_the_weight_twice_gives_the_same_answer) {
    given_a_body_weighing(180); /* Human の男 */

    ASSERT_EQ_INT(180, player_body_weight());
    ASSERT_EQ_INT(180, player_body_weight());
}

/* 0 は「まだ決まっていない」で、**既存のテストがこれを当てにしている** ——
 * tests/check_strength_test.c:244 の期待値 1300 は 10 × 130 + **0**。 */
TEST(zero_is_the_starting_state_and_the_carrying_limit_leans_on_it) {
    given_a_body_weighing(0);

    ASSERT_EQ_INT(0, player_body_weight());
}

/* **`_adjust` が無いことをここで固定する。** 2 度めの数は 1 度めに足される
 * のではなく置きかわる —— **人物は太らない**。ほかの 6 つの問い
 * （種族を除く）はどれも足す窓口を持っている。 */
TEST(setting_the_weight_again_replaces_rather_than_adds) {
    given_a_body_weighing(60);
    given_a_body_weighing(180);

    ASSERT_EQ_INT(180, player_body_weight());
}

/* create.c:313 と :316 は男女で **race[] の別の列**を引く（Halfling なら
 * 男 60・女 50）。窓口から見れば 2 行は同じ 1 本で、**最後に置いたほうが残る**。 */
TEST(the_two_lines_of_creation_are_the_same_one_window) {
    given_a_body_weighing(60); /* 男の列 */
    given_a_body_weighing(50); /* 女の列 */

    ASSERT_EQ_INT(50, player_body_weight());
}

/* Halfling の男は randnor(60, 3) なので、50〜65 あたりの 2 桁が出る。
 * **起動して確かめるときに見られるのはこの桁だけ**（数そのものは毎回振れる）。 */
TEST(a_halfling_sized_body_makes_the_round_trip) {
    given_a_body_weighing(59);

    ASSERT_EQ_INT(59, player_body_weight());
}

/* ------------------------------------------------------------------
 * 呼び手の割り算 -- 窓口は int を返し、丸めは呼び手の側で起きる
 * ------------------------------------------------------------------ */

/* 3 つの規則が 3 つの違う数で割る（moria4.c:910 は `/10`、:920 は `/60`、
 * :1000 は `/2`）。**窓口は割らない** —— 割るのは呼び手の仕事で、
 * int を返すので切り捨ては C のいつもの形になる。 */
TEST(the_callers_do_their_own_dividing_on_a_plain_int) {
    given_a_body_weighing(65);

    ASSERT_EQ_INT(6, player_body_weight() / 10);  /* 盾の命中 */
    ASSERT_EQ_INT(1, player_body_weight() / 60);  /* 盾の打撃（＋3 は呼び手） */
    ASSERT_EQ_INT(32, player_body_weight() / 2);  /* 扉への体当たり */
}

/* ------------------------------------------------------------------
 * セーブファイルと wizard -- short 1 つ
 * ------------------------------------------------------------------ */

/* save.c:632 は器の番地に読んでいた（`rd_short(&m_ptr->wt)`）。窓口ごしに
 * すると局所の uint16_t に受けてから置くことになるが、**置きなおす窓口は
 * 創成と同じ 1 本** —— 15 つめが立てた問いへの 7 度めの答えで、
 * 守りの点数を除く 5 つと同じ側。 */
TEST(loading_a_saved_game_uses_the_very_same_window) {
    given_a_body_weighing(0);

    uint16_t from_the_file = 137;
    player_body_weight_set(from_the_file);

    ASSERT_EQ_INT(137, player_body_weight());
}

TEST(a_saved_game_replaces_rather_than_adds) {
    given_a_body_weighing(60);

    player_body_weight_set(180);

    ASSERT_EQ_INT(180, player_body_weight());
}

/* wizard.c:243 は prompt に今の数を見せ、:249 が入れた数を置く ——
 * **見せるのと置くのが 1 本ずつ**で、あいだの `tmp_val > -1` は
 * この prompt の規則なので呼び手に残す（所見 24）。 */
TEST(the_wizards_tweak_reads_and_then_writes_through_the_windows) {
    given_a_body_weighing(60);

    const int shown_in_the_prompt = player_body_weight();
    player_body_weight_set(shown_in_the_prompt + 40); /* 手で打ちなおした数 */

    ASSERT_EQ_INT(100, player_body_weight());
}

/* ------------------------------------------------------------------
 * 幅と留め -- 窓口は何も断らない（所見 24）
 * ------------------------------------------------------------------ */

/* もとのフィールドは何も留めていなかったので、窓口も留めない。
 * 人の体重として有りえない数でも通る —— **断るのは wizard の prompt だけ**で、
 * それはこの問いの規則ではない。 */
TEST(the_window_refuses_nothing_the_field_refused_nothing) {
    given_a_body_weighing(60000);

    ASSERT_EQ_INT(60000, player_body_weight());
}

TEST(the_widest_weight_a_short_can_hold_is_kept) {
    given_a_body_weighing(65535);

    ASSERT_EQ_INT(65535, player_body_weight());
}

/* 置き場は 2 バイトのまま。`py.misc.wt = randnor(...)` が切り落としていた
 * のと同じところで切れる。 */
TEST(the_store_is_two_bytes_wide_so_sixty_five_thousand_five_hundred_thirty_six_lands_on_zero) {
    given_a_body_weighing(65536);

    ASSERT_EQ_INT(0, player_body_weight());
}

TEST(a_negative_weight_wraps_the_way_the_short_always_did) {
    given_a_body_weighing(-1);

    ASSERT_EQ_INT(65535, player_body_weight());
}

int main(void) {
    RUN_TEST(the_weight_is_whatever_creation_rolled);
    RUN_TEST(reading_the_weight_twice_gives_the_same_answer);
    RUN_TEST(zero_is_the_starting_state_and_the_carrying_limit_leans_on_it);
    RUN_TEST(setting_the_weight_again_replaces_rather_than_adds);
    RUN_TEST(the_two_lines_of_creation_are_the_same_one_window);
    RUN_TEST(a_halfling_sized_body_makes_the_round_trip);

    RUN_TEST(the_callers_do_their_own_dividing_on_a_plain_int);

    RUN_TEST(loading_a_saved_game_uses_the_very_same_window);
    RUN_TEST(a_saved_game_replaces_rather_than_adds);
    RUN_TEST(the_wizards_tweak_reads_and_then_writes_through_the_windows);

    RUN_TEST(the_window_refuses_nothing_the_field_refused_nothing);
    RUN_TEST(the_widest_weight_a_short_can_hold_is_kept);
    RUN_TEST(the_store_is_two_bytes_wide_so_sixty_five_thousand_five_hundred_thirty_six_lands_on_zero);
    RUN_TEST(a_negative_weight_wraps_the_way_the_short_always_did);

    return TEST_SUMMARY();
}
