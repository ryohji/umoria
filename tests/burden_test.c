/* 重さに負けているかどうかの置き場のテスト -- 現在のふるまいを保護する
 *
 * ここにも計算は無い。重さの判定そのものは check_strength()（misc3.c:975）に
 * あり、この module が預かるのは**その結果の覚え**だけ。保護するのは
 * 「置き場としてのふるまい」で、意味のある性質は 4 つある。
 *
 *   1. 2 つは**別の記録**であること。武器の旗と荷の段数は同じ関数
 *      （check_strength）が続けて書くが、片方だけ変わる場合がある（武器を
 *      持ちかえても荷の重さは変わらない）。片方が他方を壊す置き場では
 *      「武器だけ重すぎになった」という状態が作れない。
 *
 *   2. 荷の側は**旗ではなく段数**であること。0 は「重くない」で、1 以上は
 *      速度を何段落とされているか。本体は change_speed(新しい段数 − 覚えた
 *      段数) と差で速度を動かすので、覚えた数が丸められると速度が狂う。
 *      2 も 5 もそのまま覚えることを見る。
 *
 *   3. 覚えた答えをそのまま返すこと。check_strength() は「いま計算した段数」と
 *      「覚えている段数」を比べて、違うときだけメッセージを出して速度を動かす。
 *      読んだときに落ちたり導出しなおしたりすると、その比較が成り立たない。
 *
 *   4. 範囲の検査を**しない**こと。古い代入も検査していなかった。負の数も
 *      大きな数もそのまま入る（本体が入れる値は 0 以上だが、それは
 *      check_strength 側の性質で、置き場の性質ではない）。
 *
 * 走りだしは「武器は重すぎない・荷は速度を落としていない」。変更前の
 * variable.c:40-41 が false と 0 を明示していたので、それを固定する
 * （#18-7-4C1 でその初期値ごと src/burden.c の static に移った）。
 *
 * テストは 1 プロセスで状態を共有する。走りだしの状態を見る 2 件は
 * main() の先頭に置いてある。
 */
/* externs.h は要らない。窓口と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "burden.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 2 件は main() の先頭で走らせる。ほかのテストが書きこむので。 */

TEST(the_weapon_starts_out_light_enough) {
    ASSERT_TRUE(!weapon_is_too_heavy());
}

TEST(the_pack_starts_out_costing_no_speed) {
    ASSERT_EQ_INT(0, pack_speed_penalty());
}

/* --- 武器の旗 ----------------------------------------------------------- */

TEST(a_weapon_that_is_too_heavy_is_remembered) {
    set_weapon_too_heavy(true);
    ASSERT_TRUE(weapon_is_too_heavy());
}

/* 武器を持ちかえたときに落とす（moria1.c の 2 か所）。一度重すぎになったら
 * 戻れない覚えかたではない。 */
TEST(the_weapon_can_become_light_enough_again) {
    set_weapon_too_heavy(true);
    set_weapon_too_heavy(false);
    ASSERT_TRUE(!weapon_is_too_heavy());
}

/* 読んでも消えない。check_strength() は同じ turn のうちに読んでから書く。 */
TEST(asking_about_the_weapon_twice_gives_the_same_answer) {
    set_weapon_too_heavy(true);
    (void)weapon_is_too_heavy();
    ASSERT_TRUE(weapon_is_too_heavy());
}

/* --- 荷の段数 ----------------------------------------------------------- */

TEST(the_speed_the_pack_costs_is_remembered) {
    set_pack_speed_penalty(2);
    ASSERT_EQ_INT(2, pack_speed_penalty());
}

/* 旗ではないので、2 と 5 は違う答えとして覚えられていなければならない。 */
TEST(a_heavier_pack_costs_more_than_a_lighter_one) {
    set_pack_speed_penalty(2);
    set_pack_speed_penalty(5);
    ASSERT_EQ_INT(5, pack_speed_penalty());
}

/* 荷を降ろしたとき（save.c の 2 か所と check_strength）。 */
TEST(the_pack_can_go_back_to_costing_nothing) {
    set_pack_speed_penalty(5);
    set_pack_speed_penalty(0);
    ASSERT_EQ_INT(0, pack_speed_penalty());
}

/* --- 2 つが別の記録であること ------------------------------------------- */

TEST(the_weapon_flag_does_not_disturb_the_pack) {
    set_pack_speed_penalty(3);
    set_weapon_too_heavy(true);
    ASSERT_EQ_INT(3, pack_speed_penalty());
}

TEST(the_pack_does_not_disturb_the_weapon_flag) {
    set_weapon_too_heavy(false);
    set_pack_speed_penalty(4);
    ASSERT_TRUE(!weapon_is_too_heavy());
}

/* --- 検査しないこと ----------------------------------------------------- */

/* 負の段数。本体は 0 以上しか入れないが、窓口は弾かない（古い代入と同じ）。 */
TEST(the_window_keeps_a_negative_number_of_steps) {
    set_pack_speed_penalty(-3);
    ASSERT_EQ_INT(-3, pack_speed_penalty());
}

/* 大きな段数も丸めない。荷の重さは制限の何倍かで決まるので上限は無い。 */
TEST(the_window_does_not_cap_the_number_of_steps) {
    set_pack_speed_penalty(1000);
    ASSERT_EQ_INT(1000, pack_speed_penalty());
}

int main(void) {
    /* 走りだしの状態を見る 2 件を最初に。以降のテストが書きこむ。 */
    RUN_TEST(the_weapon_starts_out_light_enough);
    RUN_TEST(the_pack_starts_out_costing_no_speed);

    RUN_TEST(a_weapon_that_is_too_heavy_is_remembered);
    RUN_TEST(the_weapon_can_become_light_enough_again);
    RUN_TEST(asking_about_the_weapon_twice_gives_the_same_answer);

    RUN_TEST(the_speed_the_pack_costs_is_remembered);
    RUN_TEST(a_heavier_pack_costs_more_than_a_lighter_one);
    RUN_TEST(the_pack_can_go_back_to_costing_nothing);

    RUN_TEST(the_weapon_flag_does_not_disturb_the_pack);
    RUN_TEST(the_pack_does_not_disturb_the_weapon_flag);

    RUN_TEST(the_window_keeps_a_negative_number_of_steps);
    RUN_TEST(the_window_does_not_cap_the_number_of_steps);

    return TEST_SUMMARY();
}
