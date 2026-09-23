/* 最大 HP の計算（misc3.c:1624 の calc_hitpoints）のテスト
 *
 * レベルごとの HP 表を読む唯一の場所。ここが表のどの段を読むかで、
 * キャラクタの最大 HP が丸ごと変わる。
 *
 *   mhp = 表のいまのレベルの合計 + con_adj() * lev
 *
 * 表は 1 起点で読む（古い添字は player_hp[lev - 1]）。1 段ずれても値が
 * 「それらしく」出てしまうので、テストで段を固定しておく。
 *
 * 呼ばれる先は 3 つだけ: レベルが上がったとき（misc3.c:1582 の gain_level）、
 * レベルが戻ったとき（spells.c:1982）、CON が変わったとき（misc3.c:513）。
 * どれも単体テストが届いていなかったので、この 1 本で押さえる。
 *
 * 注意すべき仕掛けが 1 つある。mhp が 0 のときは何も書かない
 * （「キャラクタ作成中は mhp が 0 になりうる」という本体のコメント）。
 * だからテストは毎回 mhp に 0 でない値を入れてから呼ぶ。作成の側は
 * create.c で mhp を先に埋めるので、本体でもこの順になっている。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、
 * いまどう振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"
#include "hp_table.h"

extern player_type py;

/* 検証対象（src/misc3.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
void calc_hitpoints(void);

#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* ------------------------------------------------------------------
 * 条件づくりの補助関数
 * ------------------------------------------------------------------ */

/* con_adj()（src/stats.c:80）は A_CON を見る。7..16 なら 0 を返すので、
 * 表の値だけを見たいテストはこれを使う。 */
static void given_neutral_constitution(void) {
    py.stats.use_stat[A_CON] = 10;
}

/* con_adj() が 2 を返す値。18..93 の範囲。 */
static void given_constitution_adding_two_per_level(void) {
    py.stats.use_stat[A_CON] = 18;
}

/* いまのレベルと、0 でない mhp。mhp が 0 だと calc_hitpoints() は
 * 何も書かないので、レベルを決めるときに一緒に埋める。
 * chp も入れておく（mhp が動くと chp は割合で追従する）。 */
static void given_level_with_hitpoints(int level, int mhp) {
    py.misc.lev = (uint16_t)level;
    py.misc.mhp = (int16_t)mhp;
    py.misc.chp = (int16_t)mhp;
    py.misc.chp_frac = 0;
}

/* 表を 1 段ずつ埋める。level 1 が先頭。 */
static void given_hp_table_entry(int level, int total) {
    set_hp_total_at_level(level, (uint16_t)total);
}

/* ------------------------------------------------------------------
 * 表のどの段を読むか（1 起点）
 * ------------------------------------------------------------------ */

TEST(the_maximum_is_the_table_entry_for_the_current_level) {
    given_neutral_constitution();
    given_hp_table_entry(1, 19);
    given_level_with_hitpoints(1, 1);
    calc_hitpoints();
    ASSERT_EQ_INT(19, py.misc.mhp);
}

/* 1 段ずれを捕まえる。レベル 2 なら表の 2 段目で、1 段目でも 3 段目でもない。 */
TEST(level_two_reads_the_second_entry_not_the_first) {
    given_neutral_constitution();
    given_hp_table_entry(1, 19);
    given_hp_table_entry(2, 30);
    given_hp_table_entry(3, 41);
    given_level_with_hitpoints(2, 1);
    calc_hitpoints();
    ASSERT_EQ_INT(30, py.misc.mhp);
}

TEST(the_last_level_reads_the_last_entry) {
    given_neutral_constitution();
    given_hp_table_entry(MAX_PLAYER_LEVEL - 1, 400);
    given_hp_table_entry(MAX_PLAYER_LEVEL, 409);
    given_level_with_hitpoints(MAX_PLAYER_LEVEL, 1);
    calc_hitpoints();
    ASSERT_EQ_INT(409, py.misc.mhp);
}

/* ------------------------------------------------------------------
 * CON の補正はレベルの数だけ掛かる
 * ------------------------------------------------------------------ */

TEST(the_constitution_bonus_is_added_once_per_level) {
    given_constitution_adding_two_per_level();
    given_hp_table_entry(3, 41);
    given_level_with_hitpoints(3, 1);
    calc_hitpoints();
    ASSERT_EQ_INT(41 + 2 * 3, py.misc.mhp);
}

TEST(a_neutral_constitution_adds_nothing) {
    given_neutral_constitution();
    given_hp_table_entry(3, 41);
    given_level_with_hitpoints(3, 1);
    calc_hitpoints();
    ASSERT_EQ_INT(41, py.misc.mhp);
}

/* ------------------------------------------------------------------
 * 下げどめと、一時的な強化
 * ------------------------------------------------------------------ */

/* 「少なくともレベル + 1 は与える」。表が空（0）のときに効く。 */
TEST(the_maximum_never_falls_below_the_level_plus_one) {
    given_neutral_constitution();
    given_hp_table_entry(5, 0);
    given_level_with_hitpoints(5, 1);
    calc_hitpoints();
    ASSERT_EQ_INT(6, py.misc.mhp);
}

TEST(heroism_adds_ten) {
    given_neutral_constitution();
    given_hp_table_entry(2, 30);
    given_level_with_hitpoints(2, 1);
    py.flags.status |= PY_HERO;
    calc_hitpoints();
    ASSERT_EQ_INT(40, py.misc.mhp);
}

TEST(super_heroism_adds_twenty) {
    given_neutral_constitution();
    given_hp_table_entry(2, 30);
    given_level_with_hitpoints(2, 1);
    py.flags.status |= PY_SHERO;
    calc_hitpoints();
    ASSERT_EQ_INT(50, py.misc.mhp);
}

/* 両方かかっているときは足し合わせる（どちらかを選ぶのではない）。 */
TEST(both_kinds_of_heroism_add_thirty) {
    given_neutral_constitution();
    given_hp_table_entry(2, 30);
    given_level_with_hitpoints(2, 1);
    py.flags.status |= PY_HERO | PY_SHERO;
    calc_hitpoints();
    ASSERT_EQ_INT(60, py.misc.mhp);
}

/* ------------------------------------------------------------------
 * 書きこむかどうかの条件
 * ------------------------------------------------------------------ */

/* mhp が 0 のあいだは何も書かない（キャラクタ作成中の状態）。 */
TEST(nothing_is_written_while_the_maximum_is_still_zero) {
    given_neutral_constitution();
    given_hp_table_entry(1, 19);
    given_level_with_hitpoints(1, 0);
    calc_hitpoints();
    ASSERT_EQ_INT(0, py.misc.mhp);
}

/* 最大が動いたら、いまの HP は割合で追従する（半分なら半分のまま）。 */
TEST(the_current_hitpoints_follow_the_maximum_in_proportion) {
    given_neutral_constitution();
    given_hp_table_entry(1, 40);
    given_level_with_hitpoints(1, 20);
    py.misc.chp = 10; /* 半分 */
    calc_hitpoints();
    ASSERT_EQ_INT(20, py.misc.chp);
}

/* 最大が動いたことは PY_HP で知らせる（画面はここでは書けない）。 */
TEST(a_changed_maximum_is_announced_with_the_hitpoint_flag) {
    given_neutral_constitution();
    given_hp_table_entry(1, 19);
    given_level_with_hitpoints(1, 1);
    calc_hitpoints();
    ASSERT_TRUE((py.flags.status & PY_HP) != 0);
}

/* 動かなかったときは知らせない。 */
TEST(an_unchanged_maximum_is_not_announced) {
    given_neutral_constitution();
    given_hp_table_entry(1, 19);
    given_level_with_hitpoints(1, 19);
    calc_hitpoints();
    ASSERT_TRUE((py.flags.status & PY_HP) == 0);
}

int main(void) {
    RUN_TEST(the_maximum_is_the_table_entry_for_the_current_level);
    RUN_TEST(level_two_reads_the_second_entry_not_the_first);
    RUN_TEST(the_last_level_reads_the_last_entry);

    RUN_TEST(the_constitution_bonus_is_added_once_per_level);
    RUN_TEST(a_neutral_constitution_adds_nothing);

    RUN_TEST(the_maximum_never_falls_below_the_level_plus_one);
    RUN_TEST(heroism_adds_ten);
    RUN_TEST(super_heroism_adds_twenty);
    RUN_TEST(both_kinds_of_heroism_add_thirty);

    RUN_TEST(nothing_is_written_while_the_maximum_is_still_zero);
    RUN_TEST(the_current_hitpoints_follow_the_maximum_in_proportion);
    RUN_TEST(a_changed_maximum_is_announced_with_the_hitpoint_flag);
    RUN_TEST(an_unchanged_maximum_is_not_announced);

    return TEST_SUMMARY();
}
