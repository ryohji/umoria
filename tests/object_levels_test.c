/* レベルごとに並べたダンジョンの品物表のテスト -- 現在の実装を保護する
 *
 * ダンジョンに置く品物を選ぶために、2 つの表が組で使われている。
 *
 *   sorted_objects[]  object_list の添字を**レベルの昇順に並べなおした**もの
 *   t_level[L]        レベルが L 以下の品物の**累計個数**
 *
 * この 2 つは 1 つの仕掛けの半分ずつで、片方だけでは意味をなさない。
 * レベル L の品物は sorted_objects の t_level[L-1] 番から t_level[L]-1 番まで
 * に並んでいる（帯）。t_level は「どこからどこまでが レベル L か」を引くための
 * 目次で、sorted_objects がその本体。
 *
 * 読むのは get_obj_num()（misc3.c:78）だけで、組みたてるのは
 * init_t_level()（main.c:279）だけ。ただし init_t_level() は main.c の static
 * なので、**テストからは届かない**（main() があるので main.c はリンクできない）。
 * だから #18-10-A1 で押さえられるのは読む側だけ。組みたてる側は module に
 * 移してから保護する（→ #18-10-A2）。
 *
 * ここで押さえたいことのうち、いちばん大事なのは
 * **get_obj_num() が返すのは sorted_objects の中の位置で、品物の番号ではない**
 * こと。呼びだし側は必ず sorted_objects[戻り値] を引きなおす。これが崩れると
 * 「ダンジョンに置かれる品物が全部ちがう」という形で出る。
 *
 * randint() は代役が固定値を返す（fixture_set_randint）。乱数で選ぶコードを
 * 決まった値で動かせるが、**同じ定数が広い上限と狭い上限の両方に使われる**
 * ので、条件づくりではレベル 1 の帯をレベル 0 の帯より広くしてある。
 * そうしないと randint(狭い上限) が上限を超える値を返し、実装のふるまいでは
 * なく代役の都合を見ることになる。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、いま
 * どう振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"

/* 検証する本物（src/misc3.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
int get_obj_num(int level, bool must_be_small);

/* 保護の対象になる 2 つの表と、そこから引かれる品物の定数表（treasure.c）。
 * #18-10-C で表が module の static に入ったら、この extern は窓口に変わる。 */
extern int16_t sorted_objects[MAX_DUNGEON_OBJ];
extern int16_t t_level[MAX_OBJ_LEVEL + 1];
extern treasure_type object_list[MAX_OBJECTS];

/* 各テストの前に必ず呼ばれる。ただし fixture_reset() はこの 2 つの表には
 * 触らない（書くのは main.c だけなので）。条件づくりで毎回組みなおす。 */
#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* --- 条件づくり ---------------------------------------------------------
 *
 * 本物の表は 344 品・51 レベルで、レベル 0 に 24 品、レベル 1 に 31 品ある。
 * そのままでは期待値が読めないので、**同じ形の小さな表**に組みかえる。
 * 使う添字は本物の object_list のもの（レベルが実際にその値である品物）。
 *
 *   位置  0  1  2 | 3  4  5  6  7  8  9 10 11 12
 *   品物 21 29 30 |24 31 79 84 85 86 91 102 123 124
 *   レベル 0  0  0 | 1  1  1  1  1  1  1   1   1   1
 *
 *   t_level[0] = 3    レベル 0 以下は 3 品（位置 0〜2）
 *   t_level[1] = 13   レベル 1 以下は 13 品（レベル 1 の帯は位置 3〜12）
 *   t_level[2..50] = 13   それより深いレベルには品物がない
 *
 * レベル 1 の帯を 10 と広くとったのは上の理由（固定値の randint が
 * 狭い上限を超えないようにする）。 */
static const int16_t LEVEL_ZERO_OBJECTS[] = {21, 29, 30};
static const int16_t LEVEL_ONE_OBJECTS[] = {24, 31, 79, 84, 85, 86, 91, 102, 123, 124};

#define LEVEL_ZERO_COUNT ((int)(sizeof LEVEL_ZERO_OBJECTS / sizeof LEVEL_ZERO_OBJECTS[0]))
#define LEVEL_ONE_COUNT ((int)(sizeof LEVEL_ONE_OBJECTS / sizeof LEVEL_ONE_OBJECTS[0]))

static void given_a_small_table_of_objects(void)
{
    for (int i = 0; i < MAX_DUNGEON_OBJ; i++) {
        sorted_objects[i] = 0;
    }
    for (int i = 0; i < LEVEL_ZERO_COUNT; i++) {
        sorted_objects[i] = LEVEL_ZERO_OBJECTS[i];
    }
    for (int i = 0; i < LEVEL_ONE_COUNT; i++) {
        sorted_objects[LEVEL_ZERO_COUNT + i] = LEVEL_ONE_OBJECTS[i];
    }

    t_level[0] = (int16_t)LEVEL_ZERO_COUNT;
    for (int level = 1; level <= MAX_OBJ_LEVEL; level++) {
        t_level[level] = (int16_t)(LEVEL_ZERO_COUNT + LEVEL_ONE_COUNT);
    }
}

/* 前提の確認：条件づくりが本当にそのレベルの品物を並べている。これが崩れると
 * 以下の期待値すべてが意味を失うので、実装ではなく条件を固定する。 */
TEST(the_fixture_really_holds_objects_of_the_levels_it_claims)
{
    given_a_small_table_of_objects();
    ASSERT_EQ_INT(object_list[sorted_objects[0]].level, 0);
    ASSERT_EQ_INT(object_list[sorted_objects[LEVEL_ZERO_COUNT - 1]].level, 0);
    ASSERT_EQ_INT(object_list[sorted_objects[LEVEL_ZERO_COUNT]].level, 1);
}

/* ------------------------------------------------------------------
 * 1. レベル 0 -- 目次を引かず、レベル 0 の帯からそのまま選ぶ
 * ------------------------------------------------------------------ */

/* 選ぶ範囲はレベル 0 の帯の幅。返るのはその中の位置（randint は 1 起点、
 * 位置は 0 起点なので 1 引く）。 */
TEST(at_the_top_level_the_choice_is_bounded_by_the_level_zero_band)
{
    given_a_small_table_of_objects();
    fixture_set_randint(1);
    ASSERT_EQ_INT(get_obj_num(0, false), 0);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), LEVEL_ZERO_COUNT);
}

/* 三角測量：出た目がそのまま位置になる。上との差は目だけ。 */
TEST(at_the_top_level_the_position_follows_the_roll)
{
    given_a_small_table_of_objects();
    fixture_set_randint(3);
    ASSERT_EQ_INT(get_obj_num(0, false), 2);
}

/* **返るのは位置であって品物の番号ではない。** 位置 1 には品物 29 が
 * 並んでいるので、呼びだし側は sorted_objects を引きなおさなければ
 * ならない。この 1 本が、抽出でいちばん壊れやすいところを押さえる。 */
TEST(the_returned_value_is_a_position_not_an_object_number)
{
    given_a_small_table_of_objects();
    fixture_set_randint(2);
    int position = get_obj_num(0, false);
    ASSERT_EQ_INT(position, 1);
    ASSERT_EQ_INT(sorted_objects[position], 29);
}

/* レベル 0 では 1 回しか振らない（深さを持ちあげる目も、3 つから選ぶ目も
 * 振らない）。乱数を振る回数が変わると乱数列がずれ、ゲーム全体が変わる。 */
TEST(at_the_top_level_only_one_roll_is_made)
{
    given_a_small_table_of_objects();
    fixture_set_randint(1);
    (void)get_obj_num(0, false);
    ASSERT_EQ_INT(fixture_randint_call_count(), 1);
}

/* ------------------------------------------------------------------
 * 2. レベル 1 以下 -- 深さを持ちあげる目と、帯の選びなおし
 * ------------------------------------------------------------------ */

/* OBJ_GREAT の目（1/12）が出ると深さが持ちあがり、選ぶ範囲が表の全体まで
 * 広がる。固定値 1 では持ちあげ先が最深（50）に張りつくので、上限は
 * t_level[50]＝表の全体になる。 */
TEST(a_lucky_roll_widens_the_choice_to_the_whole_table)
{
    given_a_small_table_of_objects();
    fixture_set_randint(1);
    ASSERT_EQ_INT(get_obj_num(1, false), 0);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), LEVEL_ZERO_COUNT + LEVEL_ONE_COUNT);
}

/* すでに最深なら持ちあげの目は振らない（else if なので）。返る位置は上と
 * 同じなので、**振った回数**で区別する —— 持ちあげる側は 4 回
 * （OBJ_GREAT・持ちあげ先・2 択・帯）、こちらは 2 回（2 択・帯）。 */
TEST(a_level_already_at_the_deepest_skips_the_lucky_roll)
{
    given_a_small_table_of_objects();
    fixture_set_randint(1);
    (void)get_obj_num(MAX_OBJ_LEVEL, false);
    ASSERT_EQ_INT(fixture_randint_call_count(), 2);
}

/* 三角測量：持ちあげられる側は 4 回振る。 */
TEST(a_level_below_the_deepest_makes_the_lucky_roll)
{
    given_a_small_table_of_objects();
    fixture_set_randint(1);
    (void)get_obj_num(1, false);
    ASSERT_EQ_INT(fixture_randint_call_count(), 4);
}

/* 2 択で外れると「3 つ選んでいちばん深いものを採る」側に入る。採れた品物の
 * レベルが 0 なら、レベル 0 の帯から選びなおす。上限が 3 に戻るのが目印。
 * （固定値の randint では 3 つの目が同じになるので、「いちばん深いものを
 *   採る」比較そのものはここでは見られない。） */
TEST(landing_on_a_top_level_object_re_rolls_inside_the_level_zero_band)
{
    given_a_small_table_of_objects();
    fixture_set_randint(2);
    ASSERT_EQ_INT(get_obj_num(1, false), 1);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), LEVEL_ZERO_COUNT);
}

/* 採れた品物のレベルが 0 でなければ、**そのレベルの帯の中だけ**で選びなおす。
 * 上限は帯の幅（10）で、返る位置は帯の先頭（3）を足したもの。ここが
 * 「目次と本体が組で働く」ところ。 */
TEST(landing_on_a_deeper_object_re_rolls_inside_that_levels_band)
{
    given_a_small_table_of_objects();
    fixture_set_randint(4);
    int position = get_obj_num(1, false);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), LEVEL_ONE_COUNT);
    ASSERT_EQ_INT(position, 6);
    /* 選ばれた位置はレベル 1 の帯（3〜12）の中にある。 */
    ASSERT_EQ_INT(object_list[sorted_objects[position]].level, 1);
}

/* ------------------------------------------------------------------
 * 3. 小さいものだけという指定
 * ------------------------------------------------------------------ */

/* set_large() の代役はつねに「大きくない」と答えるので、繰りかえしは 1 周で
 * 抜ける。指定のあるなしで結果が変わらないことを押さえる（本物の
 * set_large() を持ちこむと、どの品物が大きいかという別の話が混ざる）。 */
TEST(asking_for_a_small_object_changes_nothing_when_nothing_is_large)
{
    given_a_small_table_of_objects();
    fixture_set_randint(1);
    int with_limit = get_obj_num(1, true);
    fixture_set_randint(1);
    int without_limit = get_obj_num(1, false);
    ASSERT_EQ_INT(with_limit, without_limit);
}

int main(void)
{
    RUN_TEST(the_fixture_really_holds_objects_of_the_levels_it_claims);

    RUN_TEST(at_the_top_level_the_choice_is_bounded_by_the_level_zero_band);
    RUN_TEST(at_the_top_level_the_position_follows_the_roll);
    RUN_TEST(the_returned_value_is_a_position_not_an_object_number);
    RUN_TEST(at_the_top_level_only_one_roll_is_made);

    RUN_TEST(a_lucky_roll_widens_the_choice_to_the_whole_table);
    RUN_TEST(a_level_already_at_the_deepest_skips_the_lucky_roll);
    RUN_TEST(a_level_below_the_deepest_makes_the_lucky_roll);
    RUN_TEST(landing_on_a_top_level_object_re_rolls_inside_the_level_zero_band);
    RUN_TEST(landing_on_a_deeper_object_re_rolls_inside_that_levels_band);

    RUN_TEST(asking_for_a_small_object_changes_nothing_when_nothing_is_large);
    return TEST_SUMMARY();
}
