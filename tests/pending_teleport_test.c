// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「teleport が待っているか」のテスト -- 現在のふるまいを保護する
 *
 * この 1 ビットがあるのは 1 つの罠のためだけ。ほかの teleport はみな
 * その場で teleport() を呼ぶが、teleport の罠（traps.c）は呼べない ——
 * 移動の途中で踏むので、飛ばす前に踏んだマスを光らせないといけない
 * （"Light up the teleport trap, before we teleport away"）。だから罠は
 * 書き置きを残し、本編のループ（dungeon.c:815）がその turn のコマンドを
 * やりきってから読んで teleport(100) を呼ぶ。
 *
 * 保護したい性質は 3 つ。
 *
 *   1. **罠は飛ばさずに予約するだけ**であること。この module は teleport の
 *      やりかたを知らない。
 *
 *   2. **teleport が起きたら書き置きは消える**こと（player_move.c:51。teleport() の
 *      出口）。消し忘れると、次の turn にもう一度飛ばされる。
 *
 *   3. **どの teleport でも消える**こと。teleport() は理由を問わず出口で消すので、
 *      罠以外の teleport も書き置きを消す。これは元のふるまいで、害はない ——
 *      書き置きが頼んでいたのは「player を飛ばすこと」で、それは済んでいる。
 *
 * 階の始まりに忘れる窓口（dungeon.c:69）は別にしてある。「起きた」ではないから
 * （src/player/pending_teleport.h に書いてある）。
 *
 * 走りだしは false（variable.c:101 は初期値なしの bool）。テストは 1 プロセスで
 * 状態を共有するので、走りだしを見る 1 件は main() の先頭に置く。
 */
/* externs.h は要らない。窓口と、bool のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "pending_teleport.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが旗を立てるので。 */

TEST(nothing_is_pending_at_the_start) {
    ASSERT_FALSE(teleport_is_pending());
}

/* --- 罠は予約する ------------------------------------------------------- */

TEST(a_trap_schedules_a_teleport) {
    schedule_teleport();
    ASSERT_TRUE(teleport_is_pending());
}

/* 1 turn に 2 度踏んでも書き置きは 1 枚。 */
TEST(scheduling_twice_is_the_same_as_once) {
    schedule_teleport();
    schedule_teleport();
    ASSERT_TRUE(teleport_is_pending());
}

/* --- 起きたら消える ----------------------------------------------------- */

TEST(a_finished_teleport_is_no_longer_pending) {
    schedule_teleport();
    teleport_done();
    ASSERT_FALSE(teleport_is_pending());
}

/* teleport() は理由を問わず出口で消す。予約が無くても消して構わない。 */
TEST(a_teleport_that_nobody_asked_for_clears_nothing_and_is_fine) {
    teleport_done();
    ASSERT_FALSE(teleport_is_pending());
}

/* 消してからまた予約できる。ここが効かないと、2 回目に罠を踏んでも飛ばない。 */
TEST(a_trap_after_a_teleport_schedules_another_one) {
    schedule_teleport();
    teleport_done();
    schedule_teleport();
    ASSERT_TRUE(teleport_is_pending());
}

/* --- 階の始まり --------------------------------------------------------- */

/* dungeon.c:69。teleport は起きない。旗だけ落ちる。 */
TEST(a_level_start_forgets_a_pending_teleport) {
    schedule_teleport();
    forget_pending_teleport();
    ASSERT_FALSE(teleport_is_pending());
}

TEST(forgetting_twice_leaves_nothing_pending) {
    forget_pending_teleport();
    forget_pending_teleport();
    ASSERT_FALSE(teleport_is_pending());
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが旗を立てる。 */
    RUN_TEST(nothing_is_pending_at_the_start);

    RUN_TEST(a_trap_schedules_a_teleport);
    RUN_TEST(scheduling_twice_is_the_same_as_once);

    RUN_TEST(a_finished_teleport_is_no_longer_pending);
    RUN_TEST(a_teleport_that_nobody_asked_for_clears_nothing_and_is_fine);
    RUN_TEST(a_trap_after_a_teleport_schedules_another_one);

    RUN_TEST(a_level_start_forgets_a_pending_teleport);
    RUN_TEST(forgetting_twice_leaves_nothing_pending);

    return TEST_SUMMARY();
}
