// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「走っているか／何歩走ったか」のテスト -- 現在のふるまいを保護する
 *
 * 走りは `.`（走る）コマンド。向きを決めて、止まる理由が出るまで動きつづける。
 * そのあいだ、移動とは関係のないいくつかの場所がふるまいを変える ——
 * `@` を描かない（ui/map_view.c の loc_symbol()）、明かりの輪をマスごとに塗らない
 * （moria1.c:1532-1598）、暗くても隣のモンスターに気づく（creature.c:43）、
 * ふさがれた道や足元の物を黙って通りすぎる（moria3.c:776）、本編のループが
 * キー入力を 10 秒待たない（dungeon.c:431）。**全部が同じ 1 つの問いを訊く。**
 *
 * 保護したい性質は 4 つ。
 *
 *   1. **走りはじめは 1 歩目から数える**こと（moria2.c:217 の begin_run()）。
 *      0 だと「走っていない」と同じ値になってしまう。
 *
 *   2. **息切れの境目が動いていない**こと。元は moria2.c:287 の
 *      `find_flag++ > 100`（"prevent infinite loops in find mode, will stop
 *      after moving 100 times"）。歩数は窓口の内側なので、境目を見るのは
 *      ここだけになる。
 *
 *   3. **「もう走れない」と言うだけで、止めはしない**こと。止めるのは呼び手
 *      （find_run() が message を出して end_find() を呼ぶ）。この module は
 *      数えるだけ。
 *
 *   4. **走りなおすと歩数はやりなおし**であること。止めたあと走りだしたら、
 *      また 100 歩ぶん走れる。
 *
 * 走りだしは 0（#18-11-6C で src/player/running.c の static になった。元の
 * variable.c:70 は初期値なしの int）。テストは 1 プロセスで状態を共有するので、
 * 走りだしを見る 1 件は main() の先頭に置き、以降は各件が最初に begin_run() か
 * stop_running() で足場を作る。
 */
/* externs.h は要らない。窓口と、bool のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "running.h"

#include "minunit.h"

/* 息切れするまで歩く。戻り値は「まだ走れる」と言われた回数。 */
static int run_until_out_of_breath(void) {
    int steps = 0;
    while (keep_running()) {
        steps++;
    }
    return steps;
}

/* --- 走っていない --------------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが走りだすので。 */

TEST(nobody_is_running_at_the_start) {
    ASSERT_FALSE(player_is_running());
    ASSERT_EQ_INT(0, running_steps());
}

/* --- 走りだしと止めかた --------------------------------------------------- */

TEST(beginning_a_run_makes_the_player_running) {
    begin_run();
    ASSERT_TRUE(player_is_running());
}

/* 0 歩ではなく 1 歩から。0 は「走っていない」と同じ値になってしまう。 */
TEST(a_run_counts_from_its_first_step) {
    begin_run();
    ASSERT_EQ_INT(1, running_steps());
}

TEST(stopping_ends_the_run) {
    begin_run();
    stop_running();
    ASSERT_FALSE(player_is_running());
}

/* dungeon.c:72。走りを切りあげるのではなく、新しい階では誰も走っていない。 */
TEST(a_level_start_leaves_nobody_running) {
    begin_run();
    forget_run();
    ASSERT_FALSE(player_is_running());
}

/* --- 歩数 ---------------------------------------------------------------- */

TEST(each_step_is_counted) {
    begin_run();
    (void)keep_running();
    (void)keep_running();
    (void)keep_running();
    ASSERT_EQ_INT(4, running_steps());
}

/* 元の式は `find_flag++ > 100` で、begin_run() が 1 から始める。つまり
 * 「まだ走れる」と言われるのはちょうど 100 回。 */
TEST(a_run_lasts_a_hundred_steps) {
    begin_run();
    ASSERT_EQ_INT(100, run_until_out_of_breath());
}

/* 数えるだけ。止めるのは find_run() の側。 */
TEST(running_out_of_breath_does_not_stop_the_run) {
    begin_run();
    (void)run_until_out_of_breath();
    ASSERT_TRUE(player_is_running());
}

/* 息切れしたあとも「走れない」と言いつづける（止めるまで）。 */
TEST(an_exhausted_run_stays_exhausted) {
    begin_run();
    (void)run_until_out_of_breath();
    ASSERT_FALSE(keep_running());
}

/* 止めてから走りなおせば、また 100 歩ぶん走れる。 */
TEST(a_new_run_gets_its_full_length_again) {
    begin_run();
    (void)run_until_out_of_breath();
    stop_running();

    begin_run();
    ASSERT_EQ_INT(1, running_steps());
    ASSERT_EQ_INT(100, run_until_out_of_breath());
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが走りだす。 */
    RUN_TEST(nobody_is_running_at_the_start);

    RUN_TEST(beginning_a_run_makes_the_player_running);
    RUN_TEST(a_run_counts_from_its_first_step);
    RUN_TEST(stopping_ends_the_run);
    RUN_TEST(a_level_start_leaves_nobody_running);

    RUN_TEST(each_step_is_counted);
    RUN_TEST(a_run_lasts_a_hundred_steps);
    RUN_TEST(running_out_of_breath_does_not_stop_the_run);
    RUN_TEST(an_exhausted_run_stays_exhausted);
    RUN_TEST(a_new_run_gets_its_full_length_again);

    return TEST_SUMMARY();
}
