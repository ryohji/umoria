/* レベルごとの HP 表の置き場のテスト -- 現在のふるまいを保護する
 *
 * ここにも計算は無い。表はキャラクタ作成のときに 1 度だけ振られて、あとは
 * 読まれるだけの数の並びで、保護するのは「置き場としてのふるまい」に尽きる。
 * ただし置き場としては 3 つ意味のある性質がある。
 *
 *   1 起点であること。本体は player_hp[lev - 1] と書いていた（misc3.c:1626）。
 *   窓口は lev をそのまま受けて −1 を自分の中でやるので、
 *   **hp_total_at_level(1) が表の先頭**、hp_total_at_level(40) が最後。
 *   ここがずれると、レベルが 1 つ違う HP をキャラクタに与えることになる。
 *
 *   枠が 40 個あって、それぞれ独立していること。作成は 1 つずつ順に埋めて
 *   いき（create.c:385-392）、直前の枠を足しこむので、隣を壊す置き場では
 *   表が作れない。
 *
 *   セーブファイルは表を丸ごと運ぶ（save.c の wr_shorts / rd_shorts）ので、
 *   窓口は並びの先頭のアドレスも渡せなければならない。そのアドレス越しに
 *   書いたものが窓口から読めることまで見る。これがセーブの読みこみの道。
 *
 * 範囲の検査は**しない**のが今のふるまい（古い添字も検査していなかった）。
 * 1..40 の外を読み書きすると表の外に触るので、それは未定義のふるまいで、
 * テストからは踏まない。窓口が黙って丸めたり 0 を返したりしないことは、
 * 1 と 40 の両端が素直に通ることで押さえる。
 *
 * テストは 1 プロセスで状態を共有する。走りだしの状態を見るテストは
 * main() の先頭に置いてある。
 */
/* externs.h は要らない。窓口と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "hp_table.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 2 件は main() の先頭で走らせる。ほかのテストが書きこむので。 */

TEST(the_table_starts_empty_at_the_first_level) {
    ASSERT_EQ_INT(0, (int)hp_total_at_level(1));
}

TEST(the_table_starts_empty_at_the_last_level) {
    ASSERT_EQ_INT(0, (int)hp_total_at_level(MAX_PLAYER_LEVEL));
}

/* --- 置き場としてのふるまい --------------------------------------------- */

TEST(what_was_put_in_a_level_stays_there) {
    set_hp_total_at_level(7, 123);
    ASSERT_EQ_INT(123, (int)hp_total_at_level(7));
}

TEST(putting_a_level_again_replaces_it) {
    set_hp_total_at_level(7, 123);
    set_hp_total_at_level(7, 456);
    ASSERT_EQ_INT(456, (int)hp_total_at_level(7));
}

/* uint16_t の端。セーブファイルは wr_shorts / rd_shorts でこの幅のまま運ぶ。 */
TEST(a_level_holds_the_largest_unsigned_short) {
    set_hp_total_at_level(7, 65535);
    ASSERT_EQ_INT(65535, (int)hp_total_at_level(7));
}

TEST(a_level_can_be_put_back_to_zero) {
    set_hp_total_at_level(7, 999);
    set_hp_total_at_level(7, 0);
    ASSERT_EQ_INT(0, (int)hp_total_at_level(7));
}

/* --- 1 起点であること --------------------------------------------------- */

/* レベル 1 が表の先頭。古い添字 player_hp[lev - 1] の −1 が窓口に移った
 * ぶんだけを見る。 */
TEST(the_first_level_is_the_first_slot) {
    set_hp_total_at_level(1, 11);
    ASSERT_EQ_INT(11, (int)hp_table_slots()[0]);
}

TEST(the_last_level_is_the_last_slot) {
    set_hp_total_at_level(MAX_PLAYER_LEVEL, 22);
    ASSERT_EQ_INT(22, (int)hp_table_slots()[MAX_PLAYER_LEVEL - 1]);
}

/* 表の外に 1 つはみ出していないこと。40 を書いても 41 番目の枠には触らない
 * ——のは確かめられないので、逆から: 40 を書いたときに 39 が動かないこと。 */
TEST(putting_the_last_level_leaves_the_one_below_alone) {
    set_hp_total_at_level(MAX_PLAYER_LEVEL - 1, 333);
    set_hp_total_at_level(MAX_PLAYER_LEVEL, 444);
    ASSERT_EQ_INT(333, (int)hp_total_at_level(MAX_PLAYER_LEVEL - 1));
}

/* --- 枠が独立していること ----------------------------------------------- */

TEST(neighbouring_levels_are_separate_records) {
    set_hp_total_at_level(10, 100);
    set_hp_total_at_level(11, 200);
    ASSERT_TRUE(hp_total_at_level(10) == 100 && hp_total_at_level(11) == 200);
}

/* 1..40 の全部が使えて、どれも他を壊さないこと。作成はこの並びを 1 つずつ
 * 埋めていく。 */
TEST(every_level_from_one_to_the_last_keeps_its_own_value) {
    for (int level = 1; level <= MAX_PLAYER_LEVEL; level++) {
        set_hp_total_at_level(level, (uint16_t)(level * 3));
    }

    int kept = 0;
    for (int level = 1; level <= MAX_PLAYER_LEVEL; level++) {
        if (hp_total_at_level(level) == (uint16_t)(level * 3)) {
            kept++;
        }
    }
    ASSERT_EQ_INT(MAX_PLAYER_LEVEL, kept);
}

/* 作成のしかたをそのまま辿る（create.c:385-392）。足しこみは作成の側の
 * 仕事で、窓口は運ぶだけ——それでも表が累積として立ちあがることを見る。 */
TEST(the_table_can_be_built_up_level_by_level) {
    set_hp_total_at_level(1, 10);
    for (int level = 2; level <= MAX_PLAYER_LEVEL; level++) {
        set_hp_total_at_level(level, (uint16_t)(5 + hp_total_at_level(level - 1)));
    }
    ASSERT_EQ_INT(10 + 5 * (MAX_PLAYER_LEVEL - 1), (int)hp_total_at_level(MAX_PLAYER_LEVEL));
}

/* --- セーブファイルに渡す並び ------------------------------------------- */

TEST(the_slots_are_the_same_place_the_levels_are_read_from) {
    set_hp_total_at_level(5, 55);
    ASSERT_EQ_INT(55, (int)hp_table_slots()[4]);
}

/* セーブの読みこみの道: rd_shorts が並びに直に書き、あとは窓口から読む。 */
TEST(what_was_written_through_the_slots_is_read_back_by_level) {
    hp_table_slots()[4] = 66;
    ASSERT_EQ_INT(66, (int)hp_total_at_level(5));
}

/* 呼ぶたびに写しを作られては、rd_shorts の書きこみがどこにも残らない。 */
TEST(the_slots_are_the_same_place_every_time) {
    ASSERT_TRUE(hp_table_slots() == hp_table_slots());
}

int main(void) {
    /* 走りだしの状態を見る 2 件を最初に。以降のテストが書きこむ。 */
    RUN_TEST(the_table_starts_empty_at_the_first_level);
    RUN_TEST(the_table_starts_empty_at_the_last_level);

    RUN_TEST(what_was_put_in_a_level_stays_there);
    RUN_TEST(putting_a_level_again_replaces_it);
    RUN_TEST(a_level_holds_the_largest_unsigned_short);
    RUN_TEST(a_level_can_be_put_back_to_zero);

    RUN_TEST(the_first_level_is_the_first_slot);
    RUN_TEST(the_last_level_is_the_last_slot);
    RUN_TEST(putting_the_last_level_leaves_the_one_below_alone);

    RUN_TEST(neighbouring_levels_are_separate_records);
    RUN_TEST(every_level_from_one_to_the_last_keeps_its_own_value);
    RUN_TEST(the_table_can_be_built_up_level_by_level);

    RUN_TEST(the_slots_are_the_same_place_the_levels_are_read_from);
    RUN_TEST(what_was_written_through_the_slots_is_read_back_by_level);
    RUN_TEST(the_slots_are_the_same_place_every_time);

    TEST_SUMMARY();
}
