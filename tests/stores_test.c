/* 町の店 6 軒の記録のテスト -- 現在のふるまいを保護する
 *
 * この区分に計算は無い。パネル（#18-3）やメッセージの履歴（#18-2）と違って、
 * store は置き場そのもので、導出される値も丸めの規則も持たない。それでも
 * 4 ファイルが配列の名前を知っていて、うち 3 つは「添字から記録の先頭を
 * 求める」ところから始めていた。
 *   store1.c  10 か所  &store[n]（在庫の出し入れと値づけ）
 *   store2.c   9 か所  &store[n]（店に入って買う・売る）
 *   save.c     3 か所  &store[i]（6 軒ぶんの読み書き）
 *   game_state.c:40    store（配列の先頭をそのまま控える）
 * src/stores.c に寄せて、窓口は store_at() / store_count() だけにする。
 *
 * 保護するのは「置き場としてのふるまい」で、それは次の 3 つに尽きる。
 *   軒数が変わらない（6 軒。セーブファイルもこの数だけ並べて書く）
 *   添字ごとに別の記録である（重なると 2 軒が在庫を共有してしまう）
 *   書いたものが残り、同じ添字はいつでも同じ記録を指す
 *     （呼びだし側は s_ptr を局所変数に持ったまま他の関数を呼ぶ）
 * 最後の 1 つが変更前は「配列だから当然」で済んでいたところで、窓口越しに
 * なると当然ではなくなる（写しを返す実装ならここで落ちる）。
 */
/* externs.h も要らない。6 軒の記録が stores.c の static になったので、
 * ふれる先は stores.h の窓口だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "stores.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */

/* これだけは他のテストより先に走らせる（下の main() の並び順が保証する）。
 * 以降のテストが記録に書きこむので、あとから見ても 0 ではない。 */
TEST(every_store_starts_out_empty)
{
    int nonzero = -1;

    /* 変更前は tables.c の 0 初期化されるグローバルだった。古い版の
     * セーブファイルを読む道（save.c:872-888）は 6 軒ぶん読まずに戻ることが
     * あり、そのとき残りの店が空であることに頼っている。 */
    for (int i = 0; i < store_count() && nonzero < 0; i++) {
        store_type *s_ptr = store_at(i);

        if (s_ptr->store_ctr != 0 || s_ptr->owner != 0 || s_ptr->store_open != 0 ||
            s_ptr->insult_cur != 0 || s_ptr->good_buy != 0 || s_ptr->bad_buy != 0) {
            nonzero = i;
        }
    }
    ASSERT_EQ_INT(nonzero, -1);
}

/* --- 軒数 --------------------------------------------------------------- */

TEST(the_town_has_six_stores)
{
    /* 町の地図の 1..6 の扉に対応する。セーブファイルもこの数だけ並べて
     * 書くので、増減は既存のセーブファイルが読めなくなることを意味する。 */
    ASSERT_EQ_INT(store_count(), MAX_STORES);
}

TEST(six_is_the_number_the_owner_table_is_divided_by)
{
    /* store_init() は MAX_OWNERS / store_count() 人ずつの組から店主を
     * 選ぶ（tables.c の owners[] は店ごとの組で並んでいる）。割りきれない
     * と組の切れ目がずれるので、ここで押さえておく。 */
    ASSERT_EQ_INT(MAX_OWNERS % store_count(), 0);
}

/* --- 添字ごとに別の記録 ------------------------------------------------- */

TEST(each_index_addresses_a_different_store)
{
    int mismatch = -1;

    /* 添字を店主番号として書きこみ、全部を読みかえす。記録が重なっていれば
     * あとの書きこみが前を上書きするので、必ず食いちがう。 */
    for (int i = 0; i < store_count(); i++) {
        store_at(i)->owner = (uint8_t)(i + 1);
    }
    for (int i = 0; i < store_count() && mismatch < 0; i++) {
        if (store_at(i)->owner != (uint8_t)(i + 1)) {
            mismatch = i;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(the_stores_do_not_overlap_in_memory)
{
    int overlap = -1;

    /* 記録は在庫 24 枠を丸ごと抱えていて大きい。隣どうしの間隔が
     * 1 軒ぶんに足りなければ、在庫が隣の店にはみ出す。 */
    for (int i = 1; i < store_count() && overlap < 0; i++) {
        if (store_at(i) - store_at(i - 1) != 1) {
            overlap = i;
        }
    }
    ASSERT_EQ_INT(overlap, -1);
}

/* --- 書いたものが残る --------------------------------------------------- */

TEST(the_same_index_always_gives_the_same_record)
{
    /* 呼びだし側は store_type *s_ptr を局所変数に持ったまま他の関数を
     * 呼ぶ（store_carry() が insert_store() を呼ぶ形）。写しを返す実装だと
     * ここで落ちる。 */
    ASSERT_TRUE(store_at(3) == store_at(3));
}

TEST(what_was_written_to_a_store_stays_there)
{
    store_type *s_ptr = store_at(2);

    /* 在庫 1 枠に値段と品物を入れて、あとから窓口越しに読みかえす。 */
    s_ptr->store_ctr = 1;
    s_ptr->store_inven[0].scost = 1234;
    s_ptr->store_inven[0].sitem.tval = TV_SWORD;
    s_ptr->good_buy = 7;
    s_ptr->bad_buy = 8;
    s_ptr->insult_cur = 9;
    s_ptr->store_open = 10;

    store_type *again = store_at(2);
    ASSERT_TRUE(again->store_ctr == 1 && again->store_inven[0].scost == 1234 &&
                again->store_inven[0].sitem.tval == TV_SWORD &&
                again->good_buy == 7 && again->bad_buy == 8 &&
                again->insult_cur == 9 && again->store_open == 10);
}

TEST(a_store_can_fill_all_of_its_shelves)
{
    store_type *s_ptr = store_at(5);
    int mismatch = -1;

    /* 在庫は STORE_INVEN_MAX 枠。store_carry() は s_ptr->store_ctr の位置まで
     * 詰めて書くので、最後の枠まで記録の中に収まっていなければならない。 */
    for (int i = 0; i < STORE_INVEN_MAX; i++) {
        s_ptr->store_inven[i].scost = i + 1;
    }
    for (int i = 0; i < STORE_INVEN_MAX && mismatch < 0; i++) {
        if (store_at(5)->store_inven[i].scost != i + 1) {
            mismatch = i;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

int main(void)
{
    RUN_TEST(every_store_starts_out_empty); /* 先に走らせる（上の註参照） */
    RUN_TEST(the_town_has_six_stores);
    RUN_TEST(six_is_the_number_the_owner_table_is_divided_by);
    RUN_TEST(each_index_addresses_a_different_store);
    RUN_TEST(the_stores_do_not_overlap_in_memory);
    RUN_TEST(the_same_index_always_gives_the_same_record);
    RUN_TEST(what_was_written_to_a_store_stays_there);
    RUN_TEST(a_store_can_fill_all_of_its_shelves);
    return TEST_SUMMARY();
}
