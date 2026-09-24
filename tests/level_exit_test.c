/* 「この階は終わったか」のテスト -- 現在のふるまいを保護する
 *
 * 1 ビットで、読み手は本編のループ（dungeon.c:806,820,825）ひとり。立つまで
 * turn を回し、立ったら play_game() に帰って次の階を作りなおし、また
 * dungeon() を呼ぶ。ほかの 8 か所は立てるだけ。
 *
 * 保護したい性質は 3 つ。
 *
 *   1. **階を出るのは深さの置きかえと対**であること。8 か所のうち 6 か所は
 *      dun_level を同じ息で書きかえていた（階段の上り下り dungeon.c:1835,1857、
 *      落とし穴 moria3.c:67、word-of-recall dungeon.c:561、深みに落ちる巻物
 *      scrolls.c:197、wizard の ^D dungeon.c:1526）。片方だけ起きると:
 *      旗を立てずに深さを変えれば、今いる階のまま深さの表示だけが変わる。
 *      深さを変えずに旗を立てれば、同じ階がもう一度作られる。
 *
 *   2. **行き先を言わない「終わり」もある**こと（死 moria1.c:1732、
 *      Quit dungeon.c:1106、それに word-of-recall で行き先が無いとき）。
 *      ここで深さを触ってはいけない。
 *
 *   3. **町（0）はほかの階と同じ**であること。word-of-recall は 0 を行き先に
 *      する（dungeon.c:565）。0 を「行き先なし」と読むと、地上に戻れない。
 *
 * 上限・下限は窓口に入れない。wizard の ^D が 0-99 なのはその prompt がそう
 * 言っているからで、巻物が 1 で止まるのは町より上へ押しあげないためで、
 * どちらも「階を出る」の規則ではない（src/level_exit.h に書いてある）。
 *
 * 走りだしは false（variable.c:100 は初期値なしの bool）。テストは 1 プロセスで
 * 状態を共有するので、走りだしを見る 1 件は main() の先頭に置く。
 */
/* externs.h は要らない。窓口と、型のための types.h だけで足りる。
 * dun_level は足場が持っている（#18-11-4C の後も残る。あちらの註を見よ）。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "level_exit.h"

#include "minunit.h"

/* 深さは別の区分の global なので、テストからは直に見る。 */
extern int16_t dun_level;

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが旗を立てるので。 */

TEST(a_level_starts_out_unfinished) {
    ASSERT_FALSE(level_is_over());
}

/* --- 階を出るのは深さの置きかえと対 ------------------------------------- */

TEST(going_to_another_level_finishes_this_one) {
    begin_level();
    leave_for_level(3);
    ASSERT_TRUE(level_is_over());
}

TEST(going_to_another_level_says_which_one) {
    begin_level();
    dun_level = 1;
    leave_for_level(3);
    ASSERT_EQ_INT(dun_level, 3);
}

/* 階段を下りる形（dungeon.c:1857 は dun_level++ と旗を並べていた）。 */
TEST(the_next_level_is_counted_from_this_one) {
    begin_level();
    dun_level = 12;
    leave_for_level(dun_level + 1);
    ASSERT_EQ_INT(dun_level, 13);
    ASSERT_TRUE(level_is_over());
}

/* 深みに落ちる巻物は 1 回で何階も下がる（scrolls.c:197）。 */
TEST(the_next_level_can_be_several_levels_away) {
    begin_level();
    dun_level = 10;
    leave_for_level(13);
    ASSERT_EQ_INT(dun_level, 13);
    ASSERT_TRUE(level_is_over());
}

/* --- 行き先を言わない終わり --------------------------------------------- */

/* 死と Quit。深さはそのまま（死んだ階が score に出る）。 */
TEST(ending_without_a_level_leaves_the_depth_alone) {
    begin_level();
    dun_level = 7;
    end_level();
    ASSERT_TRUE(level_is_over());
    ASSERT_EQ_INT(dun_level, 7);
}

/* --- 町はほかの階と同じ ------------------------------------------------- */

/* word-of-recall で地上に戻る形（dungeon.c:565）。0 は立派な行き先。 */
TEST(the_town_is_a_level_like_any_other) {
    begin_level();
    dun_level = 20;
    leave_for_level(0);
    ASSERT_EQ_INT(dun_level, 0);
    ASSERT_TRUE(level_is_over());
}

/* --- 階の始まり --------------------------------------------------------- */

TEST(beginning_a_level_leaves_it_unfinished) {
    leave_for_level(5);
    begin_level();
    ASSERT_FALSE(level_is_over());
}

/* dungeon.c:67 の時点で dun_level はもう「今から始める階」なので、
 * 始まりは深さについて何も言わない。 */
TEST(beginning_a_level_says_nothing_about_the_depth) {
    dun_level = 9;
    begin_level();
    ASSERT_EQ_INT(dun_level, 9);
}

/* 階をまたいで何度でも。2 回目の行き先が 1 回目に上書きされない。 */
TEST(leaving_again_names_the_new_target) {
    leave_for_level(2);
    begin_level();
    leave_for_level(3);
    ASSERT_EQ_INT(dun_level, 3);
    ASSERT_TRUE(level_is_over());
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが旗を立てる。 */
    RUN_TEST(a_level_starts_out_unfinished);

    RUN_TEST(going_to_another_level_finishes_this_one);
    RUN_TEST(going_to_another_level_says_which_one);
    RUN_TEST(the_next_level_is_counted_from_this_one);
    RUN_TEST(the_next_level_can_be_several_levels_away);

    RUN_TEST(ending_without_a_level_leaves_the_depth_alone);

    RUN_TEST(the_town_is_a_level_like_any_other);

    RUN_TEST(beginning_a_level_leaves_it_unfinished);
    RUN_TEST(beginning_a_level_says_nothing_about_the_depth);
    RUN_TEST(leaving_again_names_the_new_target);

    return TEST_SUMMARY();
}
