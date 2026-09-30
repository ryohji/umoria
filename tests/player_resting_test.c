// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「休んでいるか、あと何ターン」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 10 つめの問い（1 つめは持っている金、2 つめは腹の具合、3 つめは
 * 画面に出す数字、4 つめは魔力、5 つめは体力、6 つめは階級と経験値、7 つめは
 * 状態の旗、8 つめは装備で決まる耐性・能力、9 つめは一時的な状態の数えおとし）。
 * 答えは 1 個 —— もとは py.flags.rest で、5 ファイルから 21 か所が触っていた。
 *
 * **数えおとしに見えて 18 個の時計とは別物。** テストの重心は 3 つ:
 *
 *   1. **0 に向かって両方向から動く。** 正なら減り、**負なら増える**（負は
 *      「体力と魔力が満ちるまで」で、rest_command.c が `*` の入力に -MAX_SHORT を
 *      置く）。**向きを取りちがえると `*` の休息が 1 ターンで終わる**か、
 *      永遠に終わらなくなる。両方向に釘を打つ。
 *   2. **0 は 0 のまま。** 元の dungeon.c は `if (rest > 0) … else if (rest < 0) …`
 *      で、**0 のときは何もしない**。9 つめの時計は無条件に減って -1 になるのが
 *      ふるまいだったが、**こちらは逆で、0 を通りすぎてはいけない**（休んで
 *      いない人が「休息が終わった」と言われる）。
 *   3. **符号は捨てない。** 画面は負を「Rest *」と出し、creature.c は
 *      `abs(rest) % 7` をモンスターの増殖の間隔に読む。窓口が符号つきの数を
 *      そのまま返すことに釘を打つ（-MAX_SHORT が int16_t に収まることも）。
 *
 * 印（player_status_flags.c の PY_REST）はこの module の外。**数を 0 にしても
 * 印は残る**（rest_command.c の rest_off() が両方を消す）ので、ここでは印を 1 度も
 * 触らない —— それ自身がこの単位の設計（player_resting.h）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に数を置きなおす。
 */
/* externs.h は要らない。窓口 1 本と、MAX_SHORT のための constant.h・
 * types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_resting.h"

#include "minunit.h"

/* 休んでいない状態から始める。**窓口で置きなおす** —— 置き場が
 * src/player/player_resting.c の static に入っても（#18-12-10C）この足場は届く。 */
static void given_not_resting(void) { player_rest_stop(); }

static void given_resting_for(int rest_turns) {
    player_rest_stop();
    player_rest_set(rest_turns);
}

/* ------------------------------------------------------------------
 * 休んでいるか
 * ------------------------------------------------------------------ */

TEST(a_character_who_was_never_told_to_rest_is_not_resting) {
    given_not_resting();

    ASSERT_TRUE(!player_resting());
}

TEST(a_count_of_turns_means_resting) {
    given_resting_for(50);

    ASSERT_TRUE(player_resting());
}

/* 負も休息。**「あと -5 ターン」ではなく「満ちるまで」**という意味だが、
 * 休んでいるかどうかを訊く 8 か所はどちらも同じに扱う。 */
TEST(a_negative_count_also_means_resting) {
    given_resting_for(-5);

    ASSERT_TRUE(player_resting());
}

TEST(stopping_ends_the_resting) {
    given_resting_for(-MAX_SHORT);

    player_rest_stop();

    ASSERT_TRUE(!player_resting());
}

TEST(stopping_leaves_no_turns) {
    given_resting_for(50);

    player_rest_stop();

    ASSERT_EQ_INT(0, player_rest_turns());
}

/* ------------------------------------------------------------------
 * 数そのもの（符号つき）
 * ------------------------------------------------------------------ */

TEST(the_count_comes_back_as_it_was_given) {
    given_resting_for(123);

    ASSERT_EQ_INT(123, player_rest_turns());
}

/* **符号を落とさない。** 画面は負を「Rest *」と出し、creature.c は
 * abs() を自分でとる。 */
TEST(a_negative_count_comes_back_negative) {
    given_resting_for(-5);

    ASSERT_EQ_INT(-5, player_rest_turns());
}

/* rest_command.c が `*` に置く値。**int16_t に収まる**ことを釘打つ
 * （窓口が int を受けるので、器の幅で切られると別の値になる）。 */
TEST(the_longest_rest_survives_the_width_of_the_container) {
    given_resting_for(-MAX_SHORT);

    ASSERT_EQ_INT(-MAX_SHORT, player_rest_turns());
}

TEST(the_largest_count_of_turns_survives_the_width_of_the_container) {
    given_resting_for(MAX_SHORT);

    ASSERT_EQ_INT(MAX_SHORT, player_rest_turns());
}

/* ------------------------------------------------------------------
 * 満ちるまで休むのか
 * ------------------------------------------------------------------ */

TEST(only_a_negative_count_is_a_rest_until_healed) {
    given_resting_for(-1);

    ASSERT_TRUE(player_rest_is_until_healed());
}

TEST(a_count_of_turns_is_not_a_rest_until_healed) {
    given_resting_for(1);

    ASSERT_TRUE(!player_rest_is_until_healed());
}

/* 休んでいない人は「満ちるまで」でもない（dungeon.c は休んでいるあいだだけ
 * これを訊くが、0 が負に数えられると体力が満ちた瞬間に rest_off() が走る）。 */
TEST(not_resting_is_not_a_rest_until_healed) {
    given_not_resting();

    ASSERT_TRUE(!player_rest_is_until_healed());
}

/* ------------------------------------------------------------------
 * 1 ターン過ぎる -- 正の側
 * ------------------------------------------------------------------ */

TEST(one_turn_takes_one_turn_off_a_count) {
    given_resting_for(5);

    (void)player_rest_count_down();

    ASSERT_EQ_INT(4, player_rest_turns());
}

TEST(no_ending_is_due_while_turns_remain) {
    given_resting_for(5);

    ASSERT_TRUE(!player_rest_count_down());
}

TEST(the_ending_is_due_on_the_turn_the_last_one_is_used) {
    given_resting_for(1);

    ASSERT_TRUE(player_rest_count_down());
}

TEST(the_last_turn_leaves_no_turns) {
    given_resting_for(1);

    (void)player_rest_count_down();

    ASSERT_EQ_INT(0, player_rest_turns());
}

TEST(running_out_ends_the_resting) {
    given_resting_for(1);

    (void)player_rest_count_down();

    ASSERT_TRUE(!player_resting());
}

/* ------------------------------------------------------------------
 * 1 ターン過ぎる -- 負の側（向きが逆）
 * ------------------------------------------------------------------ */

TEST(one_turn_of_a_rest_until_healed_moves_the_count_up) {
    given_resting_for(-5);

    (void)player_rest_count_down();

    ASSERT_EQ_INT(-4, player_rest_turns());
}

TEST(no_ending_is_due_while_a_negative_count_remains) {
    given_resting_for(-5);

    ASSERT_TRUE(!player_rest_count_down());
}

TEST(the_ending_is_due_when_a_negative_count_reaches_zero) {
    given_resting_for(-1);

    ASSERT_TRUE(player_rest_count_down());
}

TEST(a_rest_until_healed_stops_being_one_when_it_reaches_zero) {
    given_resting_for(-1);

    (void)player_rest_count_down();

    ASSERT_TRUE(!player_rest_is_until_healed());
}

/* `*` の休息は 1 ターンでは終わらない。**向きを取りちがえると
 * -MAX_SHORT が -32768 になり、int16_t の底を越える。** */
TEST(the_longest_rest_is_still_running_after_one_turn) {
    given_resting_for(-MAX_SHORT);

    (void)player_rest_count_down();

    ASSERT_EQ_INT(-MAX_SHORT + 1, player_rest_turns());
}

/* ------------------------------------------------------------------
 * 0 は 0 のまま（9 つめの時計とちょうど逆）
 * ------------------------------------------------------------------ */

TEST(counting_down_when_not_resting_brings_no_ending) {
    given_not_resting();

    ASSERT_TRUE(!player_rest_count_down());
}

TEST(counting_down_when_not_resting_leaves_the_count_at_zero) {
    given_not_resting();

    (void)player_rest_count_down();

    ASSERT_EQ_INT(0, player_rest_turns());
}

/* 終わったあとにもう 1 ターン過ぎても、休息は終わったまま
 * （dungeon.c は毎ターン通るので、0 のあとも呼ばれつづける）。 */
TEST(the_turn_after_a_rest_ran_out_brings_no_second_ending) {
    given_resting_for(1);
    (void)player_rest_count_down();

    ASSERT_TRUE(!player_rest_count_down());
}

int main(void) {
    RUN_TEST(a_character_who_was_never_told_to_rest_is_not_resting);
    RUN_TEST(a_count_of_turns_means_resting);
    RUN_TEST(a_negative_count_also_means_resting);
    RUN_TEST(stopping_ends_the_resting);
    RUN_TEST(stopping_leaves_no_turns);

    RUN_TEST(the_count_comes_back_as_it_was_given);
    RUN_TEST(a_negative_count_comes_back_negative);
    RUN_TEST(the_longest_rest_survives_the_width_of_the_container);
    RUN_TEST(the_largest_count_of_turns_survives_the_width_of_the_container);

    RUN_TEST(only_a_negative_count_is_a_rest_until_healed);
    RUN_TEST(a_count_of_turns_is_not_a_rest_until_healed);
    RUN_TEST(not_resting_is_not_a_rest_until_healed);

    RUN_TEST(one_turn_takes_one_turn_off_a_count);
    RUN_TEST(no_ending_is_due_while_turns_remain);
    RUN_TEST(the_ending_is_due_on_the_turn_the_last_one_is_used);
    RUN_TEST(the_last_turn_leaves_no_turns);
    RUN_TEST(running_out_ends_the_resting);

    RUN_TEST(one_turn_of_a_rest_until_healed_moves_the_count_up);
    RUN_TEST(no_ending_is_due_while_a_negative_count_remains);
    RUN_TEST(the_ending_is_due_when_a_negative_count_reaches_zero);
    RUN_TEST(a_rest_until_healed_stops_being_one_when_it_reaches_zero);
    RUN_TEST(the_longest_rest_is_still_running_after_one_turn);

    RUN_TEST(counting_down_when_not_resting_brings_no_ending);
    RUN_TEST(counting_down_when_not_resting_leaves_the_count_at_zero);
    RUN_TEST(the_turn_after_a_rest_ran_out_brings_no_second_ending);

    return TEST_SUMMARY();
}
