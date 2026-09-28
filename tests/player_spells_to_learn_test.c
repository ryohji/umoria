// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「あと何個呪文を覚えられるか」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 14 つめの問い（1 つめは持っている金、2 つめは腹の具合、3 つめは
 * 画面に出す数字、4 つめは魔力、5 つめは体力、6 つめは階級と経験値、7 つめは
 * 状態の旗、8 つめは装備で決まる耐性・能力、9 つめは一時的な状態の数えおとし、
 * 10 つめは休息の残りターン、11 つめはいまの速さ、12 つめは赤外視の届く距離、
 * 13 つめは光る手）。答えは 1 バイト —— もとは py.flags.new_spells で、
 * 2 ファイルから 9 か所が触っていた。**`struct flags` から出る最後の「問い」。**
 *
 * **名前はフィールド名から変えた。** `new_spells` は「新しい呪文」と読めるが、
 * これは呪文ではなく**数**（どの呪文を覚えているかは src/spells_known.c）。
 * この module より前に書かれた tests/gain_spells_test.c がすでに
 * `spells_to_learn` と呼んでいたので、それに合わせた。
 *
 * **使うと減る蓄えだが、13 つめの光る手とは 2 つ違う:**
 *
 *   1. **1 より大きくなる。** レベルと能力値が許す数から、すでに覚えている数を
 *      引いた余り。魔法使いはレベル 1 で 4 個ということもある。
 *   2. **作りなおされる。** 光る手は巻物でしか立たないが、この数は
 *      calc_spells() がレベルの上がるたびに**置きなおす** —— 増えるだけでなく
 *      **減ることもある**（能力値を吸われると覚えていた呪文を忘れる）。
 *      だから窓口は「足す」ではなく「置く」。
 *
 * 9 式の許される数・本が足りない差・message 3 つ・PY_STUDY の書きなおしは
 * **この module の外**（player_spells_to_learn.h）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に数を置きなおす。
 */
/* externs.h は要らない。窓口 2 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_spells_to_learn.h"

#include "minunit.h"

/* 1 つも覚えられないところから始める（戦士と、まだ数が置かれていない状態）。
 * **窓口で置きなおす** —— 置き場が src/player_spells_to_learn.c の static に
 * 入っても（#18-12-14C）この足場は届く。 */
static void given_nothing_to_learn(void) { player_spells_to_learn_set(0); }

/* calc_spells() が数を置いたところから始める。 */
static void given_spells_to_learn(int count) {
    player_spells_to_learn_set(0);
    player_spells_to_learn_set(count);
}

/* ------------------------------------------------------------------
 * 数そのもの -- 0 は「いま覚えられない」
 * ------------------------------------------------------------------ */

TEST(a_fighter_can_never_learn_anything) {
    given_nothing_to_learn();

    ASSERT_EQ_INT(0, player_spells_to_learn());
}

TEST(the_count_comes_back_as_calc_spells_left_it) {
    given_spells_to_learn(4);

    ASSERT_EQ_INT(4, player_spells_to_learn());
}

TEST(reading_the_count_twice_gives_the_same_answer) {
    given_spells_to_learn(2);

    (void)player_spells_to_learn();

    ASSERT_EQ_INT(2, player_spells_to_learn());
}

/* ------------------------------------------------------------------
 * 置きなおし -- 足しではない
 * ------------------------------------------------------------------ */

/* **置きかえ。** calc_spells() はレベルが上がるたびに走って、そのときの
 * 「許される数 − 覚えている数」を丸ごと置く。**足しにすると、レベルが
 * 上がるたびに覚えられる数が積みあがる。** */
TEST(storing_a_count_replaces_whatever_was_there) {
    given_spells_to_learn(4);

    player_spells_to_learn_set(6);

    ASSERT_EQ_INT(6, player_spells_to_learn());
}

/* **減ることもある。** 能力値を吸われると許される数が下がり、calc_spells() は
 * 呪文を忘れさせたうえで小さい数を置く。 */
TEST(a_drained_stat_leaves_a_smaller_count) {
    given_spells_to_learn(6);

    player_spells_to_learn_set(2);

    ASSERT_EQ_INT(2, player_spells_to_learn());
}

/* 0 を置くのは「もう覚えられない」で、「置かない」ではない
 * （状態行の "Study" が消えるのはこの 0）。 */
TEST(storing_zero_is_nothing_left_to_learn) {
    given_spells_to_learn(3);

    player_spells_to_learn_set(0);

    ASSERT_EQ_INT(0, player_spells_to_learn());
}

/* ------------------------------------------------------------------
 * 学んで減らす -- 減らすのは呼び手、置くのは窓口
 * ------------------------------------------------------------------ */

/* gain_spells() は局所変数で減らしてから、最後に 1 度だけ置く。
 * **1 個学んだあとの形。** */
TEST(learning_one_spell_leaves_the_rest) {
    given_spells_to_learn(3);

    player_spells_to_learn_set(player_spells_to_learn() - 1);

    ASSERT_EQ_INT(2, player_spells_to_learn());
}

/* ぜんぶ学んだら 0 になる。 */
TEST(learning_them_all_empties_the_count) {
    given_spells_to_learn(3);

    for (int learned = 0; learned < 3; learned++) {
        player_spells_to_learn_set(player_spells_to_learn() - 1);
    }

    ASSERT_EQ_INT(0, player_spells_to_learn());
}

/* **本が足りなかったぶんは残る。** gain_spells() は学べた数に
 * 「本が無くて学べなかった差」を足しもどして置く（本を買えばまた学べる）。 */
TEST(what_could_not_be_learned_for_want_of_a_book_stays) {
    given_spells_to_learn(3);

    /* 3 個ぶん許されているが、手元の本には 1 個しか載っていなかった:
     * 1 個学んで（残り 0）、差の 2 を足しもどす。 */
    player_spells_to_learn_set(0 + 2);

    ASSERT_EQ_INT(2, player_spells_to_learn());
}

/* ------------------------------------------------------------------
 * セーブファイル -- バイトがそのまま通る
 * ------------------------------------------------------------------ */

TEST(loading_a_saved_game_brings_the_count_back) {
    given_nothing_to_learn();

    player_spells_to_learn_set(5);

    ASSERT_EQ_INT(5, player_spells_to_learn());
}

/* 出ていくバイトは入ってきたバイト（save.c は窓口の返す数をそのまま
 * wr_byte する）。**器はバイトなので 256 通り。** */
TEST(the_byte_that_goes_out_is_the_one_that_came_in) {
    given_nothing_to_learn();

    for (int byte = 0; byte < 256; byte++) {
        player_spells_to_learn_set(byte);

        ASSERT_EQ_INT(byte, player_spells_to_learn());
    }
}

/* 器の幅いっぱいまで通る。 */
TEST(the_widest_byte_the_container_holds_survives) {
    given_nothing_to_learn();

    player_spells_to_learn_set(255);

    ASSERT_EQ_INT(255, player_spells_to_learn());
}

/* **留めは無い。** もとのコードもこの数を範囲で検査していないので、窓口も
 * しない（→ 台帳の所見 24）。バイトの器を超えると回りこむ。 */
TEST(nothing_clamps_the_count_to_the_spells_that_exist) {
    given_nothing_to_learn();

    player_spells_to_learn_set(64);

    ASSERT_EQ_INT(64, player_spells_to_learn());
}

int main(void) {
    RUN_TEST(a_fighter_can_never_learn_anything);
    RUN_TEST(the_count_comes_back_as_calc_spells_left_it);
    RUN_TEST(reading_the_count_twice_gives_the_same_answer);

    RUN_TEST(storing_a_count_replaces_whatever_was_there);
    RUN_TEST(a_drained_stat_leaves_a_smaller_count);
    RUN_TEST(storing_zero_is_nothing_left_to_learn);

    RUN_TEST(learning_one_spell_leaves_the_rest);
    RUN_TEST(learning_them_all_empties_the_count);
    RUN_TEST(what_could_not_be_learned_for_want_of_a_book_stays);

    RUN_TEST(loading_a_saved_game_brings_the_count_back);
    RUN_TEST(the_byte_that_goes_out_is_the_one_that_came_in);
    RUN_TEST(the_widest_byte_the_container_holds_survives);
    RUN_TEST(nothing_clamps_the_count_to_the_spells_that_exist);

    return TEST_SUMMARY();
}
