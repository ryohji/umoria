// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「入力が尽きたか」のテスト -- 現在のふるまいを保護する
 *
 * player の打つ文字が来なくなることがある —— 端末が消えた（HANGUP）、あるいは
 * 入力がファイルやパイプで終わりまで読んでしまった。そうなると読むたびに
 * すぐ EOF が返るので、もう誰にも何も訊けない。ゲームは救えるものを保存して
 * 出ていくしかない。これはゲーム全体についての 1 つの事実で、どれかの prompt に
 * ついての話ではない。
 *
 * 保護したい性質は 3 つ。
 *
 *   1. **1 回で「尽きた」になり、戻らない**こと。尽きた入力は復活しないので、
 *      消す窓口は無い（src/input_ended.h）。元のコードも 0 に戻す場所を
 *      1 つも持っていなかった。
 *
 *   2. **それでも数えている**こと。EOF のあと inkey() は ESCAPE を返すので
 *      呼び手の prompt は素通りし、ゲームはまた訊きに来ることがある ——
 *      つまり EOF が続くと回りうる。数えているのはそのためだけ。
 *
 *   3. **我慢の境目が動いていない**こと。元は io.c:81 の `eof_flag > 100` で、
 *      101 回目の EOF で panic save して落ちる。100 回目はまだ落ちない。
 *      この規則は窓口の内側にあるので（呼び手 1 つ・数える唯一の理由）、
 *      境目を見るのはここだけになる。
 *
 * **このテストは順番に並んだ 1 本道として読む。** 数は戻らない（戻す窓口が
 * 無いのが性質 1 そのもの）ので、テストごとに初期化はできない。main() の
 * 並びが進みかたで、各件は「ここまで来たときにどう見えるか」を言っている。
 */
/* externs.h は要らない。窓口と、bool のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "input_ended.h"

#include "minunit.h"

/* 数がちょうど n になるまで EOF を読ませる。 */
static void read_eofs_until(int n) {
    while (input_end_count() < n) {
        note_input_ended();
    }
}

/* --- まだ尽きていない --------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。以降のテストが数を進めるので。 */

TEST(the_input_is_fine_at_the_start) {
    ASSERT_EQ_INT(0, input_end_count());
    ASSERT_FALSE(input_has_ended());
    ASSERT_FALSE(input_end_is_hopeless());
}

/* --- 1 回で尽きる ------------------------------------------------------- */

TEST(one_eof_ends_the_input) {
    note_input_ended();
    ASSERT_TRUE(input_has_ended());
    ASSERT_EQ_INT(1, input_end_count());
}

/* 1 回目はまだ我慢する。ここが逆だと、1 度の HANGUP で墓石が見られなくなる。 */
TEST(one_eof_is_not_yet_hopeless) {
    ASSERT_FALSE(input_end_is_hopeless());
}

/* 戻す窓口が無いことの現れ。2 回目を読んでも「尽きた」は尽きたまま。 */
TEST(the_input_stays_ended) {
    note_input_ended();
    ASSERT_TRUE(input_has_ended());
    ASSERT_EQ_INT(2, input_end_count());
}

/* --- 我慢の境目 --------------------------------------------------------- */

/* 100 回目ではまだ落ちない（元の式は > 100）。 */
TEST(a_hundred_eofs_are_still_worth_trying) {
    read_eofs_until(100);
    ASSERT_EQ_INT(100, input_end_count());
    ASSERT_FALSE(input_end_is_hopeless());
}

/* 101 回目で落ちる。io.c はここで panic save して、失敗しても死なせる。 */
TEST(the_hundred_and_first_eof_is_hopeless) {
    note_input_ended();
    ASSERT_EQ_INT(101, input_end_count());
    ASSERT_TRUE(input_end_is_hopeless());
}

/* 境目を越えたあとも越えたまま。 */
TEST(it_stays_hopeless) {
    note_input_ended();
    ASSERT_TRUE(input_end_is_hopeless());
}

/* 2 つの問いは別物だが、望みが無いなら尽きてもいる。 */
TEST(hopeless_input_has_also_ended) {
    ASSERT_TRUE(input_has_ended());
}

int main(void) {
    /* 1 本道。上から順に数が進む。 */
    RUN_TEST(the_input_is_fine_at_the_start);

    RUN_TEST(one_eof_ends_the_input);
    RUN_TEST(one_eof_is_not_yet_hopeless);
    RUN_TEST(the_input_stays_ended);

    RUN_TEST(a_hundred_eofs_are_still_worth_trying);
    RUN_TEST(the_hundred_and_first_eof_is_hopeless);
    RUN_TEST(it_stays_hopeless);
    RUN_TEST(hopeless_input_has_also_ended);

    return TEST_SUMMARY();
}
