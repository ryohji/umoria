/* プレイヤーの居場所の置き場のテスト -- 現在のふるまいを保護する
 *
 * ここに計算は無い。char_row / char_col は置き場そのもので、保護するのは
 * 「置き場としてのふるまい」に尽きる。それでも固定しておきたい性質が 3 つある。
 *
 *   1. 行と列は別々の記録だが、**書きかえは必ず 2 つそろって**起きる。本体の
 *      書きこみ 5 組はすべて行と列を続けて代入していて、片方だけ動かす箇所が
 *      1 つも無い。窓口を player_place(row, col) の 1 本にしたのはそれが理由で、
 *      ここでは「両方が入ること」「片方が他方を壊さないこと」を見る。
 *   2. 窓口は範囲を検査しない。ダンジョンの外の行・負の行がそのまま入る。
 *      変更前の代入も検査していないので、ここで検査を足すとふるまいが変わる。
 *      「検査しないこと」を明示的に固定して、あとから親切心で clamp を
 *      入れられないようにする。
 *   3. player_pos_forget() が書く -1 は**誰も読みかえさない**。generate.c が
 *      新しい階を作る前に置きなおすだけで、位置を -1 や 0 と比べる箇所は
 *      src/ に 1 つも無い。つまり -1 は「どこにもいない」という状態ではなく
 *      単なる再設定であり、テストも「-1 が入る」ことしか主張しない。
 *
 * テストは 1 プロセスで状態を共有する。走りだしの状態を見る 2 件は
 * main() の先頭に置いてある。
 */
/* externs.h は要らない。窓口と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_pos.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 2 件は main() の先頭で走らせる。ほかのテストが書きこむので。 */

TEST(the_player_starts_at_row_zero) {
    ASSERT_EQ_INT(0, player_row());
}

TEST(the_player_starts_at_column_zero) {
    ASSERT_EQ_INT(0, player_col());
}

/* --- 置き場としてのふるまい --------------------------------------------- */

TEST(placing_the_player_records_the_row) {
    player_place(12, 34);
    ASSERT_EQ_INT(12, player_row());
}

TEST(placing_the_player_records_the_column) {
    player_place(12, 34);
    ASSERT_EQ_INT(34, player_col());
}

TEST(placing_the_player_again_replaces_the_row) {
    player_place(12, 34);
    player_place(56, 78);
    ASSERT_EQ_INT(56, player_row());
}

TEST(placing_the_player_again_replaces_the_column) {
    player_place(12, 34);
    player_place(56, 78);
    ASSERT_EQ_INT(78, player_col());
}

/* 行と列が 1 つの記録に混ざっていないこと。入れかえの取りちがえも
 * ここで落ちる。 */
TEST(the_row_and_the_column_are_separate_records) {
    player_place(3, 9);
    ASSERT_TRUE(player_row() == 3 && player_col() == 9);
}

/* --- 検査しないこと ----------------------------------------------------- */

/* ダンジョンの外。cave[][] の添字にできない行だが、窓口は受けとる。 */
TEST(the_window_keeps_a_row_past_the_bottom_of_the_dungeon) {
    player_place(MAX_HEIGHT + 10, 0);
    ASSERT_EQ_INT(MAX_HEIGHT + 10, player_row());
}

TEST(the_window_keeps_a_column_past_the_right_of_the_dungeon) {
    player_place(0, MAX_WIDTH + 10);
    ASSERT_EQ_INT(MAX_WIDTH + 10, player_col());
}

TEST(the_window_keeps_a_negative_row) {
    player_place(-5, -7);
    ASSERT_EQ_INT(-5, player_row());
}

TEST(the_window_keeps_a_negative_column) {
    player_place(-5, -7);
    ASSERT_EQ_INT(-7, player_col());
}

/* int16_t の端。セーブファイルは wr_short / rd_short でこの幅のまま運ぶので、
 * 記録の幅が変わるとセーブ形式が壊れる。 */
TEST(the_row_holds_the_largest_signed_short) {
    player_place(32767, 0);
    ASSERT_EQ_INT(32767, player_row());
}

TEST(the_column_holds_the_smallest_signed_short) {
    player_place(0, -32768);
    ASSERT_EQ_INT(-32768, player_col());
}

/* 窓口の引数は int だが記録は int16_t なので、幅を越えた値は縮む。変更前の
 * 代入（char_row = y;）がやっていた縮みと同じもので、窓口が勝手に広げたり
 * 弾いたりしないことの確認。 */
TEST(the_window_does_not_widen_the_record) {
    player_place(32768, 0);
    ASSERT_EQ_INT(-32768, player_row());
}

/* --- 置きなおし --------------------------------------------------------- */

TEST(forgetting_puts_the_row_off_the_level) {
    player_place(10, 20);
    player_pos_forget();
    ASSERT_EQ_INT(-1, player_row());
}

TEST(forgetting_puts_the_column_off_the_level) {
    player_place(10, 20);
    player_pos_forget();
    ASSERT_EQ_INT(-1, player_col());
}

/* 忘れたあとも置き場としては何も変わらない（generate.c は忘れてから置く）。 */
TEST(the_player_can_be_placed_after_being_forgotten) {
    player_pos_forget();
    player_place(4, 5);
    ASSERT_TRUE(player_row() == 4 && player_col() == 5);
}

int main(void) {
    /* 走りだしの状態を見る 2 件を最初に。以降のテストが書きこむ。 */
    RUN_TEST(the_player_starts_at_row_zero);
    RUN_TEST(the_player_starts_at_column_zero);

    RUN_TEST(placing_the_player_records_the_row);
    RUN_TEST(placing_the_player_records_the_column);
    RUN_TEST(placing_the_player_again_replaces_the_row);
    RUN_TEST(placing_the_player_again_replaces_the_column);
    RUN_TEST(the_row_and_the_column_are_separate_records);

    RUN_TEST(the_window_keeps_a_row_past_the_bottom_of_the_dungeon);
    RUN_TEST(the_window_keeps_a_column_past_the_right_of_the_dungeon);
    RUN_TEST(the_window_keeps_a_negative_row);
    RUN_TEST(the_window_keeps_a_negative_column);
    RUN_TEST(the_row_holds_the_largest_signed_short);
    RUN_TEST(the_column_holds_the_smallest_signed_short);
    RUN_TEST(the_window_does_not_widen_the_record);

    RUN_TEST(forgetting_puts_the_row_off_the_level);
    RUN_TEST(forgetting_puts_the_column_off_the_level);
    RUN_TEST(the_player_can_be_placed_after_being_forgotten);

    TEST_SUMMARY();
}
