// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「抵抗はいくつか」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 20 つめの問いで、**`struct misc` から出る 6 つめ**
 * （どこまで潜ったか・体力の骰子・守りの点数・素の命中力・罠と鍵をはずす腕に
 * つづく）。答えは short 1 本 —— もとは py.misc.save で、7 ファイルから
 * 11 か所が名ざしていた（11 行）。
 *
 * **1 フィールド 1 つの答えで、19 つめと同じ素直な形**（16 つめ「体力の骰子」
 * から数えて 3 例め）。**19 つめより素直な点が 1 つある** ——
 * 創成時に焼きこむ下駄が無い（`create.c:110` は `= r_ptr->bsav` だけ）。
 *
 * **大きいほど抵抗しやすい。** 読み手は `randint(100)` と比べる。
 * **負にもなる**（Half-Orc の種族の土台は -3、Half-Troll は -8）。
 *
 * **1 つの数から等級が 2 つ出る** —— 人物画面の「Saving Throw」は
 * A_WIS と `CLA_SAVE`、「Magic Device」は **A_INT と `CLA_DEVICE`** を足す。
 * 杖と魔法棒の成功率もこの数が土台（`device_use_chance()` の第 1 引数）。
 * **どちらの等級もこの module の外**（player_saving_throw.h に書いてある）。
 *
 * 判定そのもの（`combat/player_damage.c` の `player_saves()`）も **この module の外** ——
 * 窓口は数を返し、`player_saves()` は真偽を返す。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に 1 本を置きなおす。
 */
/* externs.h は要らない。窓口 3 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_saving_throw.h"

#include "minunit.h"

/* 種族を選んだところから始める（create.c:110 と同じ形）。
 * 引数は race[] の bsav ただ 1 つ —— 19 つめの `given_a_race_with_base` が
 * 引数 2 つだったのは、あちらが創成時の DEX の下駄を焼きこむから。 */
static void given_a_race_with_base(int bsav) { player_saving_throw_set(bsav); }

/* ------------------------------------------------------------------
 * 抵抗そのもの -- 1 つの数、1 つの答え
 * ------------------------------------------------------------------ */

TEST(the_number_is_whatever_creation_put_there) {
    given_a_race_with_base(18); /* Halfling */

    ASSERT_EQ_INT(18, player_saving_throw());
}

TEST(reading_the_number_twice_gives_the_same_answer) {
    given_a_race_with_base(0); /* Human */
    player_saving_throw_adjust(18);

    ASSERT_EQ_INT(18, player_saving_throw());
    ASSERT_EQ_INT(18, player_saving_throw());
}

/* 0 は「まだ種族を選んでいない」でもあり、ありうる答えでもある ——
 * Human の土台はちょうど 0。 */
TEST(zero_is_both_the_starting_state_and_a_real_answer) {
    given_a_race_with_base(0);

    ASSERT_EQ_INT(0, player_saving_throw());
}

/* **同じ 1 本が抵抗と道具の両方に答える。** 読み手が足すものは違う
 * （A_WIS と `CLA_SAVE` ／ A_INT と `CLA_DEVICE`）が、**土台は 1 つ** ——
 * 窓口が 2 本にならない理由がこれ。 */
TEST(one_number_answers_both_the_saving_throw_and_the_device) {
    given_a_race_with_base(12); /* Gnome */

    const int for_resisting = player_saving_throw();
    const int for_using_a_wand = player_saving_throw();

    ASSERT_EQ_INT(12, for_resisting);
    ASSERT_EQ_INT(for_resisting, for_using_a_wand);
}

/* ------------------------------------------------------------------
 * 種族の土台 -- 8 行そのまま（負が 2 つある）
 * ------------------------------------------------------------------ */

TEST(the_small_folk_start_better_at_it) {
    given_a_race_with_base(18); /* Halfling */
    const int halfling = player_saving_throw();

    given_a_race_with_base(0); /* Human */

    ASSERT_EQ_INT(18, halfling);
    ASSERT_EQ_INT(0, player_saving_throw());
}

/* **負は本物の答え。** Half-Troll は -8 から始まる（この道でいちばん低い
 * 種族の土台 —— 罠と鍵をはずす腕の -5 より低い）。 */
TEST(a_race_can_be_worse_than_nothing_at_it) {
    given_a_race_with_base(-8); /* Half-Troll */

    ASSERT_EQ_INT(-8, player_saving_throw());
}

TEST(each_of_the_eight_racial_bases_lands_where_it_was_put) {
    /* race[].bsav: Human 0・Half-Elf 3・Elf 6・Halfling 18・Gnome 12・
     * Dwarf 9・Half-Orc -3・Half-Troll -8。 */
    const int bases[] = {0, 3, 6, 18, 12, 9, -3, -8};

    for (int i = 0; i < 8; i++) {
        given_a_race_with_base(bases[i]);

        ASSERT_EQ_INT(bases[i], player_saving_throw());
    }
}

/* **創成時に何も焼きこまれない**（19 つめとの違い）。`create.c:110` が渡すのは
 * 種族の表の数そのままで、能力値の下駄は 1 つも混ざらない。 */
TEST(nothing_is_baked_in_when_the_race_puts_its_base) {
    given_a_race_with_base(6); /* Elf */

    ASSERT_EQ_INT(6, player_saving_throw());
}

/* ------------------------------------------------------------------
 * 階級ぶんを足す -- create.c の `+=` そのまま
 * ------------------------------------------------------------------ */

/* Human の Warrior は 0 ＋ 18。 */
TEST(the_class_adds_to_what_the_race_left_behind) {
    given_a_race_with_base(0);

    player_saving_throw_adjust(18); /* Warrior */

    ASSERT_EQ_INT(18, player_saving_throw());
}

/* **遊びのなかでいちばん抵抗するのは Halfling の Mage**（18 ＋ 36。
 * 種族の `rtclass` 0x0B がこの組を認めている）。 */
TEST(the_best_pair_in_the_game_is_a_halfling_mage) {
    given_a_race_with_base(18);

    player_saving_throw_adjust(36); /* Mage */

    ASSERT_EQ_INT(54, player_saving_throw());
}

/* **いちばん弱いのは Half-Troll の Warrior**（-8 ＋ 18。`rtclass` 0x05 が
 * Half-Troll に認めるのは Warrior と Priest だけ）。 */
TEST(the_worst_pair_in_the_game_is_a_half_troll_warrior) {
    given_a_race_with_base(-8);

    player_saving_throw_adjust(18); /* Warrior */

    ASSERT_EQ_INT(10, player_saving_throw());
}

TEST(each_of_the_six_class_amounts_is_added_as_it_stands) {
    /* class[].msav: Warrior 18・Mage 36・Priest 30・Rogue 30・Ranger 30・
     * Paladin 24 —— **呪文を使う階級のほうが強い**（罠と鍵をはずす腕とは
     * 並びが違う。あちらは Rogue が飛びぬけていた）。 */
    const int amounts[] = {18, 36, 30, 30, 30, 24};

    for (int i = 0; i < 6; i++) {
        given_a_race_with_base(3); /* Half-Elf */

        player_saving_throw_adjust(amounts[i]);

        ASSERT_EQ_INT(3 + amounts[i], player_saving_throw());
    }
}

/* 足しは積む（遊びのなかで 2 度呼ぶ人はいないが、窓口は `+=` のまま）。 */
TEST(adjusting_twice_adds_twice) {
    given_a_race_with_base(0);

    player_saving_throw_adjust(18);
    player_saving_throw_adjust(18);

    ASSERT_EQ_INT(36, player_saving_throw());
}

/* 負を足すこともできる —— もとのコードは検査していない（→ 所見 24）。 */
TEST(the_window_takes_a_negative_amount_because_the_field_did) {
    given_a_race_with_base(9); /* Dwarf */

    player_saving_throw_adjust(-30);

    ASSERT_EQ_INT(-21, player_saving_throw());
}

/* ------------------------------------------------------------------
 * 置きなおし -- 種族・セーブの読みもどし・wizard 画面が同じ 1 本
 * ------------------------------------------------------------------ */

/* **セーブの読みもどしは種族の土台と同じ文**（15 つめに立てた問いへの
 * 5 度めの答えで、体力の骰子・素の命中力・罠と鍵をはずす腕と同じ「同じ」。
 * 守りの点数だけが「違う」——これで 5 対 1）。 */
TEST(loading_a_saved_game_uses_the_very_same_window) {
    given_a_race_with_base(0);

    player_saving_throw_set(73);

    ASSERT_EQ_INT(73, player_saving_throw());
}

/* 置きなおしは積みではない —— 強い人物のところへ弱いセーブを読んでも
 * 混ざらない。 */
TEST(a_saved_game_replaces_rather_than_adds) {
    given_a_race_with_base(18);
    player_saving_throw_adjust(36);

    player_saving_throw_set(20);

    ASSERT_EQ_INT(20, player_saving_throw());
}

/* wizard 画面も同じ 1 本。 */
TEST(the_wizard_screen_uses_the_very_same_window) {
    given_a_race_with_base(0);

    player_saving_throw_set(100);

    ASSERT_EQ_INT(100, player_saving_throw());
}

/* **wizard 画面の文と留めは食いちがっている** —— 「(0-100)」と出しておいて
 * 受けるのは 0 から 200（罠と鍵をはずす腕と同じ検査で、文だけが違う）。
 * **どちらの数も窓口の持ちものではない** ——もとのフィールドも留めて
 * いなかった（→ 所見 24）。 */
TEST(the_window_refuses_nothing_the_field_refused_nothing) {
    given_a_race_with_base(0);

    player_saving_throw_set(200);
    ASSERT_EQ_INT(200, player_saving_throw());

    player_saving_throw_set(999);
    ASSERT_EQ_INT(999, player_saving_throw());

    player_saving_throw_set(-999);
    ASSERT_EQ_INT(-999, player_saving_throw());
}

/* ------------------------------------------------------------------
 * セーブファイル -- short 1 本
 * ------------------------------------------------------------------ */

TEST(the_short_that_goes_out_is_the_one_that_came_in) {
    const int values[] = {0, 3, 18, 54, 100, -8, 32767, -32768};

    for (int i = 0; i < 8; i++) {
        player_saving_throw_set(values[i]);

        ASSERT_EQ_INT(values[i], player_saving_throw());
    }
}

int main(void) {
    RUN_TEST(the_number_is_whatever_creation_put_there);
    RUN_TEST(reading_the_number_twice_gives_the_same_answer);
    RUN_TEST(zero_is_both_the_starting_state_and_a_real_answer);
    RUN_TEST(one_number_answers_both_the_saving_throw_and_the_device);

    RUN_TEST(the_small_folk_start_better_at_it);
    RUN_TEST(a_race_can_be_worse_than_nothing_at_it);
    RUN_TEST(each_of_the_eight_racial_bases_lands_where_it_was_put);
    RUN_TEST(nothing_is_baked_in_when_the_race_puts_its_base);

    RUN_TEST(the_class_adds_to_what_the_race_left_behind);
    RUN_TEST(the_best_pair_in_the_game_is_a_halfling_mage);
    RUN_TEST(the_worst_pair_in_the_game_is_a_half_troll_warrior);
    RUN_TEST(each_of_the_six_class_amounts_is_added_as_it_stands);
    RUN_TEST(adjusting_twice_adds_twice);
    RUN_TEST(the_window_takes_a_negative_amount_because_the_field_did);

    RUN_TEST(loading_a_saved_game_uses_the_very_same_window);
    RUN_TEST(a_saved_game_replaces_rather_than_adds);
    RUN_TEST(the_wizard_screen_uses_the_very_same_window);
    RUN_TEST(the_window_refuses_nothing_the_field_refused_nothing);

    RUN_TEST(the_short_that_goes_out_is_the_one_that_came_in);

    return TEST_SUMMARY();
}
