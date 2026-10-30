// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 売った品を店の在庫に入れる store_carry() のテスト
 * （src/world/store/store_stock.c）。
 *
 * 同じ品が在庫にあれば数を足して 1 行にまとめる。数は uint8_t なので、
 * 足して 256 以上になるなら別の行に入れる（バグ候補 B26：前は見ずに足し、
 * 桁があふれた）。品は矢（表の 80 番）。矢のように束で売る品（subval が
 * ITEM_GROUP_MIN より大きい）は束ごと売られ、店の数に上限が無いので、
 * 足して 256 に届きうる（1 つずつ売る品は店で 24 に抑えられる）。 */

#include "config.h"
#include "constant.h"
#include "types.h"
#include "item_kinds.h"
#include "stores.h"
#include "world_store.h"

#include "minunit.h"

enum { GENERAL_STORE = 0, ARROW = 80 };

static void stock(int number) {
    store_type *s_ptr = store_at(GENERAL_STORE);
    s_ptr->store_ctr = 1;
    invcopy(&s_ptr->store_inven[0].sitem, ARROW);
    s_ptr->store_inven[0].sitem.number = (uint8_t)number;
}

static int carry(int number) {
    inven_type item;
    invcopy(&item, ARROW);
    item.number = (uint8_t)number;
    int pos;
    store_carry(GENERAL_STORE, &pos, &item);
    return pos;
}

TEST(a_sold_item_adds_to_the_same_item_in_stock) {
    stock(5);
    ASSERT_EQ_INT(carry(3), 0);
    ASSERT_EQ_INT(store_at(GENERAL_STORE)->store_inven[0].sitem.number, 8);
}

TEST(the_sum_is_kept_when_it_is_255) {
    stock(200);
    (void)carry(55);
    ASSERT_EQ_INT(store_at(GENERAL_STORE)->store_ctr, 1);
}

TEST(a_sum_of_256_goes_to_a_new_line) {
    stock(200);
    (void)carry(56);
    ASSERT_EQ_INT(store_at(GENERAL_STORE)->store_ctr, 2);
}

TEST(the_line_in_stock_keeps_its_number) {
    stock(200);
    (void)carry(100);
    ASSERT_EQ_INT(store_at(GENERAL_STORE)->store_inven[0].sitem.number, 200);
}

int main(void) {
    RUN_TEST(a_sold_item_adds_to_the_same_item_in_stock);
    RUN_TEST(the_sum_is_kept_when_it_is_255);
    RUN_TEST(a_sum_of_256_goes_to_a_new_line);
    RUN_TEST(the_line_in_stock_keeps_its_number);
    return TEST_SUMMARY();
}
