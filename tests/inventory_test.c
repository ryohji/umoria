/* 持ち物と装備の置き場のテスト -- 現在のふるまいを保護する
 *
 * この区分にも計算は無い。店（#18-4）と同じで inventory[] は置き場そのもの
 * だが、店と違って 1 本の配列が 2 役を持っている。
 *   0 .. 21   持ち物（背負っている袋。詰めて使う）
 *   22 .. 33  装備（身につけている物。INVEN_* の定数が枠の名前）
 * 実測（REFACTORING_PLAN.md「#18-5（持ち物 6 個）の実測」）では inventory[]
 * の参照 199 箇所のうち装備側だけが約 112 で、持ち物側の約 64 より多い。
 * そこで窓口を 2 つに割った（src/inventory.h と src/equipment.h）。添字空間は
 * 割らない——22 を引くような変換を入れると呼びだし側の算術が変わる。
 *
 * 保護するのは「置き場としてのふるまい」で、次の 4 つに尽きる。
 *   枠ごとに別の記録である（重なると 2 つの物が同じ枠を共有してしまう）
 *   書いたものが残り、同じ添字はいつでも同じ枠を指す
 *     （呼びだし側は i_ptr を局所変数に持ったまま他の関数を呼ぶ）
 *   装備の境界が動かない（INVEN_WIELD = 22・要素数 34。セーブファイルの
 *     並びもこの数を前提にしている）
 *   跨ぎの窓口が全域を覆い、2 つの窓口と同じ枠を指す
 * 3 つめの equip_ctr は着手前まで tests/ に 1 度も現れず、装備境界の
 * 不変条件は完全に無保護だった。
 *
 * テストは 1 プロセスで状態を共有する。走りだしの状態を見るテストは
 * main() の先頭に置いてある（下の註を参照）。
 */
/* externs.h は要らない。窓口 2 つと、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "equipment.h"
#include "inventory.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */

/* これだけは他のテストより先に走らせる（下の main() の並び順が保証する）。
 * 以降のテストが枠と件数に書きこむので、あとから見ても 0 ではない。 */
TEST(every_slot_starts_out_empty)
{
    int nonzero = -1;

    /* 変更前は treasure.c の 0 初期化されるグローバルだった。新しいゲームの
     * 始まりは inven_ctr = 0 と TV_NOTHING（= 0）の枠に頼っている。 */
    for (int i = 0; i < inventory_and_equipment_slot_count() && nonzero < 0; i++) {
        inven_type *i_ptr = inventory_and_equipment_at(i);

        if (i_ptr->tval != 0 || i_ptr->number != 0 || i_ptr->weight != 0) {
            nonzero = i;
        }
    }
    ASSERT_EQ_INT(nonzero, -1);
}

TEST(the_counts_and_the_weight_start_out_zero)
{
    /* 3 つとも 0 から始まる。これも main() の先頭で見る。 */
    ASSERT_TRUE(inventory_count() == 0 && equipment_count() == 0 &&
                inventory_weight() == 0);
}

/* --- 装備境界の不変条件 ------------------------------------------------- */

TEST(the_pack_has_twenty_two_slots)
{
    ASSERT_EQ_INT(inventory_slot_count(), 22);
}

TEST(the_equipment_starts_where_the_pack_ends)
{
    /* constant.h:198 の「must be first item in equipment list」がこれ。
     * 持ち物の枠数と装備の 1 つめの添字は同じ数でなければならない。 */
    ASSERT_EQ_INT(equipment_first_slot(), inventory_slot_count());
}

TEST(the_first_equipment_slot_is_inven_wield)
{
    ASSERT_EQ_INT(equipment_first_slot(), INVEN_WIELD);
}

TEST(there_are_thirty_four_slots_in_all)
{
    /* 要素数 34（INVEN_ARRAY_SIZE、constant.h:75 に「Do not change」）。
     * セーブファイルは装備 12 枠を並べて書くので、増減は既存のセーブ
     * ファイルが読めなくなることを意味する。 */
    ASSERT_EQ_INT(inventory_and_equipment_slot_count(), 34);
}

TEST(the_equipment_has_twelve_slots)
{
    /* 22..33 の 12 枠。セーブファイルは装備をこの数だけ並べて書く
     * （save.c:205,666 の INVEN_WIELD .. INVEN_ARRAY_SIZE のループ）。 */
    ASSERT_EQ_INT(equipment_end_slot() - equipment_first_slot(), 12);
}

TEST(the_equipment_ends_where_the_whole_array_ends)
{
    ASSERT_EQ_INT(equipment_end_slot(), inventory_and_equipment_slot_count());
}

/* --- 枠ごとに別の記録 --------------------------------------------------- */

TEST(each_pack_index_addresses_a_different_slot)
{
    int mismatch = -1;

    /* 添字を個数として書きこみ、全部を読みかえす。枠が重なっていれば
     * あとの書きこみが前を上書きするので、必ず食いちがう。 */
    for (int i = 0; i < inventory_slot_count(); i++) {
        inventory_at(i)->number = (uint8_t)(i + 1);
    }
    for (int i = 0; i < inventory_slot_count() && mismatch < 0; i++) {
        if (inventory_at(i)->number != (uint8_t)(i + 1)) {
            mismatch = i;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(each_equipment_index_addresses_a_different_slot)
{
    int mismatch = -1;

    /* 装備側も同じ。ここが重なると、たとえば右手の指輪と左手の指輪が
     * 同じ物になる。 */
    for (int i = equipment_first_slot(); i < equipment_end_slot(); i++) {
        equipment_at(i)->number = (uint8_t)(i + 1);
    }
    for (int i = equipment_first_slot(); i < equipment_end_slot() && mismatch < 0; i++) {
        if (equipment_at(i)->number != (uint8_t)(i + 1)) {
            mismatch = i;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(the_slots_do_not_overlap_in_memory)
{
    int overlap = -1;

    /* 隣どうしの間隔が 1 枠ぶんに足りなければ、物が隣の枠にはみ出す。
     * 跨ぎ窓口で全域を見るので、持ち物と装備のつなぎ目もここで見ている。 */
    for (int i = 1; i < inventory_and_equipment_slot_count() && overlap < 0; i++) {
        if (inventory_and_equipment_at(i) - inventory_and_equipment_at(i - 1) != 1) {
            overlap = i;
        }
    }
    ASSERT_EQ_INT(overlap, -1);
}

/* --- 書いたものが残る --------------------------------------------------- */

TEST(the_same_index_always_gives_the_same_slot)
{
    /* 呼びだし側は inven_type *i_ptr を局所変数に持ったまま他の関数を
     * 呼ぶ（inven_carry() が objdes() を呼ぶ形）。写しを返す実装だと
     * ここで落ちる。 */
    ASSERT_TRUE(inventory_at(3) == inventory_at(3));
}

TEST(what_was_written_to_a_pack_slot_stays_there)
{
    inven_type *i_ptr = inventory_at(5);

    /* 1 枠に物の中身を入れて、あとから窓口越しに読みかえす。 */
    i_ptr->tval = TV_SWORD;
    i_ptr->weight = 150;
    i_ptr->number = 1;
    i_ptr->cost = 300;
    i_ptr->inscrip[0] = 'x';
    i_ptr->inscrip[1] = '\0';

    inven_type *again = inventory_at(5);
    ASSERT_TRUE(again->tval == TV_SWORD && again->weight == 150 &&
                again->number == 1 && again->cost == 300 &&
                again->inscrip[0] == 'x' && again->inscrip[1] == '\0');
}

TEST(a_whole_slot_can_be_copied_onto_another)
{
    /* 呼びだし側には枠ごとの代入がある（moria1.c:728-729 の武器の持ちかえ、
     * misc3.c の詰めなおし）。窓口越しでも構造体の代入で書ける形を守る。 */
    equipment_at(INVEN_WIELD)->tval = TV_BOW;
    equipment_at(INVEN_WIELD)->cost = 4321;
    *equipment_at(INVEN_AUX) = *equipment_at(INVEN_WIELD);

    ASSERT_TRUE(equipment_at(INVEN_AUX)->tval == TV_BOW &&
                equipment_at(INVEN_AUX)->cost == 4321);
}

/* --- 件数と重量 --------------------------------------------------------- */

TEST(the_pack_count_can_be_set_and_read_back)
{
    inventory_set_count(1);
    int one = inventory_count();

    inventory_set_count(22);
    ASSERT_TRUE(one == 1 && inventory_count() == 22);
}

TEST(the_pack_count_keeps_a_negative_value)
{
    /* 現状の記録。窓口は薄いので下限を守らない。呼びだし側は
     * inven_ctr-- を無条件でやる箇所がある（desc.c:248、misc3.c:845,876）。 */
    inventory_set_count(-1);
    ASSERT_EQ_INT(inventory_count(), -1);
}

TEST(the_equipment_count_can_be_set_and_read_back)
{
    equipment_set_count(2);
    int two = equipment_count();

    equipment_set_count(12);
    ASSERT_TRUE(two == 2 && equipment_count() == 12);
}

TEST(the_equipment_count_is_not_capped_at_the_number_of_slots)
{
    /* 現状の記録。装備の枠は 12 しかないが、窓口は上限を守らない。 */
    equipment_set_count(99);
    ASSERT_EQ_INT(equipment_count(), 99);
}

TEST(the_weight_can_be_set_and_read_back)
{
    inventory_set_weight(0);
    int zero = inventory_weight();

    inventory_set_weight(1500);
    ASSERT_TRUE(zero == 0 && inventory_weight() == 1500);
}

TEST(the_weight_wraps_around_at_the_int16_limit)
{
    /* 現状の記録。窓口は int でやりとりするが、実体は int16_t
     * （treasure.c:562）なので 32767 を超えると折りかえす。呼びだし側は
     * inven_weight += weight * number を無条件にやる。 */
    inventory_set_weight(40000);
    ASSERT_EQ_INT(inventory_weight(), -25536);
}

/* --- 跨ぎの窓口 --------------------------------------------------------- */

TEST(the_crossing_window_agrees_with_the_pack_window)
{
    int mismatch = -1;

    for (int i = 0; i < inventory_slot_count() && mismatch < 0; i++) {
        if (inventory_and_equipment_at(i) != inventory_at(i)) {
            mismatch = i;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(the_crossing_window_agrees_with_the_equipment_window)
{
    int mismatch = -1;

    for (int i = equipment_first_slot(); i < equipment_end_slot() && mismatch < 0; i++) {
        if (inventory_and_equipment_at(i) != equipment_at(i)) {
            mismatch = i;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(the_crossing_window_covers_every_slot_exactly_once)
{
    /* 全域 34 枠が、先頭から 1 枠ずつ隙間なく並んでいること。get_item()
     * （moria1.c:1189）は 0 から INVEN_ARRAY_SIZE-1 までを 1 続きの添字空間
     * として扱う。 */
    int mismatch = -1;
    inven_type *first = inventory_and_equipment_at(0);

    for (int i = 0; i < inventory_and_equipment_slot_count() && mismatch < 0; i++) {
        if (inventory_and_equipment_at(i) != first + i) {
            mismatch = i;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

/* --- 境界は誰も守っていない --------------------------------------------- */

TEST(the_pack_window_reaches_the_equipment_when_given_twenty_two)
{
    /* 現状の記録。窓口は薄いので境界を守らない。持ち物の枠は 0..21 なのに
     * inventory_at(22) は装備の 1 つめ（INVEN_WIELD）に届く。変更前も
     * inventory[22] がそこだったので、これはふるまいの保存であって
     * 直すべき欠陥の記録ではない（直すと呼びだし側の意味が変わる）。 */
    ASSERT_TRUE(inventory_at(inventory_slot_count()) == equipment_at(INVEN_WIELD));
}

TEST(the_equipment_window_reaches_the_pack_when_given_a_small_index)
{
    /* 逆向きも同じ。装備の窓口に 0 を渡すと持ち物の 1 枠めに届く。
     * 添字空間を割らないと決めた（22 を引く変換を入れない）結果である。 */
    ASSERT_TRUE(equipment_at(0) == inventory_at(0));
}

int main(void)
{
    /* 走りだしの状態を見る 2 件は先に走らせる（上の註参照）。 */
    RUN_TEST(every_slot_starts_out_empty);
    RUN_TEST(the_counts_and_the_weight_start_out_zero);

    RUN_TEST(the_pack_has_twenty_two_slots);
    RUN_TEST(the_equipment_starts_where_the_pack_ends);
    RUN_TEST(the_first_equipment_slot_is_inven_wield);
    RUN_TEST(there_are_thirty_four_slots_in_all);
    RUN_TEST(the_equipment_has_twelve_slots);
    RUN_TEST(the_equipment_ends_where_the_whole_array_ends);

    RUN_TEST(each_pack_index_addresses_a_different_slot);
    RUN_TEST(each_equipment_index_addresses_a_different_slot);
    RUN_TEST(the_slots_do_not_overlap_in_memory);
    RUN_TEST(the_same_index_always_gives_the_same_slot);
    RUN_TEST(what_was_written_to_a_pack_slot_stays_there);
    RUN_TEST(a_whole_slot_can_be_copied_onto_another);

    RUN_TEST(the_pack_count_can_be_set_and_read_back);
    RUN_TEST(the_pack_count_keeps_a_negative_value);
    RUN_TEST(the_equipment_count_can_be_set_and_read_back);
    RUN_TEST(the_equipment_count_is_not_capped_at_the_number_of_slots);
    RUN_TEST(the_weight_can_be_set_and_read_back);
    RUN_TEST(the_weight_wraps_around_at_the_int16_limit);

    RUN_TEST(the_crossing_window_agrees_with_the_pack_window);
    RUN_TEST(the_crossing_window_agrees_with_the_equipment_window);
    RUN_TEST(the_crossing_window_covers_every_slot_exactly_once);

    RUN_TEST(the_pack_window_reaches_the_equipment_when_given_twenty_two);
    RUN_TEST(the_equipment_window_reaches_the_pack_when_given_a_small_index);
    return TEST_SUMMARY();
}
