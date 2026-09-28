/* 「体力の骰子は何面か」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 16 つめの問いで、**`struct misc` から出る 2 つめ**（1 つめは
 * どこまで潜ったか）。答えは 1 バイト —— もとは py.misc.hitdie で、
 * 3 ファイルから 9 か所が名ざしていた（うち 1 つは player.c のコメント）。
 *
 * **単位は面の数**（体力そのものでも骰子そのものでもない）。
 * create.c が 1 レベルにつき 1 度 randint(面数) を振るので、
 * **この数は 1 レベルが増やせる最大**。
 *
 * **人物を作るときに 2 つの表で決まって、そのあと二度と動かない。**
 * 種族が土台を置き（Halfling 6・Gnome 7・Elf 8・Half-Elf 9・Dwarf 9・
 * Human 10・Half-Orc 10・Half-Troll 12）、階級がそれに足す
 * （Mage 0・Priest 2・Ranger 4・Rogue 6・Paladin 6・Warrior 9）。
 * だから実際に現れるのは 6（Halfling の Mage）から 21（Half-Troll の
 * Warrior）まで。
 *
 * **遊んでいる間は誰も読まない。** 読み手 5 つはぜんぶ create.c の
 * 体力を振る 20 行の中で、あとはセーブの 2 か所だけ
 * （player_hit_die.h に「それでも問いである」理由がある）。
 *
 * 体力そのもの・振りなおしの下限上限・体力表・2 つの表は
 * **この module の外**（player_hit_die.h）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に面数を置きなおす。
 */
/* externs.h は要らない。窓口 3 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_hit_die.h"

#include "minunit.h"

/* 種族が土台を置いたところから始める（create.c:107 と同じ形）。 */
static void given_a_race_whose_base_is(int faces) { player_hit_die_set(faces); }

/* ------------------------------------------------------------------
 * 骰子そのもの
 * ------------------------------------------------------------------ */

TEST(the_number_of_faces_comes_back_as_it_was_left) {
    given_a_race_whose_base_is(10);

    ASSERT_EQ_INT(10, player_hit_die());
}

TEST(reading_the_number_twice_gives_the_same_answer) {
    given_a_race_whose_base_is(8);

    ASSERT_EQ_INT(8, player_hit_die());
    ASSERT_EQ_INT(8, player_hit_die());
}

TEST(a_character_with_no_race_yet_has_no_faces) {
    given_a_race_whose_base_is(0);

    ASSERT_EQ_INT(0, player_hit_die());
}

/* ------------------------------------------------------------------
 * 種族が土台を置く -- create.c:107
 * ------------------------------------------------------------------ */

/* 表の 8 行ぜんぶ。**土台を置くのは決めであって足しではない** ——
 * 人物を 2 度作っても骰子が 2 倍にならない（player_infra_range.h と同じ話）。 */
TEST(every_race_puts_its_own_base_here) {
    const int bases[] = {10, 9, 8, 6, 7, 9, 10, 12};

    for (int i = 0; i < 8; i++) {
        given_a_race_whose_base_is(bases[i]);

        ASSERT_EQ_INT(bases[i], player_hit_die());
    }
}

TEST(choosing_a_race_twice_does_not_double_the_die) {
    given_a_race_whose_base_is(12);

    player_hit_die_set(12);

    ASSERT_EQ_INT(12, player_hit_die());
}

TEST(a_later_race_replaces_an_earlier_one) {
    given_a_race_whose_base_is(12);

    player_hit_die_set(6);

    ASSERT_EQ_INT(6, player_hit_die());
}

/* ------------------------------------------------------------------
 * 階級が足す -- create.c:386
 * ------------------------------------------------------------------ */

TEST(the_class_adds_its_share_to_the_races_base) {
    given_a_race_whose_base_is(10);

    player_hit_die_adjust(9);

    ASSERT_EQ_INT(19, player_hit_die());
}

/* 階級表の 6 行ぜんぶを Human の土台 10 に足す。 */
TEST(every_class_adds_its_own_share) {
    const int adjustments[] = {9, 0, 2, 6, 4, 6};

    for (int i = 0; i < 6; i++) {
        given_a_race_whose_base_is(10);

        player_hit_die_adjust(adjustments[i]);

        ASSERT_EQ_INT(10 + adjustments[i], player_hit_die());
    }
}

/* Mage は 0 を足す。**足さないのではなく 0 を足す** ——
 * create.c は階級を選んだら必ずこの窓口を通る。 */
TEST(a_mage_adds_nothing_and_keeps_the_races_base) {
    given_a_race_whose_base_is(7);

    player_hit_die_adjust(0);

    ASSERT_EQ_INT(7, player_hit_die());
}

/* 遊びのなかで現れるいちばん小さい骰子と、いちばん大きい骰子。 */
TEST(the_smallest_die_in_the_game_is_a_halfling_mage) {
    given_a_race_whose_base_is(6);

    player_hit_die_adjust(0);

    ASSERT_EQ_INT(6, player_hit_die());
}

TEST(the_largest_die_in_the_game_is_a_half_troll_warrior) {
    given_a_race_whose_base_is(12);

    player_hit_die_adjust(9);

    ASSERT_EQ_INT(21, player_hit_die());
}

/* 足しは積む。もとのコードが `+=` だったので、2 度呼べば 2 度分増える
 * （create.c は 1 度しか呼ばないが、窓口の意味は「足す」）。 */
TEST(adjusting_twice_adds_twice) {
    given_a_race_whose_base_is(6);

    player_hit_die_adjust(2);
    player_hit_die_adjust(3);

    ASSERT_EQ_INT(11, player_hit_die());
}

/* **負を断る窓口ではない。** 階級表の adj_hd はぜんぶ 0 以上なので
 * 遊びのなかでは起きないが、もとのコードも検査していないので
 * 窓口もしない（→ 台帳の所見 24）。 */
TEST(nothing_refuses_to_take_faces_away) {
    given_a_race_whose_base_is(10);

    player_hit_die_adjust(-4);

    ASSERT_EQ_INT(6, player_hit_die());
}

/* ------------------------------------------------------------------
 * セーブファイル -- 置きなおしは種族の土台と同じ窓口
 * ------------------------------------------------------------------ */

TEST(loading_a_saved_game_brings_the_die_back) {
    given_a_race_whose_base_is(0);

    player_hit_die_set(19);

    ASSERT_EQ_INT(19, player_hit_die());
}

/* **ファイルの 1 バイトは骰子まるごと。** 大きい骰子の人物が遊んでいる
 * ところへ小さい骰子のセーブを読みこんでも、面数はそのファイルのものになる
 * （足しではないので混ざらない）。 */
TEST(the_saved_file_may_put_back_a_smaller_die) {
    given_a_race_whose_base_is(21);
    player_hit_die_adjust(0);

    player_hit_die_set(6);

    ASSERT_EQ_INT(6, player_hit_die());
}

TEST(the_byte_that_goes_out_is_the_one_that_came_in) {
    for (int faces = 0; faces < 256; faces++) {
        player_hit_die_set(faces);

        ASSERT_EQ_INT(faces, player_hit_die());
    }
}

/* 器は 1 バイトなので 255 までしか入らない。**留めは無い** ——
 * もとのコードも範囲を検査していないので、窓口もしない。 */
TEST(the_widest_byte_the_container_holds_survives) {
    given_a_race_whose_base_is(255);

    ASSERT_EQ_INT(255, player_hit_die());
}

int main(void) {
    RUN_TEST(the_number_of_faces_comes_back_as_it_was_left);
    RUN_TEST(reading_the_number_twice_gives_the_same_answer);
    RUN_TEST(a_character_with_no_race_yet_has_no_faces);

    RUN_TEST(every_race_puts_its_own_base_here);
    RUN_TEST(choosing_a_race_twice_does_not_double_the_die);
    RUN_TEST(a_later_race_replaces_an_earlier_one);

    RUN_TEST(the_class_adds_its_share_to_the_races_base);
    RUN_TEST(every_class_adds_its_own_share);
    RUN_TEST(a_mage_adds_nothing_and_keeps_the_races_base);
    RUN_TEST(the_smallest_die_in_the_game_is_a_halfling_mage);
    RUN_TEST(the_largest_die_in_the_game_is_a_half_troll_warrior);
    RUN_TEST(adjusting_twice_adds_twice);
    RUN_TEST(nothing_refuses_to_take_faces_away);

    RUN_TEST(loading_a_saved_game_brings_the_die_back);
    RUN_TEST(the_saved_file_may_put_back_a_smaller_die);
    RUN_TEST(the_byte_that_goes_out_is_the_one_that_came_in);
    RUN_TEST(the_widest_byte_the_container_holds_survives);

    return TEST_SUMMARY();
}
