// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* in_bounds() と mmove() のテスト
 *
 * in_bounds(y, x): 座標が範囲内か（0 < y < height-1, 0 < x < width-1）
 * mmove(dir, *y, *x): 方向 1〜9 から新しい座標を計算し、範囲内なら更新して true
 *
 * dungeon_size を操作して、様々なサイズで検証する。
 * mmove は 9 方向を表で総当たりにする（HANDOVER 第 7 節）。
 * 期待値は現在の実装の動作。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_size.h"
#include "externs.h"
#include "minunit.h"

#include <stdbool.h>

/* in_bounds のテスト */

TEST(in_bounds_returns_false_for_origin)
{
    set_dungeon_size(10, 10);
    ASSERT_FALSE(in_bounds(0, 0));
}

TEST(in_bounds_returns_false_for_boundary_edges)
{
    set_dungeon_size(10, 10);
    int mismatch = 0;
    if (in_bounds(0, 5)) mismatch++;
    if (in_bounds(9, 5)) mismatch++;
    if (in_bounds(5, 0)) mismatch++;
    if (in_bounds(5, 9)) mismatch++;
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(in_bounds_returns_true_for_interior)
{
    set_dungeon_size(10, 10);
    int mismatch = 0;
    if (!in_bounds(1, 1)) mismatch++;
    if (!in_bounds(5, 5)) mismatch++;
    if (!in_bounds(8, 8)) mismatch++;
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(in_bounds_works_with_different_sizes)
{
    set_dungeon_size(22, 66);
    int mismatch = 0;
    if (in_bounds(0, 0)) mismatch++;
    if (in_bounds(21, 65)) mismatch++;
    if (!in_bounds(1, 1)) mismatch++;
    if (!in_bounds(20, 64)) mismatch++;
    ASSERT_EQ_INT(mismatch, 0);
}

/* mmove のテスト（表で総当たり、HANDOVER 第 7 節） */

TEST(mmove_moves_in_all_nine_directions)
{
    set_dungeon_size(100, 100);

    struct {
        int dir;
        int start_y, start_x;
        int expect_y, expect_x;
    } cases[] = {
        {1, 10, 20, 11, 19},  // Southwest
        {2, 10, 20, 11, 20},  // South
        {3, 10, 20, 11, 21},  // Southeast
        {4, 10, 20, 10, 19},  // West
        {5, 10, 20, 10, 20},  // Stay
        {6, 10, 20, 10, 21},  // East
        {7, 10, 20,  9, 19},  // Northwest
        {8, 10, 20,  9, 20},  // North
        {9, 10, 20,  9, 21},  // Northeast
    };

    int mismatch = 0;
    for (int i = 0; i < 9; i++) {
        int y = cases[i].start_y;
        int x = cases[i].start_x;
        bool moved = mmove(cases[i].dir, &y, &x);
        if (!moved || y != cases[i].expect_y || x != cases[i].expect_x) {
            mismatch = (i + 1) * 100 + (y * 10 + x);
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(mmove_returns_false_when_out_of_bounds)
{
    set_dungeon_size(10, 10);
    int y = 0, x = 0;
    bool moved = mmove(8, &y, &x);
    int mismatch = 0;
    if (moved) mismatch++;
    if (y != 0) mismatch++;
    if (x != 0) mismatch++;
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(mmove_returns_true_when_in_bounds)
{
    set_dungeon_size(10, 10);
    int y = 5, x = 5;
    bool moved = mmove(2, &y, &x);
    int mismatch = 0;
    if (!moved) mismatch++;
    if (y != 6) mismatch++;
    if (x != 5) mismatch++;
    ASSERT_EQ_INT(mismatch, 0);
}

int main(void)
{
    RUN_TEST(in_bounds_returns_false_for_origin);
    RUN_TEST(in_bounds_returns_false_for_boundary_edges);
    RUN_TEST(in_bounds_returns_true_for_interior);
    RUN_TEST(in_bounds_works_with_different_sizes);
    RUN_TEST(mmove_moves_in_all_nine_directions);
    RUN_TEST(mmove_returns_false_when_out_of_bounds);
    RUN_TEST(mmove_returns_true_when_in_bounds);
    return TEST_SUMMARY();
}
