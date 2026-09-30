// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「床に落ちているもの」のテスト -- 現在のふるまいを保護する
 *
 * ダンジョンとその中身から出す 7 つめの問い（#18-14-7）。もとは
 * treasure.c:553 の `inven_type t_list[MAX_TALLOC];` と :561 の
 * `int16_t tcptr;` で、17 ファイル 127 参照（t_list 113・tcptr 14）。
 * **2 つの名前で 1 つの仕組み** —— 表は隙間なく詰めてあり、tcptr はその
 * 詰めがどこまで届いているかを言う。#18-14-4 のモンスターの表とまったく
 * 同じ形で、**マスの片われ**（cave[y][x].cptr がモンスター、.tptr が
 * こちら）。
 *
 * 押さえたいことの重心は 4 つ:
 *   **①これは「宝の表」ではない** —— 扉・階段・瓦礫・罠・店の入口も
 *     この表の行で、剣と同じ invcopy() で置かれる。マスに載っている
 *     もののうち**モンスターでないものぜんぶ**が入る。
 *   **②走りだしの印は 0 ではなく MIN_TRIX（= 1）。予約は 1 行だけ** ——
 *     cave 側の tptr == 0 が「何も無い」。モンスターの表と違って
 *     「プレイヤーの行」は無い（プレイヤーは床に落ちていない）。
 *   **③行 0 は読まれる** —— moria3.c は carry() でものを拾ったすぐ後に
 *     `t_list[c_ptr->tptr].tval` を読む。delete_object() がそのマスの
 *     tptr を 0 に戻しているので、この読みは行 0 に落ちて TV_NOTHING を
 *     得る。**だから階の頭で行 0 を白紙にするのは片づけではなく仕事。**
 *   **④空の行は定数ではない** —— 定義表の 1 行（object_list[OBJ_NOTHING]）
 *     を写したもの。写す invcopy() は desc.c にあるので、テストでは
 *     tests/floor_items_fixture.c が代役を立て、「どの品目を写したか」
 *     （index の欄）だけを見る。
 *
 * セーブファイルには**印と、その数だけの行**が出る。#18-14-4 と同じで、
 * **読みもどした値が本当に使われる**（何行読むかを決めるのがこの印）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に階を始めなおす。
 */
/* externs.h は要らない。窓口 7 本と、型と MIN_TRIX / MAX_TALLOC /
 * OBJ_NOTHING のための constant.h / types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "floor_items.h"

#include "minunit.h"

/* 新しい階に降りたところから始める（generate.c の generate_cave() が
 * 階の頭で呼ぶのと同じ 1 本）。 */
static void given_a_fresh_level(void) { floor_items_reset(); }

/* 行を n 個配ったところ。配った行番号は MIN_TRIX から順に出る。 */
static void given_slots_claimed(int n) {
    floor_items_reset();
    for (int i = 0; i < n; i++) {
        (void)floor_items_claim_slot();
    }
}

/* 行が空かどうか。**空とは「nothing を写した行」** —— 欄がぜんぶ 0 では
 * なく、定義表の OBJ_NOTHING 番を写したという印が立っていること
 * （足場の代役は写した番号だけを残す。上の④）。 */
static bool row_is_blank(int index) {
    return floor_item_at(index)->index == OBJ_NOTHING;
}

/* ------------------------------------------------------------------
 * 走りだし -- 置き場の初期値そのもの
 * ------------------------------------------------------------------ */

/* **この 1 件だけは足場を呼ばない。** 呼ぶと floor_items_reset() が
 * MIN_TRIX を書いてしまい、**置き場の初期値（ゼロ初期化の印）が観測
 * できなくなる** —— 初期値を 5 に変えてもこの 1 件しか赤にならない。
 * だから main() の**いちばん最初**に置いてあり、ここより前に窓口を呼ぶ件を
 * 足してはいけない（#18-14-1〜6 と同じ作法。所見 52）。
 *
 * **ゼロの印はゲームの中では起こらない** —— 最初の階を作るときに
 * generate_cave() が必ず floor_items_reset() を通り、そこで MIN_TRIX が
 * 書かれる。押さえているのは**置き場そのもの**で、**0 と MIN_TRIX の差が
 * この問いの芯**（0 のままだと行 0 が配られ、cave 側の「何も無い」に
 * 化ける）。 */
TEST(the_mark_starts_at_zero_before_any_level_is_made) {
    ASSERT_EQ_INT(0, floor_items_used());
    ASSERT_TRUE(floor_items_used() < MIN_TRIX);
}

/* ------------------------------------------------------------------
 * 階の頭 -- 印が MIN_TRIX に戻り、行が全部空になる
 * ------------------------------------------------------------------ */

/* **走りだしの印は 0 ではなく 1。予約は 1 行だけ** —— モンスターの表は
 * 「いない」と「プレイヤー」で 2 行を取っていたが、床のものは
 * 「何も無い」の 1 行だけ（上の②）。 */
TEST(a_fresh_level_starts_at_the_first_row_that_may_be_handed_out) {
    given_a_fresh_level();

    ASSERT_EQ_INT(MIN_TRIX, floor_items_used());
    ASSERT_EQ_INT(1, floor_items_used());
}

TEST(a_fresh_level_has_every_row_blank) {
    given_a_fresh_level();

    for (int i = 0; i < MAX_TALLOC; i++) {
        ASSERT_TRUE(row_is_blank(i));
    }
}

/* **行 0 も白紙にする。** ループを MIN_TRIX から始めると、行 0 は前の階の
 * ものを持ったままになり、拾ったあとの `t_list[c_ptr->tptr]` がそれを
 * 読む（上の③）。 */
TEST(a_fresh_level_blanks_the_row_that_means_nothing_is_here) {
    given_a_fresh_level();

    ASSERT_TRUE(row_is_blank(0));
    ASSERT_EQ_INT(OBJ_NOTHING, floor_item_at(0)->index);
}

/* **書きこんだ行も階の頭で空に戻る。** ループの終端を MAX_TALLOC より
 * 手前にすると、表の末尾に前の階のものが残る。 */
TEST(a_fresh_level_blanks_rows_that_were_written) {
    given_a_fresh_level();
    floor_item_at(MIN_TRIX)->index = 77;
    floor_item_at(MAX_TALLOC - 1)->index = 77;

    floor_items_reset();

    ASSERT_TRUE(row_is_blank(MIN_TRIX));
    ASSERT_TRUE(row_is_blank(MAX_TALLOC - 1));
}

/* 途中まで配っていても階の頭で印は戻る。 */
TEST(a_fresh_level_forgets_how_far_the_last_one_was_filled) {
    given_slots_claimed(30);

    floor_items_reset();

    ASSERT_EQ_INT(MIN_TRIX, floor_items_used());
}

/* ------------------------------------------------------------------
 * 行を配る -- 印の指す行を返して印を進める
 * ------------------------------------------------------------------ */

TEST(the_first_row_handed_out_is_the_first_that_may_be_used) {
    given_a_fresh_level();

    ASSERT_EQ_INT(MIN_TRIX, floor_items_claim_slot());
}

TEST(rows_are_handed_out_in_order) {
    given_a_fresh_level();

    for (int i = MIN_TRIX; i < MIN_TRIX + 10; i++) {
        ASSERT_EQ_INT(i, floor_items_claim_slot());
    }
}

/* 配ると印は**その行の 1 つ先**を指す（返った番号ではない）。 */
TEST(claiming_moves_the_mark_past_the_row_it_returns) {
    given_a_fresh_level();

    const int row = floor_items_claim_slot();

    ASSERT_EQ_INT(row + 1, floor_items_used());
}

/* **行 0 は配られない。** 階の頭から配りきるまで、返る番号はぜんぶ
 * MIN_TRIX 以上。 */
TEST(row_zero_is_never_handed_out) {
    given_a_fresh_level();

    while (!floor_items_is_full()) {
        ASSERT_TRUE(floor_items_claim_slot() >= MIN_TRIX);
    }
}

/* **配る窓口は行を空にしない** —— 中身を書くのは呼び手（invcopy() を
 * 呼ぶ 14 か所か、行ごと写す 5 か所）で、ここで空にするとその前に
 * 書いた値が消える。 */
TEST(claiming_does_not_blank_the_row) {
    given_a_fresh_level();
    floor_item_at(MIN_TRIX)->index = 42;

    set_floor_items_used(MIN_TRIX);
    const int row = floor_items_claim_slot();

    ASSERT_EQ_INT(MIN_TRIX, row);
    ASSERT_EQ_INT(42, floor_item_at(row)->index);
}

/* ------------------------------------------------------------------
 * 満杯 -- popt() が詰めなおしを始める境目
 * ------------------------------------------------------------------ */

TEST(a_fresh_level_is_not_full) {
    given_a_fresh_level();

    ASSERT_FALSE(floor_items_is_full());
}

/* 満杯の境目は「印が表の大きさに届いたとき」。 */
TEST(the_table_is_full_when_the_mark_reaches_the_end) {
    set_floor_items_used(MAX_TALLOC - 1);
    ASSERT_FALSE(floor_items_is_full());

    set_floor_items_used(MAX_TALLOC);
    ASSERT_TRUE(floor_items_is_full());
}

/* 階の頭から配りきると**ちょうど MAX_TALLOC - MIN_TRIX 行**（174 行）で
 * 満杯になる。表の大きさ 175 と 1 つずれるのが予約の 1 行。 */
TEST(the_whole_table_holds_all_but_the_one_reserved_row) {
    given_a_fresh_level();

    int handed_out = 0;
    while (!floor_items_is_full()) {
        (void)floor_items_claim_slot();
        handed_out++;
    }

    ASSERT_EQ_INT(MAX_TALLOC - MIN_TRIX, handed_out);
    ASSERT_EQ_INT(MAX_TALLOC, floor_items_used());
}

/* ------------------------------------------------------------------
 * 行を返す -- 印を 1 つ戻して末尾を空にする
 * ------------------------------------------------------------------ */

TEST(dropping_the_last_row_moves_the_mark_back_by_one) {
    given_slots_claimed(5);
    const int16_t before = floor_items_used();

    floor_items_drop_last();

    ASSERT_EQ_INT(before - 1, floor_items_used());
}

/* **返す窓口は末尾の行を空にする。** ここを飛ばすと、行ごと写す呼び手
 * （wizard.c・inven_ops.c・moria4.c）以外は invcopy() で全部書くので普段は
 * 見えないが、pusht() が末尾を空にしている字面はこれ。 */
TEST(dropping_the_last_row_blanks_it) {
    given_slots_claimed(5);
    const int last = floor_items_used() - 1;
    floor_item_at(last)->index = 99;

    floor_items_drop_last();

    ASSERT_TRUE(row_is_blank(last));
}

/* 返した行は次の配りでそのまま出てくる（詰めてある約束そのもの）。 */
TEST(a_dropped_row_is_the_next_one_handed_out) {
    given_slots_claimed(5);
    const int last = floor_items_used() - 1;

    floor_items_drop_last();

    ASSERT_EQ_INT(last, floor_items_claim_slot());
}

/* **返すのは末尾 1 行だけ** —— その手前は触らない。 */
TEST(dropping_the_last_row_leaves_the_row_before_it_alone) {
    given_slots_claimed(5);
    const int last = floor_items_used() - 1;
    floor_item_at(last - 1)->index = 55;

    floor_items_drop_last();

    ASSERT_EQ_INT(55, floor_item_at(last - 1)->index);
}

/* 配って返してを繰りかえしても印は動かない（store_create() と
 * files.c の下見がやっているのはこれ —— 床でないものを床の行で
 * こしらえて、そのまま返す）。 */
TEST(claiming_and_dropping_leave_the_mark_where_it_was) {
    given_slots_claimed(5);
    const int16_t before = floor_items_used();

    for (int i = 0; i < 20; i++) {
        (void)floor_items_claim_slot();
        floor_items_drop_last();
    }

    ASSERT_EQ_INT(before, floor_items_used());
}

/* ------------------------------------------------------------------
 * 行の場所 -- 番号と番地が同じことを言う
 * ------------------------------------------------------------------ */

/* **行は隙間なく並んでいる** —— n 番めの次の番地が n+1 番め。
 * game_state.c が表の頭を外に渡せるのはこれが成りたつからで、
 * 窓口が行を写して返すようになったら（あるいは表を割ったら）壊れる。 */
TEST(the_row_after_one_row_is_the_next_row) {
    given_a_fresh_level();

    for (int i = 0; i < MAX_TALLOC - 1; i++) {
        ASSERT_TRUE(floor_item_at(i) + 1 == floor_item_at(i + 1));
    }
}

/* 頭は 0 行め（game_state.c が渡すもの）。 */
TEST(the_base_of_the_table_is_row_zero) {
    given_a_fresh_level();

    ASSERT_TRUE(floor_item_at(0) == floor_item_at(MIN_TRIX) - MIN_TRIX);
}

/* 同じ番号を訊けば同じ行が返る（窓口が写しを作らない）。 */
TEST(the_same_index_gives_the_same_row) {
    given_a_fresh_level();
    floor_item_at(MIN_TRIX)->index = 13;

    ASSERT_EQ_INT(13, floor_item_at(MIN_TRIX)->index);
    ASSERT_TRUE(floor_item_at(MIN_TRIX) == floor_item_at(MIN_TRIX));
}

/* ------------------------------------------------------------------
 * セーブファイル -- 印と、その数だけの行
 * ------------------------------------------------------------------ */

/* 入れた印がそのまま出てくる（丸めも囲いもしない）。save.c は
 * MAX_TALLOC を超えていたら壊れたファイルとして扱うが、その判断は
 * あちらに残す —— 窓口は検めない。 */
TEST(the_mark_that_goes_in_is_the_one_that_comes_out) {
    given_a_fresh_level();

    const int16_t values[] = {0, MIN_TRIX, 7, MAX_TALLOC - 1, MAX_TALLOC};
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        set_floor_items_used(values[i]);

        ASSERT_EQ_INT(values[i], floor_items_used());
    }
}

/* **印を置きなおすのは階の頭とは違う** —— 行の中身は触らない。
 * 読みもどしは「この数だけ行を読む」ためのもので、白紙にしてしまうと
 * 直後に読みこむ行が消える。 */
TEST(putting_the_mark_back_does_not_touch_the_rows) {
    given_slots_claimed(20);
    floor_item_at(MIN_TRIX)->index = 31;

    set_floor_items_used(MIN_TRIX);

    ASSERT_EQ_INT(31, floor_item_at(MIN_TRIX)->index);
}

/* **戻した印は本当に効く** —— 読みもどした数がそのまま「何行が
 * 埋まっているか」になる（save.c はこの数だけ行を読む）。 */
TEST(putting_the_mark_back_decides_how_many_rows_are_filled) {
    given_slots_claimed(20);

    floor_items_reset();
    ASSERT_EQ_INT(MIN_TRIX, floor_items_used());

    set_floor_items_used(21);

    ASSERT_EQ_INT(21, floor_items_used());
    ASSERT_EQ_INT(21, floor_items_claim_slot());
}

/* ------------------------------------------------------------------
 * 読みは副作用を持たない
 * ------------------------------------------------------------------ */

TEST(asking_does_not_change_the_answer) {
    given_slots_claimed(3);

    for (int i = 0; i < 100; i++) {
        (void)floor_items_used();
        (void)floor_items_is_full();
        (void)floor_item_at(MIN_TRIX);
    }

    ASSERT_EQ_INT(MIN_TRIX + 3, floor_items_used());
}

int main(void) {
    /* 足場を呼ばない 1 件。**この行より前に何も足さないこと**（上の註）。 */
    RUN_TEST(the_mark_starts_at_zero_before_any_level_is_made);

    RUN_TEST(a_fresh_level_starts_at_the_first_row_that_may_be_handed_out);
    RUN_TEST(a_fresh_level_has_every_row_blank);
    RUN_TEST(a_fresh_level_blanks_the_row_that_means_nothing_is_here);
    RUN_TEST(a_fresh_level_blanks_rows_that_were_written);
    RUN_TEST(a_fresh_level_forgets_how_far_the_last_one_was_filled);

    RUN_TEST(the_first_row_handed_out_is_the_first_that_may_be_used);
    RUN_TEST(rows_are_handed_out_in_order);
    RUN_TEST(claiming_moves_the_mark_past_the_row_it_returns);
    RUN_TEST(row_zero_is_never_handed_out);
    RUN_TEST(claiming_does_not_blank_the_row);

    RUN_TEST(a_fresh_level_is_not_full);
    RUN_TEST(the_table_is_full_when_the_mark_reaches_the_end);
    RUN_TEST(the_whole_table_holds_all_but_the_one_reserved_row);

    RUN_TEST(dropping_the_last_row_moves_the_mark_back_by_one);
    RUN_TEST(dropping_the_last_row_blanks_it);
    RUN_TEST(a_dropped_row_is_the_next_one_handed_out);
    RUN_TEST(dropping_the_last_row_leaves_the_row_before_it_alone);
    RUN_TEST(claiming_and_dropping_leave_the_mark_where_it_was);

    RUN_TEST(the_row_after_one_row_is_the_next_row);
    RUN_TEST(the_base_of_the_table_is_row_zero);
    RUN_TEST(the_same_index_gives_the_same_row);

    RUN_TEST(the_mark_that_goes_in_is_the_one_that_comes_out);
    RUN_TEST(putting_the_mark_back_does_not_touch_the_rows);
    RUN_TEST(putting_the_mark_back_decides_how_many_rows_are_filled);

    RUN_TEST(asking_does_not_change_the_answer);

    return TEST_SUMMARY();
}
