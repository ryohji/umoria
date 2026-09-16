/* セーブの置き場のテスト -- 現在のふるまいを保護する
 *
 * savefile / character_generated / character_saved / panic_save は置き場だが、
 * この module には 3 つの述語もある。置き場と述語で保護のとりかたが違う。
 *
 *   savefile は「アドレスを返す窓口」である。main.c は strcpy() で
 *   コマンド行や MORIA_SAV の中身を書きこみ、save.c は別名を打たれたときに
 *   書きかえる。窓口が値の写しを返すと書きこみが届かない（inventory_at() /
 *   died_from と同じ形）。同じ窓口がいつでも同じ記録を指すことを固定する。
 *
 *   3 つの述語（has_unsaved_character / has_live_character /
 *   character_is_in_play）は、変更前は呼びだし側の if 文の中に直に書かれて
 *   いた。関数として呼べないので、変更前の式をテスト側に写しとり
 *   （legacy_*）、入力の全通りで新しい実体と突きあわせる。#7（device.c）や
 *   #18-1（options.c）と同じ「テスト側に写す」方式。組みあわせは
 *   2 個・3 個の真偽なので 4 通り・8 通りで総当たりできる。
 *
 *   character_is_in_play は turn の符号を見る。境界は 0 で、そこが
 *   signals.c:170 の `turn > 0` との分かれ目になる（0 は本物の turn なので
 *   この窓口では真）。ずれたらここで落ちる。
 *
 * テストは 1 プロセスで状態を共有する。走りだしの状態を見るテストは
 * main() の先頭に置いてある。
 */
/* externs.h は要らない。窓口と、型のための types.h だけで足りる。
 * progress.h / score_death.h は、述語が読む turn と death を動かすため。 */
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "progress.h"
#include "save_state.h"
#include "score_death.h"

#include "minunit.h"

/* --- 変更前の式の写し --------------------------------------------------- */
/* signals.c:166 / death.c:462 / io.c:72 の 3 つの字面は同じ 1 つの式である。
 * 代表として death.c:462 の並びを写す。 */
static bool legacy_has_unsaved_character(bool generated, bool saved) {
    return generated && !saved;
}

/* signals.c:199 の `!death && !character_saved && character_generated`。 */
static bool legacy_has_live_character(bool dead, bool saved, bool generated) {
    return !dead && !saved && generated;
}

/* save.c:471 / save.c:963 / death.c:455 の `turn >= 0`。 */
static bool legacy_character_is_in_play(int32_t turn) {
    return turn >= 0;
}

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 4 件は main() の先頭で走らせる。ほかのテストが書きこむので。 */

TEST(the_game_starts_with_no_save_file_name) {
    ASSERT_EQ_STR("", save_file_path());
}

TEST(the_game_starts_with_no_character_generated) {
    ASSERT_FALSE(character_is_generated());
}

TEST(the_game_starts_with_nothing_saved) {
    ASSERT_FALSE(character_is_saved());
}

TEST(the_game_starts_without_a_panic_save) {
    ASSERT_FALSE(is_panic_save());
}

/* --- セーブファイルの名前 ----------------------------------------------- */

/* main.c:130 は strcpy() で書く。窓口が写しを返していたら届かない。 */
TEST(what_is_written_through_the_window_is_read_back_through_it) {
    (void)strcpy(save_file_path(), "moria.sav");
    ASSERT_EQ_STR("moria.sav", save_file_path());
}

/* save.c:245 は別名を打たれたときに上書きする。片道ではない。 */
TEST(the_save_file_name_can_be_overwritten) {
    (void)strcpy(save_file_path(), "moria.sav");
    (void)strcpy(save_file_path(), "/tmp/other.sav");
    ASSERT_EQ_STR("/tmp/other.sav", save_file_path());
}

TEST(the_window_hands_back_the_same_record_every_time) {
    char *first = save_file_path();
    char *second = save_file_path();
    ASSERT_TRUE(first == second);
}

/* vtype は 80 バイト。長い絶対パスがそのまま入る幅を固定する。 */
TEST(the_save_file_name_holds_a_full_length_path) {
    char full[80];
    (void)memset(full, 'p', sizeof full);
    full[sizeof full - 1] = '\0';

    (void)strcpy(save_file_path(), full);
    ASSERT_EQ_STR(full, save_file_path());
}

/* --- 3 つの旗 ----------------------------------------------------------- */

TEST(generating_a_character_is_remembered) {
    set_character_generated(true);
    ASSERT_TRUE(character_is_generated());
}

/* death.c:112 が新しいキャラクターを作りなおすときに偽へ戻す。片道ではない。 */
TEST(the_generated_flag_can_be_cleared_again) {
    set_character_generated(true);
    set_character_generated(false);
    ASSERT_FALSE(character_is_generated());
}

TEST(saving_the_character_is_remembered) {
    set_character_saved(true);
    ASSERT_TRUE(character_is_saved());
}

/* save.c:69 は書きだしたあとに真、続きを遊びはじめると偽に戻す。
 * 「ファイルが最新かどうか」であって片道の掛け金ではない。 */
TEST(the_saved_flag_can_be_cleared_again) {
    set_character_saved(true);
    set_character_saved(false);
    ASSERT_FALSE(character_is_saved());
}

TEST(a_panic_save_is_remembered) {
    set_panic_save(true);
    ASSERT_TRUE(is_panic_save());
}

TEST(the_panic_flag_can_be_cleared_again) {
    set_panic_save(true);
    set_panic_save(false);
    ASSERT_FALSE(is_panic_save());
}

/* --- 4 つが別の記録であること ------------------------------------------- */

/* #19A2 で分かったこと: 4 つを 1 つの順で書く 1 件だけでは足りない。最後に
 * 書いた記録が、それより前の取りちがえを上書きしてしまう。どの記録を最後に
 * 書いても他の 3 つが動かないことを、順を変えて 4 件で押さえる。 */

TEST(writing_the_generated_flag_leaves_the_other_records_alone) {
    (void)strcpy(save_file_path(), "alpha.sav");
    set_character_saved(true);
    set_panic_save(true);
    set_character_generated(false);

    ASSERT_TRUE(strcmp(save_file_path(), "alpha.sav") == 0 && character_is_saved() && is_panic_save());
}

TEST(writing_the_saved_flag_leaves_the_other_records_alone) {
    (void)strcpy(save_file_path(), "beta.sav");
    set_character_generated(true);
    set_panic_save(true);
    set_character_saved(false);

    ASSERT_TRUE(strcmp(save_file_path(), "beta.sav") == 0 && character_is_generated() && is_panic_save());
}

TEST(writing_the_panic_flag_leaves_the_other_records_alone) {
    (void)strcpy(save_file_path(), "gamma.sav");
    set_character_generated(true);
    set_character_saved(true);
    set_panic_save(false);

    ASSERT_TRUE(strcmp(save_file_path(), "gamma.sav") == 0 && character_is_generated() && character_is_saved());
}

TEST(writing_the_save_file_name_leaves_the_other_records_alone) {
    set_character_generated(true);
    set_character_saved(true);
    set_panic_save(true);
    (void)strcpy(save_file_path(), "delta.sav");

    ASSERT_TRUE(character_is_generated() && character_is_saved() && is_panic_save());
}

/* --- 書きだしていないキャラクターがいるか -------------------------------- */
/* 旗 2 つなので 4 通りを総当たりできる。写した式との一致を全通りで見る。 */

TEST(the_unsaved_question_matches_the_old_expression_everywhere) {
    for (int generated = 0; generated < 2; generated++) {
        for (int saved = 0; saved < 2; saved++) {
            set_character_generated(generated != 0);
            set_character_saved(saved != 0);

            ASSERT_EQ_INT((int)legacy_has_unsaved_character(generated != 0, saved != 0), (int)save_state_has_unsaved_character());
        }
    }
}

/* 総当たりが通っても、4 つの角がどれなのかは読んで分からない。名前を付けて
 * 別に置く（総当たりが式ごと壊れたときにも意味が残るように）。 */

TEST(a_generated_and_unwritten_character_is_unsaved) {
    set_character_generated(true);
    set_character_saved(false);
    ASSERT_TRUE(save_state_has_unsaved_character());
}

TEST(a_character_already_written_is_not_unsaved) {
    set_character_generated(true);
    set_character_saved(true);
    ASSERT_FALSE(save_state_has_unsaved_character());
}

/* キャラクターがまだ無ければ、書きだしていなくても保存するものは無い。 */
TEST(no_character_at_all_is_not_unsaved) {
    set_character_generated(false);
    set_character_saved(false);
    ASSERT_FALSE(save_state_has_unsaved_character());
}

/* 起こりえない組みあわせだが、式は式なので偽と決まっている。 */
TEST(a_saved_flag_without_a_character_is_not_unsaved) {
    set_character_generated(false);
    set_character_saved(true);
    ASSERT_FALSE(save_state_has_unsaved_character());
}

/* --- 生きているキャラクターがいるか -------------------------------------- */
/* 旗 3 つなので 8 通り。死んだ旗は score_death.h 側の記録で、この窓口が
 * よそを読む唯一の場所（signals.c:199 が唯一の呼びだし側）。 */

TEST(the_live_question_matches_the_old_expression_everywhere) {
    for (int dead = 0; dead < 2; dead++) {
        for (int saved = 0; saved < 2; saved++) {
            for (int generated = 0; generated < 2; generated++) {
                set_player_dead(dead != 0);
                set_character_saved(saved != 0);
                set_character_generated(generated != 0);

                ASSERT_EQ_INT((int)legacy_has_live_character(dead != 0, saved != 0, generated != 0), (int)save_state_has_live_character());
            }
        }
    }
}

TEST(a_living_unwritten_character_is_worth_a_panic_save) {
    set_player_dead(false);
    set_character_generated(true);
    set_character_saved(false);
    ASSERT_TRUE(save_state_has_live_character());
}

/* 死んでいれば panic save はしない。死は death.c が別に扱う。 */
TEST(a_dead_character_is_not_worth_a_panic_save) {
    set_player_dead(true);
    set_character_generated(true);
    set_character_saved(false);
    ASSERT_FALSE(save_state_has_live_character());
}

/* --- キャラクターが場にいるか -------------------------------------------- */

/* 走りだしの -1 は turn ではない。 */
TEST(nobody_is_in_play_before_the_first_turn) {
    progress_set_turn(-1);
    ASSERT_FALSE(save_state_character_is_in_play());
}

/* ここが境目。0 は本物の turn なのでこの窓口は真を返す。signals.c:170 の
 * `turn > 0` は同じ turn 0 で偽になる。まとめてはいけない理由がこれ。 */
TEST(turn_zero_counts_as_being_in_play) {
    progress_set_turn(0);
    ASSERT_TRUE(save_state_character_is_in_play());
}

TEST(the_first_turn_counts_as_being_in_play) {
    progress_set_turn(1);
    ASSERT_TRUE(save_state_character_is_in_play());
}

/* 長い一局。int32_t の上端まで真のまま。 */
TEST(a_long_game_is_still_in_play) {
    progress_set_turn(2147483647);
    ASSERT_TRUE(save_state_character_is_in_play());
}

TEST(the_most_negative_turn_is_not_in_play) {
    progress_set_turn(INT32_MIN);
    ASSERT_FALSE(save_state_character_is_in_play());
}

/* 符号の境目を、写した式と両側で突きあわせる。 */
TEST(the_in_play_question_matches_the_old_comparison_around_zero) {
    for (int32_t turn = -3; turn <= 3; turn++) {
        progress_set_turn(turn);
        ASSERT_EQ_INT((int)legacy_character_is_in_play(turn), (int)save_state_character_is_in_play());
    }
}

int main(void) {
    /* 走りだしの状態を見る 4 件を最初に。以降のテストが書きこむ。 */
    RUN_TEST(the_game_starts_with_no_save_file_name);
    RUN_TEST(the_game_starts_with_no_character_generated);
    RUN_TEST(the_game_starts_with_nothing_saved);
    RUN_TEST(the_game_starts_without_a_panic_save);

    RUN_TEST(what_is_written_through_the_window_is_read_back_through_it);
    RUN_TEST(the_save_file_name_can_be_overwritten);
    RUN_TEST(the_window_hands_back_the_same_record_every_time);
    RUN_TEST(the_save_file_name_holds_a_full_length_path);

    RUN_TEST(generating_a_character_is_remembered);
    RUN_TEST(the_generated_flag_can_be_cleared_again);
    RUN_TEST(saving_the_character_is_remembered);
    RUN_TEST(the_saved_flag_can_be_cleared_again);
    RUN_TEST(a_panic_save_is_remembered);
    RUN_TEST(the_panic_flag_can_be_cleared_again);

    RUN_TEST(writing_the_generated_flag_leaves_the_other_records_alone);
    RUN_TEST(writing_the_saved_flag_leaves_the_other_records_alone);
    RUN_TEST(writing_the_panic_flag_leaves_the_other_records_alone);
    RUN_TEST(writing_the_save_file_name_leaves_the_other_records_alone);

    RUN_TEST(the_unsaved_question_matches_the_old_expression_everywhere);
    RUN_TEST(a_generated_and_unwritten_character_is_unsaved);
    RUN_TEST(a_character_already_written_is_not_unsaved);
    RUN_TEST(no_character_at_all_is_not_unsaved);
    RUN_TEST(a_saved_flag_without_a_character_is_not_unsaved);

    RUN_TEST(the_live_question_matches_the_old_expression_everywhere);
    RUN_TEST(a_living_unwritten_character_is_worth_a_panic_save);
    RUN_TEST(a_dead_character_is_not_worth_a_panic_save);

    RUN_TEST(nobody_is_in_play_before_the_first_turn);
    RUN_TEST(turn_zero_counts_as_being_in_play);
    RUN_TEST(the_first_turn_counts_as_being_in_play);
    RUN_TEST(a_long_game_is_still_in_play);
    RUN_TEST(the_most_negative_turn_is_not_in_play);
    RUN_TEST(the_in_play_question_matches_the_old_comparison_around_zero);

    TEST_SUMMARY();
}
