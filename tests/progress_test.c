/* 進行の置き場のテスト -- 現在のふるまいを保護する
 *
 * ここにも計算は無い。turn / 種 2 つ / wizard 2 つは置き場そのもので、
 * 保護するのは「置き場としてのふるまい」に尽きる。ただし turn には 1 つだけ
 * 意味のある性質がある。
 *
 *   走りだしは -1 で、それは「turn 番目」ではなく「キャラがいない」印。
 *   主ループ（dungeon.c:91）の最初の 1 周で -1 から 0 に上がるので、
 *   **0 は本物の turn** であり `turn > 0` と `turn >= 0` は別の条件になる。
 *
 * 本体は実際に 3 通りの比較を使い分けている（`>= 0` が save.c:471 と
 * death.c:455、`< 0` が save.c:730・878、`> 0` が signals.c:170）。窓口で
 * 1 つの述語にまとめると turn == 0 のふるまいが変わるので、まとめない。
 * その境界をここで固定しておく。符号に名前を付けるのは save_state.h の側。
 *
 * 種 2 つは uint32_t の全域が通ることを見る（save.c は rd_long でこの型の
 * まま読み書きするので、折りかえしや符号の解釈が変わるとセーブ形式が壊れる）。
 *
 * テストは 1 プロセスで状態を共有する。走りだしの状態を見るテストは
 * main() の先頭に置いてある。
 */
/* externs.h は要らない。窓口と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "progress.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 4 件は main() の先頭で走らせる。ほかのテストが書きこむので。 */

TEST(the_game_starts_before_the_first_turn) {
    ASSERT_EQ_INT(-1, progress_turn());
}

TEST(the_game_starts_with_no_color_seed) {
    ASSERT_EQ_INT(0, (int)progress_color_seed());
}

TEST(the_game_starts_with_no_town_seed) {
    ASSERT_EQ_INT(0, (int)progress_town_seed());
}

TEST(the_game_starts_outside_wizard_mode) {
    ASSERT_FALSE(progress_wizard_mode());
}

TEST(the_game_starts_with_no_wizard_request) {
    ASSERT_FALSE(progress_wizard_requested());
}

/* --- turn: 数えあげ ----------------------------------------------------- */

TEST(advancing_from_the_starting_value_reaches_turn_zero) {
    progress_set_turn(-1);
    progress_advance_turn();
    ASSERT_EQ_INT(0, progress_turn());
}

TEST(advancing_adds_exactly_one) {
    progress_set_turn(1000);
    progress_advance_turn();
    ASSERT_EQ_INT(1001, progress_turn());
}

TEST(advancing_twice_adds_exactly_two) {
    progress_set_turn(1000);
    progress_advance_turn();
    progress_advance_turn();
    ASSERT_EQ_INT(1002, progress_turn());
}

/* turn == 0 が本物の turn であること。`> 0` と `>= 0` が分かれる 1 点で、
 * 窓口が述語にまとめてはいけない理由がこれ。 */
TEST(turn_zero_is_not_negative) {
    progress_set_turn(0);
    ASSERT_TRUE(progress_turn() >= 0);
}

TEST(turn_zero_is_not_positive) {
    progress_set_turn(0);
    ASSERT_FALSE(progress_turn() > 0);
}

TEST(the_starting_value_is_negative) {
    progress_set_turn(-1);
    ASSERT_TRUE(progress_turn() < 0);
}

/* --- turn: 置き場としてのふるまい --------------------------------------- */

TEST(what_was_put_in_the_turn_counter_stays_there) {
    progress_set_turn(123456);
    ASSERT_EQ_INT(123456, progress_turn());
}

TEST(the_turn_counter_can_be_put_back_to_the_starting_value) {
    progress_set_turn(999);
    progress_set_turn(-1);
    ASSERT_EQ_INT(-1, progress_turn());
}

/* int32_t の端。セーブファイルは wr_long / rd_long でこの幅のまま運ぶ。 */
TEST(the_turn_counter_holds_the_largest_signed_value) {
    progress_set_turn(2147483647);
    ASSERT_EQ_INT(2147483647, progress_turn());
}

TEST(the_turn_counter_holds_the_smallest_signed_value) {
    progress_set_turn(-2147483647 - 1);
    ASSERT_TRUE(progress_turn() == -2147483647 - 1);
}

/* --- 種 ----------------------------------------------------------------- */

TEST(what_was_put_in_the_color_seed_stays_there) {
    progress_set_color_seed(0x12345678u);
    ASSERT_TRUE(progress_color_seed() == 0x12345678u);
}

TEST(what_was_put_in_the_town_seed_stays_there) {
    progress_set_town_seed(0x9abcdef0u);
    ASSERT_TRUE(progress_town_seed() == 0x9abcdef0u);
}

/* 符号なしの全域。ここが折りかえるとセーブファイルの往復が壊れる。 */
TEST(the_color_seed_holds_the_largest_unsigned_value) {
    progress_set_color_seed(0xFFFFFFFFu);
    ASSERT_TRUE(progress_color_seed() == 0xFFFFFFFFu);
}

TEST(the_town_seed_holds_the_largest_unsigned_value) {
    progress_set_town_seed(0xFFFFFFFFu);
    ASSERT_TRUE(progress_town_seed() == 0xFFFFFFFFu);
}

TEST(the_two_seeds_are_separate_records) {
    progress_set_color_seed(111u);
    progress_set_town_seed(222u);
    ASSERT_TRUE(progress_color_seed() == 111u && progress_town_seed() == 222u);
}

/* --- wizard ------------------------------------------------------------- */

TEST(entering_wizard_mode_is_remembered) {
    progress_set_wizard_mode(true);
    ASSERT_TRUE(progress_wizard_mode());
}

TEST(leaving_wizard_mode_is_remembered) {
    progress_set_wizard_mode(true);
    progress_set_wizard_mode(false);
    ASSERT_FALSE(progress_wizard_mode());
}

TEST(asking_for_wizard_mode_is_remembered) {
    progress_set_wizard_requested(true);
    ASSERT_TRUE(progress_wizard_requested());
}

/* 起動時は「頼んだ」が真で「入った」が偽という状態を通る（main.c:85 で頼み、
 * main.c:144 で許す）。2 つが別の記録でなければその状態が作れない。 */
TEST(asking_for_wizard_mode_does_not_by_itself_enter_it) {
    progress_set_wizard_mode(false);
    progress_set_wizard_requested(true);
    ASSERT_FALSE(progress_wizard_mode());
}

TEST(entering_wizard_mode_does_not_clear_the_request) {
    progress_set_wizard_requested(true);
    progress_set_wizard_mode(true);
    ASSERT_TRUE(progress_wizard_requested());
}

/* --- 用途をまたいで独立していること ------------------------------------- */

TEST(the_turn_counter_and_the_seeds_are_separate_records) {
    progress_set_turn(77);
    progress_set_color_seed(88u);
    progress_set_town_seed(99u);
    ASSERT_TRUE(progress_turn() == 77 && progress_color_seed() == 88u && progress_town_seed() == 99u);
}

TEST(advancing_the_turn_leaves_wizard_mode_alone) {
    progress_set_wizard_mode(true);
    progress_set_turn(5);
    progress_advance_turn();
    ASSERT_TRUE(progress_wizard_mode());
}

int main(void) {
    /* 走りだしの状態を見る 5 件を最初に。以降のテストが書きこむ。 */
    RUN_TEST(the_game_starts_before_the_first_turn);
    RUN_TEST(the_game_starts_with_no_color_seed);
    RUN_TEST(the_game_starts_with_no_town_seed);
    RUN_TEST(the_game_starts_outside_wizard_mode);
    RUN_TEST(the_game_starts_with_no_wizard_request);

    RUN_TEST(advancing_from_the_starting_value_reaches_turn_zero);
    RUN_TEST(advancing_adds_exactly_one);
    RUN_TEST(advancing_twice_adds_exactly_two);
    RUN_TEST(turn_zero_is_not_negative);
    RUN_TEST(turn_zero_is_not_positive);
    RUN_TEST(the_starting_value_is_negative);

    RUN_TEST(what_was_put_in_the_turn_counter_stays_there);
    RUN_TEST(the_turn_counter_can_be_put_back_to_the_starting_value);
    RUN_TEST(the_turn_counter_holds_the_largest_signed_value);
    RUN_TEST(the_turn_counter_holds_the_smallest_signed_value);

    RUN_TEST(what_was_put_in_the_color_seed_stays_there);
    RUN_TEST(what_was_put_in_the_town_seed_stays_there);
    RUN_TEST(the_color_seed_holds_the_largest_unsigned_value);
    RUN_TEST(the_town_seed_holds_the_largest_unsigned_value);
    RUN_TEST(the_two_seeds_are_separate_records);

    RUN_TEST(entering_wizard_mode_is_remembered);
    RUN_TEST(leaving_wizard_mode_is_remembered);
    RUN_TEST(asking_for_wizard_mode_is_remembered);
    RUN_TEST(asking_for_wizard_mode_does_not_by_itself_enter_it);
    RUN_TEST(entering_wizard_mode_does_not_clear_the_request);

    RUN_TEST(the_turn_counter_and_the_seeds_are_separate_records);
    RUN_TEST(advancing_the_turn_leaves_wizard_mode_alone);

    TEST_SUMMARY();
}
