// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

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
 * 読むのは get_obj_num()（object_alloc.c:69）だけで、組みたてるのは 1 箇所だけ。
 * その組みたてる側は main.c の static な init_t_level() で、**テストからは
 * 届かなかった**（main() があるので main.c はリンクできない）。だから
 * #18-10-A1 で押さえたのは読む側だけ（第 1〜3 節）。#18-10-A2 で
 * src/item/object_levels.c の object_levels_init() に移したことで、数え上げの
 * ソートが初めて届くようになった（第 4 節）。
 *
 * ここで押さえたいことのうち、いちばん大事なのは
 * **get_obj_num() が返すのは表の中の位置で、品物の番号ではない**こと。
 * 呼びだし側は必ず object_at_level_position(戻り値) で引きなおす。これが
 * 崩れると「ダンジョンに置かれる品物が全部ちがう」という形で出る。
 *
 * randint() は代役が固定値を返す（fixture_set_randint）。乱数で選ぶコードを
 * 決まった値で動かせるが、**同じ定数が広い上限と狭い上限の両方に渡される**
 * ので、目の選びかたには注意が要る。狭い上限を超える目を選ぶと、代役が
 * 上限を無視して返してしまい、実装のふるまいではなく代役の都合を見ることに
 * なる（帯の中で選びなおす経路がこれに当たる。目を 25 にした理由）。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、いま
 * どう振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"

#include "object_levels.h"

/* 検証する本物（src/dungeon/object_alloc.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
int get_obj_num(int level, bool must_be_small);

/* 表から引かれる品物の定数表（treasure.c）。表そのものは #18-10-C で
 * object_levels.c の static に入ったので、窓口越しにしか触れない。 */
extern treasure_type object_list[MAX_OBJECTS];

/* 各テストの前に必ず呼ばれる。ただし fixture_reset() は表には触らない
 * （表を消す窓口はないし、要らない —— object_levels_init() は毎回同じ表を
 *   組みなおすので、それが条件づくりになる）。 */
#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* --- 条件づくり ---------------------------------------------------------
 *
 * **本物の表**（object_list の 344 品・51 レベル）で動かす。
 *
 * #18-10-A1 では小さな表に組みかえていたが、#18-10-C で表が module の static に
 * 入り、**書きこむ窓口がなくなった**。テストのためだけに書く窓口を開ける
 * こともできるが、それは表を外から壊せる道を残すのと同じなので開けていない。
 *
 * 代わりに期待値を**窓口の答えで表す**（24 という数ではなく
 * objects_up_to_level(0) と書く）。そうすると object_list が変わっても
 * テストは意味を保つ。数そのものを見るのは canary の 1 本だけ。
 *
 * 本物の表の形（測った値）:
 *   レベル 0 の帯   位置   0〜 23   24 品
 *   レベル 1 の帯   位置  24〜 54   31 品
 *   表の全体                       344 品
 */
static void given_the_real_table_of_objects(void)
{
    object_levels_init();
}

/* 前提の確認：本物の表がリンクされていて、帯の形が上のとおりである。
 * これが崩れると以下の期待値すべてが意味を失うので、先に固定する。 */
TEST(the_real_table_has_the_shape_the_expectations_assume)
{
    given_the_real_table_of_objects();
    ASSERT_EQ_INT(objects_up_to_level(0), 24);
    ASSERT_EQ_INT(objects_at_level(1), 31);
    ASSERT_EQ_INT(objects_up_to_level(MAX_OBJ_LEVEL), MAX_DUNGEON_OBJ);
}

/* ------------------------------------------------------------------
 * 1. レベル 0 -- 目次を引かず、レベル 0 の帯からそのまま選ぶ
 * ------------------------------------------------------------------ */

/* 選ぶ範囲はレベル 0 の帯の幅。返るのはその中の位置（randint は 1 起点、
 * 位置は 0 起点なので 1 引く）。 */
TEST(at_the_top_level_the_choice_is_bounded_by_the_level_zero_band)
{
    given_the_real_table_of_objects();
    fixture_set_randint(1);
    ASSERT_EQ_INT(get_obj_num(0, false), 0);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), objects_up_to_level(0));
}

/* 三角測量：出た目がそのまま位置になる。上との差は目だけ。 */
TEST(at_the_top_level_the_position_follows_the_roll)
{
    given_the_real_table_of_objects();
    fixture_set_randint(3);
    ASSERT_EQ_INT(get_obj_num(0, false), 2);
}

/* **返るのは位置であって品物の番号ではない。** 呼びだし側（object_alloc.c:124 と
 * files.c:136）は必ず object_at_level_position() で引きなおす。位置 1 に
 * 並んでいるのはレベル 0 の品物で、その番号は 1 ではない。
 * この 1 本が、抽出でいちばん壊れやすいところを押さえる。 */
TEST(the_returned_value_is_a_position_not_an_object_number)
{
    given_the_real_table_of_objects();
    fixture_set_randint(2);
    int position = get_obj_num(0, false);
    ASSERT_EQ_INT(position, 1);
    ASSERT_TRUE(object_at_level_position(position) != position);
    ASSERT_EQ_INT(object_list[object_at_level_position(position)].level, 0);
}

/* レベル 0 では 1 回しか振らない（深さを持ちあげる目も、3 つから選ぶ目も
 * 振らない）。乱数を振る回数が変わると乱数列がずれ、ゲーム全体が変わる。 */
TEST(at_the_top_level_only_one_roll_is_made)
{
    given_the_real_table_of_objects();
    fixture_set_randint(1);
    (void)get_obj_num(0, false);
    ASSERT_EQ_INT(fixture_randint_call_count(), 1);
}

/* ------------------------------------------------------------------
 * 2. レベル 1 以下 -- 深さを持ちあげる目と、帯の選びなおし
 * ------------------------------------------------------------------ */

/* OBJ_GREAT の目（1/12）が出ると深さが持ちあがり、選ぶ範囲が表の全体まで
 * 広がる。固定値 1 では持ちあげ先が最深（50）に張りつくので、上限は
 * 表の全体（344 品）になる。 */
TEST(a_lucky_roll_widens_the_choice_to_the_whole_table)
{
    given_the_real_table_of_objects();
    fixture_set_randint(1);
    ASSERT_EQ_INT(get_obj_num(1, false), 0);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), MAX_DUNGEON_OBJ);
}

/* すでに最深なら持ちあげの目は振らない（else if なので）。返る位置は上と
 * 同じなので、**振った回数**で区別する —— 持ちあげる側は 4 回
 * （OBJ_GREAT・持ちあげ先・2 択・帯）、こちらは 2 回（2 択・帯）。 */
TEST(a_level_already_at_the_deepest_skips_the_lucky_roll)
{
    given_the_real_table_of_objects();
    fixture_set_randint(1);
    (void)get_obj_num(MAX_OBJ_LEVEL, false);
    ASSERT_EQ_INT(fixture_randint_call_count(), 2);
}

/* 三角測量：持ちあげられる側は 4 回振る。 */
TEST(a_level_below_the_deepest_makes_the_lucky_roll)
{
    given_the_real_table_of_objects();
    fixture_set_randint(1);
    (void)get_obj_num(1, false);
    ASSERT_EQ_INT(fixture_randint_call_count(), 4);
}

/* 2 択で外れると「3 つ選んでいちばん深いものを採る」側に入る。採れた品物の
 * レベルが 0 なら、レベル 0 の帯から選びなおす。上限がレベル 0 の帯の幅に
 * 戻るのが目印。
 * （固定値の randint では 3 つの目が同じになるので、「いちばん深いものを
 *   採る」比較そのものはここでは見られない。） */
TEST(landing_on_a_top_level_object_re_rolls_inside_the_level_zero_band)
{
    given_the_real_table_of_objects();
    fixture_set_randint(2);
    ASSERT_EQ_INT(get_obj_num(1, false), 1);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), objects_up_to_level(0));
}

/* 採れた品物のレベルが 0 でなければ、**そのレベルの帯の中だけ**で選びなおす。
 * ここが「目次と本体が組で働く」ところ。
 *
 * レベル 1 の品物に当てるには位置が 24 以上でなければならないので、目は 25。
 * 選びなおしの上限は帯の幅（31）で、返る位置は帯の先頭（24）を足したもの ——
 * 25 - 1 + 24 = 48。 */
TEST(landing_on_a_deeper_object_re_rolls_inside_that_levels_band)
{
    given_the_real_table_of_objects();
    fixture_set_randint(25);
    int position = get_obj_num(1, false);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), objects_at_level(1));
    ASSERT_EQ_INT(position, 48);
    /* 選ばれた位置はレベル 1 の帯の中にある。 */
    ASSERT_EQ_INT(object_list[object_at_level_position(position)].level, 1);
}

/* ------------------------------------------------------------------
 * 3. 小さいものだけという指定
 * ------------------------------------------------------------------ */

/* set_large() の代役は、答えを並べなければ「大きくない」と答えるので、
 * 繰りかえしは 1 周で抜ける。指定のあるなしで結果が変わらないことを押さえる
 * （本物の set_large() を持ちこむと、どの品物が大きいかという別の話が混ざる）。 */
TEST(asking_for_a_small_object_changes_nothing_when_nothing_is_large)
{
    given_the_real_table_of_objects();
    fixture_set_randint(1);
    int with_limit = get_obj_num(1, true);
    fixture_set_randint(1);
    int without_limit = get_obj_num(1, false);
    ASSERT_EQ_INT(with_limit, without_limit);
}

/* 大きいかどうかは、選んだ位置に並んでいる品物について訊く。位置を品物の
 * 番号として渡すと別の品物を訊くことになる（目 2 で位置 1。位置 1 の品物の
 * 番号は 1 ではない）。 */
TEST(the_size_is_asked_of_the_object_at_the_chosen_position)
{
    given_the_real_table_of_objects();
    fixture_set_randint(2);
    int position = get_obj_num(1, true);
    ASSERT_EQ_INT(position, 1);
    ASSERT_EQ_INT(fixture_set_large_call_count(), 1);
    ASSERT_TRUE(fixture_set_large_last_item() ==
                &object_list[object_at_level_position(position)]);
}

/* 大きいと答えると選びなおす。選びなおすのは繰りかえしの中だけで、深さを
 * 持ちあげる 2 回は振りなおさない（1 周 2 回なので 2 + 2 × 2 = 6 回）。 */
TEST(a_large_object_is_chosen_again)
{
    given_the_real_table_of_objects();
    fixture_set_randint(1);
    fixture_set_large_answers("y");
    (void)get_obj_num(1, true);
    ASSERT_EQ_INT(fixture_set_large_call_count(), 2);
    ASSERT_EQ_INT(fixture_randint_call_count(), 6);
}

/* 指定がなければ大きさは訊かない。大きいと答える用意があっても 1 周で抜ける。 */
TEST(without_the_limit_the_size_is_not_asked)
{
    given_the_real_table_of_objects();
    fixture_set_randint(1);
    fixture_set_large_answers("y");
    (void)get_obj_num(1, false);
    ASSERT_EQ_INT(fixture_set_large_call_count(), 0);
    ASSERT_EQ_INT(fixture_randint_call_count(), 4);
}

/* ------------------------------------------------------------------
 * 4. 表を組みたてる側（#18-10-A2 で main.c の static から module へ移した）
 *
 * 上の節が「組みあがった表をどう引くか」を見るのに対して、こちらは
 * **組みたてかたそのもの**を見る。
 *
 * 数え上げのソート（counting sort）で、レベルごとの個数を数えて累計に変え、
 * それを後ろから詰めていく。O(n) で並べかえる代わりに、t_level を**途中で
 * 目次として使う**ので、順番を 1 つ入れかえると静かに壊れる。
 *
 * どれも「表全体が満たすべき性質」なので、テスト本体から分岐と繰りかえしを
 * 追いだすために、数えるのは補助関数にして違反の件数を 0 と比べる。
 * ------------------------------------------------------------------ */

/* object_list を直に数えなおす、実装と独立した照合用の数えかた。 */
static int count_objects_at_level_in_the_definitions(int level)
{
    int count = 0;
    for (int i = 0; i < MAX_DUNGEON_OBJ; i++) {
        if (object_list[i].level == level) {
            count++;
        }
    }
    return count;
}

/* 同じ品物が 2 つの位置に現れている件数。表は object_list の並べかえなので
 * 0 でなければならない。 */
static int count_objects_appearing_twice(void)
{
    static bool seen[MAX_OBJECTS];
    int duplicates = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        seen[i] = false;
    }
    for (int position = 0; position < MAX_DUNGEON_OBJ; position++) {
        int16_t object = object_at_level_position(position);
        if (seen[object]) {
            duplicates++;
        }
        seen[object] = true;
    }
    return duplicates;
}

/* 前の位置より浅い品物が来ている件数。昇順に並んでいれば 0。 */
static int count_positions_that_go_backwards(void)
{
    int backwards = 0;
    for (int position = 1; position < MAX_DUNGEON_OBJ; position++) {
        int previous = object_list[object_at_level_position(position - 1)].level;
        int current = object_list[object_at_level_position(position)].level;
        if (current < previous) {
            backwards++;
        }
    }
    return backwards;
}

/* 帯の中にそのレベル以外の品物が混じっている件数。 */
static int count_objects_outside_their_band(void)
{
    int strays = 0;
    for (int level = 1; level <= MAX_OBJ_LEVEL; level++) {
        int first = first_position_at_level(level);
        for (int i = 0; i < objects_at_level(level); i++) {
            if (object_list[object_at_level_position(first + i)].level != level) {
                strays++;
            }
        }
    }
    return strays;
}

/* 表は object_list の全品目を並べかえたもの。いちばん深いレベルまでの累計が
 * 全体の数と一致する。ここが足りないと、選ばれない品物が生まれる。 */
TEST(building_the_table_covers_every_dungeon_object)
{
    object_levels_init();
    ASSERT_EQ_INT(objects_up_to_level(MAX_OBJ_LEVEL), MAX_DUNGEON_OBJ);
}

/* 並べかえなので、同じ品物が 2 度現れてはいけない（詰める位置の計算を
 * 1 つ間違えると、ある品物が 2 度出て別の品物が消える）。 */
TEST(every_position_holds_a_different_object)
{
    object_levels_init();
    ASSERT_EQ_INT(count_objects_appearing_twice(), 0);
}

/* 位置の順に見ていくと、レベルは下がらない。 */
TEST(the_positions_run_from_shallow_to_deep)
{
    object_levels_init();
    ASSERT_EQ_INT(count_positions_that_go_backwards(), 0);
}

/* 帯の中身はそのレベルの品物だけ。get_obj_num() がレベル別に選びなおすとき
 * （object_alloc.c:109）、この性質だけを頼りにしている。 */
TEST(the_band_of_a_level_holds_only_objects_of_that_level)
{
    object_levels_init();
    ASSERT_EQ_INT(count_objects_outside_their_band(), 0);
}

/* レベルごとの個数は object_list を数えなおしたものと一致する。 */
TEST(the_count_at_a_level_matches_the_object_definitions)
{
    object_levels_init();
    ASSERT_EQ_INT(objects_at_level(1), count_objects_at_level_in_the_definitions(1));
    ASSERT_EQ_INT(objects_at_level(2), count_objects_at_level_in_the_definitions(2));
}

/* 本物のデータがリンクされていることの目印。レベル 0 の品物は 24 品で、
 * その帯は位置 0 から始まる。数が変わったら object_list が変わったとき。 */
TEST(the_shallowest_band_starts_at_the_beginning_and_holds_twenty_four)
{
    object_levels_init();
    ASSERT_EQ_INT(objects_up_to_level(0), 24);
    ASSERT_EQ_INT(first_position_at_level(1), 24);
}

/* 2 度組みたてても同じ表になる。累計を取る前に 0 に戻しているからで、
 * その 1 行を落とすと 2 度目から数が倍になる。 */
TEST(building_the_table_twice_gives_the_same_table)
{
    object_levels_init();
    int first_time = objects_up_to_level(MAX_OBJ_LEVEL);
    int16_t first_object = object_at_level_position(0);
    object_levels_init();
    ASSERT_EQ_INT(objects_up_to_level(MAX_OBJ_LEVEL), first_time);
    ASSERT_EQ_INT(object_at_level_position(0), first_object);
}

int main(void)
{
    RUN_TEST(the_real_table_has_the_shape_the_expectations_assume);

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
    RUN_TEST(the_size_is_asked_of_the_object_at_the_chosen_position);
    RUN_TEST(a_large_object_is_chosen_again);
    RUN_TEST(without_the_limit_the_size_is_not_asked);

    RUN_TEST(building_the_table_covers_every_dungeon_object);
    RUN_TEST(every_position_holds_a_different_object);
    RUN_TEST(the_positions_run_from_shallow_to_deep);
    RUN_TEST(the_band_of_a_level_holds_only_objects_of_that_level);
    RUN_TEST(the_count_at_a_level_matches_the_object_definitions);
    RUN_TEST(the_shallowest_band_starts_at_the_beginning_and_holds_twenty_four);
    RUN_TEST(building_the_table_twice_gives_the_same_table);
    return TEST_SUMMARY();
}
