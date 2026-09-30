// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「赤外視の届く距離」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 12 つめの問い（1 つめは持っている金、2 つめは腹の具合、3 つめは
 * 画面に出す数字、4 つめは魔力、5 つめは体力、6 つめは階級と経験値、7 つめは
 * 状態の旗、8 つめは装備で決まる耐性・能力、9 つめは一時的な状態の数えおとし、
 * 10 つめは休息の残りターン、11 つめはいまの速さ）。答えは 1 個 —— もとは
 * py.flags.see_infra で、6 ファイルから 9 か所が触っていた。
 *
 * **数えおとしではなく現在値。** 11 つめの速さと同じ形だが、重心は 3 つとも違う:
 *
 *   1. **単位はます。1 ます = 10 フィート。** 人物画面が "%d feet" を書くときに
 *      10 倍する（abilities.c）ので、**窓口が 10 倍した数を返すと画面が
 *      100 フィート単位になる**。ます単位のまま返ることに釘を打つ。
 *   2. **0 が走りだしでない。** 種族が create.c で置く（Human 0・Half-Elf 2・
 *      Elf 3・Halfling 4・Gnome 4・Dwarf 5・Half-Orc 3）ので、**決める窓口の
 *      呼び手が 2 つある**（種族とセーブの読み）。この列の 11 問はどれも
 *      0 から始まったので、決める窓口はセーブの読み専用だった。**種族の置きが
 *      足しになると、作りなおすたびに距離が伸びる。**
 *   3. **出どころが 3 つ重なる。** 種族（置く）・装備（`± p1`）・時限の赤外視
 *      （±1）。装備を外すのと薬が切れるのは**引く向きの足し引き**で、留めが
 *      無いので 0 を通りすぎて負にもなる。
 *
 * ふるまいを変える読み手は creature.c:57 の 1 か所だけで、そこは
 * 「距離のうちか」と「そのモンスターが温かいか（CD_INFRA）」を一緒に訊く。
 * **後半はモンスターの話なのでこの module の外**（player_infra_range.h）。
 * フィートへの換算（abilities.c）と薬のターン数（player_timed_effects.c）も外。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に距離を置きなおす。
 */
/* externs.h は要らない。窓口 1 本と、MAX_SHORT のための constant.h・
 * types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_infra_range.h"

#include "minunit.h"

/* 赤外視を持たない種族（Human）から始める。**窓口で置きなおす** ——
 * 置き場が src/player/player_infra_range.c の static に入っても（#18-12-12C）
 * この足場は届く。 */
static void given_no_infra_vision(void) { player_infra_range_set(0); }

/* 種族が置いた距離から始める（Dwarf なら 5）。 */
static void given_a_race_that_sees(int num_squares) {
    player_infra_range_set(0);
    player_infra_range_set(num_squares);
}

/* ------------------------------------------------------------------
 * 距離そのもの -- ます単位で、0 は見えない
 * ------------------------------------------------------------------ */

TEST(a_human_sees_no_warm_blood_at_any_distance) {
    given_no_infra_vision();

    ASSERT_EQ_INT(0, player_infra_range());
}

TEST(the_range_comes_back_as_the_race_gave_it) {
    given_a_race_that_sees(5);

    ASSERT_EQ_INT(5, player_infra_range());
}

/* **ます単位のまま返る。** 10 倍するのは人物画面の仕事（abilities.c が
 * "%d feet" を書くときに 3 ます → "30 feet" にする）。落とすと画面が
 * 10 倍に見え、creature.c の距離くらべも 10 倍の範囲になる。 */
TEST(the_range_is_in_squares_not_feet) {
    given_a_race_that_sees(3);

    ASSERT_EQ_INT(3, player_infra_range());
}

TEST(reading_the_range_twice_gives_the_same_answer) {
    given_a_race_that_sees(4);

    (void)player_infra_range();

    ASSERT_EQ_INT(4, player_infra_range());
}

/* 決める窓口は**置きかえ**（種族とセーブの読みが呼ぶ。足すのではない）。
 * **足しになると、人物を作りなおすたびに距離が伸びる。** */
TEST(deciding_the_range_replaces_whatever_was_there) {
    given_a_race_that_sees(5);

    player_infra_range_set(2);

    ASSERT_EQ_INT(2, player_infra_range());
}

/* Human を選びなおしたら 0 に戻る（「置かない」ではなく「0 を置く」）。 */
TEST(deciding_zero_squares_is_a_race_with_no_infra_vision) {
    given_a_race_that_sees(5);

    player_infra_range_set(0);

    ASSERT_EQ_INT(0, player_infra_range());
}

/* ------------------------------------------------------------------
 * 足し引き -- 装備と時限の赤外視
 * ------------------------------------------------------------------ */

/* TR_INFRA の品を身につけたとき（player_bonuses.c の py_bonuses(t_ptr, 1)）。 */
TEST(an_item_of_infra_vision_pushes_the_range_out_by_its_amount) {
    given_no_infra_vision();

    player_infra_range_adjust(2);

    ASSERT_EQ_INT(2, player_infra_range());
}

/* 同じ品を外したとき（py_bonuses(t_ptr, -1)）は元に戻る。 */
TEST(taking_the_item_off_brings_the_range_back) {
    given_no_infra_vision();
    player_infra_range_adjust(2);

    player_infra_range_adjust(-2);

    ASSERT_EQ_INT(0, player_infra_range());
}

/* 時限の赤外視が効きはじめたとき（dungeon.c の ++）。**ちょうど 1 ます。** */
TEST(the_potion_pushes_the_range_out_by_one_square) {
    given_no_infra_vision();

    player_infra_range_adjust(1);

    ASSERT_EQ_INT(1, player_infra_range());
}

/* 時限が切れたとき（dungeon.c の --）。 */
TEST(the_potion_running_out_brings_the_range_back_by_one_square) {
    given_no_infra_vision();
    player_infra_range_adjust(1);

    player_infra_range_adjust(-1);

    ASSERT_EQ_INT(0, player_infra_range());
}

/* 種族の上に足される。**Dwarf が薬を飲めば 6 ます。** */
TEST(adjusting_starts_from_whatever_the_race_gave) {
    given_a_race_that_sees(5);

    player_infra_range_adjust(1);

    ASSERT_EQ_INT(6, player_infra_range());
}

/* 出どころは重なる。**種族 3 ＋ 品 2 ＋ 薬 1 = 6 ます。** */
TEST(the_race_the_item_and_the_potion_all_stack) {
    given_a_race_that_sees(3);

    player_infra_range_adjust(2);
    player_infra_range_adjust(1);

    ASSERT_EQ_INT(6, player_infra_range());
}

/* 薬が先に切れても、品のぶんは残る（外れる順番に縛られない）。 */
TEST(the_potion_running_out_leaves_the_item_behind) {
    given_a_race_that_sees(3);
    player_infra_range_adjust(2);
    player_infra_range_adjust(1);

    player_infra_range_adjust(-1);

    ASSERT_EQ_INT(5, player_infra_range());
}

TEST(adjusting_by_no_squares_changes_nothing) {
    given_a_race_that_sees(4);

    player_infra_range_adjust(0);

    ASSERT_EQ_INT(4, player_infra_range());
}

/* ------------------------------------------------------------------
 * 留めが無い -- 0 を通りすぎ、器の幅だけが限り
 * ------------------------------------------------------------------ */

/* 上の留めは無い。品を 10 個ぶん重ねれば 10 ます。 */
TEST(nothing_stops_the_range_from_growing) {
    given_no_infra_vision();

    for (int item = 0; item < 10; item++) {
        player_infra_range_adjust(1);
    }

    ASSERT_EQ_INT(10, player_infra_range());
}

/* 下の留めも無い。**引きすぎれば 0 を通りすぎて負になる**（もとのコードも
 * どこでも検査していない。→ 台帳の所見 24）。creature.c の距離くらべは
 * `see_infra > 0` を先に訊くので、負でもモンスターは見えない。 */
TEST(taking_off_more_than_was_granted_carries_the_range_below_zero) {
    given_a_race_that_sees(1);

    player_infra_range_adjust(-3);

    ASSERT_EQ_INT(-2, player_infra_range());
}

/* セーブファイルは short をそのまま戻すので、器の幅いっぱいまで通る。 */
TEST(the_widest_range_the_container_holds_survives) {
    given_a_race_that_sees(MAX_SHORT);

    ASSERT_EQ_INT(MAX_SHORT, player_infra_range());
}

TEST(the_narrowest_the_container_holds_survives) {
    given_a_race_that_sees(-MAX_SHORT);

    ASSERT_EQ_INT(-MAX_SHORT, player_infra_range());
}

int main(void) {
    RUN_TEST(a_human_sees_no_warm_blood_at_any_distance);
    RUN_TEST(the_range_comes_back_as_the_race_gave_it);
    RUN_TEST(the_range_is_in_squares_not_feet);
    RUN_TEST(reading_the_range_twice_gives_the_same_answer);
    RUN_TEST(deciding_the_range_replaces_whatever_was_there);
    RUN_TEST(deciding_zero_squares_is_a_race_with_no_infra_vision);

    RUN_TEST(an_item_of_infra_vision_pushes_the_range_out_by_its_amount);
    RUN_TEST(taking_the_item_off_brings_the_range_back);
    RUN_TEST(the_potion_pushes_the_range_out_by_one_square);
    RUN_TEST(the_potion_running_out_brings_the_range_back_by_one_square);
    RUN_TEST(adjusting_starts_from_whatever_the_race_gave);
    RUN_TEST(the_race_the_item_and_the_potion_all_stack);
    RUN_TEST(the_potion_running_out_leaves_the_item_behind);
    RUN_TEST(adjusting_by_no_squares_changes_nothing);

    RUN_TEST(nothing_stops_the_range_from_growing);
    RUN_TEST(taking_off_more_than_was_granted_carries_the_range_below_zero);
    RUN_TEST(the_widest_range_the_container_holds_survives);
    RUN_TEST(the_narrowest_the_container_holds_survives);

    return TEST_SUMMARY();
}
