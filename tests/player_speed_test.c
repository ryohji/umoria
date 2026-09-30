// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「いまの速さ」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 11 つめの問い（1 つめは持っている金、2 つめは腹の具合、3 つめは
 * 画面に出す数字、4 つめは魔力、5 つめは体力、6 つめは階級と経験値、7 つめは
 * 状態の旗、8 つめは装備で決まる耐性・能力、9 つめは一時的な状態の数えおとし、
 * 10 つめは休息の残りターン）。答えは 1 個 —— もとは py.flags.speed で、
 * 5 ファイルから 9 か所が触っていた。
 *
 * **数えおとしでも残りターンでもない。** 誰も減らさない「いまの段数」で、0 が
 * ふつう。テストの重心は 3 つ:
 *
 *   1. **符号が逆。正が遅い。** prt_speed()（status_line.c）は 1 で "Slow"・2 以上で
 *      "Very Slow"・0 で空白・-1 で "Fast"・それ以下で "Very Fast"。急ぎの薬は
 *      change_speed(-1)、TR_SPEED の品は change_speed(-amount)、遅くする薬は
 *      change_speed(1)。**取りちがえると速い／遅いがそのまま入れかわる**ので、
 *      両方の向きから釘を打つ。
 *   2. **足し引きで、置きかえではない。** 効果は重なる（急ぎの薬を 2 本飲めば
 *      2 段）。決める窓口（player_speed_set）はセーブファイルの読みだけが呼ぶ
 *      もので、足す窓口と取りちがえないことに釘を打つ。
 *   3. **留めが無い。** 上限も下限もどこにも書かれていないので、窓口も留めない
 *      （器の幅 —— short 1 つぶん —— だけが限り）。
 *
 * モンスターはこの module の外。change_speed()（player_bonuses.c）は同じ数を
 * m_list[i].cspeed 全員に足し、monster/monster_place.c は新しいモンスターを置くときに混ぜる。
 * **プレイヤーが遅いぶんをモンスターを速くすることで表す設計**なので、ここでは
 * モンスターを 1 匹も触らない —— それ自身がこの単位の設計（player_speed.h）。
 * 食いけの 2 乗（dungeon.c）と探索の 1 段引き（status_line.c）も外。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に段数を置きなおす。
 */
/* externs.h は要らない。窓口 1 本と、MAX_SHORT のための constant.h・
 * types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_speed.h"

#include "minunit.h"

/* ふつうの速さから始める。**窓口で置きなおす** —— 置き場が src/player/player_speed.c の
 * static に入っても（#18-12-11C）この足場は届く。 */
static void given_normal_speed(void) { player_speed_set(0); }

static void given_speed_steps(int num_steps) {
    player_speed_set(0);
    player_speed_set(num_steps);
}

/* ------------------------------------------------------------------
 * 段数そのもの
 * ------------------------------------------------------------------ */

TEST(a_character_nothing_has_happened_to_is_at_normal_speed) {
    given_normal_speed();

    ASSERT_EQ_INT(0, player_speed());
}

TEST(the_steps_come_back_as_they_were_given) {
    given_speed_steps(2);

    ASSERT_EQ_INT(2, player_speed());
}

/* 負もそのまま返る。**落とすと速いキャラクタがふつうの速さに見える。** */
TEST(negative_steps_come_back_negative) {
    given_speed_steps(-3);

    ASSERT_EQ_INT(-3, player_speed());
}

TEST(reading_the_steps_twice_gives_the_same_answer) {
    given_speed_steps(-1);

    (void)player_speed();

    ASSERT_EQ_INT(-1, player_speed());
}

/* 決める窓口は**置きかえ**（セーブファイルの読みが呼ぶ。足すのではない）。 */
TEST(deciding_the_steps_replaces_whatever_was_there) {
    given_speed_steps(4);

    player_speed_set(-1);

    ASSERT_EQ_INT(-1, player_speed());
}

TEST(deciding_zero_steps_is_normal_speed) {
    given_speed_steps(-5);

    player_speed_set(0);

    ASSERT_EQ_INT(0, player_speed());
}

/* ------------------------------------------------------------------
 * 符号の向き -- 正が遅い、負が速い
 * ------------------------------------------------------------------ */

/* 急ぎの薬（player_bonuses.c の change_speed(-1)）。**数は下がる。** */
TEST(hasting_the_character_takes_the_steps_down) {
    given_normal_speed();

    player_speed_adjust(-1);

    ASSERT_EQ_INT(-1, player_speed());
}

/* 遅くする薬・罠（change_speed(1)）。**数は上がる。** */
TEST(slowing_the_character_takes_the_steps_up) {
    given_normal_speed();

    player_speed_adjust(1);

    ASSERT_EQ_INT(1, player_speed());
}

/* TR_SPEED の品を身につけたとき（change_speed(-amount)）。 */
TEST(an_item_of_speed_takes_the_steps_down_by_its_amount) {
    given_normal_speed();

    player_speed_adjust(-3);

    ASSERT_EQ_INT(-3, player_speed());
}

/* 同じ品を外したとき（change_speed(amount)）は元に戻る。 */
TEST(taking_the_item_of_speed_off_puts_the_steps_back) {
    given_normal_speed();
    player_speed_adjust(-3);

    player_speed_adjust(3);

    ASSERT_EQ_INT(0, player_speed());
}

TEST(haste_and_slowness_cancel_each_other_out) {
    given_normal_speed();

    player_speed_adjust(-1);
    player_speed_adjust(1);

    ASSERT_EQ_INT(0, player_speed());
}

/* ------------------------------------------------------------------
 * 足し引き
 * ------------------------------------------------------------------ */

/* 効果は重なる。**2 本めの薬で 2 段**（置きかえなら 1 段のままになる）。 */
TEST(two_hastes_stack_into_two_steps) {
    given_normal_speed();

    player_speed_adjust(-1);
    player_speed_adjust(-1);

    ASSERT_EQ_INT(-2, player_speed());
}

TEST(adjusting_starts_from_whatever_the_steps_already_were) {
    given_speed_steps(2);

    player_speed_adjust(1);

    ASSERT_EQ_INT(3, player_speed());
}

TEST(adjusting_by_no_steps_changes_nothing) {
    given_speed_steps(-2);

    player_speed_adjust(0);

    ASSERT_EQ_INT(-2, player_speed());
}

/* 0 を通りすぎる。**重い荷で 1 段遅いキャラクタが急ぎの薬を 2 本飲めば速い。**
 * （休息の数えおとしは 0 で止まるが、こちらは止まらない） */
TEST(adjusting_carries_the_steps_across_zero) {
    given_speed_steps(1);

    player_speed_adjust(-2);

    ASSERT_EQ_INT(-1, player_speed());
}

/* 荷は**差**で届く（inven_ops.c の check_strength が
 * change_speed(新しい段数 − 覚えた段数) を呼ぶ。覚えた段数は burden.c）。
 * 荷を下ろして差が戻れば、合計も戻る。 */
TEST(a_pack_penalty_that_arrives_as_a_difference_goes_away_the_same_way) {
    given_normal_speed();

    player_speed_adjust(2);
    player_speed_adjust(-2);

    ASSERT_EQ_INT(0, player_speed());
}

/* ------------------------------------------------------------------
 * 留めが無い・器の幅
 * ------------------------------------------------------------------ */

/* 薬を 10 本飲めば 10 段。**"Very Fast" の上に留めは無い。** */
TEST(nothing_stops_the_steps_from_going_further_down) {
    given_normal_speed();

    for (int potion = 0; potion < 10; potion++) {
        player_speed_adjust(-1);
    }

    ASSERT_EQ_INT(-10, player_speed());
}

TEST(nothing_stops_the_steps_from_going_further_up) {
    given_normal_speed();

    for (int trap = 0; trap < 10; trap++) {
        player_speed_adjust(1);
    }

    ASSERT_EQ_INT(10, player_speed());
}

/* セーブファイルは short をそのまま戻すので、器の幅いっぱいまで通る。 */
TEST(the_widest_slow_the_container_holds_survives) {
    given_speed_steps(MAX_SHORT);

    ASSERT_EQ_INT(MAX_SHORT, player_speed());
}

TEST(the_widest_haste_the_container_holds_survives) {
    given_speed_steps(-MAX_SHORT);

    ASSERT_EQ_INT(-MAX_SHORT, player_speed());
}

int main(void) {
    RUN_TEST(a_character_nothing_has_happened_to_is_at_normal_speed);
    RUN_TEST(the_steps_come_back_as_they_were_given);
    RUN_TEST(negative_steps_come_back_negative);
    RUN_TEST(reading_the_steps_twice_gives_the_same_answer);
    RUN_TEST(deciding_the_steps_replaces_whatever_was_there);
    RUN_TEST(deciding_zero_steps_is_normal_speed);

    RUN_TEST(hasting_the_character_takes_the_steps_down);
    RUN_TEST(slowing_the_character_takes_the_steps_up);
    RUN_TEST(an_item_of_speed_takes_the_steps_down_by_its_amount);
    RUN_TEST(taking_the_item_of_speed_off_puts_the_steps_back);
    RUN_TEST(haste_and_slowness_cancel_each_other_out);

    RUN_TEST(two_hastes_stack_into_two_steps);
    RUN_TEST(adjusting_starts_from_whatever_the_steps_already_were);
    RUN_TEST(adjusting_by_no_steps_changes_nothing);
    RUN_TEST(adjusting_carries_the_steps_across_zero);
    RUN_TEST(a_pack_penalty_that_arrives_as_a_difference_goes_away_the_same_way);

    RUN_TEST(nothing_stops_the_steps_from_going_further_down);
    RUN_TEST(nothing_stops_the_steps_from_going_further_up);
    RUN_TEST(the_widest_slow_the_container_holds_survives);
    RUN_TEST(the_widest_haste_the_container_holds_survives);

    return TEST_SUMMARY();
}
