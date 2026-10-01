// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* distance() のテスト -- 現在の実装を保護する
 *
 * distance()（もと misc1.c:217、#54 で dungeon/geometry.c へ移った）は
 * 引数だけから戻り値が決まる純粋関数で、
 * グローバル変数・乱数・I/O に依存しない。テストの第一号としてここを選んだ。
 * 被参照 16 箇所（視界判定、射程、モンスターAI）で回帰検知の価値が高い。
 *
 * #59-1 で src/core/distance.c に移し、#59-2 で本物をリンクする形に替えた。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、
 * この関数がいま何を返すかを固定することが目的。
 */
#include "distance.h"
#include "minunit.h"

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
