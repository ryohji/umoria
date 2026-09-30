// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「この階にいるモンスター」のテスト -- 現在のふるまいを保護する
 *
 * ダンジョンとその中身から出す 4 つめの問い（#18-14-4）。もとは
 * monsters.c:764 の `monster_type m_list[MAX_MALLOC];` と :778 の
 * `int16_t mfptr;` で、12 ファイル 101 参照（m_list 73・mfptr 28）。
 * **2 つの名前で 1 つの仕組み**なので 1 つの module にする ——
 * 表は**隙間なく詰めて**あり、mfptr はその詰めがどこまで届いているかを
 * 言う。だから「埋まっている行」は MIN_MONIX から mfptr - 1 までで、
 * **14 か所が mfptr から数えおろす同じループを手で書いている**。
 *
 * 押さえたいことの重心は 3 つ:
 *   **①詰めてあるという約束** —— 途中の行は空にできない。抜けた穴には
 *     末尾の行を移してきて表を 1 行縮める（monster_death.c の
 *     fix2_delete_monster）。窓口 monster_list_drop_last() が縮める側の
 *     半分で、**末尾を空にして印を 1 つ戻す**。
 *   **②走りだしの印は 0 ではなく MIN_MONIX（= 2）** —— 0 行めは
 *     「モンスターがいない」、1 行めは「プレイヤー」で cave 側の約束
 *     （どちらも配られない）。**置き場のゼロ初期化はこの約束を満たして
 *     いない**ので、階を作るときに必ず戻す 1 行がある。
 *   **③配る側は行を空にしない** —— monster_list_claim_slot() は印の
 *     指す行番号を返して印を進めるだけで、中身は呼び手が全部書く。
 *
 * セーブファイルには**印と、その数だけの行**が出る。#18-14-3 の予算と
 * 違って**こちらは読みもどした値が本当に使われる**（何行読むかを決める
 * のがこの印）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に階を始めなおす。
 */
/* externs.h は要らない。窓口 8 本と、型と MIN_MONIX / MAX_MALLOC のための
 * constant.h / types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "monster_list.h"

#include "minunit.h"

/* 新しい階に降りたところから始める（generate.c の generate_cave() が
 * 階の頭で呼ぶのと同じ 1 本）。
 * **窓口で戻しなおす** —— #18-14-4C で置き場が src/monster/monster_list.c の static に
 * 入ったので、この足場が唯一の道（#18-14-1〜3 と同じ）。 */
static void given_a_fresh_level(void) { monster_list_reset(); }

/* 行を n 個配ったところ。配った行番号は MIN_MONIX から順に出る。 */
static void given_slots_claimed(int n) {
    monster_list_reset();
    for (int i = 0; i < n; i++) {
        (void)monster_list_claim_slot();
    }
}

/* 行が空かどうか。monsters.c:776 の blank_monster と同じ 10 フィールド。 */
static bool row_is_blank(int index) {
    const monster_type *const row = monster_list_at(index);
    return row->hp == 0 && row->csleep == 0 && row->cspeed == 0 && row->fy == 0 && row->fx == 0 && row->cdis == 0 && row->ml == 0 && row->stunned == 0 && row->confused == 0;
}

/* ------------------------------------------------------------------
 * 走りだし -- 置き場の初期値そのもの
 * ------------------------------------------------------------------ */

/* **この 1 件だけは足場を呼ばない。** 呼ぶと monster_list_reset() が
 * MIN_MONIX を書いてしまい、**置き場の初期値（ゼロ初期化の印）が観測
 * できなくなる** —— 初期値を 5 に変えてもこの 1 件しか赤にならない。
 * だから main() の**いちばん最初**に置いてあり、ここより前に窓口を呼ぶ件を
 * 足してはいけない（#18-14-1〜3 と同じ作法。所見 52）。
 *
 * **ゼロの印はゲームの中では起こらない** —— 最初の階を作るときに
 * generate_cave() が必ず monster_list_reset() を通り、そこで MIN_MONIX が
 * 書かれる（もとは mlink() という包みだったが、中身が窓口 1 本になったので
 * #18-14-4B① で包みをやめた）。
 * 押さえているのは**置き場そのもの**で、**0 と MIN_MONIX の差がこの問いの
 * 芯**（0 のままだと 0 行めと 1 行めが配られ、cave 側の「いない」と
 * 「プレイヤー」に化ける）。 */
TEST(the_mark_starts_at_zero_before_any_level_is_made) {
    ASSERT_EQ_INT(0, monster_list_used());
    ASSERT_TRUE(monster_list_used() < MIN_MONIX);
}

/* ------------------------------------------------------------------
 * 階の頭 -- 印が MIN_MONIX に戻り、行が全部空になる
 * ------------------------------------------------------------------ */

/* **走りだしの印は 0 ではなく 2。** 0 行めは「モンスターがいない」、
 * 1 行めは「プレイヤー」なので配ってはいけない（上の②）。 */
TEST(a_fresh_level_starts_at_the_first_row_that_may_be_handed_out) {
    given_a_fresh_level();

    ASSERT_EQ_INT(MIN_MONIX, monster_list_used());
    ASSERT_EQ_INT(2, monster_list_used());
}

TEST(a_fresh_level_has_every_row_blank) {
    given_a_fresh_level();

    for (int i = 0; i < MAX_MALLOC; i++) {
        ASSERT_TRUE(row_is_blank(i));
    }
}

/* **書きこんだ行も階の頭で空に戻る。** ループの終端を MAX_MALLOC より
 * 手前にすると、表の末尾に前の階のモンスターが残る。 */
TEST(a_fresh_level_blanks_rows_that_were_written) {
    given_a_fresh_level();
    monster_list_at(MIN_MONIX)->hp = 77;
    monster_list_at(MAX_MALLOC - 1)->hp = 77;

    monster_list_reset();

    ASSERT_TRUE(row_is_blank(MIN_MONIX));
    ASSERT_TRUE(row_is_blank(MAX_MALLOC - 1));
}

/* ------------------------------------------------------------------
 * 行を配る -- 印の指す行を返して印を進める
 * ------------------------------------------------------------------ */

TEST(the_first_row_handed_out_is_the_first_that_may_be_used) {
    given_a_fresh_level();

    ASSERT_EQ_INT(MIN_MONIX, monster_list_claim_slot());
}

TEST(rows_are_handed_out_in_order) {
    given_a_fresh_level();

    for (int i = MIN_MONIX; i < MIN_MONIX + 10; i++) {
        ASSERT_EQ_INT(i, monster_list_claim_slot());
    }
}

/* 配ると印は**その行の 1 つ先**を指す（返った番号ではない）。 */
TEST(claiming_moves_the_mark_past_the_row_it_returns) {
    given_a_fresh_level();

    const int row = monster_list_claim_slot();

    ASSERT_EQ_INT(row + 1, monster_list_used());
}

/* **0 行めと 1 行めは配られない。** 階の頭から配りきるまで、返る番号は
 * ぜんぶ MIN_MONIX 以上。 */
TEST(rows_zero_and_one_are_never_handed_out) {
    given_a_fresh_level();

    while (!monster_list_is_full()) {
        ASSERT_TRUE(monster_list_claim_slot() >= MIN_MONIX);
    }
}

/* **配る窓口は行を空にしない** —— 中身を書くのは呼び手（place_monster）で、
 * ここで空にするとその前に書いた値が消える（上の③）。 */
TEST(claiming_does_not_blank_the_row) {
    given_a_fresh_level();
    monster_list_at(MIN_MONIX)->hp = 42;

    set_monster_list_used(MIN_MONIX);
    const int row = monster_list_claim_slot();

    ASSERT_EQ_INT(MIN_MONIX, row);
    ASSERT_EQ_INT(42, monster_list_at(row)->hp);
}

/* ------------------------------------------------------------------
 * 空きの数と満杯
 * ------------------------------------------------------------------ */

TEST(a_fresh_level_is_not_full) {
    given_a_fresh_level();

    ASSERT_FALSE(monster_list_is_full());
}

/* 階の頭の空きは表ぜんぶではなく、**配らない 2 行を引いたぶん**。 */
TEST(a_fresh_level_has_all_but_the_reserved_rows_free) {
    given_a_fresh_level();

    ASSERT_EQ_INT(MAX_MALLOC - MIN_MONIX, monster_list_free_slots());
}

TEST(each_claim_takes_one_free_slot) {
    given_a_fresh_level();

    for (int taken = 1; taken <= 10; taken++) {
        (void)monster_list_claim_slot();

        ASSERT_EQ_INT(MAX_MALLOC - MIN_MONIX - taken, monster_list_free_slots());
    }
}

/* 満杯の境目は「印が表の大きさに届いたとき」。 */
TEST(the_table_is_full_when_the_mark_reaches_the_end) {
    set_monster_list_used(MAX_MALLOC - 1);
    ASSERT_FALSE(monster_list_is_full());

    set_monster_list_used(MAX_MALLOC);
    ASSERT_TRUE(monster_list_is_full());
}

TEST(full_means_no_free_slots_are_left) {
    set_monster_list_used(MAX_MALLOC);

    ASSERT_TRUE(monster_list_is_full());
    ASSERT_EQ_INT(0, monster_list_free_slots());
}

/* 階の頭から配りきると**ちょうど MAX_MALLOC - MIN_MONIX 行**で満杯になる。 */
TEST(the_whole_table_holds_all_but_the_reserved_rows) {
    given_a_fresh_level();

    int handed_out = 0;
    while (!monster_list_is_full()) {
        (void)monster_list_claim_slot();
        handed_out++;
    }

    ASSERT_EQ_INT(MAX_MALLOC - MIN_MONIX, handed_out);
    ASSERT_EQ_INT(MAX_MALLOC, monster_list_used());
}

/* ------------------------------------------------------------------
 * 行を返す -- 末尾を空にして印を 1 つ戻す
 * ------------------------------------------------------------------ */

TEST(dropping_the_last_row_moves_the_mark_back_by_one) {
    given_slots_claimed(5);
    const int16_t before = monster_list_used();

    monster_list_drop_last();

    ASSERT_EQ_INT(before - 1, monster_list_used());
}

/* **返す窓口は末尾の行を空にする。** ここを飛ばすと、次にその行が配られた
 * ときに前のモンスターの値が混ざる（呼び手は全フィールドを書くので普段は
 * 見えないが、fix2_delete_monster が末尾を空にしている字面はこれ）。 */
TEST(dropping_the_last_row_blanks_it) {
    given_slots_claimed(5);
    const int last = monster_list_used() - 1;
    monster_list_at(last)->hp = 99;

    monster_list_drop_last();

    ASSERT_TRUE(row_is_blank(last));
}

/* 返した行は次の配りでそのまま出てくる（詰めてある約束そのもの）。 */
TEST(a_dropped_row_is_the_next_one_handed_out) {
    given_slots_claimed(5);
    const int last = monster_list_used() - 1;

    monster_list_drop_last();

    ASSERT_EQ_INT(last, monster_list_claim_slot());
}

/* 配って返してを繰りかえしても印は動かない。 */
TEST(claiming_and_dropping_leave_the_mark_where_it_was) {
    given_slots_claimed(5);
    const int16_t before = monster_list_used();

    for (int i = 0; i < 20; i++) {
        (void)monster_list_claim_slot();
        monster_list_drop_last();
    }

    ASSERT_EQ_INT(before, monster_list_used());
}

/* ------------------------------------------------------------------
 * 行の場所 -- 番号と番地が同じことを言う
 * ------------------------------------------------------------------ */

/* **行は隙間なく並んでいる** —— n 番めの次の番地が n+1 番め。
 * game_state.c が表の頭を外に渡せるのはこれが成りたつからで、
 * 窓口が行を写して返すようになったら（あるいは表を割ったら）壊れる。 */
TEST(the_row_after_one_row_is_the_next_row) {
    given_a_fresh_level();

    for (int i = 0; i < MAX_MALLOC - 1; i++) {
        ASSERT_TRUE(monster_list_at(i) + 1 == monster_list_at(i + 1));
    }
}

/* 頭は 0 行め（game_state.c が渡すもの）。 */
TEST(the_base_of_the_table_is_row_zero) {
    given_a_fresh_level();

    ASSERT_TRUE(monster_list_at(0) == monster_list_at(MIN_MONIX) - MIN_MONIX);
}

/* 同じ番号を訊けば同じ行が返る（窓口が写しを作らない）。 */
TEST(the_same_index_gives_the_same_row) {
    given_a_fresh_level();
    monster_list_at(MIN_MONIX)->hp = 13;

    ASSERT_EQ_INT(13, monster_list_at(MIN_MONIX)->hp);
    ASSERT_TRUE(monster_list_at(MIN_MONIX) == monster_list_at(MIN_MONIX));
}

/* ------------------------------------------------------------------
 * セーブファイル -- 印と、その数だけの行
 * ------------------------------------------------------------------ */

/* 入れた印がそのまま出てくる（丸めも囲いもしない）。 */
TEST(the_mark_that_goes_in_is_the_one_that_comes_out) {
    given_a_fresh_level();

    const int16_t values[] = {0, MIN_MONIX, 7, MAX_MALLOC - 1, MAX_MALLOC};
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        set_monster_list_used(values[i]);

        ASSERT_EQ_INT(values[i], monster_list_used());
    }
}

/* **戻した印は本当に効く** —— #18-14-3 の予算と違って、読みもどした数が
 * そのまま「何行が埋まっているか」になる（save.c はこの数だけ行を読む）。 */
TEST(putting_the_mark_back_decides_how_many_rows_are_filled) {
    given_slots_claimed(20);

    monster_list_reset();
    ASSERT_EQ_INT(MIN_MONIX, monster_list_used());

    set_monster_list_used(22);

    ASSERT_EQ_INT(22, monster_list_used());
    ASSERT_EQ_INT(MAX_MALLOC - 22, monster_list_free_slots());
}

/* ------------------------------------------------------------------
 * 読みは副作用を持たない
 * ------------------------------------------------------------------ */

TEST(asking_does_not_change_the_answer) {
    given_slots_claimed(3);

    for (int i = 0; i < 100; i++) {
        (void)monster_list_used();
        (void)monster_list_is_full();
        (void)monster_list_free_slots();
        (void)monster_list_at(MIN_MONIX);
    }

    ASSERT_EQ_INT(MIN_MONIX + 3, monster_list_used());
}

int main(void) {
    /* 足場を呼ばない 1 件。**この行より前に何も足さないこと**（上の註）。 */
    RUN_TEST(the_mark_starts_at_zero_before_any_level_is_made);

    RUN_TEST(a_fresh_level_starts_at_the_first_row_that_may_be_handed_out);
    RUN_TEST(a_fresh_level_has_every_row_blank);
    RUN_TEST(a_fresh_level_blanks_rows_that_were_written);

    RUN_TEST(the_first_row_handed_out_is_the_first_that_may_be_used);
    RUN_TEST(rows_are_handed_out_in_order);
    RUN_TEST(claiming_moves_the_mark_past_the_row_it_returns);
    RUN_TEST(rows_zero_and_one_are_never_handed_out);
    RUN_TEST(claiming_does_not_blank_the_row);

    RUN_TEST(a_fresh_level_is_not_full);
    RUN_TEST(a_fresh_level_has_all_but_the_reserved_rows_free);
    RUN_TEST(each_claim_takes_one_free_slot);
    RUN_TEST(the_table_is_full_when_the_mark_reaches_the_end);
    RUN_TEST(full_means_no_free_slots_are_left);
    RUN_TEST(the_whole_table_holds_all_but_the_reserved_rows);

    RUN_TEST(dropping_the_last_row_moves_the_mark_back_by_one);
    RUN_TEST(dropping_the_last_row_blanks_it);
    RUN_TEST(a_dropped_row_is_the_next_one_handed_out);
    RUN_TEST(claiming_and_dropping_leave_the_mark_where_it_was);

    RUN_TEST(the_row_after_one_row_is_the_next_row);
    RUN_TEST(the_base_of_the_table_is_row_zero);
    RUN_TEST(the_same_index_gives_the_same_row);

    RUN_TEST(the_mark_that_goes_in_is_the_one_that_comes_out);
    RUN_TEST(putting_the_mark_back_decides_how_many_rows_are_filled);

    RUN_TEST(asking_does_not_change_the_answer);

    return TEST_SUMMARY();
}
