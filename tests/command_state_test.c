/* 「打っているコマンドについて覚えていること」のテスト -- 現在のふるまいを保護する
 *
 * 覚えていることは 3 つあり、どれも「コマンドに繰りかえしの回数を付けられる」
 * ことから出ている。回数（`#` か数字を打ってからコマンド）、向きを覚えて
 * 使いまわすか、そして直前に打ったコマンド。
 *
 * 保護したい性質は 5 つ。
 *
 *   1. **0 が「繰りかえしていない」**であること（variable.c:75 は初期値なしの
 *      int）。回数を訊く 5 か所は全部これを見ている。
 *
 *   2. **「数を受けとって終わりにする」が 1 つの動き**であること
 *      （dungeon.c の ^P・^G・^D・'+' と moria1.c の rest() が同じ 2 行を
 *      書いていた）。受けとったあとは繰りかえしていない。
 *
 *   3. **「退避して戻す」は入れ子になれる**こと。dungeon.c の `-` の hack が
 *      回数を抱えたまま get_dir() を呼び、その get_dir() がまた抱える。だから
 *      窓口は値を呼び手に返す（module の中に 1 つ置き場を持つと壊れる）。
 *
 *   4. **「繰りかえし中か」と「向きを覚えて使うか」は別の問い**であること。
 *      数を打った 1 回目は回数があるのに向きは訊く（そこで覚える）。
 *      2 回目以降が覚えた向きを使う。
 *
 *   5. **前のコマンドは「同じキーか」だけを答える**こと（^P を 2 回続けると
 *      履歴を全部見せる、V を 2 回続けると描きなおさない）。文字そのものは
 *      窓口の外に出ない。
 *
 * 走りだしは回数 0・向きは訊く・前のコマンドは空白（#18-11-7C で実体が
 * src/command_state.c の static になり、初期値もそこに入った）。
 * テストは 1 プロセスで状態を共有するので、走りだしを見る 1 件は main() の
 * 先頭に置き、以降は各件が最初に窓口で足場を作る。
 */
/* externs.h は要らない。窓口と、bool のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "command_state.h"

#include "minunit.h"

/* --- 走りだし ------------------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが状態を動かすので。 */

TEST(nothing_is_remembered_at_the_start) {
    ASSERT_FALSE(command_is_repeating());
    ASSERT_EQ_INT(0, command_count_remaining());
    ASSERT_FALSE(direction_is_remembered());
    ASSERT_TRUE(previous_command_was(' '));
}

/* --- 繰りかえしの回数 ----------------------------------------------------- */

TEST(a_count_starts_the_repeat) {
    begin_command_count(5);
    ASSERT_TRUE(command_is_repeating());
    ASSERT_EQ_INT(5, command_count_remaining());
}

TEST(each_repeat_uses_one_up) {
    begin_command_count(3);
    consume_command_count();
    ASSERT_EQ_INT(2, command_count_remaining());
    consume_command_count();
    ASSERT_EQ_INT(1, command_count_remaining());
}

/* 最後の 1 つを使いきっても、繰りかえしを終えるのは呼び手（dungeon.c の
 * ループが次の周で回数を見る）。この module は数えるだけ。 */
TEST(using_up_the_last_repeat_ends_it) {
    begin_command_count(1);
    consume_command_count();
    ASSERT_FALSE(command_is_repeating());
    ASSERT_EQ_INT(0, command_count_remaining());
}

TEST(cancelling_ends_the_repeat) {
    begin_command_count(9);
    cancel_command_count();
    ASSERT_FALSE(command_is_repeating());
    ASSERT_EQ_INT(0, command_count_remaining());
}

/* ^P・^G・^D・'+'・rest() が書いていた 2 行。受けとると終わる。 */
TEST(taking_the_count_gives_the_number_and_ends_the_repeat) {
    begin_command_count(40);
    ASSERT_EQ_INT(40, take_command_count());
    ASSERT_FALSE(command_is_repeating());
}

/* 繰りかえしていないときに受けとると 0。呼び手はその 0 を見て「回数が
 * 付いていなかった」と分かり、自分で訊きにいく（^D の prompt など）。 */
TEST(taking_nothing_gives_zero) {
    cancel_command_count();
    ASSERT_EQ_INT(0, take_command_count());
}

/* --- 退避して戻す --------------------------------------------------------- */

/* message を出しても prompt を出しても、繰りかえしは終わらない。 */
TEST(a_held_count_survives_a_cancel) {
    begin_command_count(7);

    int held = hold_command_count();
    cancel_command_count();
    ASSERT_FALSE(command_is_repeating());

    resume_command_count(held);
    ASSERT_EQ_INT(7, command_count_remaining());
}

/* dungeon.c の `-` の hack が抱えたまま get_dir() を呼び、get_dir() がまた
 * 抱える形。窓口が module の中に 1 つ置き場を持っていたら、内側の戻しが
 * 外側の値を上書きしてしまう。 */
TEST(held_counts_can_nest) {
    begin_command_count(12);

    int outer = hold_command_count();
    int inner = hold_command_count();

    cancel_command_count();
    resume_command_count(inner);
    ASSERT_EQ_INT(12, command_count_remaining());

    cancel_command_count();
    resume_command_count(outer);
    ASSERT_EQ_INT(12, command_count_remaining());
}

/* --- 向きを覚えて使うか --------------------------------------------------- */

TEST(a_repeat_reuses_the_remembered_direction) {
    reuse_remembered_direction();
    ASSERT_TRUE(direction_is_remembered());
}

TEST(a_fresh_command_asks_for_the_direction) {
    reuse_remembered_direction();
    ask_for_direction_again();
    ASSERT_FALSE(direction_is_remembered());
}

/* **これが 2 つを 1 つにできない理由。** 数を打った 1 回目は回数があるのに
 * 向きは訊く（そこで get_dir() が覚える）。もし「回数があるなら覚えた向きを
 * 使う」にしてしまうと、1 回目に覚えていない向きを使うことになる。 */
TEST(the_first_run_of_a_counted_command_still_asks) {
    begin_command_count(20);
    ask_for_direction_again();

    ASSERT_TRUE(command_is_repeating());
    ASSERT_FALSE(direction_is_remembered());
}

/* --- 前のコマンド --------------------------------------------------------- */

TEST(the_previous_command_is_remembered) {
    note_command('V');
    ASSERT_TRUE(previous_command_was('V'));
}

TEST(a_different_key_is_not_the_previous_command) {
    note_command('V');
    ASSERT_FALSE(previous_command_was('P'));
}

/* 同じキーを 2 回続けたかどうかだけが問われる（^P の 2 回目は履歴を全部、
 * V の 2 回目は描きなおさない）。 */
TEST(only_the_last_command_is_kept) {
    note_command('V');
    note_command('P');
    ASSERT_FALSE(previous_command_was('V'));
    ASSERT_TRUE(previous_command_was('P'));
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが状態を動かす。 */
    RUN_TEST(nothing_is_remembered_at_the_start);

    RUN_TEST(a_count_starts_the_repeat);
    RUN_TEST(each_repeat_uses_one_up);
    RUN_TEST(using_up_the_last_repeat_ends_it);
    RUN_TEST(cancelling_ends_the_repeat);
    RUN_TEST(taking_the_count_gives_the_number_and_ends_the_repeat);
    RUN_TEST(taking_nothing_gives_zero);

    RUN_TEST(a_held_count_survives_a_cancel);
    RUN_TEST(held_counts_can_nest);

    RUN_TEST(a_repeat_reuses_the_remembered_direction);
    RUN_TEST(a_fresh_command_asks_for_the_direction);
    RUN_TEST(the_first_run_of_a_counted_command_still_asks);

    RUN_TEST(the_previous_command_is_remembered);
    RUN_TEST(a_different_key_is_not_the_previous_command);
    RUN_TEST(only_the_last_command_is_kept);

    return TEST_SUMMARY();
}
