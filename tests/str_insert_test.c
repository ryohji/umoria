/* 文字列への差しこみのテスト -- 現在の実装を保護する
 *
 * 対象は src/misc3.c の insert_str（1649-1683）と insert_lnum（1685-1716）。
 * #41 でこの 2 つを新しい module（src/str_insert.c）へ移すので、移す前に
 * ここで出力を 1 文字も変わらないように押さえる。
 *
 * 既存テストが通していない経路を埋めるのが主眼:
 *   insert_str  -- insert が NULL でない経路（objdes_test が通すのは食料 2 件
 *                  だけで、そこでは insert が CNIL）、一致しない場合、
 *                  mtc_str が object_str より長い場合、同じ雛形が 2 回
 *                  現れる場合、先頭と末尾での一致
 *   insert_lnum -- show_sign が真の経路（呼びだし 2 箇所（store2.c:145-146）は
 *                  どちらも false なので、+ が付く形はどのテストも通って
 *                  いない）、1 文字目だけ一致して続きが違う場合、一致が
 *                  無い場合
 *
 * 差しこむ雛形の形は実際の呼びだしから採った（desc.c:463-464 の "ch~"→"ches"
 * と "~"→"s"、store2.c:145-146 の "%A1" / "%A2"）。
 *
 * 溢れは試さない。insert_str の作業領域は char out_val[80] 固定で、strcat が
 * 長さを見ないので、差しこんだ結果が 80 バイトを超えると壊れる（バグ候補として
 * 報告する）。ここで溢れさせても固定できるのは未定義動作なので、入力はすべて
 * vtype（80 バイト）に収まる範囲にしている。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、いま
 * どう振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"

/* 検証対象。#41 の移動が済むまで実体は misc3.c にあり、宣言は externs.h に
 * ある。externs.h は ncurses まで引きこむので、必要な 2 つだけをここに書く。 */
void insert_str(char *object_str, const char *mtc_str, const char *insert);
void insert_lnum(char *object_str, const char *mtc_str, int32_t number, int show_sign);

#include "minunit.h"

/* テストの条件づくり。雛形を書きこめる場所に写してから差しこみを行い、
 * 結果を返す。insert_str は object_str を書きかえるので、文字列リテラルを
 * そのまま渡すことはできない。
 * 大きさは insert_lnum が内部で使う vtype に合わせる。 */
static const char *str_inserted(const char *template_str, const char *mtc_str, const char *insert)
{
    static vtype work;

    (void)strcpy(work, template_str);
    insert_str(work, mtc_str, insert);
    return work;
}

/* insert_lnum 版。show_sign は int（bool ではない）で受ける。 */
static const char *lnum_inserted(const char *template_str, const char *mtc_str, int32_t number, int show_sign)
{
    static vtype work;

    (void)strcpy(work, template_str);
    insert_lnum(work, mtc_str, number, show_sign);
    return work;
}

/* --- insert_str --- */

/* desc.c:463-464 の複数形化。insert が NULL でない経路はここだけで通る。 */
TEST(ch_tilde_becomes_ches) { ASSERT_EQ_STR(str_inserted("Torch~", "ch~", "ches"), "Torches"); }

TEST(tilde_becomes_s) { ASSERT_EQ_STR(str_inserted("Arrow~", "~", "s"), "Arrows"); }

/* insert が NULL なら雛形は消えるだけ（単数形のときの呼びだし）。 */
TEST(null_insert_just_removes_the_template) { ASSERT_EQ_STR(str_inserted("Arrow~", "~", NULL), "Arrow"); }

/* 一致が無ければ object_str は変わらない。 */
TEST(no_match_leaves_the_string_untouched) { ASSERT_EQ_STR(str_inserted("Arrow", "~", "s"), "Arrow"); }

/* mtc_str が object_str より長い場合。bound が object_str より手前に来るので
 * 走査は 1 度も回らない。 */
TEST(mtc_str_longer_than_object_str_leaves_the_string_untouched) { ASSERT_EQ_STR(str_inserted("ch", "chest~", "s"), "ch"); }

/* 同じ雛形が 2 回現れる場合。置きかわるのは先頭の 1 つだけ。 */
TEST(only_the_first_of_two_templates_is_replaced) { ASSERT_EQ_STR(str_inserted("Torch~ and Torch~", "ch~", "ches"), "Torches and Torch~"); }

/* 先頭での一致（pc == object_str の端）。 */
TEST(match_at_the_head_is_replaced) { ASSERT_EQ_STR(str_inserted("~ of Wonder", "~", "Wands"), "Wands of Wonder"); }

/* 末尾での一致（pc == bound の端）。object_str の全体が mtc_str の場合。 */
TEST(match_at_the_tail_is_replaced) { ASSERT_EQ_STR(str_inserted("~", "~", "s"), "s"); }

/* insert が空文字列の場合。NULL とふるまいが分かれないことを押さえる。 */
TEST(empty_insert_removes_the_template_like_null_does) { ASSERT_EQ_STR(str_inserted("Arrow~", "~", ""), "Arrow"); }

/* 途中まで一致して外れる位置がある場合（内側のループが break して次へ進む）。 */
TEST(a_partial_match_is_skipped_and_the_scan_continues) { ASSERT_EQ_STR(str_inserted("chch~", "ch~", "ches"), "chches"); }

/* --- insert_lnum --- */

TEST(show_sign_prepends_plus_to_a_positive_number) { ASSERT_EQ_STR(lnum_inserted("I offer %A1 gold", "%A1", 250, 1), "I offer +250 gold"); }

TEST(show_sign_prepends_plus_to_zero) { ASSERT_EQ_STR(lnum_inserted("I offer %A1 gold", "%A1", 0, 1), "I offer +0 gold"); }

TEST(show_sign_does_not_touch_a_negative_number) { ASSERT_EQ_STR(lnum_inserted("I offer %A1 gold", "%A1", -50, 1), "I offer -50 gold"); }

/* store2.c:145-146 の呼びだしはこちら（show_sign = false）。 */
TEST(without_show_sign_a_positive_number_has_no_plus) { ASSERT_EQ_STR(lnum_inserted("I offer %A1 gold", "%A1", 250, 0), "I offer 250 gold"); }

TEST(without_show_sign_a_negative_number_keeps_its_minus) { ASSERT_EQ_STR(lnum_inserted("I offer %A1 gold", "%A1", -50, 0), "I offer -50 gold"); }

/* 1 文字目だけ一致して続きが違う場合。do/while が次の候補を探しに行く。 */
TEST(a_first_char_match_with_a_different_rest_is_skipped) { ASSERT_EQ_STR(lnum_inserted("50% of %A2 is", "%A2", 300, 0), "50% of 300 is"); }

/* 先頭での一致。 */
TEST(lnum_match_at_the_head_is_replaced) { ASSERT_EQ_STR(lnum_inserted("%A1 gold offered", "%A1", 250, 0), "250 gold offered"); }

/* 1 文字目は見つかるが完全一致は無い場合（do/while が探しつくして抜ける）。 */
TEST(a_first_char_match_without_a_full_match_leaves_the_string_untouched) { ASSERT_EQ_STR(lnum_inserted("50% off", "%A1", 250, 1), "50% off"); }

/* 一致が無ければ object_str は変わらない。 */
TEST(lnum_no_match_leaves_the_string_untouched) { ASSERT_EQ_STR(lnum_inserted("no marker here", "%A1", 250, 1), "no marker here"); }

/* 最後の 1 文字だけ違う雛形は一致しない。store2.c は同じ文にある "%A1" と
 * "%A2" を 2 回の呼びだしで別々に埋めるので、この区別が要る。 */
TEST(a_marker_differing_in_the_last_char_is_not_matched) { ASSERT_EQ_STR(lnum_inserted("I offer %A1 for %A2", "%A2", 300, 0), "I offer %A1 for 300"); }

TEST(the_earlier_of_two_similar_markers_is_replaced_alone) { ASSERT_EQ_STR(lnum_inserted("I offer %A1 for %A2", "%A1", 250, 0), "I offer 250 for %A2"); }

/* 末尾での一致（str2 が空になる）。 */
TEST(lnum_match_at_the_tail_is_replaced) { ASSERT_EQ_STR(lnum_inserted("asking %A2", "%A2", 300, 1), "asking +300"); }

int main(void)
{
    RUN_TEST(ch_tilde_becomes_ches);
    RUN_TEST(tilde_becomes_s);
    RUN_TEST(null_insert_just_removes_the_template);
    RUN_TEST(no_match_leaves_the_string_untouched);
    RUN_TEST(mtc_str_longer_than_object_str_leaves_the_string_untouched);
    RUN_TEST(only_the_first_of_two_templates_is_replaced);
    RUN_TEST(match_at_the_head_is_replaced);
    RUN_TEST(match_at_the_tail_is_replaced);
    RUN_TEST(empty_insert_removes_the_template_like_null_does);
    RUN_TEST(a_partial_match_is_skipped_and_the_scan_continues);
    RUN_TEST(show_sign_prepends_plus_to_a_positive_number);
    RUN_TEST(show_sign_prepends_plus_to_zero);
    RUN_TEST(show_sign_does_not_touch_a_negative_number);
    RUN_TEST(without_show_sign_a_positive_number_has_no_plus);
    RUN_TEST(without_show_sign_a_negative_number_keeps_its_minus);
    RUN_TEST(a_first_char_match_with_a_different_rest_is_skipped);
    RUN_TEST(lnum_match_at_the_head_is_replaced);
    RUN_TEST(a_first_char_match_without_a_full_match_leaves_the_string_untouched);
    RUN_TEST(lnum_no_match_leaves_the_string_untouched);
    RUN_TEST(a_marker_differing_in_the_last_char_is_not_matched);
    RUN_TEST(the_earlier_of_two_similar_markers_is_replaced_alone);
    RUN_TEST(lnum_match_at_the_tail_is_replaced);
    return TEST_SUMMARY();
}
