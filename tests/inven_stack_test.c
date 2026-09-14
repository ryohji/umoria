/* アイテムのスタック（重ね置き）可否判定のテスト -- 現在の実装を保護する
 *
 * misc3.c:1161 に「this code must be identical to the inven_carry() code
 * below」、misc3.c:1241 に対応するコメントがある。inven_check_num() と
 * inven_carry() が同じ条件を二重に持っており、片方だけ直すとアイテムが
 * 消失する。開発者自身が同一性の必要を認識している箇所。
 *
 * このテストは写しではなく src/misc3.c の実体をリンクして検証する。
 * misc3.c は 2212 行・6 責務（画面描画・能力値計算・持ち物管理・呪文・
 * 戦闘・移動）が同居しているので、リンクすると 57 個の未解決シンボルが
 * 芋づるで付いてくる。それを tests/misc3_stubs.c の代役で埋めた。
 * 定数表（tables.c）・インベントリ（treasure.c）・py と各種テーブル
 * （player.c）・known1_p（desc.c）は本物をリンクしている。
 *
 * 写しにしなかった理由: ステップ B で条件を 1 箇所に抽出するとき、
 * 写しでは抽出後の実体を検証できず保護にならない。
 *
 * グローバル状態（inventory, object_ident, py, inven_ctr）に依存するので
 * MU_SETUP で fixture_reset() を呼ぶ。これがないと実行順で結果が変わる。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、
 * いまどう振るまうかを固定することが目的。
 */
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"
#include "inventory.h"

/* 検証対象（src/misc3.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
bool inven_check_num(inven_type *t_ptr);
int inven_carry(inven_type *i_ptr);

/* 判定に使う本物（src/desc.c） */
int known1_p(inven_type *i_ptr);
void known1(inven_type *i_ptr);

/* 各テストの前に必ず呼ばれる。グローバル状態が毎回まっさらに戻る。 */
#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* ------------------------------------------------------------------
 * 条件づくりの補助関数
 *
 * inven_check_num() は inven_ctr < INVEN_WIELD なら無条件で true を
 * 返すので、スタック条件そのものを見るにはインベントリを満杯に
 * しなければならない。以下の関数はその条件を作る。
 * ------------------------------------------------------------------ */

/* 巻物（TV_SCROLL1）を選んだ理由は object_offset() が 4 を返し、
 * subval が ITEM_SINGLE_STACK_MIN 以上なら known1_p() の判定対象に
 * なること。ID_STOREBOUGHT は立てないので未鑑定から始まる。 */
static void set_item(inven_type *i_ptr, int tval, int subval, int number,
                     int p1)
{
    /* 先に丸ごと消す。ident を残すと前のテストの鑑定状態が漏れる。
     * new_item は static なので fixture_reset() では消えない。 */
    memset(i_ptr, 0, sizeof *i_ptr);
    i_ptr->tval = (uint8_t)tval;
    i_ptr->subval = (uint8_t)subval;
    i_ptr->number = (uint8_t)number;
    i_ptr->p1 = (int16_t)p1;
    i_ptr->weight = 1;
}

/* インベントリを INVEN_WIELD 個で埋める。inven_ctr < INVEN_WIELD の
 * 早期 return を通りすぎさせ、スタック条件の評価に入らせるため。
 *
 * 埋めぐさには inventory[0] と衝突しない値を使う。tval は TV_NOTHING
 * ではなく TV_FOOD、subval は 0 にして「スタックできないが存在する」
 * 状態を作る（subval 0 < ITEM_SINGLE_STACK_MIN なので条件 3 で落ちる）。 */
static void given_full_inventory(void)
{
    for (int i = 1; i < INVEN_WIELD; i++) {
        set_item(inventory_at(i), TV_FOOD, 0, 1, 0);
    }
    inventory_set_count(INVEN_WIELD);
}

/* インベントリの先頭に既存アイテムを置き、残りを埋めて満杯にする。
 * これが「スタック先の候補が 1 つだけある満杯のインベントリ」。 */
static void given_existing_item(int subval, int number, int p1)
{
    given_full_inventory();
    set_item(inventory_at(0), TV_SCROLL1, subval, number, p1);
}

/* 新規アイテムを組みたてて返す。static にして寿命を確保する。 */
static inven_type new_item;

static inven_type *incoming_item(int tval, int subval, int number, int p1)
{
    set_item(&new_item, tval, subval, number, p1);
    return &new_item;
}

/* ------------------------------------------------------------------
 * 0. inven_ctr < INVEN_WIELD の早期 return
 * ------------------------------------------------------------------ */

/* インベントリに空きがあれば、スタック条件を一切見ずに true を返す。
 * 新しい枠に入れられるので判定が不要ということ。 */
TEST(inven_check_num_accepts_any_item_when_inventory_has_room)
{
    inventory_set_count(0);
    ASSERT_TRUE(inven_check_num(incoming_item(TV_SCROLL1, 0, 1, 0)));
}

/* 三角測量：空きが 1 つでも（inven_ctr が INVEN_WIELD - 1）まだ true。 */
TEST(inven_check_num_accepts_item_when_one_slot_remains)
{
    given_full_inventory();
    inventory_set_count(INVEN_WIELD - 1);
    ASSERT_TRUE(inven_check_num(incoming_item(TV_SCROLL1, 0, 1, 0)));
}

/* 境界：inven_ctr がちょうど INVEN_WIELD なら早期 return を抜け、
 * スタック条件の評価に入る。スタックできる相手がいないので false。 */
TEST(inven_check_num_rejects_unstackable_item_when_inventory_is_full)
{
    given_full_inventory();
    ASSERT_FALSE(inven_check_num(incoming_item(TV_SCROLL1, 0, 1, 0)));
}

/* 前提の確認：満杯にしてもスタックできる相手がいれば true になる。
 * 以下のテストすべてがこの前提に乗っているので固定する。 */
TEST(inven_check_num_accepts_stackable_item_when_inventory_is_full)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    ASSERT_TRUE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0)));
}

/* ------------------------------------------------------------------
 * 条件 1: t->tval == n->tval  種別が同じ
 * ------------------------------------------------------------------ */

/* 種別（tval）が違えばスタックしない。巻物とポーションは重ねられない。 */
TEST(items_do_not_stack_when_tval_differs)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    ASSERT_FALSE(
        inven_check_num(incoming_item(TV_POTION1, ITEM_SINGLE_STACK_MIN, 1, 0)));
}

/* 三角測量：種別が同じなら（他の条件も満たせば）スタックする。
 * 上のテストとの差は tval だけ。 */
TEST(items_stack_when_tval_matches)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    ASSERT_TRUE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0)));
}

/* ------------------------------------------------------------------
 * 条件 2: t->subval == n->subval  副種別が同じ
 * ------------------------------------------------------------------ */

/* 副種別（subval）が違えばスタックしない。同じ「巻物」でも中身が
 * 別のものは重ねられない。 */
TEST(items_do_not_stack_when_subval_differs)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    ASSERT_FALSE(inven_check_num(
        incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN + 1, 1, 0)));
}

/* 三角測量：副種別が同じならスタックする。上との差は subval だけ。 */
TEST(items_stack_when_subval_matches)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN + 1, 1, 0);
    ASSERT_TRUE(inven_check_num(
        incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN + 1, 1, 0)));
}

/* ------------------------------------------------------------------
 * 条件 3: n->subval >= ITEM_SINGLE_STACK_MIN (=64)
 *
 * 64 未満のアイテムは 1 つずつ枠を占める（矢や食料の一部）。
 * inven_check_num は外側の else if、inven_carry は条件式の中に持つが
 * 論理は等価。
 * ------------------------------------------------------------------ */

/* 境界の下側：subval が 63 なら単品スタックの範囲外でスタックしない。
 * 既存側も新規側も 63 で完全に一致しているのに false になる。 */
TEST(items_do_not_stack_when_subval_is_below_single_stack_min)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN - 1, 1, 0);
    ASSERT_FALSE(inven_check_num(
        incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN - 1, 1, 0)));
}

/* 境界の上側：subval がちょうど 64 ならスタックする。
 * 上のテストとの差は subval が 1 大きいだけ。 */
TEST(items_stack_when_subval_is_exactly_single_stack_min)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    ASSERT_TRUE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0)));
}

/* 三角測量：subval が 0 でもスタックしない（63 と同じ側）。 */
TEST(items_do_not_stack_when_subval_is_zero)
{
    given_existing_item(0, 1, 0);
    ASSERT_FALSE(inven_check_num(incoming_item(TV_SCROLL1, 0, 1, 0)));
}

/* ------------------------------------------------------------------
 * 条件 4: t->number + n->number < 256  個数のオーバーフロー防止
 *
 * number は uint8_t なので 255 が上限。合計が 256 になると 0 に
 * 巻きもどってアイテムが消える。この条件がそれを防いでいる。
 * ------------------------------------------------------------------ */

/* 境界の上側：合計が 255 ならまだスタックできる。
 * 254 + 1 = 255 < 256。number の上限そのもの。 */
TEST(items_stack_when_number_total_is_two_hundred_fifty_five)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 254, 0);
    ASSERT_TRUE(inven_check_num(
        incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0)));
}

/* 境界の下側：合計が 256 になるとスタックしない。
 * 255 + 1 = 256 なので「< 256」が偽。uint8_t の巻きもどりを防いでいる。 */
TEST(items_do_not_stack_when_number_total_reaches_two_hundred_fifty_six)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 255, 0);
    ASSERT_FALSE(inven_check_num(
        incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0)));
}

/* 三角測量：合計が 256 を超えてもスタックしない。
 * 200 + 100 = 300。number は uint8_t なので個々の値は 255 まで。 */
TEST(items_do_not_stack_when_number_total_exceeds_two_hundred_fifty_six)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 200, 0);
    ASSERT_FALSE(inven_check_num(
        incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 100, 0)));
}

/* 三角測量：合計が小さければ当然スタックする。1 + 1 = 2。 */
TEST(items_stack_when_number_total_is_small)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    ASSERT_TRUE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0)));
}

/* ------------------------------------------------------------------
 * 条件 5: n->subval < ITEM_GROUP_MIN (=192) || t->p1 == n->p1
 *
 * 192 未満は「常にスタックする」ので p1 を見ない。192 以上は
 * p1（充填回数・残り使用回数など）が一致しないと重ねられない。
 * 杖の残り使用回数が違うものを混ぜると回数が失われるため。
 * ------------------------------------------------------------------ */

/* 境界の下側：subval が 191 なら p1 が違ってもスタックする。
 * 192 未満なので条件 5 の右辺（p1 の一致）は評価されない。 */
TEST(items_stack_with_different_p1_when_subval_is_below_group_min)
{
    given_existing_item(ITEM_GROUP_MIN - 1, 1, 10);
    ASSERT_TRUE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_GROUP_MIN - 1, 1, 20)));
}

/* 境界の上側：subval がちょうど 192 なら p1 が違うとスタックしない。
 * 上のテストとの差は subval が 1 大きいだけ。ここから p1 を見はじめる。 */
TEST(items_do_not_stack_with_different_p1_when_subval_is_exactly_group_min)
{
    given_existing_item(ITEM_GROUP_MIN, 1, 10);
    ASSERT_FALSE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_GROUP_MIN, 1, 20)));
}

/* 三角測量：subval が 192 でも p1 が同じならスタックする。
 * 上のテストとの差は新規側の p1 だけ。 */
TEST(items_stack_with_same_p1_when_subval_is_exactly_group_min)
{
    given_existing_item(ITEM_GROUP_MIN, 1, 10);
    ASSERT_TRUE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_GROUP_MIN, 1, 10)));
}

/* 三角測量：192 より大きくても同じ扱い。p1 が違えばスタックしない。 */
TEST(items_do_not_stack_with_different_p1_when_subval_is_above_group_min)
{
    given_existing_item(ITEM_GROUP_MIN + 5, 1, 3);
    ASSERT_FALSE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_GROUP_MIN + 5, 1, 7)));
}

/* 三角測量：192 より大きく p1 が同じならスタックする。 */
TEST(items_stack_with_same_p1_when_subval_is_above_group_min)
{
    given_existing_item(ITEM_GROUP_MIN + 5, 1, 3);
    ASSERT_TRUE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_GROUP_MIN + 5, 1, 3)));
}

/* 三角測量：64 以上 192 未満の範囲でも p1 は見ない。
 * これが条件 3（64 以上）と条件 5（192 未満）の両方を満たす帯。 */
TEST(items_stack_with_different_p1_at_single_stack_min)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 1);
    ASSERT_TRUE(inven_check_num(
        incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 99)));
}

/* ------------------------------------------------------------------
 * 条件 6: known1_p(t) == known1_p(n)  鑑定状態が一致
 *
 * 鑑定済みと未鑑定を重ねると、どちらの状態なのか表示できなくなる。
 * known1_p() は object_ident[] を引くので、鑑定状態は
 * 「同じ tval・subval の全アイテム」で共有されている。
 * そのため片方だけ鑑定するには ID_STOREBOUGHT を使う（store_bought_p()
 * が真なら known1_p は無条件に OD_KNOWN1 を返す）。
 * ------------------------------------------------------------------ */

/* 両方が未鑑定ならスタックする。fixture_reset() 直後の既定の状態。 */
TEST(items_stack_when_both_are_unidentified)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    ASSERT_TRUE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0)));
}

/* 両方が鑑定済みならスタックする。known1() は object_ident[] を
 * 立てるので、同じ tval・subval の両方に一度で効く。 */
TEST(items_stack_when_both_are_identified)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    known1(inventory_at(0));
    ASSERT_TRUE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0)));
}

/* 新規側だけが鑑定済み（店で買ったもの）ならスタックしない。
 * ID_STOREBOUGHT が立つと known1_p は OD_KNOWN1 を返すので、
 * object_ident[] を共有していても鑑定状態が食いちがう。 */
TEST(items_do_not_stack_when_only_incoming_item_is_identified)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    inven_type *i_ptr = incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0);
    i_ptr->ident |= ID_STOREBOUGHT;
    ASSERT_FALSE(inven_check_num(i_ptr));
}

/* 三角測量：既存側だけが鑑定済みでもスタックしない。向きによらず不可。 */
TEST(items_do_not_stack_when_only_existing_item_is_identified)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    inventory_at(0)->ident |= ID_STOREBOUGHT;
    ASSERT_FALSE(
        inven_check_num(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0)));
}

/* 三角測量：両方が店で買ったものならスタックする。
 * 上の 2 つとの差は、両側に ID_STOREBOUGHT を立てたことだけ。 */
TEST(items_stack_when_both_are_store_bought)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    inventory_at(0)->ident |= ID_STOREBOUGHT;
    inven_type *i_ptr = incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0);
    i_ptr->ident |= ID_STOREBOUGHT;
    ASSERT_TRUE(inven_check_num(i_ptr));
}

/* ------------------------------------------------------------------
 * inven_carry() 側の同じ条件
 *
 * ここが本題。inven_check_num() と inven_carry() は同じ 5 条件を
 * 二重に持っており、片方だけ直すとアイテムが消失する。
 * inven_carry() は判定結果を戻り値ではなく副作用で示すので、
 * 「既存の number が増えたか（スタックした）」「新しい枠に入ったか
 * （inven_ctr が増えた）」で観測する。
 * ------------------------------------------------------------------ */

/* スタックできる相手がいれば、既存アイテムの number に加算される。
 * 1 + 3 = 4。枠は増えない。 */
TEST(inven_carry_adds_number_to_existing_item_when_stackable)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    inven_carry(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 3, 0));
    ASSERT_EQ_INT(inventory_at(0)->number, 4);
}

/* スタックしたときは枠が増えない。inven_ctr は変わらないまま。 */
TEST(inven_carry_does_not_increase_inven_ctr_when_stackable)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    inven_carry(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 3, 0));
    ASSERT_EQ_INT(inventory_count(), INVEN_WIELD);
}

/* スタックした位置（locn）が返る。先頭に置いた相手なので 0。 */
TEST(inven_carry_returns_position_of_stacked_item)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    ASSERT_EQ_INT(
        inven_carry(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 3, 0)), 0);
}

/* 条件 3（subval >= 64）を満たさないと inven_carry もスタックしない。
 * 既存の number は 1 のまま増えない。inven_check_num の
 * items_do_not_stack_when_subval_is_below_single_stack_min と対になる。 */
TEST(inven_carry_does_not_stack_when_subval_is_below_single_stack_min)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN - 1, 1, 0);
    inven_carry(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN - 1, 3, 0));
    ASSERT_EQ_INT(inventory_at(0)->number, 1);
}

/* 条件 4（合計 < 256）を満たさないと inven_carry もスタックしない。
 * 255 + 1 = 256 なので加算されず、255 のまま残る。
 * ここで加算してしまうと uint8_t が 0 に巻きもどってアイテムが消える。 */
TEST(inven_carry_does_not_stack_when_number_total_reaches_two_hundred_fifty_six)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 255, 0);
    inven_carry(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0));
    ASSERT_EQ_INT(inventory_at(0)->number, 255);
}

/* 境界の逆側：合計が 255 なら inven_carry も加算する。254 + 1 = 255。 */
TEST(inven_carry_stacks_when_number_total_is_two_hundred_fifty_five)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 254, 0);
    inven_carry(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0));
    ASSERT_EQ_INT(inventory_at(0)->number, 255);
}

/* 条件 5（192 以上は p1 の一致が必要）を満たさないとスタックしない。 */
TEST(inven_carry_does_not_stack_with_different_p1_at_group_min)
{
    given_existing_item(ITEM_GROUP_MIN, 1, 10);
    inven_carry(incoming_item(TV_SCROLL1, ITEM_GROUP_MIN, 3, 20));
    ASSERT_EQ_INT(inventory_at(0)->number, 1);
}

/* 三角測量：192 以上でも p1 が同じならスタックする。1 + 3 = 4。 */
TEST(inven_carry_stacks_with_same_p1_at_group_min)
{
    given_existing_item(ITEM_GROUP_MIN, 1, 10);
    inven_carry(incoming_item(TV_SCROLL1, ITEM_GROUP_MIN, 3, 10));
    ASSERT_EQ_INT(inventory_at(0)->number, 4);
}

/* 条件 6（鑑定状態の一致）を満たさないとスタックしない。
 * 新規側だけが店で買ったもの（鑑定済み）なので加算されない。 */
TEST(inven_carry_does_not_stack_when_only_incoming_item_is_identified)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    inven_type *i_ptr = incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 3, 0);
    i_ptr->ident |= ID_STOREBOUGHT;
    inven_carry(i_ptr);
    ASSERT_EQ_INT(inventory_at(0)->number, 1);
}

/* 条件 1（tval の一致）を満たさないとスタックせず、新しい枠に挿入される。
 * TV_POTION1(75) > TV_SCROLL1(70) なので else if の (typ > t_ptr->tval) が
 * locn=0 で成立し、既存アイテムを後ろへずらして先頭に割りこむ。
 * したがって inventory[0] は新規アイテム（number 3）になり、
 * 既存アイテムは inventory[1] に移る。 */
TEST(inven_carry_inserts_before_existing_item_when_tval_is_greater)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    inven_carry(incoming_item(TV_POTION1, ITEM_SINGLE_STACK_MIN, 3, 0));
    ASSERT_EQ_INT(inventory_at(1)->number, 1);
}

/* 同じ場面で、スタックしていないので枠が 1 つ増える。
 * inven_check_num() は同じ条件で false を返す（items_do_not_stack_when_
 * tval_differs）のに、inven_carry() は満杯でも挿入してしまう。
 *
 * SUSPICIOUS: inven_ctr が INVEN_WIELD(22) のときに挿入すると 23 になる。
 * inventory は INVEN_ARRAY_SIZE(34) 確保されており、22〜33 は装備欄なので
 * 配列外には出ないが、持ち物の枠が装備欄を侵食している。
 * inven_check_num() を先に呼んで false なら呼ばない、という約束が
 * 守られている前提のコードで、約束が破られたときの防御がない。
 * ここでは実装を変えないので、現在のふるまいをそのまま固定する。 */
TEST(inven_carry_increases_inven_ctr_past_inven_wield_when_full)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    inven_carry(incoming_item(TV_POTION1, ITEM_SINGLE_STACK_MIN, 3, 0));
    ASSERT_EQ_INT(inventory_count(), INVEN_WIELD + 1);
}

/* 条件 2（subval の一致）を満たさないとスタックしない。 */
TEST(inven_carry_does_not_stack_when_subval_differs)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    inven_carry(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN + 1, 3, 0));
    ASSERT_EQ_INT(inventory_at(0)->number, 1);
}

/* 空のインベントリに入れると新しい枠を使い、inven_ctr が 1 増える。
 * スタックの相手がいないので挿入の経路を通る。 */
TEST(inven_carry_increases_inven_ctr_when_inserting_into_empty_inventory)
{
    inventory_set_count(0);
    inven_carry(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 1, 0));
    ASSERT_EQ_INT(inventory_count(), 1);
}

/* TODO: 仕様確認 -- inven_carry() のループには終了条件がない。
 *   for (locn = 0;; locn++)     misc3.c:1251
 * break は「スタックした」か「割りこんだ」ときだけ。どちらも起きない
 * アイテムを渡すと inventory[] の末尾を越えて読みつづける。
 *
 * 割りこみ条件は misc3.c:1262 の
 *   (typ == t_ptr->tval && subt < t_ptr->subval && always_known1p)
 *   || (typ > t_ptr->tval)
 * なので、インベントリ全枠の tval が新規アイテムの tval より大きく、
 * かつスタックもできない状態を作ると break に到達しない。
 *
 * AddressSanitizer で確認済み（このテストには含めない。UB なので
 * 実行結果が保証されず、テストとして固定できないため）:
 *   全 34 枠を tval=90 で埋め、tval=10・subval=0 の新規アイテムを渡すと
 *   src/misc3.c:1254 で global-buffer-overflow（inventory の 20 バイト先を
 *   読む）。読んだ先が偶然 break 条件を満たすまで走るので、何が起きるかは
 *   隣接するグローバル変数の内容次第。
 *
 * 実際の呼びだし側は inven_check_num() で先に確認する約束になっているが、
 * inven_carry() 側にその約束を守らせる仕組みがない。約束が破られた場合の
 * 防御（locn < inven_ctr などの上限）が存在しない。
 * ここでは実装を変えないので、報告だけ残す。
 *
 * 重さも更新される。number * weight が inven_weight に加算される。
 * set_item() が weight = 1 を入れているので 3 * 1 = 3。 */
TEST(inven_carry_adds_item_weight_to_inventory_weight)
{
    given_existing_item(ITEM_SINGLE_STACK_MIN, 1, 0);
    inven_carry(incoming_item(TV_SCROLL1, ITEM_SINGLE_STACK_MIN, 3, 0));
    ASSERT_EQ_INT(inventory_weight(), 3);
}

int main(void)
{
    RUN_TEST(inven_check_num_accepts_any_item_when_inventory_has_room);
    RUN_TEST(inven_check_num_accepts_item_when_one_slot_remains);
    RUN_TEST(inven_check_num_rejects_unstackable_item_when_inventory_is_full);
    RUN_TEST(inven_check_num_accepts_stackable_item_when_inventory_is_full);
    RUN_TEST(items_do_not_stack_when_tval_differs);
    RUN_TEST(items_stack_when_tval_matches);
    RUN_TEST(items_do_not_stack_when_subval_differs);
    RUN_TEST(items_stack_when_subval_matches);
    RUN_TEST(items_do_not_stack_when_subval_is_below_single_stack_min);
    RUN_TEST(items_stack_when_subval_is_exactly_single_stack_min);
    RUN_TEST(items_do_not_stack_when_subval_is_zero);
    RUN_TEST(items_stack_when_number_total_is_two_hundred_fifty_five);
    RUN_TEST(items_do_not_stack_when_number_total_reaches_two_hundred_fifty_six);
    RUN_TEST(items_do_not_stack_when_number_total_exceeds_two_hundred_fifty_six);
    RUN_TEST(items_stack_when_number_total_is_small);
    RUN_TEST(items_stack_with_different_p1_when_subval_is_below_group_min);
    RUN_TEST(items_do_not_stack_with_different_p1_when_subval_is_exactly_group_min);
    RUN_TEST(items_stack_with_same_p1_when_subval_is_exactly_group_min);
    RUN_TEST(items_do_not_stack_with_different_p1_when_subval_is_above_group_min);
    RUN_TEST(items_stack_with_same_p1_when_subval_is_above_group_min);
    RUN_TEST(items_stack_with_different_p1_at_single_stack_min);
    RUN_TEST(items_stack_when_both_are_unidentified);
    RUN_TEST(items_stack_when_both_are_identified);
    RUN_TEST(items_do_not_stack_when_only_incoming_item_is_identified);
    RUN_TEST(items_do_not_stack_when_only_existing_item_is_identified);
    RUN_TEST(items_stack_when_both_are_store_bought);
    RUN_TEST(inven_carry_adds_number_to_existing_item_when_stackable);
    RUN_TEST(inven_carry_does_not_increase_inven_ctr_when_stackable);
    RUN_TEST(inven_carry_returns_position_of_stacked_item);
    RUN_TEST(inven_carry_does_not_stack_when_subval_is_below_single_stack_min);
    RUN_TEST(inven_carry_does_not_stack_when_number_total_reaches_two_hundred_fifty_six);
    RUN_TEST(inven_carry_stacks_when_number_total_is_two_hundred_fifty_five);
    RUN_TEST(inven_carry_does_not_stack_with_different_p1_at_group_min);
    RUN_TEST(inven_carry_stacks_with_same_p1_at_group_min);
    RUN_TEST(inven_carry_does_not_stack_when_only_incoming_item_is_identified);
    RUN_TEST(inven_carry_inserts_before_existing_item_when_tval_is_greater);
    RUN_TEST(inven_carry_increases_inven_ctr_past_inven_wield_when_full);
    RUN_TEST(inven_carry_does_not_stack_when_subval_differs);
    RUN_TEST(inven_carry_increases_inven_ctr_when_inserting_into_empty_inventory);
    RUN_TEST(inven_carry_adds_item_weight_to_inventory_weight);
    return TEST_SUMMARY();
}
