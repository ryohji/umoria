// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 飛び道具の通し番号のテスト -- 現在のふるまいを保護する
 *
 * ダンジョンで作られた矢や石の束は、1 つずつ違う番号を p1 に持つ
 * （item_enchant.c:816-820）。その番号を配るのがこの module で、番号そのものには
 * 意味が無い —— items_can_stack()（inven_ops.c:130）が「p1 が同じかどうか」だけを
 * 見て、別の場所で拾った同じ矢を混ぜないために使う。数でも順序でもない。
 *
 * 保護したい性質は 3 つ。
 *
 *   1. **配る番号が毎回ちがう**こと。同じ番号を 2 度配ると、別の場所で拾った
 *      2 つの束が 1 つに混ざる（それはふるまいの変更になる）。
 *
 *   2. **上限で int16 の下端に折りかえす**こと。変更前は `++` ではなく
 *      「MAX_SHORT なら -MAX_SHORT - 1、でなければ ++」と書かれていた。
 *      p1 は int16_t なので、上限を越えた値は入らない。ここは変更前の字面を
 *      そのまま持ってきている。
 *
 *   3. **セーブから戻した値の続きから配る**こと。save.c は番号をそのまま
 *      書いて読みなおすので、生の窓口が覚えた値を返さないと、再開した
 *      ゲームで前の束と同じ番号が出る。
 *
 * 走りだしは 0（variable.c:64 に = 0 と書いてある）。「先に 1 つ進めてから
 * 配る」順なので、**最初に配られる番号は 1**。
 *
 * テストは 1 プロセスで状態を共有する。走りだしの状態を見る 1 件は
 * main() の先頭に置いてある。
 */
/* externs.h は要らない。窓口と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "missile_serial.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが番号を進めるので。 */

TEST(the_first_batch_is_stamped_one) {
    ASSERT_EQ_INT(next_missile_serial(), 1);
}

/* --- 番号を配る --------------------------------------------------------- */

TEST(the_next_batch_gets_the_next_number) {
    set_missile_serial(41);
    ASSERT_EQ_INT(next_missile_serial(), 42);
}

/* この module がある理由そのもの。同じ番号を 2 度配ったら、別の場所で拾った
 * 2 つの束が混ざってしまう。 */
TEST(two_batches_never_get_the_same_number) {
    set_missile_serial(100);
    int16_t first = next_missile_serial();
    ASSERT_TRUE(first != next_missile_serial());
}

/* 配った番号は覚えられている（2 つの窓口が同じ置き場を見ている）。 */
TEST(the_number_just_handed_out_is_the_one_remembered) {
    set_missile_serial(7);
    int16_t handed_out = next_missile_serial();
    ASSERT_EQ_INT(missile_serial_value(), handed_out);
}

/* --- 上限での折りかえし ------------------------------------------------- */

/* 変更前の字面どおり。`++` だと int16_t に入らない値になる。 */
TEST(the_number_after_the_top_is_the_bottom) {
    set_missile_serial(MAX_SHORT);
    ASSERT_EQ_INT(next_missile_serial(), -MAX_SHORT - 1);
}

/* 上限の 1 つ手前では折りかえさない（境界の反対側）。 */
TEST(the_number_just_below_the_top_still_goes_up) {
    set_missile_serial(MAX_SHORT - 1);
    ASSERT_EQ_INT(next_missile_serial(), MAX_SHORT);
}

/* 折りかえした直後も 1 つずつ進む。負の番号でも p1 の比較には足りる。 */
TEST(counting_carries_on_after_the_wrap) {
    set_missile_serial(MAX_SHORT);
    (void)next_missile_serial();
    ASSERT_EQ_INT(next_missile_serial(), -MAX_SHORT);
}

/* --- セーブから戻した値 ------------------------------------------------- */

/* 生の窓口は覚えた値をそのまま返す（save.c が書いて読みなおす経路）。 */
TEST(the_counter_remembers_what_was_put_in_it) {
    set_missile_serial(12345);
    ASSERT_EQ_INT(missile_serial_value(), 12345);
}

/* 再開したゲームは前の続きから配る。ここが効かないと、セーブ前の束と
 * 同じ番号が出る。 */
TEST(a_restored_game_carries_on_from_where_it_stopped) {
    set_missile_serial(12345);
    ASSERT_EQ_INT(next_missile_serial(), 12346);
}

/* 折りかえした後の負の値もそのまま戻せる（セーブは int16 をそのまま書く）。 */
TEST(a_negative_counter_survives_being_restored) {
    set_missile_serial(-MAX_SHORT - 1);
    ASSERT_EQ_INT(missile_serial_value(), -MAX_SHORT - 1);
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが番号を進める。 */
    RUN_TEST(the_first_batch_is_stamped_one);

    RUN_TEST(the_next_batch_gets_the_next_number);
    RUN_TEST(two_batches_never_get_the_same_number);
    RUN_TEST(the_number_just_handed_out_is_the_one_remembered);

    RUN_TEST(the_number_after_the_top_is_the_bottom);
    RUN_TEST(the_number_just_below_the_top_still_goes_up);
    RUN_TEST(counting_carries_on_after_the_wrap);

    RUN_TEST(the_counter_remembers_what_was_put_in_it);
    RUN_TEST(a_restored_game_carries_on_from_where_it_stopped);
    RUN_TEST(a_negative_counter_survives_being_restored);

    return TEST_SUMMARY();
}
