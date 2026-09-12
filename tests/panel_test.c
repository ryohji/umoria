/* パネル（画面が見せているダンジョンの範囲）のテスト -- 現在のふるまいを保護する
 *
 * 添字 2 個（panel_row / panel_col）から座標 6 個（row_min / row_max /
 * col_min / col_max / row_prt / col_prt）が決まる。決まりかたを知る場所が
 * 分かれていた。
 *   misc1.c:153-160  6 個を導出する（panel_bounds）
 *   misc1.c:165-200  添字を動かすかどうか決める（get_panel）
 *   io.c:134,143     row_prt / col_prt を引いて画面座標にする
 *   misc1.c:489,494 / spells.c:55,244 ほか  4 辺の間を歩く
 *   generate.c:1260-1263  4 個を導出せずに手で 0 にする
 *   generate.c:1274-1277, 1282-1285  パネル数の式（同じ 4 行が 2 回）
 * src/panel.c に寄せる。
 *
 * 保護のとりかた。導出も判定も本体の関数の中に直に書かれていて、片方
 * （get_panel）は end_find() の副作用つきで外から呼びにくい。#7 / #18-1 /
 * #18-2 と同じ「テスト側に写す」方式で、変更前の misc1.c / generate.c の
 * 計算をこのファイルに写しとり（legacy_*）、新しい実体と突きあわせる。
 *
 * 突きあわせは総当たり。ダンジョン全域の外側 4 マスまで、パネルの初期位置
 * （「まだ見ていない」を表す -1 と 0 埋めも含む）、force の両方について、
 * 動いたかどうかと座標 6 個すべてを比べる。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "panel.h"

#include "minunit.h"

#include <stdio.h>

/* 本体のグローバル（variable.c）。ステップ C で panel.c の static になる。
 * それまでは写しと実体が同じ場所を見ていなければ比べる意味がない。 */
extern int16_t max_panel_rows, max_panel_cols;
extern int panel_row, panel_col;
extern int panel_row_min, panel_row_max;
extern int panel_col_min, panel_col_max;
extern int panel_col_prt, panel_row_prt;

/* --- 変更前の写し ------------------------------------------------------- */

/* パネルの状態そのもの。写しはこちらの上で動かし、実体はグローバルの上で
 * 動かして、結果を比べる。 */
struct panel_state {
    int row, col;
    int row_min, row_max, col_min, col_max;
    int row_prt, col_prt;
};

/* misc1.c:153-160（panel_bounds）の写し。1 と 13 は変更前のまま
 * 直に書いてある（新しい実体は PANEL_MAP_TOP_ROW / PANEL_MAP_LEFT_COL）。 */
static void legacy_bounds(struct panel_state *p)
{
    p->row_min = p->row * (SCREEN_HEIGHT / 2);
    p->row_max = p->row_min + SCREEN_HEIGHT - 1;
    p->row_prt = p->row_min - 1;
    p->col_min = p->col * (SCREEN_WIDTH / 2);
    p->col_max = p->col_min + SCREEN_WIDTH - 1;
    p->col_prt = p->col_min - 13;
}

/* misc1.c:165-200（get_panel）の写し。end_find() の副作用だけ外している
 * （呼ぶかどうかは「動いたか」で決まるので、戻り値で観測できる）。 */
static int legacy_get_panel(struct panel_state *p, int y, int x, int force)
{
    bool panel;

    int prow = p->row;
    int pcol = p->col;
    if (force || (y < p->row_min + 2) || (y > p->row_max - 2)) {
        prow = ((y - SCREEN_HEIGHT / 4) / (SCREEN_HEIGHT / 2));
        if (prow > max_panel_rows) {
            prow = max_panel_rows;
        } else if (prow < 0) {
            prow = 0;
        }
    }
    if (force || (x < p->col_min + 3) || (x > p->col_max - 3)) {
        pcol = ((x - SCREEN_WIDTH / 4) / (SCREEN_WIDTH / 2));
        if (pcol > max_panel_cols) {
            pcol = max_panel_cols;
        } else if (pcol < 0) {
            pcol = 0;
        }
    }
    if ((prow != p->row) || (pcol != p->col)) {
        p->row = prow;
        p->col = pcol;
        legacy_bounds(p);
        panel = true;
    } else {
        panel = false;
    }
    return panel;
}

/* --- 総当たりの下ごしらえ ---------------------------------------------- */

static void load(const struct panel_state *p)
{
    panel_row = p->row;
    panel_col = p->col;
    panel_row_min = p->row_min;
    panel_row_max = p->row_max;
    panel_col_min = p->col_min;
    panel_col_max = p->col_max;
    panel_row_prt = p->row_prt;
    panel_col_prt = p->col_prt;
}

static bool same(const struct panel_state *p)
{
    return panel_row == p->row && panel_col == p->col &&
           panel_row_min == p->row_min && panel_row_max == p->row_max &&
           panel_col_min == p->col_min && panel_col_max == p->col_max &&
           panel_row_prt == p->row_prt && panel_col_prt == p->col_prt;
}

/* 出発点にする状態。まともなパネル位置のほか、「まだ見ていない」を表す
 * 2 つ（dungeon.c の -1、generate_cave() の 0 埋め）も混ぜる。 */
static int starting_states(struct panel_state *out)
{
    int n = 0;

    for (int row = 0; row <= 4; row++) {
        for (int col = 0; col <= 4; col++) {
            struct panel_state p = {row, col, 0, 0, 0, 0, 0, 0};

            legacy_bounds(&p);
            out[n++] = p;
        }
    }

    struct panel_state forgotten_position = {-1, -1, 0, 0, 0, 0, 0, 0};
    legacy_bounds(&forgotten_position);
    out[n++] = forgotten_position;

    /* generate_cave() の形。添字は最終パネル、4 辺は手で 0。 */
    struct panel_state forgotten_bounds = {4, 4, 0, 0, 0, 0, -1, -13};
    out[n++] = forgotten_bounds;

    /* 両方を忘れた形。dungeon() が generate_cave() のあとで -1 を入れる。 */
    struct panel_state forgotten_both = {-1, -1, 0, 0, 0, 0, -1, -13};
    out[n++] = forgotten_both;

    return n;
}

/* --- テスト ------------------------------------------------------------- */

TEST(a_panel_moves_exactly_where_the_original_moved_it)
{
    struct panel_state starts[32];
    int start_count = starting_states(starts);
    int mismatch = -1;

    /* ダンジョンは 66 x 198。パネル数はその大きさから決まる 4 x 4。 */
    panel_set_max_indexes(4, 4);

    /* 外側 4 マスまで含める。map_area() は 4 辺から randint(10) / randint(20)
     * ぶん外へ出た座標を作るので、範囲外を渡されることがある。 */
    for (int s = 0; s < start_count && mismatch < 0; s++) {
        for (int y = -4; y < MAX_HEIGHT + 4 && mismatch < 0; y++) {
            for (int x = -4; x < MAX_WIDTH + 4; x++) {
                for (int force = 0; force <= 1; force++) {
                    struct panel_state expected = starts[s];
                    int legacy = legacy_get_panel(&expected, y, x, force);

                    load(&starts[s]);
                    bool moved = panel_move_to(y, x, force != 0);

                    if (moved != (legacy != 0) || !same(&expected)) {
                        mismatch = y;
                        break;
                    }
                }
                if (mismatch >= 0) {
                    break;
                }
            }
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(a_town_sized_dungeon_has_a_single_panel)
{
    struct panel_state starts[32];
    int start_count = starting_states(starts);
    int mismatch = -1;

    /* 町は 1 パネルぶんの大きさしかない（max_panel_* が 0）。上の総当たりは
     * ダンジョン側の 4 なので、丸めの向きが逆になる町も見る。 */
    panel_set_max_indexes(0, 0);

    for (int s = 0; s < start_count && mismatch < 0; s++) {
        for (int y = -4; y < SCREEN_HEIGHT + 4 && mismatch < 0; y++) {
            for (int x = -4; x < SCREEN_WIDTH + 4; x++) {
                for (int force = 0; force <= 1; force++) {
                    struct panel_state expected = starts[s];
                    int legacy = legacy_get_panel(&expected, y, x, force);

                    load(&starts[s]);
                    bool moved = panel_move_to(y, x, force != 0);

                    if (moved != (legacy != 0) || !same(&expected)) {
                        mismatch = y;
                        break;
                    }
                }
                if (mismatch >= 0) {
                    break;
                }
            }
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(the_four_edges_are_the_ones_the_original_walked_between)
{
    /* prt_map() や map_area() は row_min..row_max、col_min..col_max を
     * そのまま for の両端に使う。窓の高さ・幅がずれると地図の描画が
     * 欠けるか、はみ出す。 */
    struct panel_state expected = {2, 3, 0, 0, 0, 0, 0, 0};

    legacy_bounds(&expected);
    load(&expected);
    ASSERT_TRUE(panel_top_row() == expected.row_min &&
                panel_bottom_row() == expected.row_max &&
                panel_left_col() == expected.col_min &&
                panel_right_col() == expected.col_max);
}

TEST(the_window_is_one_screen_high_and_one_screen_wide)
{
    struct panel_state p = {2, 3, 0, 0, 0, 0, 0, 0};

    legacy_bounds(&p);
    load(&p);
    ASSERT_TRUE(panel_bottom_row() - panel_top_row() == SCREEN_HEIGHT - 1 &&
                panel_right_col() - panel_left_col() == SCREEN_WIDTH - 1);
}

TEST(containment_matches_the_original_over_the_whole_dungeon)
{
    struct panel_state p = {2, 3, 0, 0, 0, 0, 0, 0};
    int mismatch = -1;

    legacy_bounds(&p);
    load(&p);

    for (int y = -4; y < MAX_HEIGHT + 4 && mismatch < 0; y++) {
        for (int x = -4; x < MAX_WIDTH + 4; x++) {
            /* misc1.c:204-211 の写し。 */
            bool legacy = (y >= p.row_min) && (y <= p.row_max) && (x >= p.col_min) && (x <= p.col_max);

            if (panel_contains(y, x) != legacy) {
                mismatch = y;
                break;
            }
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(dungeon_coordinates_land_where_the_original_printed_them)
{
    struct panel_state p = {2, 3, 0, 0, 0, 0, 0, 0};
    int mismatch = -1;

    legacy_bounds(&p);
    load(&p);

    for (int y = 0; y < MAX_HEIGHT && mismatch < 0; y++) {
        for (int x = 0; x < MAX_WIDTH; x++) {
            /* io.c:134-135 の写し。 */
            if (panel_screen_row(y) != y - p.row_prt || panel_screen_col(x) != x - p.col_prt) {
                mismatch = y;
                break;
            }
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(the_top_left_of_the_window_is_where_prt_map_starts_drawing)
{
    /* row_prt / col_prt が引き算のためだけの値だという証拠。窓の左上隅が
     * 地図の左上隅（メッセージ行の下、状態欄の右）に来る。prt_map() 側は
     * 同じ 2 つの定数を使う。 */
    /* ここだけは写しから状態を流しこまず、module 自身に決めさせる
     * （引き算の相手を module が導出しているので、写しを載せると
     * 導出のほうを見ないまま通ってしまう）。 */
    panel_set_dungeon_size(MAX_HEIGHT, MAX_WIDTH);
    panel_forget_position();
    (void)panel_move_to(MAX_HEIGHT / 2, MAX_WIDTH / 2, false);

    ASSERT_TRUE(panel_screen_row(panel_top_row()) == PANEL_MAP_TOP_ROW &&
                panel_screen_col(panel_left_col()) == PANEL_MAP_LEFT_COL);
}

TEST(the_dungeon_size_decides_how_many_panels_there_are)
{
    int mismatch = -1;

    /* generate.c:1274-1277 / 1282-1285 の写し（同じ 4 行が 2 回あった）。
     * 町（22 x 66）とダンジョン（66 x 198）の両方で突きあわせる。 */
    const int sizes[2][2] = {{SCREEN_HEIGHT, SCREEN_WIDTH}, {MAX_HEIGHT, MAX_WIDTH}};

    for (int i = 0; i < 2; i++) {
        int legacy_max_rows = (sizes[i][0] / SCREEN_HEIGHT) * 2 - 2;
        int legacy_max_cols = (sizes[i][1] / SCREEN_WIDTH) * 2 - 2;

        panel_set_dungeon_size(sizes[i][0], sizes[i][1]);
        if (panel_max_row_index() != legacy_max_rows || panel_max_col_index() != legacy_max_cols ||
            panel_row_index() != legacy_max_rows || panel_col_index() != legacy_max_cols) {
            mismatch = i;
            break;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(the_panel_counts_survive_a_save_file_round_trip)
{
    /* save.c は大きさではなく数そのものを書く。読みこんだ値がそのまま
     * 戻ることを確かめる（変更前は (uint16_t *) のキャストで直に読んでいた）。 */
    panel_set_dungeon_size(MAX_HEIGHT, MAX_WIDTH);
    int saved_rows = panel_max_row_index();
    int saved_cols = panel_max_col_index();

    panel_set_dungeon_size(SCREEN_HEIGHT, SCREEN_WIDTH);
    panel_set_max_indexes(saved_rows, saved_cols);
    ASSERT_TRUE(panel_max_row_index() == saved_rows && panel_max_col_index() == saved_cols);
}

TEST(forgetting_the_position_puts_the_indexes_outside_the_valid_range)
{
    /* dungeon.c:65 の -1。次の panel_move_to() が必ず「動いた」を返し、
     * 地図が引きなおされることがこの値の役目。 */
    panel_forget_position();
    ASSERT_TRUE(panel_row_index() < 0 && panel_col_index() < 0);
}

TEST(forgetting_the_bounds_makes_every_point_look_off_screen)
{
    /* generate_cave() の 0 埋め。4 辺が 1 点（0, 0）に潰れるので、ダンジョン
     * の中のどこにいても「窓の外」と判定され、次の panel_move_to() が
     * 4 辺を引きなおす。 */
    panel_forget_bounds();
    ASSERT_TRUE(!panel_contains(MAX_HEIGHT / 2, MAX_WIDTH / 2));
}

int main(void)
{
    RUN_TEST(a_panel_moves_exactly_where_the_original_moved_it);
    RUN_TEST(a_town_sized_dungeon_has_a_single_panel);
    RUN_TEST(the_four_edges_are_the_ones_the_original_walked_between);
    RUN_TEST(the_window_is_one_screen_high_and_one_screen_wide);
    RUN_TEST(containment_matches_the_original_over_the_whole_dungeon);
    RUN_TEST(dungeon_coordinates_land_where_the_original_printed_them);
    RUN_TEST(the_top_left_of_the_window_is_where_prt_map_starts_drawing);
    RUN_TEST(the_dungeon_size_decides_how_many_panels_there_are);
    RUN_TEST(the_panel_counts_survive_a_save_file_round_trip);
    RUN_TEST(forgetting_the_position_puts_the_indexes_outside_the_valid_range);
    RUN_TEST(forgetting_the_bounds_makes_every_point_look_off_screen);
    return TEST_SUMMARY();
}
