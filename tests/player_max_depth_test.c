// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「どこまで潜ったか」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 15 つめの問いで、**`struct misc` から出る 1 つめ**（1 つめは
 * 持っている金、2 つめは腹の具合、3 つめは画面に出す数字、4 つめは魔力、
 * 5 つめは体力、6 つめは階級と経験値、7 つめは状態の旗、8 つめは装備で決まる
 * 耐性・能力、9 つめは一時的な状態の数えおとし、10 つめは休息の残りターン、
 * 11 つめはいまの速さ、12 つめは赤外視の届く距離、13 つめは光る手、
 * 14 つめはあと何個呪文を覚えられるか）。答えは 1 つの short —— もとは
 * py.misc.max_dlv で、3 ファイルから 8 か所が触っていた。
 *
 * **これまでの 14 問と違って、体でも腕でもなく「どこに行ったか」の記録。**
 * 単位は迷宮の階（dun_level と同じ）で、町が 0 階。
 *
 * **この数は減らない。** dungeon.c が毎ターンの初めに「いまの階のほうが深ければ
 * 書きかえる」をしていたのを窓口の内側に入れたので（→ 台帳の所見 24・25）、
 * 階段を上がっても記録は残る。**下げられる窓口は 1 本だけで、セーブファイルの
 * 読みもどし専用**（player_max_depth.h）。
 *
 * 得点の 1 階 100 点・帰還の巻物が引きずり下ろす先・セーブの位置は
 * **この module の外**（player_max_depth.h）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に記録を置きなおす
 * （置きなおしはセーブの窓口。**遊んでいる間に使ってよい窓口ではない**）。
 */
/* externs.h は要らない。窓口 3 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_max_depth.h"

#include "minunit.h"

/* 町から下りたことがない人物から始める。 */
static void given_never_left_town(void) { player_max_depth_set(0); }

/* もう何階か潜った人物から始める。 */
static void given_deepest_reached(int level) { player_max_depth_set(level); }

/* ------------------------------------------------------------------
 * 記録そのもの -- 0 は「町から下りたことがない」
 * ------------------------------------------------------------------ */

TEST(a_new_character_has_never_left_town) {
    given_never_left_town();

    ASSERT_EQ_INT(0, player_max_depth());
}

TEST(the_record_comes_back_as_it_was_left) {
    given_deepest_reached(12);

    ASSERT_EQ_INT(12, player_max_depth());
}

TEST(reading_the_record_twice_gives_the_same_answer) {
    given_deepest_reached(7);

    ASSERT_EQ_INT(7, player_max_depth());
    ASSERT_EQ_INT(7, player_max_depth());
}

/* ------------------------------------------------------------------
 * 潜ると記録が伸びる -- dungeon.c の毎ターンの 1 行
 * ------------------------------------------------------------------ */

TEST(going_below_the_town_for_the_first_time_is_recorded) {
    given_never_left_town();

    player_note_depth_reached(1);

    ASSERT_EQ_INT(1, player_max_depth());
}

TEST(going_deeper_than_ever_before_moves_the_record) {
    given_deepest_reached(5);

    player_note_depth_reached(6);

    ASSERT_EQ_INT(6, player_max_depth());
}

TEST(stepping_down_one_level_at_a_time_leaves_the_deepest) {
    given_never_left_town();

    for (int level = 1; level <= 10; level++) {
        player_note_depth_reached(level);
    }

    ASSERT_EQ_INT(10, player_max_depth());
}

/* **深くなくても呼ばれる。** dungeon.c は階の初めに毎回この窓口を通るので、
 * 同じ階で何度呼ばれても記録は動かない。 */
TEST(reaching_the_same_level_again_changes_nothing) {
    given_deepest_reached(9);

    player_note_depth_reached(9);

    ASSERT_EQ_INT(9, player_max_depth());
}

/* ------------------------------------------------------------------
 * 記録は減らない -- 比べが窓口の内側に入ったことの釘
 * ------------------------------------------------------------------ */

TEST(climbing_back_up_does_not_lower_the_record) {
    given_deepest_reached(20);

    player_note_depth_reached(19);

    ASSERT_EQ_INT(20, player_max_depth());
}

TEST(returning_to_town_does_not_erase_the_record) {
    given_deepest_reached(20);

    player_note_depth_reached(0);

    ASSERT_EQ_INT(20, player_max_depth());
}

TEST(a_long_climb_back_to_the_town_leaves_the_deepest) {
    given_deepest_reached(15);

    for (int level = 14; level >= 0; level--) {
        player_note_depth_reached(level);
    }

    ASSERT_EQ_INT(15, player_max_depth());
}

/* 帰還の巻物が見る「まだ潜っていない」は 0 かどうかだけ。0 のままであることを
 * 押さえる（引きずり下ろす先を決めるのは dungeon.c の仕事）。 */
TEST(the_scroll_of_recall_has_nowhere_to_pull_a_townsman) {
    given_never_left_town();

    player_note_depth_reached(0);

    ASSERT_EQ_INT(0, player_max_depth());
}

/* ------------------------------------------------------------------
 * セーブファイル -- 置きなおしは減る向きにも効く
 * ------------------------------------------------------------------ */

TEST(loading_a_saved_game_brings_the_record_back) {
    given_never_left_town();

    player_max_depth_set(31);

    ASSERT_EQ_INT(31, player_max_depth());
}

/* **セーブの窓口だけは下げられる。** 深い人物が遊んでいるところへ浅い人物の
 * セーブを読みこんでも、記録はそのファイルのものになる。 */
TEST(the_saved_file_may_put_back_a_shallower_record) {
    given_deepest_reached(40);

    player_max_depth_set(3);

    ASSERT_EQ_INT(3, player_max_depth());
}

TEST(the_short_that_goes_out_is_the_one_that_came_in) {
    for (int level = 0; level < 256; level++) {
        player_max_depth_set(level);

        ASSERT_EQ_INT(level, player_max_depth());
    }
}

/* 器は short なので、迷宮の階よりずっと大きい数も通る。**留めは無い** ——
 * もとのコードもこの数を範囲で検査していないので、窓口もしない
 * （→ 台帳の所見 24）。 */
TEST(the_widest_short_the_container_holds_survives) {
    given_never_left_town();

    player_max_depth_set(65535);

    ASSERT_EQ_INT(65535, player_max_depth());
}

TEST(nothing_clamps_the_record_to_the_deepest_level_that_exists) {
    given_never_left_town();

    player_note_depth_reached(1000);

    ASSERT_EQ_INT(1000, player_max_depth());
}

int main(void) {
    RUN_TEST(a_new_character_has_never_left_town);
    RUN_TEST(the_record_comes_back_as_it_was_left);
    RUN_TEST(reading_the_record_twice_gives_the_same_answer);

    RUN_TEST(going_below_the_town_for_the_first_time_is_recorded);
    RUN_TEST(going_deeper_than_ever_before_moves_the_record);
    RUN_TEST(stepping_down_one_level_at_a_time_leaves_the_deepest);
    RUN_TEST(reaching_the_same_level_again_changes_nothing);

    RUN_TEST(climbing_back_up_does_not_lower_the_record);
    RUN_TEST(returning_to_town_does_not_erase_the_record);
    RUN_TEST(a_long_climb_back_to_the_town_leaves_the_deepest);
    RUN_TEST(the_scroll_of_recall_has_nowhere_to_pull_a_townsman);

    RUN_TEST(loading_a_saved_game_brings_the_record_back);
    RUN_TEST(the_saved_file_may_put_back_a_shallower_record);
    RUN_TEST(the_short_that_goes_out_is_the_one_that_came_in);
    RUN_TEST(the_widest_short_the_container_holds_survives);
    RUN_TEST(nothing_clamps_the_record_to_the_deepest_level_that_exists);

    return TEST_SUMMARY();
}
