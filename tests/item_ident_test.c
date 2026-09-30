// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* アイテムの効果が判明したときの処理のテスト -- 現在の実装を保護する
 *
 * potions.c / eat.c / scrolls.c に重複していた ident ブロックは
 * src/item/item_ident.c の learn_item_effect() に抽出された。このテストは
 * その実体をリンクして検証する（写しではない）。
 *
 * potions.c / eat.c / scrolls.c 自体はリンクできない。効果処理の巨大な
 * switch が画面表示・ダンジョン・モンスターへ芋づるで依存するため。
 * item_ident.c は known1_p() / identify() / sample() / prt_experience() と
 * グローバルな py / inventory にしか依存しないので、desc.c の本物と
 * fixture.c の代役だけでリンクできる。
 *
 * グローバル状態（inventory, 品目ごとの覚え, py）に依存するので
 * MU_SETUP で fixture_reset() を呼ぶ。これがないと実行順で結果が変わる。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、
 * いまどう振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "item_ident.h"
#include "player_level.h"

#include "fixture.h"

#include "inventory.h"

extern player_type py;

/* 検証に使う本物（src/item/desc.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
int known1_p(inven_type *i_ptr);
void identify(int *item);
void sample(inven_type *i_ptr);
void known1(inven_type *i_ptr);
void store_bought(inven_type *i_ptr);

/* fixture.c のスタブ */
void prt_experience(void);

/* 各テストの前に必ず呼ばれる。グローバル状態が毎回まっさらに戻る。 */
#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* 実体（src/item/item_ident.c）への呼びだし。抽出前の呼びだし側は item_val を
 * 局所変数として持っていたので、ここでも同じく変数に受けて渡す。
 * 戻り値（更新後の i_ptr）はこのテストでは見ない。 */
static void apply_ident(bool ident, int item_val)
{
    (void)learn_item_effect(ident, &item_val);
}

/* テストの条件づくり。分岐やループをテスト本体に持ちこまないため、
 * 「未鑑定の巻物を持ったプレイヤー」をここで組みたてる。
 * 巻物（TV_SCROLL1）を選んだ理由は 7 群のうち 5 番めの群（添字 4）に入り、
 * subval が ITEM_SINGLE_STACK_MIN 以上なら known1_p の判定対象に
 * なること。store_bought フラグは立てないので未鑑定から始まる。 */
static void given_unknown_item(int item_level, int level_reached)
{
    inven_type *i_ptr = inventory_at(0);
    i_ptr->tval = TV_SCROLL1;
    i_ptr->subval = ITEM_SINGLE_STACK_MIN; /* 64。単品スタックの下限 */
    i_ptr->number = 1;
    i_ptr->level = (uint8_t)item_level;
    inventory_set_count(1);
    player_set_level((uint16_t)level_reached);
    player_set_experience_factor(100);
}

/* 枠の選びかた（第 4 節）を見るための条件づくり。品目の種類（tval）と
 * 番号（subval）を指定して、持ち物の指定の枠に未鑑定のまま置く。
 * given_unknown_item() は巻物の subval 64 に決めうちなので、種類と番号を
 * 変えて枠の対応を見るにはこちらを使う。 */
static inven_type *given_a_kind_of_item(int slot, int tval, int subval)
{
    inven_type *i_ptr = inventory_at(slot);
    i_ptr->tval = (uint8_t)tval;
    i_ptr->subval = (uint8_t)subval;
    i_ptr->number = 1;
    i_ptr->level = 5;
    i_ptr->ident = 0;
    if (inventory_count() <= slot) {
        inventory_set_count(slot + 1);
    }
    return i_ptr;
}

/* prt_experience() は fixture.c のスタブ。本物（level_ops.c:55）は
 * 経験値の上限打ち切りとレベルアップ判定と画面描画を兼ねているが、
 * ここで見たいのは加算式なので代役にしている。加算された exp が
 * そのまま観測できる。 */

/* ------------------------------------------------------------------
 * 1. 経験値の加算式  (i_ptr->level + (player_level() >> 1)) / player_level()
 * ------------------------------------------------------------------ */

/* 代表値：プレイヤーレベル 1 では (level + 0) / 1 なのでアイテムの
 * レベルがそのまま経験値になる。 */
TEST(experience_gain_is_item_level_when_player_level_is_one)
{
    given_unknown_item(5, 1);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 5);
}

/* 三角測量：アイテムのレベルが変わればそのまま反映される。 */
TEST(experience_gain_follows_item_level_when_player_level_is_one)
{
    given_unknown_item(12, 1);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 12);
}

/* プレイヤーレベルが上がると 1 件あたりの経験値は減る。
 * (10 + (4>>1)) / 4 = 12/4 = 3。 */
TEST(experience_gain_is_divided_by_player_level)
{
    given_unknown_item(10, 4);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 3);
}

/* 三角測量：レベルが偶数なら lev>>1 は正確に半分。
 * (10 + (2>>1)) / 2 = 11/2 = 5（切り捨て）。 */
TEST(experience_gain_truncates_division_at_even_player_level)
{
    given_unknown_item(10, 2);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 5);
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
    ASSERT_EQ_INT(player_experience(), 3);
}

/* 端数の切りあげが効く境界：真の商が .5 のとき 1 つ上に丸まる。
 * (5 + (2>>1)) / 2 = 6/2 = 3。真の 5/2 = 2.5 なので切りあげ。 */
TEST(experience_gain_rounds_half_way_case_up_at_even_player_level)
{
    given_unknown_item(5, 2);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 3);
}

/* アイテムのレベルがプレイヤーレベルの半分未満だと経験値は 0 になる。
 * (1 + (4>>1)) / 4 = 3/4 = 0。 */
TEST(experience_gain_is_zero_when_item_level_far_below_player_level)
{
    given_unknown_item(1, 4);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 0);
}

/* アイテムのレベルが 0 なら、lev>>1 の項だけでは商に届かず 0 のまま。
 * (0 + (2>>1)) / 2 = 1/2 = 0。 */
TEST(experience_gain_is_zero_for_item_level_zero_at_player_level_two)
{
    given_unknown_item(0, 2);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 0);
}

/* 三角測量：レベル 0 のアイテムでもプレイヤーレベル 1 なら 0/1 = 0。 */
TEST(experience_gain_is_zero_for_item_level_zero_at_player_level_one)
{
    given_unknown_item(0, 1);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 0);
}

/* TODO: 仕様確認 -- 階級が 0 だと 0 除算でクラッシュする。
 * 通常の遊びかたでは到達しない。create.c:86 が 1 で初期化し、経験値を
 * 失う lose_exp()（spells.c）も i を 1 から数えなおすので 1 が下限。
 * ただし階級はセーブファイルから読みこまれる（save.c）ので、
 * 壊れた／改変されたセーブでは 0 になりうる。この式には防御がない。
 * ここでは実装を変えないので、0 を渡すテストは書かない（クラッシュする）。
 *
 * 高レベルのプレイヤーには端数がほとんど効かない。
 * (40 + (40>>1)) / 40 = 60/40 = 1。 */
TEST(experience_gain_is_one_when_item_level_equals_player_level)
{
    given_unknown_item(40, 40);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 1);
}

/* 経験値は上書きではなく加算される。既存の 100 に 5 が足される。 */
TEST(experience_gain_is_added_to_existing_experience)
{
    given_unknown_item(5, 1);
    player_set_experience(100);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 105);
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
    ASSERT_EQ_INT(player_experience(), 0);
}

/* 三角測量：既知でなければ同じ条件で経験値がつく。
 * 上のテストとの差は identify() を先に呼んだかどうかだけ。 */
TEST(experience_is_gained_when_item_is_not_yet_known)
{
    given_unknown_item(5, 1);
    apply_ident(true, 0);
    ASSERT_EQ_INT(player_experience(), 5);
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
 * 表を直に読んでいたが、#18-9-C で実体が item_ident.c の static に移った
 * ので窓口で訊く。見ている枠は同じ（巻物の枠、以前の object_ident[4<<6]）。 */
TEST(unknown_item_is_marked_tried_when_effect_is_not_identified)
{
    given_unknown_item(5, 1);
    apply_ident(false, 0);
    ASSERT_TRUE(item_kind_was_tried(inventory_at(0)));
}

/* 三角測量：ident が真なら「試した」ではなく鑑定済みになる。
 * identify() は OD_TRIED を落とすので、立っていない。 */
TEST(identified_item_is_not_marked_tried)
{
    given_unknown_item(5, 1);
    apply_ident(true, 0);
    ASSERT_FALSE(item_kind_was_tried(inventory_at(0)));
}

/* ident が偽なら経験値はつかない。効果が判明していないので当然。 */
TEST(no_experience_is_gained_when_effect_is_not_identified)
{
    given_unknown_item(5, 1);
    apply_ident(false, 0);
    ASSERT_EQ_INT(player_experience(), 0);
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
    ASSERT_FALSE(item_kind_was_tried(inventory_at(0)));
}

/* ------------------------------------------------------------------
 * 4. 品目ごとの枠の選びかた
 *
 * 品目ごとの覚えは「種類ごとに 64 枠」を 7 並べた 448 枠の表で、枠の
 * 選びかたは desc.c の 6 か所が同じ 3 行を繰りかえして計算していた
 * （群の番号 0〜6 を 6 ビット左へ、番号の下 6 ビットを足す）。#18-9 で
 * この計算は窓口の内側（item_ident.c の record_of()）に入った。
 *
 * ここで見るのは「別の品目が別の枠を使う」ことと「枠を持たない品目は
 * 常に既知になる」こと。どちらも崩れると、ある薬を鑑定したら別の巻物まで
 * 鑑定済みになる／未鑑定の指輪が鑑定済みに見えるという形で出る。
 * ------------------------------------------------------------------ */

/* 同じ種類でも番号が違えば別の枠。巻物 64 を「試した」にしても巻物 65 は
 * 試していない。 */
TEST(two_kinds_of_the_same_sort_keep_separate_records)
{
    inven_type *tried = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    inven_type *untried = given_a_kind_of_item(1, TV_SCROLL1, ITEM_SINGLE_STACK_MIN + 1);
    sample(tried);
    ASSERT_FALSE(item_kind_was_tried(untried));
    ASSERT_EQ_INT(known1_p(untried), 0);
}

/* 種類が違えば、同じ番号でも別の枠（6 ビット左へずらすのがこのため）。
 * 巻物は offset 4、薬は offset 5。 */
TEST(the_same_number_in_different_sorts_keeps_separate_records)
{
    inven_type *scroll = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    inven_type *potion = given_a_kind_of_item(1, TV_POTION1, ITEM_SINGLE_STACK_MIN);
    known1(scroll);
    ASSERT_TRUE(item_kind_is_known(scroll));
    ASSERT_FALSE(item_kind_is_known(potion));
}

/* 番号の上のビットは枠を選ばない。64 は「単品でスタックする」という別の
 * 意味を持つビットで、下 6 ビットだけが品目を指す。**64 と 0 は同じ枠**。 */
TEST(the_stacking_bit_of_the_number_does_not_choose_the_record)
{
    inven_type *stacking = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    inven_type *plain = given_a_kind_of_item(1, TV_SCROLL1, 0);
    known1(stacking);
    ASSERT_EQ_INT(known1_p(plain), OD_KNOWN1);
}

/* 表の 7 並びのいちばん上（食べ物、7 つめの群）も枠を持つ。ここが落ちると
 * キノコだけ鑑定を覚えない表になる。 */
TEST(the_last_sort_of_the_table_has_records_of_its_own)
{
    inven_type *mushroom = given_a_kind_of_item(0, TV_FOOD, 0);
    known1(mushroom);
    ASSERT_TRUE(item_kind_is_known(mushroom));
}

/* 枠を持たない種類（剣など、色や銘のない品目）は**表を引かずに**常に既知。
 * 持ち物の並び順を保つためで、群の番号が -1 になる側。 */
TEST(a_sort_with_no_record_is_always_known)
{
    inven_type *sword = given_a_kind_of_item(0, TV_SWORD, 0);
    ASSERT_EQ_INT(known1_p(sword), OD_KNOWN1);
}

/* 食べ物は番号で分かれる。MAX_MUSH（22）以上はキノコではないので枠を
 * 持たず、常に既知。表の 384〜405 の 22 枠だけが使われる。 */
TEST(food_above_the_mushroom_range_has_no_record_and_is_always_known)
{
    inven_type *bread = given_a_kind_of_item(0, TV_FOOD, MAX_MUSH);
    ASSERT_EQ_INT(known1_p(bread), OD_KNOWN1);
}

/* 三角測量：同じ食べ物でも MAX_MUSH の 1 つ下なら枠を持つので未鑑定。 */
TEST(food_inside_the_mushroom_range_has_a_record_and_starts_unknown)
{
    inven_type *mushroom = given_a_kind_of_item(0, TV_FOOD, MAX_MUSH - 1);
    ASSERT_EQ_INT(known1_p(mushroom), 0);
}

/* 店で買った品は**表を引かずに**既知（店は品名を言うので）。枠は未鑑定の
 * ままなので、同じ品目をダンジョンで拾えばまだ未鑑定に見える。 */
TEST(a_store_bought_item_is_known_without_marking_its_record)
{
    inven_type *bought = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    inven_type *found = given_a_kind_of_item(1, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    store_bought(bought);
    ASSERT_EQ_INT(known1_p(bought), OD_KNOWN1);
    ASSERT_EQ_INT(known1_p(found), 0);
}

/* 鑑定は「試した」を落とす（両方の印が同じ枠の別のビットにある）。
 * desc.c が 2 行で 1 つの枠を書きかえていたところ（#18-9-B で窓口 1 つに）。 */
TEST(marking_a_kind_known_clears_that_it_was_tried)
{
    inven_type *i_ptr = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    sample(i_ptr);
    known1(i_ptr);
    ASSERT_FALSE(item_kind_was_tried(i_ptr));
    ASSERT_TRUE(item_kind_is_known(i_ptr));
}

/* ------------------------------------------------------------------
 * 5. 品目ごとの覚えの窓口（#18-9-A2 で足した src/item/item_ident.c の側）
 *
 * #18-9-C で表そのものが item_ident.c の static になった。ここで見るのは
 * 「窓口が desc.c と同じ表を見ていること」と「枠を持たない品目を訊かれても
 * 答えが定まること」。
 *
 * 別の表の複製になっていれば、上の第 4 節はグリーンのまま窓口だけが
 * 嘘をつく。だから**片方で書いて他方で読む**形で確かめる。
 * ------------------------------------------------------------------ */

/* 窓口で鑑定済みにすると、desc.c の古い読み手からも既知に見える。 */
TEST(marking_a_kind_known_through_the_window_is_seen_by_the_old_reader)
{
    inven_type *i_ptr = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    item_kind_mark_known(i_ptr);
    ASSERT_EQ_INT(known1_p(i_ptr), OD_KNOWN1);
}

/* 逆向き：desc.c が立てた「試した」印が窓口からも見える。 */
TEST(the_window_sees_the_tried_mark_that_desc_set)
{
    inven_type *i_ptr = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    sample(i_ptr);
    ASSERT_TRUE(item_kind_was_tried(i_ptr));
}

/* 窓口も同じ枠の対応を使う。番号が違えば別の枠。 */
TEST(the_window_keeps_separate_records_for_separate_kinds)
{
    inven_type *marked = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    inven_type *other = given_a_kind_of_item(1, TV_SCROLL1, ITEM_SINGLE_STACK_MIN + 1);
    item_kind_mark_known(marked);
    ASSERT_FALSE(item_kind_is_known(other));
}

/* 鑑定は「試した」を落とす（2 行が 1 つの窓口になったところ）。 */
TEST(marking_a_kind_known_through_the_window_clears_the_tried_mark)
{
    inven_type *i_ptr = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    item_kind_mark_tried(i_ptr);
    item_kind_mark_known(i_ptr);
    ASSERT_FALSE(item_kind_was_tried(i_ptr));
}

/* 三角測量：印を落とすだけの窓口は鑑定の印に触らない（unsample の側）。 */
TEST(clearing_the_tried_mark_leaves_the_kind_known)
{
    inven_type *i_ptr = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    item_kind_mark_known(i_ptr);
    item_kind_clear_tried(i_ptr);
    ASSERT_TRUE(item_kind_is_known(i_ptr));
}

/* 2 つの印は別のビット。試しただけでは既知にならない。 */
TEST(trying_a_kind_does_not_make_it_known)
{
    inven_type *i_ptr = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    item_kind_mark_tried(i_ptr);
    ASSERT_FALSE(item_kind_is_known(i_ptr));
}

/* 枠を持たない品目（剣）。known1_p が表を引かずに既知と答える側で、
 * inven_ops.c:223（当時は misc3.c:1025）も #18-9-B より前は object_offset() == -1 で
 * これを訊いていた。 */
TEST(a_kind_with_no_record_is_reported_as_having_none)
{
    inven_type *sword = given_a_kind_of_item(0, TV_SWORD, 0);
    ASSERT_FALSE(item_kind_has_record(sword));
}

/* 三角測量：枠を持つ品目は持つと答える。 */
TEST(a_kind_with_a_record_is_reported_as_having_one)
{
    inven_type *scroll = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    ASSERT_TRUE(item_kind_has_record(scroll));
}

/* 枠を持たない品目を訊かれたら「知らない・試していない」と答える
 * （表の外を読まない）。 */
TEST(a_kind_with_no_record_is_neither_known_nor_tried)
{
    inven_type *sword = given_a_kind_of_item(0, TV_SWORD, 0);
    ASSERT_FALSE(item_kind_is_known(sword));
    ASSERT_FALSE(item_kind_was_tried(sword));
}

/* 枠を持たない品目に印をつけても何も起きない（表の外へ書かない）。
 * 古い形は 5 か所それぞれで offset < 0 を検査して return していた。 */
TEST(marking_a_kind_with_no_record_changes_nothing)
{
    inven_type *sword = given_a_kind_of_item(0, TV_SWORD, 0);
    item_kind_mark_known(sword);
    item_kind_mark_tried(sword);
    ASSERT_FALSE(item_kind_was_tried(sword));
}

/* セーブ用の生の窓口は本物の表を渡す。別の配列の複製だと、読みこんだ
 * 鑑定の記録が誰にも見えない。 */
TEST(the_record_bytes_are_the_table_itself)
{
    inven_type *i_ptr = given_a_kind_of_item(0, TV_SCROLL1, ITEM_SINGLE_STACK_MIN);
    item_kind_record_bytes()[4 << 6] |= OD_KNOWN1;
    ASSERT_TRUE(item_kind_is_known(i_ptr));
}

/* 書きだす長さは表の全体。ここが短いと、セーブファイルが上のほうの種類
 * （薬・キノコ）の記録を落とす。 */
TEST(the_record_count_covers_the_whole_table)
{
    ASSERT_EQ_INT(item_kind_record_count(), OBJECT_IDENT_SIZE);
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

    RUN_TEST(two_kinds_of_the_same_sort_keep_separate_records);
    RUN_TEST(the_same_number_in_different_sorts_keeps_separate_records);
    RUN_TEST(the_stacking_bit_of_the_number_does_not_choose_the_record);
    RUN_TEST(the_last_sort_of_the_table_has_records_of_its_own);
    RUN_TEST(a_sort_with_no_record_is_always_known);
    RUN_TEST(food_above_the_mushroom_range_has_no_record_and_is_always_known);
    RUN_TEST(food_inside_the_mushroom_range_has_a_record_and_starts_unknown);
    RUN_TEST(a_store_bought_item_is_known_without_marking_its_record);
    RUN_TEST(marking_a_kind_known_clears_that_it_was_tried);

    RUN_TEST(marking_a_kind_known_through_the_window_is_seen_by_the_old_reader);
    RUN_TEST(the_window_sees_the_tried_mark_that_desc_set);
    RUN_TEST(the_window_keeps_separate_records_for_separate_kinds);
    RUN_TEST(marking_a_kind_known_through_the_window_clears_the_tried_mark);
    RUN_TEST(clearing_the_tried_mark_leaves_the_kind_known);
    RUN_TEST(trying_a_kind_does_not_make_it_known);
    RUN_TEST(a_kind_with_no_record_is_reported_as_having_none);
    RUN_TEST(a_kind_with_a_record_is_reported_as_having_one);
    RUN_TEST(a_kind_with_no_record_is_neither_known_nor_tried);
    RUN_TEST(marking_a_kind_with_no_record_changes_nothing);
    RUN_TEST(the_record_bytes_are_the_table_itself);
    RUN_TEST(the_record_count_covers_the_whole_table);
    return TEST_SUMMARY();
}
