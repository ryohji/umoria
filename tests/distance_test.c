/* distance() のテスト -- 現在の実装を保護する
 *
 * misc1.c:217 の distance() は引数だけから戻り値が決まる純粋関数で、
 * グローバル変数・乱数・I/O に依存しない。テストの第一号としてここを選んだ。
 * 被参照 16 箇所（視界判定、射程、モンスターAI）で回帰検知の価値が高い。
 *
 * misc1.c 全体をリンクするとダンジョン生成や画面表示への依存が芋づるで
 * 付いてくるので、対象関数のみをこのファイルに取りこんでテストする。
 * 将来 misc1.c が依存の少ない単位に分割されたら #include に切りかえる。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、
 * この関数がいま何を返すかを固定することが目的。
 */
#include "minunit.h"

/* --- misc1.c:217 の実装をそのまま写したもの（変更していない） --- */
static int distance(int y1, int x1, int y2, int x2) {
    int dy = y1 - y2;
    if (dy < 0) {
        dy = -dy;
    }

    int dx = x1 - x2;
    if (dx < 0) {
        dx = -dx;
    }

    return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}
/* --- ここまで --- */

/* 代表値：同一点は 0 */
TEST(distance_between_same_point_is_zero)
{
    ASSERT_EQ_INT(distance(0, 0, 0, 0), 0);
}

/* 三角測量：軸方向 1 歩は縦横どちらも 1 */
TEST(distance_of_one_step_east_is_one)
{
    ASSERT_EQ_INT(distance(0, 0, 0, 1), 1);
}

TEST(distance_of_one_step_south_is_one)
{
    ASSERT_EQ_INT(distance(0, 0, 1, 0), 1);
}

/* 直線距離。軸に沿う場合は真の距離と一致する */
TEST(distance_along_axis_equals_exact_length)
{
    ASSERT_EQ_INT(distance(0, 0, 0, 10), 10);
}

/* 3-4-5 の直角三角形。真の距離 5 と一致する */
TEST(distance_of_three_four_triangle_is_five)
{
    ASSERT_EQ_INT(distance(0, 0, 3, 4), 5);
}

/* 引数を入れかえても同じ。dy と dx の扱いが対称であることの確認 */
TEST(distance_is_symmetric_between_dy_and_dx)
{
    ASSERT_EQ_INT(distance(0, 0, 4, 3), 5);
}

/* 対角は真の距離（7.07）より安い近似になる。
 * この関数は真のユークリッド距離ではなく、
 * 「長い辺 + 短い辺の半分」で近似している。 */
TEST(distance_of_pure_diagonal_five_is_seven)
{
    ASSERT_EQ_INT(distance(0, 0, 5, 5), 7);
}

TEST(distance_of_diagonal_one_is_one)
{
    ASSERT_EQ_INT(distance(0, 0, 1, 1), 1);
}

/* 符号：負方向でも絶対値で扱われる */
TEST(distance_ignores_direction_sign)
{
    ASSERT_EQ_INT(distance(0, 0, -3, -4), 5);
}

/* 原点以外を始点にしても、差分だけで決まる */
TEST(distance_depends_only_on_difference)
{
    ASSERT_EQ_INT(distance(5, 5, 2, 1), 5);
}

int main(void)
{
    RUN_TEST(distance_between_same_point_is_zero);
    RUN_TEST(distance_of_one_step_east_is_one);
    RUN_TEST(distance_of_one_step_south_is_one);
    RUN_TEST(distance_along_axis_equals_exact_length);
    RUN_TEST(distance_of_three_four_triangle_is_five);
    RUN_TEST(distance_is_symmetric_between_dy_and_dx);
    RUN_TEST(distance_of_pure_diagonal_five_is_seven);
    RUN_TEST(distance_of_diagonal_one_is_one);
    RUN_TEST(distance_ignores_direction_sign);
    RUN_TEST(distance_depends_only_on_difference);
    return TEST_SUMMARY();
}
