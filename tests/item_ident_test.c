/* アイテムの効果が判明したときの処理のテスト -- 現在の実装を保護する
 *
 * potions.c / eat.c / scrolls.c に重複していた ident ブロックは
 * src/item_ident.c の learn_item_effect() に抽出された。このテストは
 * その実体をリンクして検証する（写しではない）。
 *
 * potions.c / eat.c / scrolls.c 自体はリンクできない。効果処理の巨大な
 * switch が画面表示・ダンジョン・モンスターへ芋づるで依存するため。
 * item_ident.c は known1_p() / identify() / sample() / prt_experience() と
 * グローバルな py / inventory にしか依存しないので、desc.c の本物と
 * fixture.c の代役だけでリンクできる。
 *
 * グローバル状態（inventory, object_ident, py）に依存するので
 * MU_SETUP で fixture_reset() を呼ぶ。これがないと実行順で結果が変わる。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、
 * いまどう振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "item_ident.h"

#include "fixture.h"

#include "inventory.h"

extern player_type py;
extern uint8_t object_ident[];

/* 検証に使う本物（src/desc.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
int known1_p(inven_type *i_ptr);
void identify(int *item);
void sample(inven_type *i_ptr);

/* fixture.c のスタブ */
void prt_experience(void);

/* 各テストの前に必ず呼ばれる。グローバル状態が毎回まっさらに戻る。 */
#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* 実体（src/item_ident.c）への呼びだし。抽出前の呼びだし側は item_val を
 * 局所変数として持っていたので、ここでも同じく変数に受けて渡す。
 * 戻り値（更新後の i_ptr）はこのテストでは見ない。 */
static void apply_ident(bool ident, int item_val)
{
    (void)learn_item_effect(ident, &item_val);
}

/* テストの条件づくり。分岐やループをテスト本体に持ちこまないため、
 * 「未鑑定の巻物を持ったプレイヤー」をここで組みたてる。
 * 巻物（TV_SCROLL1）を選んだ理由は object_offset() が 4 を返し、
 * subval が ITEM_SINGLE_STACK_MIN 以上なら known1_p の判定対象に
 * なること。store_bought フラグは立てないので未鑑定から始まる。 */
static void given_unknown_item(int item_level, int player_level)
{
    inven_type *i_ptr = inventory_at(0);
    i_ptr->tval = TV_SCROLL1;
    i_ptr->subval = ITEM_SINGLE_STACK_MIN; /* 64。単品スタックの下限 */
    i_ptr->number = 1;
    i_ptr->level = (uint8_t)item_level;
    inventory_set_count(1);
    py.misc.lev = (uint16_t)player_level;
    py.misc.expfact = 100;
}

/* prt_experience() は fixture.c のスタブ。本物（misc3.c:1838）は
 * 経験値の上限打ち切りとレベルアップ判定と画面描画を兼ねているが、
 * ここで見たいのは加算式なので代役にしている。加算された exp が
 * そのまま観測できる。 */

/* ------------------------------------------------------------------
 * 1. 経験値の加算式  (i_ptr->level + (m_ptr->lev >> 1)) / m_ptr->lev
 * ------------------------------------------------------------------ */

/* 代表値：プレイヤーレベル 1 では (level + 0) / 1 なのでアイテムの
 * レベルがそのまま経験値になる。 */
TEST(experience_gain_is_item_level_when_player_level_is_one)
{
    given_unknown_item(5, 1);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 5);
}

/* 三角測量：アイテムのレベルが変わればそのまま反映される。 */
TEST(experience_gain_follows_item_level_when_player_level_is_one)
{
    given_unknown_item(12, 1);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 12);
}

/* プレイヤーレベルが上がると 1 件あたりの経験値は減る。
 * (10 + (4>>1)) / 4 = 12/4 = 3。 */
TEST(experience_gain_is_divided_by_player_level)
{
    given_unknown_item(10, 4);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 3);
}

/* 三角測量：レベルが偶数なら lev>>1 は正確に半分。
 * (10 + (2>>1)) / 2 = 11/2 = 5（切り捨て）。 */
TEST(experience_gain_truncates_division_at_even_player_level)
{
    given_unknown_item(10, 2);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 5);
}

/* 三角測量：レベルが奇数の場合。lev>>1 は切り捨てられるが、
 * 分母が奇数だと真の商が x.5 になりえないので、結果は四捨五入と一致する。
 * (10 + (3>>1)) / 3 = 11/3 = 3。真の 10/3 = 3.33 の四捨五入も 3。
 * （lev が 3, 5 の全ケースで四捨五入と一致することを確認済み。
 *   実装コメントの「round half-way case up」は正確） */
TEST(experience_gain_shifts_odd_player_level_down_before_dividing)
{
    given_unknown_item(10, 3);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 3);
}

/* 端数の切りあげが効く境界：真の商が .5 のとき 1 つ上に丸まる。
 * (5 + (2>>1)) / 2 = 6/2 = 3。真の 5/2 = 2.5 なので切りあげ。 */
TEST(experience_gain_rounds_half_way_case_up_at_even_player_level)
{
    given_unknown_item(5, 2);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 3);
}

/* アイテムのレベルがプレイヤーレベルの半分未満だと経験値は 0 になる。
 * (1 + (4>>1)) / 4 = 3/4 = 0。 */
TEST(experience_gain_is_zero_when_item_level_far_below_player_level)
{
    given_unknown_item(1, 4);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 0);
}

/* アイテムのレベルが 0 なら、lev>>1 の項だけでは商に届かず 0 のまま。
 * (0 + (2>>1)) / 2 = 1/2 = 0。 */
TEST(experience_gain_is_zero_for_item_level_zero_at_player_level_two)
{
    given_unknown_item(0, 2);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 0);
}

/* 三角測量：レベル 0 のアイテムでもプレイヤーレベル 1 なら 0/1 = 0。 */
TEST(experience_gain_is_zero_for_item_level_zero_at_player_level_one)
{
    given_unknown_item(0, 1);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 0);
}

/* TODO: 仕様確認 -- m_ptr->lev が 0 だと 0 除算でクラッシュする。
 * 通常の遊びかたでは到達しない。create.c:86 が 1 で初期化し、経験値を
 * 失う lose_exp()（spells.c:1956）も i を 1 から数えなおすので 1 が下限。
 * ただし m_ptr->lev はセーブファイルから読みこまれる（save.c:655）ので、
 * 壊れた／改変されたセーブでは 0 になりうる。この式には防御がない。
 * ここでは実装を変えないので、0 を渡すテストは書かない（クラッシュする）。
 *
 * 高レベルのプレイヤーには端数がほとんど効かない。
 * (40 + (40>>1)) / 40 = 60/40 = 1。 */
TEST(experience_gain_is_one_when_item_level_equals_player_level)
{
    given_unknown_item(40, 40);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 1);
}

/* 経験値は上書きではなく加算される。既存の 100 に 5 が足される。 */
TEST(experience_gain_is_added_to_existing_experience)
{
    given_unknown_item(5, 1);
    py.misc.exp = 100;
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 105);
}

/* ------------------------------------------------------------------
 * 2. known1_p による分岐
 * ------------------------------------------------------------------ */

/* 前提の確認：この条件づくりで作るアイテムは未鑑定から始まる。
 * これが崩れると以下のテストすべてが意味を失うので固定する。 */
TEST(item_prepared_by_fixture_starts_unknown)
{
    given_unknown_item(5, 1);
    ASSERT_EQ_INT(known1_p(inventory_at(0)), 0);
}

/* ident が真で未鑑定なら identify() が呼ばれ、既知になる。 */
TEST(unknown_item_becomes_known_when_effect_is_identified)
{
    given_unknown_item(5, 1);
    apply_ident(true, 0);
    ASSERT_EQ_INT(known1_p(inventory_at(0)), OD_KNOWN1);
}

/* すでに既知なら経験値はつかない。二重取得を防ぐ分岐。 */
TEST(no_experience_is_gained_when_item_is_already_known)
{
    given_unknown_item(5, 1);
    identify(&(int){0});
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 0);
}

/* 三角測量：既知でなければ同じ条件で経験値がつく。
 * 上のテストとの差は identify() を先に呼んだかどうかだけ。 */
TEST(experience_is_gained_when_item_is_not_yet_known)
{
    given_unknown_item(5, 1);
    apply_ident(true, 0);
    ASSERT_EQ_INT(py.misc.exp, 5);
}

/* 既知のアイテムは identify() も走らないので、既知のまま変わらない。 */
TEST(already_known_item_stays_known_after_identifying_effect)
{
    given_unknown_item(5, 1);
    identify(&(int){0});
    apply_ident(true, 0);
    ASSERT_EQ_INT(known1_p(inventory_at(0)), OD_KNOWN1);
}

/* ------------------------------------------------------------------
 * 3. ident が偽のときは sample() が呼ばれる
 * ------------------------------------------------------------------ */

/* sample() は object_ident に OD_TRIED を立てる。効果が判明しなかった
 * 未知のアイテムは「試した」と記録され、説明に反映される。
 * 巻物（object_offset() == 4）の subval 64 は object_ident[4<<6] を使う。 */
TEST(unknown_item_is_marked_tried_when_effect_is_not_identified)
{
    given_unknown_item(5, 1);
    apply_ident(false, 0);
    ASSERT_EQ_INT(object_ident[4 << 6] & OD_TRIED, OD_TRIED);
}

/* 三角測量：ident が真なら「試した」ではなく鑑定済みになる。
 * identify() は OD_TRIED を落とすので、立っていない。 */
TEST(identified_item_is_not_marked_tried)
{
    given_unknown_item(5, 1);
    apply_ident(true, 0);
    ASSERT_EQ_INT(object_ident[4 << 6] & OD_TRIED, 0);
}

/* ident が偽なら経験値はつかない。効果が判明していないので当然。 */
TEST(no_experience_is_gained_when_effect_is_not_identified)
{
    given_unknown_item(5, 1);
    apply_ident(false, 0);
    ASSERT_EQ_INT(py.misc.exp, 0);
}

/* ident が偽なら鑑定もされない。未知のまま。 */
TEST(item_stays_unknown_when_effect_is_not_identified)
{
    given_unknown_item(5, 1);
    apply_ident(false, 0);
    ASSERT_EQ_INT(known1_p(inventory_at(0)), 0);
}

/* 既知のアイテムには sample() が呼ばれない（else if の条件が偽）。
 * すでに知っているものを「試した」と記録する意味がないため。
 * identify() が OD_TRIED を落としたあと、立てなおされないことを見る。 */
TEST(already_known_item_is_not_marked_tried_when_effect_is_not_identified)
{
    given_unknown_item(5, 1);
    identify(&(int){0});
    apply_ident(false, 0);
    ASSERT_EQ_INT(object_ident[4 << 6] & OD_TRIED, 0);
}

int main(void)
{
    RUN_TEST(experience_gain_is_item_level_when_player_level_is_one);
    RUN_TEST(experience_gain_follows_item_level_when_player_level_is_one);
    RUN_TEST(experience_gain_is_divided_by_player_level);
    RUN_TEST(experience_gain_truncates_division_at_even_player_level);
    RUN_TEST(experience_gain_shifts_odd_player_level_down_before_dividing);
    RUN_TEST(experience_gain_rounds_half_way_case_up_at_even_player_level);
    RUN_TEST(experience_gain_is_zero_when_item_level_far_below_player_level);
    RUN_TEST(experience_gain_is_zero_for_item_level_zero_at_player_level_two);
    RUN_TEST(experience_gain_is_zero_for_item_level_zero_at_player_level_one);
    RUN_TEST(experience_gain_is_one_when_item_level_equals_player_level);
    RUN_TEST(experience_gain_is_added_to_existing_experience);
    RUN_TEST(item_prepared_by_fixture_starts_unknown);
    RUN_TEST(unknown_item_becomes_known_when_effect_is_identified);
    RUN_TEST(no_experience_is_gained_when_item_is_already_known);
    RUN_TEST(experience_is_gained_when_item_is_not_yet_known);
    RUN_TEST(already_known_item_stays_known_after_identifying_effect);
    RUN_TEST(unknown_item_is_marked_tried_when_effect_is_not_identified);
    RUN_TEST(identified_item_is_not_marked_tried);
    RUN_TEST(no_experience_is_gained_when_effect_is_not_identified);
    RUN_TEST(item_stays_unknown_when_effect_is_not_identified);
    RUN_TEST(already_known_item_is_not_marked_tried_when_effect_is_not_identified);
    return TEST_SUMMARY();
}
