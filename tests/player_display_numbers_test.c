/* 「画面に出す数字」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 3 つめの問い（1 つめは持っている金、2 つめは腹の具合）。
 * フィールドは 4 つあるが 1 つの問いに答えている —— 人物画面と勘定欄と
 * 書きだしに出る「+ To Hit」「+ To Damage」「+ To AC」「Total AC」で、
 * もとは py.misc.dis_th・dis_td・dis_tac・dis_ac。
 *
 * **これは戦いに使う数ではない。** 戦いに使うのは本物のほう
 * （ptohit・ptodam・ptoac・pac）で、そちらは py に残る。この問いが持って
 * いるのは「人物に何と言うか」だけ。
 *
 * 保護したい性質は 6 つ。
 *
 *   1. **走りだしは 4 つとも 0** であること（もとの 4 つは初期化子なしの
 *      構造体の中にあった）。本物の値は create.c が人物を作りおわってから、
 *      あるいは save.c がファイルから読んで置く。
 *
 *   2. **組みなおしは本物の写しから始まる**こと。
 *      player_display_start_from_real() が 3 つの修正を本物から写し、
 *      **鎧の合計だけは 0 に戻す**（まだ 1 つも数えていないから）。
 *      moria1.c の calc_bonuses() と create.c が同じ 4 行を書いていた。
 *
 *   3. **足すのは 1 つずつで、窓口は理由を知らない**こと。鑑定済みか・
 *      呪われているか・弓か・武器が重すぎるかを決めるのは呼び手（moria1.c）で、
 *      窓口が受けとるのは数だけ。**負の数も通る** —— 呪いの品も、重すぎる
 *      武器の罰（本物の ptohit には乗らない）も、下げる側に来る。
 *
 *   4. **「+ To AC」は「Total AC」の一部**であること。装備を数えおわったら
 *      player_display_fold_to_ac() で畳む。**畳むのは足し算なので 2 度呼ぶと
 *      2 度乗る** —— 呼び手は 1 度だけ呼ぶ（もとの `dis_ac += dis_tac;` と
 *      同じ）。
 *
 *   5. **窓口は上も下も見ない**こと。丸めない・止めない・0 で下げ止まらない。
 *      無敵の +100 と祝福の +2 は dungeon.c が旗の変わり目に直に足し引きして
 *      いて（calc_bonuses() を呼びなおさない）、**足した順と引いた順が違っても
 *      元に戻る**。
 *
 *   6. **4 つは互いに独立**であること。1 つの窓口が触るのは 1 つの数だけ
 *      （save の読みが 4 つを入れちがえていないことの裏取りでもある）。
 *
 * 置き場は #18-12-3C で src/player_display_numbers.c の static になり、
 * 初期値もそこに入った（それまで要っていた足場
 * tests/player_display_numbers_fixture.c は消した。static にしたあとも同じ
 * 名前の器を足場に残すと、窓口に届かない別の器が生き残ってテストが何も
 * 検証しなくなる —— HANDOVER 第 7 節）。テストは 1 プロセスで
 * 状態を共有するので、走りだしを見る 1 件は main() の先頭に置き、以降は
 * 各件が最初に窓口で足場を作る。
 */
/* externs.h は要らない。窓口と、int16_t のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_display_numbers.h"

#include "minunit.h"

/* --- 走りだし ------------------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが状態を動かすので。 */

TEST(the_sheet_starts_at_nothing) {
    ASSERT_EQ_INT(0, player_display_to_hit());
    ASSERT_EQ_INT(0, player_display_to_dam());
    ASSERT_EQ_INT(0, player_display_to_ac());
    ASSERT_EQ_INT(0, player_display_ac());
}

/* --- 本物の写しから始める ------------------------------------------------- */

/* もとの 4 行（moria1.c:117-120）。3 つは本物から写し、鎧の合計は 0。 */
TEST(starting_over_copies_the_real_plusses) {
    player_display_start_from_real(3, 4, 5);

    ASSERT_EQ_INT(3, player_display_to_hit());
    ASSERT_EQ_INT(4, player_display_to_dam());
    ASSERT_EQ_INT(5, player_display_to_ac());
    ASSERT_EQ_INT(0, player_display_ac());
}

/* **組みなおしは前回の答えを引きずらない。** calc_bonuses() は装備が変わる
 * たびに呼ばれるので、ここで 0 に戻らないと鎧が積みあがっていく。 */
TEST(starting_over_forgets_what_the_sheet_said) {
    player_display_start_from_real(3, 4, 5);
    player_display_add_to_hit(10);
    player_display_add_ac(20);

    player_display_start_from_real(1, 1, 1);

    ASSERT_EQ_INT(1, player_display_to_hit());
    ASSERT_EQ_INT(1, player_display_to_dam());
    ASSERT_EQ_INT(1, player_display_to_ac());
    ASSERT_EQ_INT(0, player_display_ac());
}

/* 本物が負なら写しも負（能力値が低いと tohit_adj() は負を返す）。 */
TEST(the_real_plusses_can_be_negative) {
    player_display_start_from_real(-2, -3, -1);

    ASSERT_EQ_INT(-2, player_display_to_hit());
    ASSERT_EQ_INT(-3, player_display_to_dam());
    ASSERT_EQ_INT(-1, player_display_to_ac());
}

/* --- 1 つずつ足す --------------------------------------------------------- */

TEST(the_equipment_adds_up) {
    player_display_start_from_real(0, 0, 0);

    player_display_add_to_hit(2);
    player_display_add_to_hit(3);
    player_display_add_to_dam(1);
    player_display_add_to_ac(4);
    player_display_add_ac(8);
    player_display_add_ac(7);

    ASSERT_EQ_INT(5, player_display_to_hit());
    ASSERT_EQ_INT(1, player_display_to_dam());
    ASSERT_EQ_INT(4, player_display_to_ac());
    ASSERT_EQ_INT(15, player_display_ac());
}

/* **下げる側も通る。** 鑑定済みの呪いの品は修正が負で、重すぎる武器の罰
 * （use_stat[A_STR] * 15 - weight）は当たり判定の写しにだけ乗る。
 * 窓口は 0 で止めない。 */
TEST(a_penalty_lowers_what_the_sheet_says) {
    player_display_start_from_real(1, 0, 0);
    player_display_add_to_hit(-40);

    ASSERT_EQ_INT(-39, player_display_to_hit());
}

TEST(the_shown_armour_can_go_below_nothing) {
    player_display_start_from_real(0, 0, 0);
    player_display_add_ac(-5);

    ASSERT_EQ_INT(-5, player_display_ac());
}

/* 足しても引いても元に戻る（呼び手が対で呼ぶから）。 */
TEST(adding_and_taking_off_comes_back) {
    player_display_start_from_real(0, 0, 0);
    player_display_add_ac(12);

    player_display_add_ac(100);
    player_display_add_ac(2);
    ASSERT_EQ_INT(114, player_display_ac());

    /* 足した順と引いた順は違う（dungeon.c は旗の変わり目で別々に動く）。 */
    player_display_add_ac(-2);
    player_display_add_ac(-100);
    ASSERT_EQ_INT(12, player_display_ac());
}

/* --- 「+ To AC」を「Total AC」に畳む -------------------------------------- */

TEST(folding_puts_the_bonus_into_the_total) {
    player_display_start_from_real(0, 0, 6);
    player_display_add_ac(10);

    player_display_fold_to_ac();

    ASSERT_EQ_INT(16, player_display_ac());
    /* 畳んでも「+ To AC」の欄は残る。人物画面は両方を出す。 */
    ASSERT_EQ_INT(6, player_display_to_ac());
}

/* **畳むのは足し算であって置きかえではない。** 2 度呼べば 2 度乗る。
 * もとの `dis_ac += dis_tac;` と同じで、呼び手は 1 度だけ呼ぶ。 */
TEST(folding_twice_counts_the_bonus_twice) {
    player_display_start_from_real(0, 0, 6);
    player_display_fold_to_ac();
    player_display_fold_to_ac();

    ASSERT_EQ_INT(12, player_display_ac());
}

/* 畳んだあとに「+ To AC」を動かしても合計は動かない（順番に意味がある。
 * 呼び手は装備を数えおわってから畳む）。 */
TEST(the_bonus_moves_after_folding_without_moving_the_total) {
    player_display_start_from_real(0, 0, 6);
    player_display_fold_to_ac();
    ASSERT_EQ_INT(6, player_display_ac());

    player_display_add_to_ac(3);

    ASSERT_EQ_INT(9, player_display_to_ac());
    ASSERT_EQ_INT(6, player_display_ac());
}

/* --- 置きかえる（save の読み） -------------------------------------------- */

TEST(setting_replaces_what_was_there) {
    player_display_start_from_real(0, 0, 0);

    player_display_set_to_hit(7);
    player_display_set_to_dam(8);
    player_display_set_to_ac(9);
    player_display_set_ac(10);

    ASSERT_EQ_INT(7, player_display_to_hit());
    ASSERT_EQ_INT(8, player_display_to_dam());
    ASSERT_EQ_INT(9, player_display_to_ac());
    ASSERT_EQ_INT(10, player_display_ac());

    player_display_set_ac(-1);
    ASSERT_EQ_INT(-1, player_display_ac());
}

/* --- 4 つは互いに独立 ----------------------------------------------------- */

/* 1 つの窓口が触るのは 1 つの数だけ。**入れちがえの裏取り** ——
 * save の読みは 4 つを別々に置くので、窓口の中で混ざっていると分からない。 */
TEST(each_number_stands_on_its_own) {
    player_display_set_to_hit(1);
    player_display_set_to_dam(2);
    player_display_set_to_ac(3);
    player_display_set_ac(4);

    player_display_add_to_hit(10);
    ASSERT_EQ_INT(11, player_display_to_hit());
    ASSERT_EQ_INT(2, player_display_to_dam());
    ASSERT_EQ_INT(3, player_display_to_ac());
    ASSERT_EQ_INT(4, player_display_ac());

    player_display_add_to_dam(10);
    ASSERT_EQ_INT(11, player_display_to_hit());
    ASSERT_EQ_INT(12, player_display_to_dam());
    ASSERT_EQ_INT(3, player_display_to_ac());
    ASSERT_EQ_INT(4, player_display_ac());

    player_display_add_to_ac(10);
    ASSERT_EQ_INT(11, player_display_to_hit());
    ASSERT_EQ_INT(12, player_display_to_dam());
    ASSERT_EQ_INT(13, player_display_to_ac());
    ASSERT_EQ_INT(4, player_display_ac());

    player_display_add_ac(10);
    ASSERT_EQ_INT(11, player_display_to_hit());
    ASSERT_EQ_INT(12, player_display_to_dam());
    ASSERT_EQ_INT(13, player_display_to_ac());
    ASSERT_EQ_INT(14, player_display_ac());
}

/* --- 通しで 1 回、calc_bonuses() の並びどおりに ---------------------------- */

/* **呼び手の並びを写しとる**（HANDOVER 第 4 節「テスト側に写す」）。
 * moria1.c:117-163 の順に窓口を呼ぶ。装備は 3 つ ——
 *
 *   - 鑑定済みの剣（+2 当たり・+3 傷・鎧 0）
 *   - 鑑定前の呪われていない鎧（素の鎧 8。修正は出さない）
 *   - 鑑定前の呪われた盾（**素の鎧 5 さえ出さない**）
 *
 * そのうえで武器が重すぎる罰（-20）と祝福（+2）が乗る。本物は
 * 8 + 5 = 13 の鎧を持つが、**画面には 8 しか出ない**。 */
TEST(a_whole_rebuild_follows_the_callers_order) {
    /* 本物の修正が +1 / +0 / +4 の人物 */
    player_display_start_from_real(1, 0, 4);

    /* 鑑定済みの剣 */
    player_display_add_to_hit(2);
    player_display_add_to_dam(3);

    /* 鑑定前・呪われていない鎧 —— 素の鎧だけが見える */
    player_display_add_ac(8);

    /* 鑑定前・呪われた盾 —— 何も見えない（呼び手が窓口を呼ばない） */

    /* 装備を数えおわったので畳む */
    player_display_fold_to_ac();

    /* 重すぎる武器の罰。本物の ptohit には乗らない */
    player_display_add_to_hit(-20);

    /* 祝福（+2）。無敵なら +100 */
    player_display_add_ac(2);

    ASSERT_EQ_INT(-17, player_display_to_hit()); /* 1 + 2 - 20 */
    ASSERT_EQ_INT(3, player_display_to_dam());   /* 0 + 3 */
    ASSERT_EQ_INT(4, player_display_to_ac());    /* 4（見える修正は無い） */
    ASSERT_EQ_INT(14, player_display_ac());      /* 8 + 4 + 2。13 の鎧のうち 8 */
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが状態を動かす。 */
    RUN_TEST(the_sheet_starts_at_nothing);

    RUN_TEST(starting_over_copies_the_real_plusses);
    RUN_TEST(starting_over_forgets_what_the_sheet_said);
    RUN_TEST(the_real_plusses_can_be_negative);

    RUN_TEST(the_equipment_adds_up);
    RUN_TEST(a_penalty_lowers_what_the_sheet_says);
    RUN_TEST(the_shown_armour_can_go_below_nothing);
    RUN_TEST(adding_and_taking_off_comes_back);

    RUN_TEST(folding_puts_the_bonus_into_the_total);
    RUN_TEST(folding_twice_counts_the_bonus_twice);
    RUN_TEST(the_bonus_moves_after_folding_without_moving_the_total);

    RUN_TEST(setting_replaces_what_was_there);

    RUN_TEST(each_number_stands_on_its_own);

    RUN_TEST(a_whole_rebuild_follows_the_callers_order);

    return TEST_SUMMARY();
}
