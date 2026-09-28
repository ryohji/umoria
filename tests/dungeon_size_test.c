// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「この階の広さ」のテスト -- 現在のふるまいを保護する
 *
 * ダンジョンとその中身から出す 5 つめの問い（#18-14-5）。もとは
 * variable.c:67 の `int16_t cur_height, cur_width;` 1 行で、
 * 8 ファイル 62 参照（cur_height 30・cur_width 32）。
 *
 * 押さえたいことの重心は 3 つ:
 *   **①この対は 2 つの値しか取らない** —— 町は 22 x 66（ちょうど 1 画面）、
 *     ダンジョンの階は 66 x 198（9 画面）。選んでいるのは generate_cave() の
 *     `if (dun_level == 0)` 1 か所だけで、**つまりこの対は「町にいるか」の
 *     言いかえ**（それが次の 6 問めの主題）。階の途中で広さは変わらない。
 *   **②2 つの名前で 1 つの行い** —— 書き手は 2 か所（generate.c の 2 対と
 *     セーブの読みもどし）で、どちらも**必ず両方**を書く。読み手 48 のうち
 *     40 が高さと幅を対で読む。片方だけ書けてしまうと誰も気づけない
 *     （#18-12-18 の pac/ptoac で見た穴）ので、**窓口は両方を取る 1 本**に
 *     して、半分だけ変える道を無くした。
 *   **③窓口は何も導かない・何も検めない** —— 幅を高さから計算したり、
 *     ありえない値をはじいたりしない。セーブファイルから来た数を
 *     そのまま置くのが読みもどしの道なので、検める側に回ると復元が壊れる。
 *
 * セーブファイルには short 2 つで出る。読みもどした値は**本当に使われる**
 * （階の形そのもの。復元では generate_cave() を通らない）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に広さを置きなおす。
 */
/* externs.h は要らない。窓口 3 本と、22/66/198 のための constant.h と、
 * セーブの往復を写すための <stdint.h> だけ。 */
#include <stdint.h>

#include "config.h"
#include "constant.h"

#include "dungeon_size.h"

#include "minunit.h"

/* 町に出たところ（generate_cave() の `dun_level == 0` の側と同じ 2 つ）。 */
static void given_the_town(void) { set_dungeon_size(SCREEN_HEIGHT, SCREEN_WIDTH); }

/* ダンジョンの階に降りたところ（同じ関数のもう片側）。 */
static void given_a_dungeon_level(void) { set_dungeon_size(MAX_HEIGHT, MAX_WIDTH); }

/* ------------------------------------------------------------------
 * 走りだし -- 置き場の初期値そのもの
 * ------------------------------------------------------------------ */

/* **この 1 件だけは足場を呼ばない。** 呼ぶと広さが入ってしまい、**置き場の
 * 初期値（ゼロ初期化）が観測できなくなる** —— 初期値を 5 に変えても
 * この 1 件しか赤にならない。だから main() の**いちばん最初**に置いてあり、
 * ここより前に窓口を呼ぶ件を足してはいけない（#18-14-1〜4 と同じ作法。
 * 所見 52）。
 *
 * **0 x 0 の階はゲームの中では起こらない** —— 最初の手番の前に必ず
 * generate_cave() が通るか、セーブファイルから広さが入る。押さえているのは
 * **置き場そのもの**で、ここが 0 でなくなると「まだ階が無い」と「広さ 0 の
 * 階がある」の区別が付かなくなる。 */
TEST(the_size_is_zero_before_any_level_is_made) {
    ASSERT_EQ_INT(0, dungeon_height());
    ASSERT_EQ_INT(0, dungeon_width());
}

/* ------------------------------------------------------------------
 * ①ゲームが作る 2 つの広さ
 * ------------------------------------------------------------------ */

/* **町はちょうど 1 画面。** だから町では画面が動かない。 */
TEST(the_town_is_exactly_one_panel) {
    given_the_town();

    ASSERT_EQ_INT(22, dungeon_height());
    ASSERT_EQ_INT(66, dungeon_width());
    ASSERT_EQ_INT(1, dungeon_height() / SCREEN_HEIGHT);
    ASSERT_EQ_INT(1, dungeon_width() / SCREEN_WIDTH);
}

/* **ダンジョンの階は 3 x 3 画面。** */
TEST(a_dungeon_level_is_nine_panels) {
    given_a_dungeon_level();

    ASSERT_EQ_INT(66, dungeon_height());
    ASSERT_EQ_INT(198, dungeon_width());
    ASSERT_EQ_INT(3, dungeon_height() / SCREEN_HEIGHT);
    ASSERT_EQ_INT(3, dungeon_width() / SCREEN_WIDTH);
}

/* 階は町より縦にも横にも広い（どちらの窓口も同じ側を返していないこと）。 */
TEST(a_dungeon_level_is_larger_than_the_town) {
    given_the_town();
    const int town_height = dungeon_height();
    const int town_width = dungeon_width();

    given_a_dungeon_level();

    ASSERT_TRUE(dungeon_height() > town_height);
    ASSERT_TRUE(dungeon_width() > town_width);
    ASSERT_TRUE(dungeon_width() > dungeon_height());
}

/* **generate.c が部屋を並べるのに使う割り算。** 高さと幅を入れかえると
 * 6 x 6 ではなく 18 x 2 になる（2 * (198 / 22) と 2 * (66 / 66)）。 */
TEST(the_room_grid_comes_from_the_panel_counts) {
    given_a_dungeon_level();

    ASSERT_EQ_INT(6, 2 * (dungeon_height() / SCREEN_HEIGHT));
    ASSERT_EQ_INT(6, 2 * (dungeon_width() / SCREEN_WIDTH));

    given_the_town();

    ASSERT_EQ_INT(2, 2 * (dungeon_height() / SCREEN_HEIGHT));
    ASSERT_EQ_INT(2, 2 * (dungeon_width() / SCREEN_WIDTH));
}

/* ------------------------------------------------------------------
 * ②2 つの名前で 1 つの行い
 * ------------------------------------------------------------------ */

/* **順番は高さが先。** 窓口の 2 つの代入を入れかえるとここが赤になる
 * （ゲームの 2 つの広さはどちらも幅 > 高さなので、それだけでは気づけない）。 */
TEST(the_height_is_the_first_argument) {
    set_dungeon_size(5, 7);

    ASSERT_EQ_INT(5, dungeon_height());
    ASSERT_EQ_INT(7, dungeon_width());
}

/* **新しい階の広さは前の階の広さを消す。** 片方を書かない窓口だと、
 * 町からダンジョンに降りたときに高さだけ 22 のまま残る。 */
TEST(a_new_level_replaces_the_whole_size) {
    given_a_dungeon_level();

    given_the_town();

    ASSERT_EQ_INT(SCREEN_HEIGHT, dungeon_height());
    ASSERT_EQ_INT(SCREEN_WIDTH, dungeon_width());
}

/* 訊いても答えは変わらない（窓口は読むだけ）。 */
TEST(asking_does_not_change_the_answer) {
    given_a_dungeon_level();

    ASSERT_EQ_INT(dungeon_height(), dungeon_height());
    ASSERT_EQ_INT(dungeon_width(), dungeon_width());
    ASSERT_EQ_INT(MAX_HEIGHT, dungeon_height());
    ASSERT_EQ_INT(MAX_WIDTH, dungeon_width());
}

/* ------------------------------------------------------------------
 * ③何も導かない・何も検めない
 * ------------------------------------------------------------------ */

/* **2 つは独立した数。** ゲームが作らない形（1 画面ぶんの高さに
 * 3 画面ぶんの幅）もそのまま置ける —— 幅を高さから導く窓口にすると
 * ここが赤になる。 */
TEST(the_two_numbers_are_independent) {
    set_dungeon_size(SCREEN_HEIGHT, MAX_WIDTH);

    ASSERT_EQ_INT(SCREEN_HEIGHT, dungeon_height());
    ASSERT_EQ_INT(MAX_WIDTH, dungeon_width());
}

/* **ありえない広さもはじかない。** セーブファイルの中身をそのまま置くのが
 * 読みもどしの道なので、検める側に回ると復元が壊れる。 */
TEST(the_window_does_not_reject_an_impossible_size) {
    given_a_dungeon_level();

    set_dungeon_size(0, 0);

    ASSERT_EQ_INT(0, dungeon_height());
    ASSERT_EQ_INT(0, dungeon_width());
}

/* ------------------------------------------------------------------
 * セーブファイルの往復
 * ------------------------------------------------------------------ */

/* **short で足りる。** save.c は `wr_short((uint16_t)cur_height)` で書き、
 * 読みもどしは 16 ビットをそのまま置きなおす。いちばん広い階でも
 * 198 なので、書いて読んだ値は元と同じ。 */
TEST(the_biggest_level_goes_through_a_short_unchanged) {
    given_a_dungeon_level();

    /* save.c の書きと読みをそのまま写す。 */
    const uint16_t written_height = (uint16_t)dungeon_height();
    const uint16_t written_width = (uint16_t)dungeon_width();
    set_dungeon_size(0, 0);
    set_dungeon_size((int16_t)written_height, (int16_t)written_width);

    ASSERT_EQ_INT(MAX_HEIGHT, dungeon_height());
    ASSERT_EQ_INT(MAX_WIDTH, dungeon_width());
}

/* **復元では generate_cave() を通らない** ので、ファイルから入れた広さが
 * そのまま階の形になる（#18-14-3 の予算と違って捨てられない）。 */
TEST(a_restored_size_is_the_size_that_is_played) {
    given_the_town();

    set_dungeon_size(MAX_HEIGHT, MAX_WIDTH);

    ASSERT_EQ_INT(MAX_HEIGHT, dungeon_height());
    ASSERT_EQ_INT(MAX_WIDTH, dungeon_width());
}

/* ------------------------------------------------------------------
 * 読み手が当てにしている境
 * ------------------------------------------------------------------ */

/* **階の内側は外周の壁を除いた 0 < y < 高さ - 1。** misc1.c の in_bounds()
 * が読む形をそのまま置いてある —— 窓口が 1 つずれると、いちばん外の
 * 壁の環が内側に数えられる。66 x 198 の階なら内側は行 1〜64・列 1〜196 で、
 * 行 65・列 197 は地図の上にはあるが内側ではない。 */
TEST(the_inside_of_a_level_excludes_the_boundary_ring) {
    given_a_dungeon_level();

    ASSERT_TRUE(1 > 0 && 1 < dungeon_height() - 1);
    ASSERT_TRUE(64 > 0 && 64 < dungeon_height() - 1);
    ASSERT_FALSE(65 < dungeon_height() - 1);
    ASSERT_TRUE(196 < dungeon_width() - 1);
    ASSERT_FALSE(197 < dungeon_width() - 1);
    ASSERT_TRUE(197 < dungeon_width());
}

int main(void) {
    /* 足場を呼ばない 1 件。**この行より前に何も足さないこと**（上の註）。 */
    RUN_TEST(the_size_is_zero_before_any_level_is_made);

    RUN_TEST(the_town_is_exactly_one_panel);
    RUN_TEST(a_dungeon_level_is_nine_panels);
    RUN_TEST(a_dungeon_level_is_larger_than_the_town);
    RUN_TEST(the_room_grid_comes_from_the_panel_counts);

    RUN_TEST(the_height_is_the_first_argument);
    RUN_TEST(a_new_level_replaces_the_whole_size);
    RUN_TEST(asking_does_not_change_the_answer);

    RUN_TEST(the_two_numbers_are_independent);
    RUN_TEST(the_window_does_not_reject_an_impossible_size);

    RUN_TEST(the_biggest_level_goes_through_a_short_unchanged);
    RUN_TEST(a_restored_size_is_the_size_that_is_played);

    RUN_TEST(the_inside_of_a_level_excludes_the_boundary_ring);

    return TEST_SUMMARY();
}
