// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「床の一枚一枚」のテスト -- 現在のふるまいを保護する
 *
 * ダンジョンとその中身から出す **8 つめ、最後の問い**（#18-14-8）。もとは
 * variable.c:138 の `cave_type cave[MAX_HEIGHT][MAX_WIDTH];` で、
 * 15 ファイル 258 参照・書き 47・**別名 145**。#18 に残っていた global の
 * どれよりも大きい。
 *
 * **最後に置いた理由**は、1 マスの 7 つの欄のうち 2 つが**ほかの表への
 * 添字**だから —— `.cptr` はモンスターの表（#18-14-4）、`.tptr` は床の
 * ものの表（#18-14-7）の行番号で、あの 2 問が窓口を持つまで「1 マスを
 * 配る」が何を意味するか決まらなかった。
 *
 * 押さえたいことの重心は 4 つ:
 *   **①表は階より大きい。** 表はいつでも 66 x 198 で、町はその左上の
 *     22 x 66 に作られる。**どこまでが使われているか**は dungeon_size.c の
 *     問いで、この module の問いではない。だから窓口は階の広さを見ない。
 *   **②白紙に戻すのは表ぜんぶ。** 階の広さで止めると、次に町へ上がった
 *     ときに歩かない行に前の階の壁が残る。
 *   **③白紙のマスは「空いた床」ではない。** fval 0 は NULL_WALL ——
 *     床でも壁でもなく「まだ決めていない」印で、あとから fill_cave() が
 *     岩に変える。cptr 0 は「モンスターがいない」、tptr 0 は「何も
 *     載っていない」で、**どちらも隣の 2 つの表の行 0 と対**（あの 2 問が
 *     行 0 を白紙にするのと、この 0 は同じ約束の両端）。
 *   **④窓口は行・列の順で受ける。** `&cave[y][x]` と同じ 2 つの添字を
 *     同じ順で取る。取りちがえると別のマスが配られるが、どちらも表の中
 *     なので落ちない（だから 1 件で押さえる）。
 *
 * 窓口は 2 本しかない（reset と square_at）。**7 つの欄は 7 つの別の問い**
 * で、この module はどれにも答えない（src/dungeon_map.h の末尾に、何が
 * 何回手で書かれているかを測って置いてある）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に階を始めなおす。
 */
/* externs.h は要らない。窓口 2 本と、型と MAX_HEIGHT / MAX_WIDTH /
 * SCREEN_HEIGHT / SCREEN_WIDTH / fval の名前のための constant.h /
 * types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_map.h"

#include "minunit.h"

/* 新しい階に降りたところから始める（generate.c の generate_cave() が
 * 階の頭で呼ぶのと同じ 1 本。もとは同ファイルの static blank_cave()）。 */
static void given_a_fresh_level(void) { dungeon_map_reset(); }

/* マスがぜんぶ 0 か。**白紙とは 7 つの欄がぜんぶ 0 であること** ——
 * fval が NULL_WALL、添字が 2 つとも「無い」、明かりが 4 つとも消えている。 */
static bool square_is_blank(int y, int x) {
    const cave_type *c_ptr = square_at(y, x);
    return c_ptr->fval == 0 && c_ptr->tptr == 0 && c_ptr->cptr == 0 &&
           c_ptr->lr == 0 && c_ptr->fm == 0 && c_ptr->pl == 0 && c_ptr->tl == 0;
}

/* そのマスの 7 つの欄をぜんぶ埋める（前の階が残した状態の代わり）。 */
static void given_a_square_written(int y, int x) {
    cave_type *c_ptr = square_at(y, x);
    c_ptr->fval = GRANITE_WALL;
    c_ptr->tptr = 7;
    c_ptr->cptr = 9;
    c_ptr->lr = 1;
    c_ptr->fm = 1;
    c_ptr->pl = 1;
    c_ptr->tl = 1;
}

/* ------------------------------------------------------------------
 * 走りだし -- 置き場の初期値そのもの
 * ------------------------------------------------------------------ */

/* **この 1 件だけは階を始めなおさない。** 呼ぶと dungeon_map_reset() が表を
 * 0 で埋めてしまい、**置き場の初期値（ゼロ初期化）が観測できなくなる**。
 * だから main() の**いちばん最初**に置いてあり、ここより前に窓口を呼ぶ件を
 * 足してはいけない（#18-14-1〜7 と同じ作法。所見 52）。#18-14-8C で
 * 置き場が src/dungeon_map.c の static に入ったので、この 1 件が見ているのは
 * **本物の走りだしの値**になった（それまでは足場
 * tests/dungeon_map_fixture.c の表で、C でファイルごと消えた）。
 *
 * **ゼロの表はゲームの中では意味を持つ** —— 最初の階を作るときに
 * generate_cave() が必ず dungeon_map_reset() を通るので、白紙の表が
 * そのまま遊ばれることは無い。押さえているのは**置き場そのもの**。 */
TEST(the_table_starts_blank_before_any_level_is_made) {
    ASSERT_TRUE(square_is_blank(0, 0));
    ASSERT_TRUE(square_is_blank(MAX_HEIGHT - 1, MAX_WIDTH - 1));
}

/* ------------------------------------------------------------------
 * 表の大きさ -- 階の広さとは別のもの（重心①）
 * ------------------------------------------------------------------ */

/* **表の四隅はいつでも配られる。** 窓口は階の広さを訊かないので、町に
 * いても（22 x 66 しか使われていなくても）表の最後のマスが取れる。 */
TEST(every_square_of_the_table_can_be_asked_for) {
    given_a_fresh_level();

    ASSERT_TRUE(square_at(0, 0) != NULL);
    ASSERT_TRUE(square_at(0, MAX_WIDTH - 1) != NULL);
    ASSERT_TRUE(square_at(MAX_HEIGHT - 1, 0) != NULL);
    ASSERT_TRUE(square_at(MAX_HEIGHT - 1, MAX_WIDTH - 1) != NULL);
}

/* **町が使うのは表の左上の一部**（SCREEN_HEIGHT x SCREEN_WIDTH）。
 * この関係は dungeon_size.c 側の 2 つの値と表の大きさの取りあわせで、
 * **表のほうが必ず大きい**（generate.c が階のときだけ表いっぱいを使う）。 */
TEST(the_town_uses_only_a_corner_of_the_table) {
    ASSERT_TRUE(SCREEN_HEIGHT < MAX_HEIGHT);
    ASSERT_TRUE(SCREEN_WIDTH < MAX_WIDTH);
    ASSERT_TRUE(square_at(SCREEN_HEIGHT, SCREEN_WIDTH) !=
                square_at(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1));
}

/* **窓口は境界の輪も配る。** in_bounds()（misc1.c）は輪の内側だけを
 * 通すが、その輪に壁を置くのは generate.c の仕事なので、ここで断っては
 * いけない。 */
TEST(the_boundary_ring_is_handed_out_like_any_other_square) {
    given_a_fresh_level();

    square_at(0, 0)->fval = BOUNDARY_WALL;
    square_at(MAX_HEIGHT - 1, MAX_WIDTH - 1)->fval = BOUNDARY_WALL;

    ASSERT_EQ_INT(BOUNDARY_WALL, square_at(0, 0)->fval);
    ASSERT_EQ_INT(BOUNDARY_WALL, square_at(MAX_HEIGHT - 1, MAX_WIDTH - 1)->fval);
}

/* ------------------------------------------------------------------
 * 同じマスは同じマス -- 窓口は別名を配る（重心④）
 * ------------------------------------------------------------------ */

TEST(the_same_coordinates_give_the_same_square) {
    given_a_fresh_level();

    ASSERT_TRUE(square_at(10, 20) == square_at(10, 20));
}

TEST(different_coordinates_give_different_squares) {
    given_a_fresh_level();

    ASSERT_TRUE(square_at(10, 20) != square_at(10, 21));
    ASSERT_TRUE(square_at(10, 20) != square_at(11, 20));
}

/* **行が先、列が後。** 添字を取りちがえても表の中には収まるので落ちない
 * —— 配られるマスが違うことでしか気づけない。 */
TEST(the_first_subscript_is_the_row) {
    given_a_fresh_level();

    square_at(1, 2)->fval = CORR_FLOOR;

    ASSERT_EQ_INT(CORR_FLOOR, square_at(1, 2)->fval);
    ASSERT_EQ_INT(0, square_at(2, 1)->fval);
}

/* **窓口が配るのは書きこめる別名。** 呼び手の 5 分の 1 は欄に代入する
 * （`c_ptr->fval = ...`）ので、次に訊いたときに見えていなければならない。 */
TEST(a_field_written_through_the_window_is_read_back) {
    given_a_fresh_level();

    square_at(30, 40)->fval = MAGMA_WALL;
    square_at(30, 40)->tptr = 12;
    square_at(30, 40)->cptr = 3;
    square_at(30, 40)->pl = 1;

    ASSERT_EQ_INT(MAGMA_WALL, square_at(30, 40)->fval);
    ASSERT_EQ_INT(12, square_at(30, 40)->tptr);
    ASSERT_EQ_INT(3, square_at(30, 40)->cptr);
    ASSERT_EQ_INT(1, (int)square_at(30, 40)->pl);
}

/* 隣のマスは巻きこまれない。 */
TEST(writing_one_square_leaves_its_neighbours_alone) {
    given_a_fresh_level();

    square_at(30, 40)->fval = MAGMA_WALL;

    ASSERT_TRUE(square_is_blank(30, 39));
    ASSERT_TRUE(square_is_blank(30, 41));
    ASSERT_TRUE(square_is_blank(29, 40));
    ASSERT_TRUE(square_is_blank(31, 40));
}

/* ------------------------------------------------------------------
 * 階の頭 -- 表ぜんぶが白紙に戻る（重心②）
 * ------------------------------------------------------------------ */

TEST(a_fresh_level_blanks_a_square_that_was_written) {
    given_a_square_written(10, 10);

    dungeon_map_reset();

    ASSERT_TRUE(square_is_blank(10, 10));
}

/* **7 つの欄ぜんぶが 0 に戻る。** 欄を 1 つでも残すと、前の階の明かりや
 * 添字が新しい階に持ちこされる（fval だけ戻す変異はここで赤になる）。 */
TEST(a_fresh_level_blanks_all_seven_fields) {
    given_a_square_written(10, 10);

    dungeon_map_reset();

    const cave_type *c_ptr = square_at(10, 10);
    ASSERT_EQ_INT(0, c_ptr->fval);
    ASSERT_EQ_INT(0, c_ptr->tptr);
    ASSERT_EQ_INT(0, c_ptr->cptr);
    ASSERT_EQ_INT(0, (int)c_ptr->lr);
    ASSERT_EQ_INT(0, (int)c_ptr->fm);
    ASSERT_EQ_INT(0, (int)c_ptr->pl);
    ASSERT_EQ_INT(0, (int)c_ptr->tl);
}

/* **最初のマスも最後のマスも掃く。** 端を 1 つ落とす変異はここで赤。 */
TEST(a_fresh_level_blanks_the_first_and_last_square_of_the_table) {
    given_a_square_written(0, 0);
    given_a_square_written(MAX_HEIGHT - 1, MAX_WIDTH - 1);

    dungeon_map_reset();

    ASSERT_TRUE(square_is_blank(0, 0));
    ASSERT_TRUE(square_is_blank(MAX_HEIGHT - 1, MAX_WIDTH - 1));
}

/* **町の外も掃く（重心②の芯）。** 町は 22 x 66 なので、階の広さで
 * 止める変異（`for y < dungeon_height()`）はここだけで赤になる。 */
TEST(a_fresh_level_blanks_the_rows_the_town_does_not_use) {
    given_a_square_written(SCREEN_HEIGHT, SCREEN_WIDTH);
    given_a_square_written(MAX_HEIGHT - 1, SCREEN_WIDTH - 1);
    given_a_square_written(SCREEN_HEIGHT - 1, MAX_WIDTH - 1);

    dungeon_map_reset();

    ASSERT_TRUE(square_is_blank(SCREEN_HEIGHT, SCREEN_WIDTH));
    ASSERT_TRUE(square_is_blank(MAX_HEIGHT - 1, SCREEN_WIDTH - 1));
    ASSERT_TRUE(square_is_blank(SCREEN_HEIGHT - 1, MAX_WIDTH - 1));
}

/* 表のどのマスも残らない（1 マスずつ見る）。 */
TEST(a_fresh_level_has_every_square_blank) {
    given_a_square_written(0, 0);
    given_a_square_written(33, 99);
    given_a_square_written(MAX_HEIGHT - 1, MAX_WIDTH - 1);

    dungeon_map_reset();

    for (int y = 0; y < MAX_HEIGHT; y++) {
        for (int x = 0; x < MAX_WIDTH; x++) {
            ASSERT_TRUE(square_is_blank(y, x));
        }
    }
}

/* 2 度呼んでも同じ（階を続けて 2 つ作るときに通る道）。 */
TEST(blanking_twice_is_the_same_as_blanking_once) {
    given_a_square_written(10, 10);

    dungeon_map_reset();
    dungeon_map_reset();

    ASSERT_TRUE(square_is_blank(10, 10));
}

/* ------------------------------------------------------------------
 * 白紙のマスの意味 -- 空いた床ではない（重心③）
 * ------------------------------------------------------------------ */

/* **fval 0 は NULL_WALL で、床でも壁でもない。** 「まだ決めていない」印で、
 * fill_cave() があとから岩に変える。0 が DARK_FLOOR だと、階が作られる前の
 * 表が「歩ける床」に見えてしまう。 */
TEST(a_blank_square_is_not_decided_yet) {
    given_a_fresh_level();

    ASSERT_EQ_INT(NULL_WALL, square_at(10, 10)->fval);
    ASSERT_TRUE(square_at(10, 10)->fval != DARK_FLOOR);
    ASSERT_TRUE(square_at(10, 10)->fval != LIGHT_FLOOR);
    ASSERT_TRUE(square_at(10, 10)->fval < MIN_CAVE_WALL);
}

/* **白紙のマスには誰も立っていない。** cptr 0 はモンスターの表の行 0 で、
 * あちらの「いない」と同じ約束（#18-14-4）。1 はプレイヤーなので、
 * 0 と 1 の差がここでも効く。 */
TEST(a_blank_square_has_nobody_standing_on_it) {
    given_a_fresh_level();

    ASSERT_EQ_INT(0, square_at(10, 10)->cptr);
    ASSERT_TRUE(square_at(10, 10)->cptr < MIN_MONIX);
}

/* **白紙のマスには何も載っていない。** tptr 0 は床のものの表の行 0 で、
 * あちらの「何も無い」と同じ約束（#18-14-7）。 */
TEST(a_blank_square_has_nothing_lying_on_it) {
    given_a_fresh_level();

    ASSERT_EQ_INT(0, square_at(10, 10)->tptr);
    ASSERT_TRUE(square_at(10, 10)->tptr < MIN_TRIX);
}

/* **白紙のマスは暗くて、まだ見られていない。** 明かり 3 つと見た印が
 * 立っていると、階が作られる前から地図が見えていることになる。 */
TEST(a_blank_square_is_dark_and_unseen) {
    given_a_fresh_level();

    const cave_type *c_ptr = square_at(10, 10);
    ASSERT_FALSE(c_ptr->pl);
    ASSERT_FALSE(c_ptr->tl);
    ASSERT_FALSE(c_ptr->fm);
    ASSERT_FALSE(c_ptr->lr);
}

/* ------------------------------------------------------------------
 * 読みに副作用が無い
 * ------------------------------------------------------------------ */

TEST(asking_does_not_change_the_answer) {
    given_a_fresh_level();
    square_at(10, 10)->fval = QUARTZ_WALL;

    for (int i = 0; i < 5; i++) {
        (void)square_at(10, 10);
        (void)square_at(0, 0);
    }

    ASSERT_EQ_INT(QUARTZ_WALL, square_at(10, 10)->fval);
    ASSERT_TRUE(square_is_blank(0, 0));
}

int main(void) {
    /* 足場を呼ばない 1 件。**この行より前に何も足さないこと**（上の註）。 */
    RUN_TEST(the_table_starts_blank_before_any_level_is_made);

    RUN_TEST(every_square_of_the_table_can_be_asked_for);
    RUN_TEST(the_town_uses_only_a_corner_of_the_table);
    RUN_TEST(the_boundary_ring_is_handed_out_like_any_other_square);

    RUN_TEST(the_same_coordinates_give_the_same_square);
    RUN_TEST(different_coordinates_give_different_squares);
    RUN_TEST(the_first_subscript_is_the_row);
    RUN_TEST(a_field_written_through_the_window_is_read_back);
    RUN_TEST(writing_one_square_leaves_its_neighbours_alone);

    RUN_TEST(a_fresh_level_blanks_a_square_that_was_written);
    RUN_TEST(a_fresh_level_blanks_all_seven_fields);
    RUN_TEST(a_fresh_level_blanks_the_first_and_last_square_of_the_table);
    RUN_TEST(a_fresh_level_blanks_the_rows_the_town_does_not_use);
    RUN_TEST(a_fresh_level_has_every_square_blank);
    RUN_TEST(blanking_twice_is_the_same_as_blanking_once);

    RUN_TEST(a_blank_square_is_not_decided_yet);
    RUN_TEST(a_blank_square_has_nobody_standing_on_it);
    RUN_TEST(a_blank_square_has_nothing_lying_on_it);
    RUN_TEST(a_blank_square_is_dark_and_unseen);

    RUN_TEST(asking_does_not_change_the_answer);

    return TEST_SUMMARY();
}
