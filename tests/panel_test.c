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
/* externs.h も variable.c も要らない。10 個の値が panel.c の static に
 * なったので、ふれる先は panel.h の窓口だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "panel.h"

#include "minunit.h"

#include <stdio.h>

/* パネル数（旧 max_panel_rows / max_panel_cols）。写しの側は本体と同じ値を
 * 見ていなければ比べる意味がないので、窓口から入れた値を控えておく。 */
static int max_panel_rows, max_panel_cols;

static void set_max_indexes(int rows, int cols)
{
    max_panel_rows = rows;
    max_panel_cols = cols;
    panel_set_max_indexes(rows, cols);
}

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

/* 出発点の作りかた。生の代入はもうできないので、窓口を使って本体を目的の
 * 状態まで運ぶ。写しの側にも同じ順で同じ操作をあてる（recipe が両方の
 * 手順書）。運べる状態しか出発点にしないので、本体が実際になりうる状態
 * だけを見ていることにもなる。
 *   まともなパネル位置: 位置を忘れさせてから、そのパネルの真ん中へ動かす
 *   4 辺を忘れた形:     generate_cave() が階を作る前に呼ぶ形
 *   位置を忘れた形:     dungeon() が新しい階の頭で呼ぶ形 */
struct start_recipe {
    int row, col;
    bool forget_bounds;
    bool forget_position;
};

static void legacy_forget_bounds(struct panel_state *p)
{
    /* generate.c:1260-1263 の写し。4 辺だけを 0 にする（prt はそのまま）。 */
    p->row_min = 0;
    p->row_max = 0;
    p->col_min = 0;
    p->col_max = 0;
}

static void legacy_forget_position(struct panel_state *p)
{
    /* dungeon.c:65 の写し。 */
    p->row = -1;
    p->col = -1;
}

static struct panel_state expected_start(const struct start_recipe *r)
{
    struct panel_state p = {r->row, r->col, 0, 0, 0, 0, 0, 0};

    legacy_bounds(&p);
    if (r->forget_bounds) {
        legacy_forget_bounds(&p);
    }
    if (r->forget_position) {
        legacy_forget_position(&p);
    }
    return p;
}

static void load(const struct start_recipe *r)
{
    /* パネル row の真ん中の座標。prow = (y - 5) / 11 がちょうど row になる。 */
    panel_forget_position();
    (void)panel_move_to(r->row * (SCREEN_HEIGHT / 2) + SCREEN_HEIGHT / 4,
                        r->col * (SCREEN_WIDTH / 2) + SCREEN_WIDTH / 4, true);
    if (r->forget_bounds) {
        panel_forget_bounds();
    }
    if (r->forget_position) {
        panel_forget_position();
    }
}

/* 見えるのは窓口越しの 8 個。row_prt / col_prt は引き算の相手なので、
 * 座標 0 を画面座標に移した値（= -prt）で見る。 */
static bool same(const struct panel_state *p)
{
    return panel_row_index() == p->row && panel_col_index() == p->col &&
           panel_top_row() == p->row_min && panel_bottom_row() == p->row_max &&
           panel_left_col() == p->col_min && panel_right_col() == p->col_max &&
           panel_screen_row(0) == -p->row_prt && panel_screen_col(0) == -p->col_prt;
}

/* 出発点の一覧。まともなパネル位置すべてと、「まだ見ていない」を表す 3 通り。 */
static int starting_states(struct start_recipe *out, int max_row, int max_col)
{
    int n = 0;

    for (int row = 0; row <= max_row; row++) {
        for (int col = 0; col <= max_col; col++) {
            struct start_recipe r = {row, col, false, false};

            out[n++] = r;
        }
    }

    struct start_recipe forgotten_bounds = {max_row, max_col, true, false};
    out[n++] = forgotten_bounds;

    struct start_recipe forgotten_position = {max_row, max_col, false, true};
    out[n++] = forgotten_position;

    /* generate_cave() が 4 辺を 0 にし、そのあと dungeon() が位置を忘れる。 */
    struct start_recipe forgotten_both = {max_row, max_col, true, true};
    out[n++] = forgotten_both;

    return n;
}

/* --- テスト ------------------------------------------------------------- */

TEST(a_panel_moves_exactly_where_the_original_moved_it)
{
    struct start_recipe starts[32];
    int mismatch = -1;

    /* ダンジョンは 66 x 198。パネル数はその大きさから決まる 4 x 4。 */
    set_max_indexes(4, 4);
    int start_count = starting_states(starts, max_panel_rows, max_panel_cols);

    /* 外側 4 マスまで含める。map_area() は 4 辺から randint(10) / randint(20)
     * ぶん外へ出た座標を作るので、範囲外を渡されることがある。 */
    for (int s = 0; s < start_count && mismatch < 0; s++) {
        for (int y = -4; y < MAX_HEIGHT + 4 && mismatch < 0; y++) {
            for (int x = -4; x < MAX_WIDTH + 4; x++) {
                for (int force = 0; force <= 1; force++) {
                    struct panel_state expected = expected_start(&starts[s]);
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
    struct start_recipe starts[32];
    int mismatch = -1;

    /* 町は 1 パネルぶんの大きさしかない（max_panel_* が 0）。上の総当たりは
     * ダンジョン側の 4 なので、丸めの向きが逆になる町も見る。 */
    set_max_indexes(0, 0);
    int start_count = starting_states(starts, max_panel_rows, max_panel_cols);

    for (int s = 0; s < start_count && mismatch < 0; s++) {
        for (int y = -4; y < SCREEN_HEIGHT + 4 && mismatch < 0; y++) {
            for (int x = -4; x < SCREEN_WIDTH + 4; x++) {
                for (int force = 0; force <= 1; force++) {
                    struct panel_state expected = expected_start(&starts[s]);
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
    struct start_recipe r = {2, 3, false, false};
    struct panel_state expected = expected_start(&r);

    set_max_indexes(4, 4);
    load(&r);
    ASSERT_TRUE(panel_top_row() == expected.row_min &&
                panel_bottom_row() == expected.row_max &&
                panel_left_col() == expected.col_min &&
                panel_right_col() == expected.col_max);
}

TEST(the_window_is_one_screen_high_and_one_screen_wide)
{
    struct start_recipe r = {2, 3, false, false};

    set_max_indexes(4, 4);
    load(&r);
    ASSERT_TRUE(panel_bottom_row() - panel_top_row() == SCREEN_HEIGHT - 1 &&
                panel_right_col() - panel_left_col() == SCREEN_WIDTH - 1);
}

TEST(containment_matches_the_original_over_the_whole_dungeon)
{
    struct start_recipe r = {2, 3, false, false};
    struct panel_state p = expected_start(&r);
    int mismatch = -1;

    set_max_indexes(4, 4);
    load(&r);

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
    struct start_recipe r = {2, 3, false, false};
    struct panel_state p = expected_start(&r);
    int mismatch = -1;

    set_max_indexes(4, 4);
    load(&r);

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
    set_max_indexes(panel_max_row_index(), panel_max_col_index());
    panel_forget_position();
    (void)panel_move_to(MAX_HEIGHT / 2, MAX_WIDTH / 2, false);

    ASSERT_TRUE(panel_screen_row(panel_top_row()) == PANEL_MAP_TOP_ROW &&
                panel_screen_col(panel_left_col()) == PANEL_MAP_LEFT_COL);
}

/* prt_map() は行番号を自分で数えていた（k = 0 から始めて 1 行ずつ ++）。
 * これは print() が座標を画面へ移す歩みと同じものなので、数えるのをやめて
 * panel_screen_row() に訊く形にした。同じかどうかをここで見る。 */
TEST(the_map_rows_come_out_consecutively_from_the_top)
{
    int mismatch = -1;

    panel_set_dungeon_size(MAX_HEIGHT, MAX_WIDTH);

    for (int panel = 0; panel <= panel_max_row_index() && mismatch < 0; panel++) {
        struct start_recipe r = {panel, 0, false, false};
        int k = 0; /* 変更前の prt_map() の数えかたの写し */

        load(&r);

        for (int i = panel_top_row(); i <= panel_bottom_row(); i++) {
            k++;
            if (panel_screen_row(i) != k) {
                mismatch = panel;
                break;
            }
        }
        /* 地図は必ず SCREEN_HEIGHT 行ぶん、1 行目から。 */
        if (k != SCREEN_HEIGHT) {
            mismatch = panel;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
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
    RUN_TEST(the_map_rows_come_out_consecutively_from_the_top);
    RUN_TEST(the_dungeon_size_decides_how_many_panels_there_are);
    RUN_TEST(the_panel_counts_survive_a_save_file_round_trip);
    RUN_TEST(forgetting_the_position_puts_the_indexes_outside_the_valid_range);
    RUN_TEST(forgetting_the_bounds_makes_every_point_look_off_screen);
    return TEST_SUMMARY();
}
