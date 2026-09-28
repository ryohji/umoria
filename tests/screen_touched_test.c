// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「画面が流されたか」の旗のテスト -- 現在のふるまいを保護する
 *
 * この 1 ビットが答えるのは 1 つの問いだけ。「前に忘れたときから今までに、
 * 画面に何か出たか」。読み手は持ち物コマンド（moria1.c:629）ひとりで、画面を
 * 退避してから 1 turn 世界に戻り、帰ってきたときに「退避した画面がまだ
 * 目の前にあるか」を知りたい。あるなら黙って描きなおせる。無いなら
 * 「Continuing with inventory command?」と player に訊く —— 見せるべきことが
 * 起きたのだから。
 *
 * 立てるのは 2 か所。put_qio()（io.c:44。buffer を画面に押しだす唯一の場所）と、
 * creature.c:66,75（モンスターが見えた／見えなくなった。1 マスだけ描きなおすので
 * put_qio() を通らない）。
 *
 * 保護したい性質は 2 つ。
 *
 *   1. **立てるのは何度でも同じ**こと。put_qio() は 1 turn に何度も呼ばれる。
 *
 *   2. **忘れてからもう一度立てられる**こと。旗は「いつから」を持たない
 *      1 ビットなので、忘れる／立てるの往復が効かないと、2 回目の中断で
 *      player に訊けなくなる（あるいは訊きすぎる）。
 *
 * 走りだしは false（variable.c:97 に = false と書いてある）。テストは
 * 1 プロセスで状態を共有するので、走りだしを見る 1 件は main() の先頭に置く。
 */
/* externs.h は要らない。窓口と、bool のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "screen_touched.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが旗を立てるので。 */

TEST(the_screen_starts_out_unflushed) {
    ASSERT_FALSE(screen_was_flushed());
}

/* --- 立てる ------------------------------------------------------------- */

TEST(a_flush_is_noticed) {
    note_screen_flushed();
    ASSERT_TRUE(screen_was_flushed());
}

/* put_qio() は 1 turn に何度も呼ばれる。2 度立てても 1 度と同じ。 */
TEST(noticing_twice_is_the_same_as_once) {
    note_screen_flushed();
    note_screen_flushed();
    ASSERT_TRUE(screen_was_flushed());
}

/* --- 忘れる ------------------------------------------------------------- */

TEST(forgetting_puts_it_back_to_unflushed) {
    note_screen_flushed();
    forget_screen_flushed();
    ASSERT_FALSE(screen_was_flushed());
}

/* 忘れても「もう何も出ない」ことにはならない。ここが効かないと、持ち物
 * コマンドを 2 回続けて中断したときに 2 回目の変化を見のがす。 */
TEST(a_flush_after_forgetting_is_noticed_again) {
    note_screen_flushed();
    forget_screen_flushed();
    note_screen_flushed();
    ASSERT_TRUE(screen_was_flushed());
}

/* 忘れるのを 2 度くりかえしても立った旗を取りこぼさない（順序の確認）。 */
TEST(forgetting_twice_leaves_it_unflushed) {
    forget_screen_flushed();
    forget_screen_flushed();
    ASSERT_FALSE(screen_was_flushed());
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが旗を立てる。 */
    RUN_TEST(the_screen_starts_out_unflushed);

    RUN_TEST(a_flush_is_noticed);
    RUN_TEST(noticing_twice_is_the_same_as_once);

    RUN_TEST(forgetting_puts_it_back_to_unflushed);
    RUN_TEST(a_flush_after_forgetting_is_noticed_again);
    RUN_TEST(forgetting_twice_leaves_it_unflushed);

    return TEST_SUMMARY();
}
