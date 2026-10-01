// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* los() のテスト -- line of sight（視線が通るか）
 *
 * los(fromY, fromX, toY, toX): 2 点間に視線が通るか
 *
 * 視線は始点と終点を除く全てのマスが透明（< MIN_CLOSED_SPACE）である必要がある。
 * 隣接マス（距離 1 以下）は常に true。
 *
 * テストで地図を組むには dungeon_map の窓口を使う。
 * 複数の分岐（水平・垂直・斜め・遮られる・遮られない）に届く。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_map.h"
#include "dungeon_size.h"
#include "externs.h"
#include "minunit.h"

#include <stdbool.h>

static void setup_empty_map(int height, int width) {
    set_dungeon_size(height, width);
    dungeon_map_reset();
    // 全てのマスを床（CORR_FLOOR = 1）にする
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            cave_type *c = square_at(y, x);
            c->fval = CORR_FLOOR;
        }
    }
}

/* 隣接マスと同一点は常に true */

TEST(los_returns_true_for_same_square)
{
    setup_empty_map(20, 20);
    ASSERT_TRUE(los(10, 10, 10, 10));
}

TEST(los_returns_true_for_adjacent_squares)
{
    setup_empty_map(20, 20);
    int mismatch = 0;
    if (!los(10, 10, 10, 11)) mismatch++;  // 東
    if (!los(10, 10, 11, 10)) mismatch++;  // 南
    if (!los(10, 10, 9, 10)) mismatch++;   // 北
    if (!los(10, 10, 10, 9)) mismatch++;   // 西
    ASSERT_EQ_INT(mismatch, 0);
}

/* 直線上の視線 */

TEST(los_returns_true_along_clear_horizontal_line)
{
    setup_empty_map(20, 20);
    ASSERT_TRUE(los(10, 5, 10, 15));
}

TEST(los_returns_true_along_clear_vertical_line)
{
    setup_empty_map(20, 20);
    ASSERT_TRUE(los(5, 10, 15, 10));
}

/* 壁があると false */

TEST(los_returns_false_when_wall_blocks_horizontal_line)
{
    setup_empty_map(20, 20);
    // (10, 10) から (10, 15) への視線
    // (10, 12) に壁を置く
    square_at(10, 12)->fval = GRANITE_WALL;
    ASSERT_FALSE(los(10, 10, 10, 15));
}

TEST(los_returns_false_when_wall_blocks_vertical_line)
{
    setup_empty_map(20, 20);
    // (5, 10) から (15, 10) への視線
    // (10, 10) に壁を置く
    square_at(10, 10)->fval = GRANITE_WALL;
    ASSERT_FALSE(los(5, 10, 15, 10));
}

/* 始点と終点は透明でなくてもよい */

TEST(los_allows_walls_at_start_and_end_points)
{
    setup_empty_map(20, 20);
    square_at(10, 10)->fval = GRANITE_WALL;
    square_at(10, 15)->fval = GRANITE_WALL;
    // 間が透明なら true
    ASSERT_TRUE(los(10, 10, 10, 15));
}

/* 対角線の視線 */

TEST(los_returns_true_along_clear_diagonal)
{
    setup_empty_map(20, 20);
    ASSERT_TRUE(los(10, 10, 15, 15));
}

TEST(los_returns_false_when_wall_blocks_diagonal)
{
    setup_empty_map(20, 20);
    // (10, 10) から (15, 15) への対角線
    // (12, 12) に壁を置く
    square_at(12, 12)->fval = GRANITE_WALL;
    ASSERT_FALSE(los(10, 10, 15, 15));
}

int main(void)
{
    RUN_TEST(los_returns_true_for_same_square);
    RUN_TEST(los_returns_true_for_adjacent_squares);
    RUN_TEST(los_returns_true_along_clear_horizontal_line);
    RUN_TEST(los_returns_true_along_clear_vertical_line);
    RUN_TEST(los_returns_false_when_wall_blocks_horizontal_line);
    RUN_TEST(los_returns_false_when_wall_blocks_vertical_line);
    RUN_TEST(los_allows_walls_at_start_and_end_points);
    RUN_TEST(los_returns_true_along_clear_diagonal);
    RUN_TEST(los_returns_false_when_wall_blocks_diagonal);
    return TEST_SUMMARY();
}
