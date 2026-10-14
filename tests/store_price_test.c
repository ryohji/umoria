// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 店の値段（src/world/store/store_price.c）のテスト -- 現在の実装を保護する
 *
 * 対象は item_value・sell_price・noneedtobargain・updatebargain の 4 本。
 * どれも画面も入力も使わないが、それまでどのテストも届いていなかった。
 *
 * 表（object_list・owners・rgold_adj）は本物を使い、期待値は表の値で表す。
 * 品物は 0 で埋めてから、種類・値・鑑定の印だけを入れる。印は本体の窓口
 * （known2・store_bought・item_kind_mark_known）で立て、呪いの印 ID_DAMD
 * だけは窓口が無いので ident に直に立てる。
 */
#include "config.h"
#include "constant.h"
#include "types.h"
#include "world_item.h"
#include "world_store.h"
#include "world_internal.h"
#include "tables.h"
#include "treasure.h"
#include "item_knowledge.h"
#include "player_identity.h"
#include "shared_stubs.h"
#include "stores.h"

#include "minunit.h"

static inven_type item(uint8_t tval, uint8_t subval, int32_t cost)
{
    inven_type i = {0};
    i.tval = tval;
    i.subval = subval;
    i.cost = cost;
    i.number = 1;
    i.index = 10;
    return i;
}

/* object_list で、種類 tval の最初の品の番号 */
static uint16_t first_index_of(uint8_t tval)
{
    for (uint16_t k = 0; k < MAX_OBJECTS; k++) {
        if (object_list[k].tval == tval) {
            return k;
        }
    }
    return 0;
}

/* ---- item_value --------------------------------------------------------- */

/* 呪いと分かっている品は 0 */
TEST(item_value_of_a_noted_cursed_item_is_zero)
{
    inven_type i = item(TV_SWORD, 1, 1000);
    known2(&i);
    i.ident |= ID_DAMD;
    ASSERT_EQ_INT(item_value(&i), 0);
}

/* 修正の分からない武器と防具は、表の元の値 */
TEST(item_value_of_unknown_weapon_and_armour_is_the_base_cost)
{
    inven_type sword = item(TV_SWORD, 1, 1000);
    inven_type boots = item(TV_BOOTS, 1, 1000);
    int32_t base = object_list[10].cost;
    ASSERT_TRUE(item_value(&sword) == base && item_value(&boots) == base);
}

/* 分かった武器は 値 + (命中 + 打撃 + 守り) * 100 */
TEST(item_value_of_known_weapon_adds_100_per_plus)
{
    inven_type i = item(TV_HAFTED, 1, 100);
    known2(&i);
    i.tohit = 1;
    i.todam = 2;
    i.toac = 3;
    ASSERT_EQ_INT(item_value(&i), 700);
}

/* 分かった武器の修正が 1 つでも負なら 0 */
TEST(item_value_of_known_weapon_with_a_negative_plus_is_zero)
{
    int nonzero = 0;
    for (int k = 0; k < 3; k++) {
        inven_type i = item(TV_BOW, 1, 100);
        known2(&i);
        i.tohit = (int16_t)(k == 0 ? -1 : 5);
        i.todam = (int16_t)(k == 1 ? -1 : 5);
        i.toac = (int16_t)(k == 2 ? -1 : 5);
        if (item_value(&i) != 0) {
            nonzero++;
        }
    }
    ASSERT_EQ_INT(nonzero, 0);
}

/* 分かった防具は守りだけを見る。命中が負でも 値 + 守り * 100 */
TEST(item_value_of_known_armour_counts_only_armour_class)
{
    inven_type i = item(TV_SOFT_ARMOR, 1, 100);
    known2(&i);
    i.tohit = -5;
    i.toac = 2;
    ASSERT_EQ_INT(item_value(&i), 300);
}

/* 分かった防具の守りが負なら 0 */
TEST(item_value_of_known_armour_with_negative_armour_class_is_zero)
{
    inven_type i = item(TV_BOOTS, 1, 100);
    known2(&i);
    i.toac = -1;
    ASSERT_EQ_INT(item_value(&i), 0);
}

/* 弾と矢は修正 1 つにつき 5。束（subval > ITEM_GROUP_MIN）は数を掛ける */
TEST(item_value_of_known_ammo_adds_5_per_plus_times_number)
{
    inven_type i = item(TV_ARROW, ITEM_GROUP_MIN + 1, 2);
    known2(&i);
    i.tohit = 1;
    i.todam = 2;
    i.toac = 3;
    i.number = 20;
    ASSERT_EQ_INT(item_value(&i), (2 + 30) * 20);
}

/* 弾と矢も、分からなければ表の元の値、修正が負なら 0 */
TEST(item_value_of_ammo_unknown_or_negative)
{
    inven_type unknown = item(TV_SPIKE, 1, 99);
    inven_type negative = item(TV_SLING_AMMO, 1, 99);
    known2(&negative);
    negative.todam = -1;
    ASSERT_TRUE(item_value(&unknown) == object_list[10].cost && item_value(&negative) == 0);
}

/* 束の掛け算は subval が ITEM_GROUP_MIN より大きいときだけ */
TEST(item_value_multiplies_only_above_the_group_minimum)
{
    inven_type at = item(TV_SPIKE, ITEM_GROUP_MIN, 3);
    inven_type above = item(TV_SPIKE, ITEM_GROUP_MIN + 1, 3);
    known2(&at);
    known2(&above);
    at.number = 4;
    above.number = 4;
    ASSERT_TRUE(item_value(&at) == 3 && item_value(&above) == 12);
}

/* 種類の分からない巻物と薬は 20、分かれば値のまま */
TEST(item_value_of_scrolls_and_potions)
{
    static const uint8_t tvals[] = {TV_SCROLL1, TV_SCROLL2, TV_POTION1, TV_POTION2};
    int mismatch = 0;
    for (int k = 0; k < 4; k++) {
        inven_type unknown = item(tvals[k], 1, 500);
        inven_type bought = item(tvals[k], 1, 500);
        store_bought(&bought);
        if (item_value(&unknown) != 20 || item_value(&bought) != 500) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 種類の分からないキノコは 1。キノコの外（subval が 64 + MAX_MUSH 以上）は値のまま */
TEST(item_value_of_food)
{
    inven_type mushroom = item(TV_FOOD, ITEM_SINGLE_STACK_MIN + MAX_MUSH - 1, 50);
    inven_type ration = item(TV_FOOD, ITEM_SINGLE_STACK_MIN + MAX_MUSH, 50);
    ASSERT_TRUE(item_value(&mushroom) == 1 && item_value(&ration) == 50);
}

/* 指輪と首飾り：種類が分からなければ 45、種類だけ分かれば表の元の値、
 * 両方分かれば値のまま */
TEST(item_value_of_rings_and_amulets)
{
    inven_type unknown = item(TV_RING, 3, 900);
    inven_type kind = item(TV_AMULET, 3, 900);
    item_kind_mark_known(&kind);
    inven_type both = item(TV_RING, 4, 900);
    store_bought(&both);
    ASSERT_TRUE(item_value(&unknown) == 45 && item_value(&kind) == object_list[10].cost &&
                item_value(&both) == 900);
}

/* 魔法棒は 50・杖は 70（種類が分からないとき）。両方分かれば 値 + (値 / 20) * 回数 */
TEST(item_value_of_wands_and_staffs)
{
    inven_type wand = item(TV_WAND, 5, 400);
    inven_type staff = item(TV_STAFF, 5, 400);
    inven_type kind = item(TV_WAND, 6, 400);
    item_kind_mark_known(&kind);
    inven_type both = item(TV_STAFF, 6, 400);
    store_bought(&both);
    both.p1 = 3;
    ASSERT_TRUE(item_value(&wand) == 50 && item_value(&staff) == 70 && item_value(&kind) == 400 &&
                item_value(&both) == 400 + 20 * 3);
}

/* 掘る道具：分からなければ表の元の値、p1 が負なら 0、ほかは表の p1 を超えた分 * 100 を
 * 足し、負になれば 0（値 50・p1 0 で、表の p1 は 1 以上）。表は本物の掘る道具 */
TEST(item_value_of_digging_tools)
{
    uint16_t k = first_index_of(TV_DIGGING);
    int base_p1 = object_list[k].p1;
    inven_type unknown = item(TV_DIGGING, 1, 300);
    unknown.index = k;
    inven_type negative = unknown;
    known2(&negative);
    negative.p1 = -1;
    inven_type plus = unknown;
    known2(&plus);
    plus.p1 = (int16_t)(base_p1 + 2);
    inven_type clamped = unknown;
    known2(&clamped);
    clamped.cost = 50;
    clamped.p1 = 0;
    ASSERT_TRUE(object_list[k].tval == TV_DIGGING && base_p1 >= 1 && item_value(&unknown) == object_list[k].cost &&
                item_value(&negative) == 0 && item_value(&plus) == 500 && item_value(&clamped) == 0);
}

/* p1 が負なら、足し算の結果が正でも 0（値 10000、p1 -1） */
TEST(item_value_of_digging_tool_with_negative_p1_ignores_the_cost)
{
    uint16_t k = first_index_of(TV_DIGGING);
    inven_type i = item(TV_DIGGING, 1, 10000);
    i.index = k;
    known2(&i);
    i.p1 = -1;
    ASSERT_TRUE(10000 + (-1 - object_list[k].p1) * 100 > 0 && item_value(&i) == 0);
}

/* ほかの種類（宝箱）は値のまま */
TEST(item_value_of_other_kinds_is_the_cost)
{
    inven_type i = item(TV_CHEST, 1, 123);
    ASSERT_EQ_INT(item_value(&i), 123);
}

/* ---- sell_price --------------------------------------------------------- */

/* 値 * rgold_adj[主の種族][人物の種族] / 100。上下の幅は主の inflate。
 * 店 0 の主と人物の種族を総当たり */
TEST(sell_price_follows_the_race_table_and_owner_inflation)
{
    int mismatch = 0;
    store_type *s = store_at(0);
    for (int owner = 0; owner < MAX_OWNERS; owner++) {
        s->owner = (uint8_t)owner;
        for (int r = 0; r < MAX_RACES; r++) {
            player_race_set(r);
            inven_type i = item(TV_CHEST, 1, 1000);
            int32_t max_sell = -1;
            int32_t min_sell = -1;
            int32_t price = sell_price(0, &max_sell, &min_sell, &i);
            int32_t want = 1000 * rgold_adj[owners[owner].owner_race][r] / 100;
            int32_t want_max = want * owners[owner].max_inflate / 100;
            int32_t want_min = want * owners[owner].min_inflate / 100;
            if (want_min > want_max) {
                want_min = want_max;
            }
            if (price != want || max_sell != want_max || min_sell != want_min) {
                mismatch++;
            }
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 値 1 の品を 95 の組み合わせ（小人の主と小人）で売ると 0 になるが、1 に止まる */
TEST(sell_price_is_at_least_one)
{
    store_type *s = store_at(0);
    int found = -1;
    for (int owner = 0; owner < MAX_OWNERS && found < 0; owner++) {
        if (rgold_adj[owners[owner].owner_race][owners[owner].owner_race] < 100) {
            found = owner;
        }
    }
    s->owner = (uint8_t)found;
    player_race_set(owners[found].owner_race);
    inven_type i = item(TV_CHEST, 1, 1);
    int32_t max_sell;
    int32_t min_sell;
    ASSERT_TRUE(found >= 0 && sell_price(0, &max_sell, &min_sell, &i) == 1);
}

/* 値が 0（呪い）か、値打ちが 0（負の修正）なら店に入れない */
TEST(sell_price_refuses_worthless_items)
{
    store_at(0)->owner = 0;
    player_race_set(0);
    inven_type free_item = item(TV_CHEST, 1, 0);
    inven_type broken = item(TV_SWORD, 1, 100);
    known2(&broken);
    broken.tohit = -1;
    int32_t max_sell = -1;
    int32_t min_sell = -1;
    int32_t a = sell_price(0, &max_sell, &min_sell, &free_item);
    int32_t b = sell_price(0, &max_sell, &min_sell, &broken);
    ASSERT_TRUE(a == 0 && b == 0 && max_sell == -1 && min_sell == -1);
}

/* 品の値が 0 なら、値打ち（修正の分からない武器は表の元の値）があっても入れない */
TEST(sell_price_refuses_an_item_of_zero_cost_even_when_it_has_value)
{
    store_at(0)->owner = 0;
    player_race_set(0);
    inven_type i = item(TV_SWORD, 1, 0);
    int32_t max_sell;
    int32_t min_sell;
    ASSERT_TRUE(item_value(&i) > 0 && sell_price(0, &max_sell, &min_sell, &i) == 0);
}

/* ---- noneedtobargain・updatebargain ------------------------------------- */

static void set_record(int good, int bad)
{
    store_at(1)->good_buy = (uint16_t)good;
    store_at(1)->bad_buy = (uint16_t)bad;
}

/* 良い買い物が MAX_SHORT なら、値段によらず値切らない */
TEST(noneedtobargain_at_max_short_good_buys)
{
    set_record(MAX_SHORT, MAX_SHORT);
    ASSERT_TRUE(noneedtobargain(1, 1000000));
}

/* 成績 = 良 - 3 * 悪 - 5。成績の 2 乗が 最低値 / 50 を上回れば値切らない
 * （成績 3：9 > 449 / 50 = 8 は真、9 > 450 / 50 = 9 は偽） */
TEST(noneedtobargain_compares_the_square_of_the_record)
{
    set_record(11, 1);
    bool below = noneedtobargain(1, 449);
    bool at = noneedtobargain(1, 450);
    ASSERT_TRUE(below && !at);
}

/* 成績が 0 以下なら、最低値が 0 でも値切る */
TEST(noneedtobargain_needs_a_positive_record)
{
    set_record(5, 0);
    ASSERT_FALSE(noneedtobargain(1, 0));
}

/* 最低値どおりに買えば良い買い物、外れれば悪い買い物 */
TEST(updatebargain_counts_good_and_bad_buys)
{
    set_record(0, 0);
    updatebargain(1, 10, 10);
    updatebargain(1, 12, 10);
    updatebargain(1, 12, 10);
    ASSERT_TRUE(store_at(1)->good_buy == 1 && store_at(1)->bad_buy == 2);
}

/* 最低値が 9 以下の買い物は数えない */
TEST(updatebargain_ignores_cheap_items)
{
    set_record(0, 0);
    updatebargain(1, 9, 9);
    updatebargain(1, 12, 9);
    ASSERT_TRUE(store_at(1)->good_buy == 0 && store_at(1)->bad_buy == 0);
}

/* どちらも MAX_SHORT で止まる */
TEST(updatebargain_stops_at_max_short)
{
    set_record(MAX_SHORT, MAX_SHORT);
    updatebargain(1, 10, 10);
    updatebargain(1, 12, 10);
    ASSERT_TRUE(store_at(1)->good_buy == MAX_SHORT && store_at(1)->bad_buy == MAX_SHORT);
}

int main(void)
{
    RUN_TEST(item_value_of_a_noted_cursed_item_is_zero);
    RUN_TEST(item_value_of_unknown_weapon_and_armour_is_the_base_cost);
    RUN_TEST(item_value_of_known_weapon_adds_100_per_plus);
    RUN_TEST(item_value_of_known_weapon_with_a_negative_plus_is_zero);
    RUN_TEST(item_value_of_known_armour_counts_only_armour_class);
    RUN_TEST(item_value_of_known_armour_with_negative_armour_class_is_zero);
    RUN_TEST(item_value_of_known_ammo_adds_5_per_plus_times_number);
    RUN_TEST(item_value_of_ammo_unknown_or_negative);
    RUN_TEST(item_value_multiplies_only_above_the_group_minimum);
    RUN_TEST(item_value_of_scrolls_and_potions);
    RUN_TEST(item_value_of_food);
    RUN_TEST(item_value_of_rings_and_amulets);
    RUN_TEST(item_value_of_wands_and_staffs);
    RUN_TEST(item_value_of_digging_tools);
    RUN_TEST(item_value_of_digging_tool_with_negative_p1_ignores_the_cost);
    RUN_TEST(item_value_of_other_kinds_is_the_cost);
    RUN_TEST(sell_price_follows_the_race_table_and_owner_inflation);
    RUN_TEST(sell_price_is_at_least_one);
    RUN_TEST(sell_price_refuses_worthless_items);
    RUN_TEST(sell_price_refuses_an_item_of_zero_cost_even_when_it_has_value);
    RUN_TEST(noneedtobargain_at_max_short_good_buys);
    RUN_TEST(noneedtobargain_compares_the_square_of_the_record);
    RUN_TEST(noneedtobargain_needs_a_positive_record);
    RUN_TEST(updatebargain_counts_good_and_bad_buys);
    RUN_TEST(updatebargain_ignores_cheap_items);
    RUN_TEST(updatebargain_stops_at_max_short);
    return TEST_SUMMARY();
}
