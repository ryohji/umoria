/* 結末の置き場のテスト -- 現在のふるまいを保護する
 *
 * ここにも計算は無い。death / died_from / birth_date / noscore / total_winner /
 * max_score は置き場そのものだが、3 つだけ意味のある性質がある。
 *
 *   died_from は「アドレスを返す窓口」である。呼びだし側は strcpy() と
 *   sprintf() で中身を書きかえるので、窓口が値の写しを返すと書きこみが
 *   届かない（inventory_at() と同じ形）。同じ添字がいつでも同じ記録を指すこと、
 *   窓口越しに書いたものが窓口越しに読めることを固定する。
 *
 *   noscore は旗ではなくビットの集合である。0x1 が蘇生、0x2 が wizard 入り、
 *   0x4 がスコアボードの重複。呼びだし側は単独のビット（prt_winner）と、
 *   語まるごとの真偽（「何か理由があるか」）の両方を見るので、窓口は語を
 *   そのまま渡す。3 ビットが互いに独立であることを固定する。
 *   なお 0x4 は本体では立たない（バグ候補 B18。save.c:914 の括弧のずれで
 *   `((!noscore) & 0x04)` が常に 0）。ここで固定するのは置き場のふるまいで、
 *   B18 は直さないので立てられること自体は変えない。
 *
 *   max_score には「大きいほうを採る」判断が無い（#18-7-1 で足した 2 個）。
 *   得点は呼ばれるたびに計算しなおされるので下がりうる。下がらないように
 *   するのは death.c:239 の呼びだし側の仕事で、窓口は入れた値をそのまま返す。
 *   total_winner は勝ったあとに死ぬと取り消される（moria1.c:1724）ので、
 *   立てたものを下ろせることも固定する。
 *
 * テストは 1 プロセスで状態を共有する。走りだしの状態を見るテストは
 * main() の先頭に置いてある。
 */
/* externs.h は要らない。窓口と、型のための types.h だけで足りる。 */
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "score_death.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 4 件は main() の先頭で走らせる。ほかのテストが書きこむので。 */

TEST(the_game_starts_with_the_player_alive) {
    ASSERT_FALSE(player_is_dead());
}

TEST(the_game_starts_with_no_cause_of_death) {
    ASSERT_EQ_STR("", death_cause());
}

TEST(the_game_starts_with_no_birth_date) {
    ASSERT_EQ_INT(0, (int)character_birth_date());
}

TEST(the_game_starts_with_nothing_disqualifying_the_score) {
    ASSERT_EQ_INT(0, score_disqualifications());
}

/* --- 死んだかどうか ----------------------------------------------------- */

TEST(the_game_starts_with_the_game_not_won) {
    ASSERT_FALSE(player_has_won());
}

TEST(the_game_starts_with_no_best_score) {
    ASSERT_EQ_INT(0, best_score_so_far());
}

TEST(dying_is_remembered) {
    set_player_dead(true);
    ASSERT_TRUE(player_is_dead());
}

/* save.c:69 が HANGUP のセーブを作るときに偽へ戻す。片道ではない。 */
TEST(the_death_flag_can_be_cleared_again) {
    set_player_dead(true);
    set_player_dead(false);
    ASSERT_FALSE(player_is_dead());
}

/* --- 何で終わったか ----------------------------------------------------- */

/* 呼びだし側は strcpy() で書く（signals.c:186 の "Interrupting" など）。
 * 窓口が写しを返していたらこの書きこみは届かない。 */
TEST(what_is_written_through_the_window_is_read_back_through_it) {
    (void)strcpy(death_cause(), "Interrupting");
    ASSERT_EQ_STR("Interrupting", death_cause());
}

TEST(the_cause_of_death_can_be_overwritten) {
    (void)strcpy(death_cause(), "software bug");
    (void)strcpy(death_cause(), "Abortion");
    ASSERT_EQ_STR("Abortion", death_cause());
}

/* signals.c:203 は sprintf() で組みたてる。 */
TEST(the_cause_of_death_can_be_built_with_sprintf) {
    (void)sprintf(death_cause(), "(panic save %d)", 11);
    ASSERT_EQ_STR("(panic save 11)", death_cause());
}

/* death.c:341 は "(saved)" と字面で突きあわせる。終わりかたは死だけではない。 */
TEST(the_cause_of_death_records_endings_that_are_not_deaths) {
    (void)strcpy(death_cause(), "(saved)");
    ASSERT_EQ_STR("(saved)", death_cause());
}

TEST(the_window_hands_back_the_same_record_every_time) {
    char *first = death_cause();
    char *second = death_cause();
    ASSERT_TRUE(first == second);
}

/* vtype は 80 バイト。セーブファイルはこの幅のまま運ぶ。 */
TEST(the_cause_of_death_holds_a_full_length_string) {
    char full[80];
    (void)memset(full, 'x', sizeof full);
    full[sizeof full - 1] = '\0';

    (void)strcpy(death_cause(), full);
    ASSERT_EQ_STR(full, death_cause());
}

/* --- 生まれた時刻 ------------------------------------------------------- */

TEST(what_was_put_in_the_birth_date_stays_there) {
    set_character_birth_date(1757808000);
    ASSERT_EQ_INT(1757808000, (int)character_birth_date());
}

/* main.c:160 は time() の戻りをそのまま入れる。int32_t の端が通ること。 */
TEST(the_birth_date_holds_the_largest_signed_value) {
    set_character_birth_date(2147483647);
    ASSERT_EQ_INT(2147483647, (int)character_birth_date());
}

TEST(the_birth_date_holds_a_negative_value) {
    set_character_birth_date(-1);
    ASSERT_EQ_INT(-1, (int)character_birth_date());
}

/* --- スコアに入らない理由 ----------------------------------------------- */

TEST(being_resurrected_disqualifies_the_score) {
    set_score_disqualifications(0x1);
    ASSERT_EQ_INT(0x1, score_disqualifications());
}

TEST(entering_wizard_mode_disqualifies_the_score) {
    set_score_disqualifications(0x2);
    ASSERT_EQ_INT(0x2, score_disqualifications());
}

/* B18 のせいで本体ではこのビットが立たないが、置き場としては立てられる。 */
TEST(being_a_duplicate_disqualifies_the_score) {
    set_score_disqualifications(0x4);
    ASSERT_EQ_INT(0x4, score_disqualifications());
}

/* prt_winner()（misc3.c:450-458）は 3 ビットを別々に見る。混ざってはいけない。 */
TEST(the_three_reasons_are_separate_bits) {
    set_score_disqualifications(0x1 | 0x4);
    ASSERT_EQ_INT(0x5, score_disqualifications());
}

TEST(the_reasons_accumulate_rather_than_replace) {
    set_score_disqualifications(0x1);
    set_score_disqualifications((int16_t)(score_disqualifications() | 0x2));
    ASSERT_EQ_INT(0x3, score_disqualifications());
}

/* misc3.c:1655 と death.c:243 は語まるごとの真偽を見る。 */
TEST(any_reason_at_all_makes_the_word_true) {
    set_score_disqualifications(0x4);
    ASSERT_TRUE(score_disqualifications() != 0);
}

TEST(no_reason_makes_the_word_false) {
    set_score_disqualifications(0);
    ASSERT_FALSE(score_disqualifications() != 0);
}

/* --- 4 つが別の記録であること ------------------------------------------- */

TEST(the_four_records_do_not_share_storage) {
    set_player_dead(true);
    (void)strcpy(death_cause(), "Poison");
    set_character_birth_date(42);
    set_score_disqualifications(0x2);

    ASSERT_TRUE(player_is_dead() && strcmp(death_cause(), "Poison") == 0 && character_birth_date() == 42 && score_disqualifications() == 0x2);
}

/* 上の 1 件だけでは足りなかった。4 つを 1 つの順で書くと、最後に書いた記録が
 * それより前の取りちがえを上書きしてしまう（実際に「生まれた時刻を書くと
 * noscore も書きかわる」変異が生き残った）。どの記録を最後に書いても他の 3 つが
 * 動かないことを、順を変えて 3 件で押さえる。 */

TEST(writing_the_birth_date_leaves_the_other_records_alone) {
    set_player_dead(true);
    (void)strcpy(death_cause(), "Starvation");
    set_score_disqualifications(0x2);
    set_character_birth_date(4242);

    ASSERT_TRUE(player_is_dead() && strcmp(death_cause(), "Starvation") == 0 && score_disqualifications() == 0x2);
}

TEST(writing_the_death_flag_leaves_the_other_records_alone) {
    (void)strcpy(death_cause(), "Drowning");
    set_character_birth_date(777);
    set_score_disqualifications(0x1);
    set_player_dead(false);

    ASSERT_TRUE(strcmp(death_cause(), "Drowning") == 0 && character_birth_date() == 777 && score_disqualifications() == 0x1);
}

TEST(writing_the_cause_of_death_leaves_the_other_records_alone) {
    set_player_dead(true);
    set_character_birth_date(555);
    set_score_disqualifications(0x4);
    (void)strcpy(death_cause(), "A giant frog");

    ASSERT_TRUE(player_is_dead() && character_birth_date() == 555 && score_disqualifications() == 0x4);
}

TEST(dying_does_not_disqualify_the_score_by_itself) {
    set_score_disqualifications(0);
    set_player_dead(true);
    ASSERT_EQ_INT(0, score_disqualifications());
}


/* --- 勝ったかどうか ----------------------------------------------------- */

TEST(winning_is_remembered) {
    set_player_has_won(true);
    ASSERT_TRUE(player_has_won());
}

TEST(the_win_can_be_taken_back) {
    /* moria1.c:1724 -- 勝ったあとに死ぬと取り消される。 */
    set_player_has_won(true);
    set_player_has_won(false);
    ASSERT_FALSE(player_has_won());
}

TEST(winning_and_dying_are_separate_records) {
    set_player_has_won(true);
    set_player_dead(false);
    ASSERT_TRUE(player_has_won() && !player_is_dead());
}

/* --- これまでの最高得点 ------------------------------------------------- */

TEST(what_was_put_in_the_best_score_stays_there) {
    set_best_score_so_far(12345);
    ASSERT_EQ_INT(12345, best_score_so_far());
}

TEST(the_best_score_holds_the_largest_signed_value) {
    /* セーブファイルは 4 バイトで読み書きする（save.c の rd_long / wr_long）。 */
    set_best_score_so_far(2147483647L);
    ASSERT_EQ_INT(2147483647L, best_score_so_far());
}

TEST(the_best_score_holds_a_negative_value) {
    /* 窓口は範囲を検査しない。本体が負を入れることは無いが、
     * 置き場としてのふるまいを固定しておく（int32_t のまま通ること）。 */
    set_best_score_so_far(-1);
    ASSERT_EQ_INT(-1, best_score_so_far());
}

TEST(the_best_score_does_not_climb_by_itself) {
    /* 窓口に「大きいほうを採る」判断は無い。低い値を入れれば低くなる
     * （下がらないようにするのは death.c:239 の呼びだし側の仕事）。 */
    set_best_score_so_far(500);
    set_best_score_so_far(100);
    ASSERT_EQ_INT(100, best_score_so_far());
}

TEST(the_best_score_and_the_win_do_not_share_storage) {
    set_best_score_so_far(777);
    set_player_has_won(false);
    ASSERT_TRUE(best_score_so_far() == 777 && !player_has_won());
}

int main(void) {
    /* 走りだしの状態を見る 4 件を最初に。以降のテストが書きこむ。 */
    RUN_TEST(the_game_starts_with_the_player_alive);
    RUN_TEST(the_game_starts_with_no_cause_of_death);
    RUN_TEST(the_game_starts_with_no_birth_date);
    RUN_TEST(the_game_starts_with_nothing_disqualifying_the_score);
    RUN_TEST(the_game_starts_with_the_game_not_won);
    RUN_TEST(the_game_starts_with_no_best_score);

    RUN_TEST(dying_is_remembered);
    RUN_TEST(the_death_flag_can_be_cleared_again);

    RUN_TEST(what_is_written_through_the_window_is_read_back_through_it);
    RUN_TEST(the_cause_of_death_can_be_overwritten);
    RUN_TEST(the_cause_of_death_can_be_built_with_sprintf);
    RUN_TEST(the_cause_of_death_records_endings_that_are_not_deaths);
    RUN_TEST(the_window_hands_back_the_same_record_every_time);
    RUN_TEST(the_cause_of_death_holds_a_full_length_string);

    RUN_TEST(what_was_put_in_the_birth_date_stays_there);
    RUN_TEST(the_birth_date_holds_the_largest_signed_value);
    RUN_TEST(the_birth_date_holds_a_negative_value);

    RUN_TEST(being_resurrected_disqualifies_the_score);
    RUN_TEST(entering_wizard_mode_disqualifies_the_score);
    RUN_TEST(being_a_duplicate_disqualifies_the_score);
    RUN_TEST(the_three_reasons_are_separate_bits);
    RUN_TEST(the_reasons_accumulate_rather_than_replace);
    RUN_TEST(any_reason_at_all_makes_the_word_true);
    RUN_TEST(no_reason_makes_the_word_false);

    RUN_TEST(the_four_records_do_not_share_storage);
    RUN_TEST(writing_the_birth_date_leaves_the_other_records_alone);
    RUN_TEST(writing_the_death_flag_leaves_the_other_records_alone);
    RUN_TEST(writing_the_cause_of_death_leaves_the_other_records_alone);
    RUN_TEST(dying_does_not_disqualify_the_score_by_itself);

    RUN_TEST(winning_is_remembered);
    RUN_TEST(the_win_can_be_taken_back);
    RUN_TEST(winning_and_dying_are_separate_records);

    RUN_TEST(what_was_put_in_the_best_score_stays_there);
    RUN_TEST(the_best_score_holds_the_largest_signed_value);
    RUN_TEST(the_best_score_holds_a_negative_value);
    RUN_TEST(the_best_score_does_not_climb_by_itself);
    RUN_TEST(the_best_score_and_the_win_do_not_share_storage);

    TEST_SUMMARY();
}
