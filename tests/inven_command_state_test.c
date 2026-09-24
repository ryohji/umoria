/* 「再開する持ち物コマンド」のテスト -- 現在のふるまいを保護する
 *
 * 持ち物のコマンド（着る・持ちかえる・外す・落とす・一覧・装備一覧）は専用の
 * 入力モードで動き、いくつかは turn を使う。だから inven_command()（moria1.c）は
 * 「turn を使ったらいったん帰って、次の turn にこの文字でまた呼んでくれ」と
 * 言って戻る。その文字を覚えているのがこの module（moria1.c:527-551 に元の
 * 約束が英語で書かれている）。
 *
 * 訊く側は 2 つ。本編のループ（dungeon.c:685。毎 turn 訊いて、待っていれば
 * 持ち物コマンドに戻す）と、店（store2.c:1050。0 になるまで回すので、値切りの
 * 途中でも持ち物コマンドが使える）。
 *
 * 保護したい性質は 3 つ。
 *
 *   1. **0 だけが「待っていない」**こと。' '（空白）は立派な答えで、
 *      「画面を戻すためだけにもう一度呼んでくれ」という意味
 *      （moria1.c:1140 の dummy command）。C では ' ' も真なので、
 *      「待っているか」の判定を空白と比べてはいけない。
 *
 *   2. **中断は 2 つで 1 つの行い**であること。「再開する文字を覚える」と
 *      「画面が流された旗を忘れる」は必ず一緒に起きる（変更前は
 *      moria1.c:1138-1144 に 2 行として並んでいた）。片方だけ起きると:
 *      旗を消し忘れれば、何も起きていない画面について player に
 *      「Continuing with inventory command?」と訊いてしまう。別の場所で
 *      消せば、視界に入ったモンスターを見のがす。
 *
 *   3. **終わりは画面について何も言わない**こと（moria1.c:632,772 の
 *      doing_inven = 0）。ここで旗まで消すと、2 の逆の事故になる。
 *
 * 走りだしは 0（variable.c:95 に = 0 と書いてある）。テストは 1 プロセスで
 * 状態を共有するので、走りだしを見る 1 件は main() の先頭に置く。
 */
/* externs.h は要らない。2 つの窓口と、型のための types.h だけで足りる。
 * screen_touched.h を取りこむのは、中断が 2 つで 1 つであることを見るため。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "inven_command_state.h"
#include "screen_touched.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが文字を置くので。 */

TEST(nothing_is_waiting_at_the_start) {
    ASSERT_EQ_INT(pending_inven_command(), 0);
}

/* --- 待っている文字 ----------------------------------------------------- */

TEST(a_suspended_command_is_the_one_to_resume) {
    suspend_inven_command('d');
    ASSERT_EQ_INT(pending_inven_command(), 'd');
}

TEST(finishing_leaves_nothing_to_resume) {
    suspend_inven_command('d');
    finish_inven_command();
    ASSERT_EQ_INT(pending_inven_command(), 0);
}

/* moria1.c:1140 の dummy command。' ' は「待っていない」ではなく
 * 「画面を戻すためだけに呼んでくれ」。dungeon.c と store2.c が 0 と比べて
 * いるのはこのため（C では ' ' も真）。 */
TEST(a_blank_command_is_still_waiting) {
    suspend_inven_command(' ');
    ASSERT_EQ_INT(pending_inven_command(), ' ');
    ASSERT_TRUE(pending_inven_command() != 0);
}

/* --- 中断は 2 つで 1 つ ------------------------------------------------- */

/* この module がある理由そのもの。旗を消し忘れると、何も起きていない画面に
 * ついて player に訊いてしまう。 */
TEST(suspending_forgets_that_the_screen_was_flushed) {
    note_screen_flushed();
    suspend_inven_command('d');
    ASSERT_FALSE(screen_was_flushed());
}

/* 中断したあとに画面が流されたら、それは player に見せるべきことが起きた
 * 合図として残る（moria1.c:629 がこれを読んで訊く）。 */
TEST(a_flush_after_suspending_is_what_prompts_the_player) {
    suspend_inven_command('d');
    note_screen_flushed();
    ASSERT_TRUE(screen_was_flushed());
    ASSERT_EQ_INT(pending_inven_command(), 'd');
}

/* 中断のたびに数えなおす。2 回目の中断でも旗は消える。 */
TEST(suspending_again_forgets_the_flag_again) {
    suspend_inven_command('d');
    note_screen_flushed();
    suspend_inven_command('t');
    ASSERT_FALSE(screen_was_flushed());
    ASSERT_EQ_INT(pending_inven_command(), 't');
}

/* --- 終わりは画面について何も言わない ----------------------------------- */

/* moria1.c:632,772 は doing_inven = 0 だけをしていた。ここで旗も消すと、
 * 視界に入ったモンスターを見のがす側の事故になる。 */
TEST(finishing_says_nothing_about_the_screen) {
    note_screen_flushed();
    finish_inven_command();
    ASSERT_TRUE(screen_was_flushed());
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが文字を置く。 */
    RUN_TEST(nothing_is_waiting_at_the_start);

    RUN_TEST(a_suspended_command_is_the_one_to_resume);
    RUN_TEST(finishing_leaves_nothing_to_resume);
    RUN_TEST(a_blank_command_is_still_waiting);

    RUN_TEST(suspending_forgets_that_the_screen_was_flushed);
    RUN_TEST(a_flush_after_suspending_is_what_prompts_the_player);
    RUN_TEST(suspending_again_forgets_the_flag_again);

    RUN_TEST(finishing_says_nothing_about_the_screen);

    return TEST_SUMMARY();
}
