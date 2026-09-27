/* 「どの種族か」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 21 つめの問いで、**`struct misc` から出る 7 つめ**
 * （どこまで潜ったか・体力の骰子・守りの点数・素の命中力・罠と鍵をはずす腕・
 * 抵抗につづく）。答えは 1 バイト —— もとは py.misc.prace で、8 ファイルから
 * 13 か所が名ざしていた（13 行）。
 *
 * **この問いは「数」ではない。** ここまでの 6 つはどれも、何かが足したり
 * 比べたり掛けたりする数だった。この 1 バイトに算術をする場所は**表の行を
 * 引くとき以外 1 つも無い** —— 13 か所ぜんぶが添字か、そのまま渡すバイト。
 * だから `_adjust` の窓口は無く、これからも無い（**人物はドワーフに
 * 「なっていく」ものではない**）。
 *
 * **書かれるのは人物の一生で 2 回だけ** —— 種族を選んだとき（create.c）と
 * セーブを読みもどしたとき（save.c）。そのあとは変わらない。
 *
 * 行番号と表の中身:
 *   0 Human   1 Half-Elf   2 Elf        3 Halfling
 *   4 Gnome   5 Dwarf      6 Half-Orc   7 Half-Troll
 * （src/player.c:107 の race[MAX_RACES]）
 *
 * **名前を返す窓口だけが表に届く**（`player_race_name()`）。表そのものは
 * module の外に置いたまま extern 1 行で引く —— src/player_level.c が
 * player_exp[] にしているのと同じ形。**だから足場は C でも消えない**
 * （消えるのは py の器だけ。tests/player_race_fixture.c に書いてある）。
 *
 * 店の値段（rgold_adj の列）・生い立ちの表の始まり（prace * 3 + 1）・
 * 種族の行そのもの（年齢・身長・選べる階級）は **どれも module の外**
 * （player_race.h に書いてある）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に行を置きなおす。
 */
/* externs.h は要らない。窓口 3 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_race.h"

#include "minunit.h"

/* 表は足場が空で持っている（tests/player_race_fixture.c）。名前を読む件が
 * 自分で入れる —— 本物の 8 つの名前を写してしまうと、src/player.c が
 * 変わったときに足場だけが古くなって気づけない。 */
extern race_type race[MAX_RACES];

/* 種族の欄に答えたところから始める（create.c:170 と同じ形）。
 * 引数は race[] の行番号ただ 1 つ。 */
static void given_the_menu_answered(int row) { player_race_set(row); }

/* 表の 8 行に名前を入れる。本物と同じ綴りを使うのは**この関数の中だけ**で、
 * 件はどれも「行 n の名前が返る」ことだけを見る。 */
static void given_the_table_has_the_eight_names(void) {
    race[0].trace = "Human";
    race[1].trace = "Half-Elf";
    race[2].trace = "Elf";
    race[3].trace = "Halfling";
    race[4].trace = "Gnome";
    race[5].trace = "Dwarf";
    race[6].trace = "Half-Orc";
    race[7].trace = "Half-Troll";
}

/* ------------------------------------------------------------------
 * 行番号そのもの -- 1 バイト、1 つの答え、足すものは無い
 * ------------------------------------------------------------------ */

TEST(the_row_is_whatever_the_menu_answered) {
    given_the_menu_answered(3); /* Halfling */

    ASSERT_EQ_INT(3, player_race());
}

TEST(reading_the_row_twice_gives_the_same_answer) {
    given_the_menu_answered(5); /* Dwarf */

    ASSERT_EQ_INT(5, player_race());
    ASSERT_EQ_INT(5, player_race());
}

/* 0 は「まだ選んでいない」でもあり、ありうる答えでもある ——
 * 表の 0 行めは Human。抵抗（#18-12-21）の 0 と同じ性質。 */
TEST(zero_is_both_the_starting_state_and_a_real_answer) {
    given_the_menu_answered(0);

    ASSERT_EQ_INT(0, player_race());
}

TEST(each_of_the_eight_rows_lands_where_it_was_put) {
    for (int row = 0; row < MAX_RACES; row++) {
        given_the_menu_answered(row);

        ASSERT_EQ_INT(row, player_race());
    }
}

/* **`_adjust` が無いことをここで固定する。** 2 度めの答えは 1 度めに
 * 足されるのではなく置きかわる —— 人物は種族を「積みあげ」ない。
 * ほかの 6 つの問いはどれも足す窓口を持っている。 */
TEST(answering_the_menu_again_replaces_rather_than_adds) {
    given_the_menu_answered(3); /* Halfling */
    given_the_menu_answered(5); /* Dwarf */

    ASSERT_EQ_INT(5, player_race());
}

/* create.c:158 は `j = s - 'a';` で押されたキーを行番号にする ——
 * **メニューの文字と行番号は同じもの**（'a' が Human、'd' が Halfling）。 */
TEST(the_menu_letter_becomes_the_row_number) {
    given_the_menu_answered('d' - 'a');

    ASSERT_EQ_INT(3, player_race());
}

/* ------------------------------------------------------------------
 * 表の名前 -- 窓口が表に届く唯一の 1 本
 * ------------------------------------------------------------------ */

TEST(the_name_comes_from_the_row_of_the_table) {
    given_the_table_has_the_eight_names();
    given_the_menu_answered(3);

    ASSERT_EQ_STR(player_race_name(), "Halfling");
}

TEST(changing_the_row_changes_the_name) {
    given_the_table_has_the_eight_names();
    given_the_menu_answered(3);
    ASSERT_EQ_STR(player_race_name(), "Halfling");

    given_the_menu_answered(7);

    ASSERT_EQ_STR(player_race_name(), "Half-Troll");
}

TEST(the_first_row_and_the_last_row_both_have_a_name) {
    given_the_table_has_the_eight_names();

    given_the_menu_answered(0);
    ASSERT_EQ_STR(player_race_name(), "Human");

    given_the_menu_answered(MAX_RACES - 1);
    ASSERT_EQ_STR(player_race_name(), "Half-Troll");
}

TEST(every_one_of_the_eight_rows_names_its_own_race) {
    const char *expected[] = {"Human",    "Half-Elf", "Elf",      "Halfling",
                              "Gnome",    "Dwarf",    "Half-Orc", "Half-Troll"};
    given_the_table_has_the_eight_names();

    for (int row = 0; row < MAX_RACES; row++) {
        given_the_menu_answered(row);

        ASSERT_EQ_STR(player_race_name(), expected[row]);
    }
}

/* **名前を読むだけでは行は動かない。** 3 か所の呼び手はどれも画面に出すだけ。 */
TEST(reading_the_name_leaves_the_row_alone) {
    given_the_table_has_the_eight_names();
    given_the_menu_answered(4); /* Gnome */

    (void)player_race_name();

    ASSERT_EQ_INT(4, player_race());
}

/* ------------------------------------------------------------------
 * セーブファイル -- byte 1 つ
 * ------------------------------------------------------------------ */

TEST(the_byte_that_goes_out_is_the_one_that_came_in) {
    for (int row = 0; row < MAX_RACES; row++) {
        player_race_set(row);

        ASSERT_EQ_INT(row, player_race());
    }
}

/* save.c:701 は器の番地に読んでいた（`rd_byte(&m_ptr->prace)`）。
 * 窓口ごしにすると局所の uint8_t に受けてから置くことになるが、
 * **置きなおす窓口は種族のメニューと同じ 1 本** —— 15 つめが立てた問いへの
 * 6 度めの答えで、守りの点数を除く 4 つと同じ側。 */
TEST(loading_a_saved_game_uses_the_very_same_window) {
    given_the_menu_answered(0);

    uint8_t from_the_file = 6; /* Half-Orc */
    player_race_set(from_the_file);

    ASSERT_EQ_INT(6, player_race());
}

TEST(a_saved_game_replaces_rather_than_adds) {
    given_the_menu_answered(2); /* Elf */

    player_race_set(5); /* Dwarf */

    ASSERT_EQ_INT(5, player_race());
}

/* death.c:273 は成績表の欄にこのバイトをそのまま入れる ——
 * `new_entry.race` は名前が近いだけの別の器で、中身は行番号のまま。 */
TEST(the_high_score_entry_takes_the_row_unchanged) {
    given_the_menu_answered(7); /* Half-Troll */

    const uint8_t into_the_entry = (uint8_t)player_race();

    ASSERT_EQ_INT(7, into_the_entry);
}

/* ------------------------------------------------------------------
 * 幅と留め -- 窓口は何も断らない（所見 24）
 * ------------------------------------------------------------------ */

/* もとのフィールドは何も留めていなかったので、窓口も留めない。
 * メニューは 0〜7 しか作らず、セーブファイルは 1 バイトしか持てないが、
 * **どちらの端も確かめてはいなかった**。 */
TEST(the_window_refuses_nothing_the_field_refused_nothing) {
    given_the_menu_answered(200);

    ASSERT_EQ_INT(200, player_race());
}

/* 置き場は 1 バイトのまま。`p_ptr->misc.prace = j;` が切り落としていたのと
 * 同じところで切れる。 */
TEST(the_store_is_one_byte_wide_so_two_hundred_and_fifty_six_lands_on_zero) {
    given_the_menu_answered(256);

    ASSERT_EQ_INT(0, player_race());
}

TEST(a_negative_row_wraps_the_way_the_byte_always_did) {
    given_the_menu_answered(-1);

    ASSERT_EQ_INT(255, player_race());
}

int main(void) {
    RUN_TEST(the_row_is_whatever_the_menu_answered);
    RUN_TEST(reading_the_row_twice_gives_the_same_answer);
    RUN_TEST(zero_is_both_the_starting_state_and_a_real_answer);
    RUN_TEST(each_of_the_eight_rows_lands_where_it_was_put);
    RUN_TEST(answering_the_menu_again_replaces_rather_than_adds);
    RUN_TEST(the_menu_letter_becomes_the_row_number);

    RUN_TEST(the_name_comes_from_the_row_of_the_table);
    RUN_TEST(changing_the_row_changes_the_name);
    RUN_TEST(the_first_row_and_the_last_row_both_have_a_name);
    RUN_TEST(every_one_of_the_eight_rows_names_its_own_race);
    RUN_TEST(reading_the_name_leaves_the_row_alone);

    RUN_TEST(the_byte_that_goes_out_is_the_one_that_came_in);
    RUN_TEST(loading_a_saved_game_uses_the_very_same_window);
    RUN_TEST(a_saved_game_replaces_rather_than_adds);
    RUN_TEST(the_high_score_entry_takes_the_row_unchanged);

    RUN_TEST(the_window_refuses_nothing_the_field_refused_nothing);
    RUN_TEST(the_store_is_one_byte_wide_so_two_hundred_and_fifty_six_lands_on_zero);
    RUN_TEST(a_negative_row_wraps_the_way_the_byte_always_did);

    return TEST_SUMMARY();
}
