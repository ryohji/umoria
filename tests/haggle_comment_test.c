/* 値切り交渉メッセージ表示のテスト -- 現在の実装を保護する
 *
 * src/store2.c:123-135 の prt_comment2()（購入時）と
 * src/store2.c:137-150 の prt_comment3()（売却時）は、配列名と要素数を
 * のぞいて中身が完全に同一の 13 行である。
 *
 *   差異 1: 配列名  comment2a/comment2b vs comment3a/comment3b
 *   差異 2: 要素数  comment2b[16] vs comment3b[15]（a 側はどちらも 3）
 *   差異 3: 呼びだし側の引数順（最重要）
 *             購入 store2.c:571  prt_comment2(last_offer, cur_ask, ...)
 *             売却 store2.c:773  prt_comment3(cur_ask, last_offer, ...)
 *
 * 差異 3 がこのテストの主眼である。両関数のパラメータ名はどちらも
 * (offer, asking) だが、呼びだし側が渡す順序が逆になっている（売買で
 * 役割が入れかわるため）。つまり %A1 に入る値と %A2 に入る値は購入と
 * 売却で逆の意味になる。ステップ B で 1 つの関数に統合するときここを
 * 取りちがえると表示が壊れるので、%A1 / %A2 のどちらにどの引数が入るかを
 * このテストで固定する。
 *
 * 観測のしかた: prt_comment2 / prt_comment3 は static なので外部から
 * 呼べない。そこで src/store2.c をこの翻訳単位に #include し、内部の
 * static 関数を直接呼ぶ。写しではないので、ステップ B で実体を書きかえれば
 * このテストがそれを検証する。
 *
 * 出力は msg_print() で画面に出るだけなので、tests/misc3_stubs.c の代役が
 * 記録した内容を fixture_message_text() で読みとる。本体は変更していない。
 *
 * randint() は代役が固定値を返す（fixture_set_randint で制御）。
 * 添字は randint(N) - 1 なので、1 を渡せば配列の先頭、N を渡せば末尾。
 *
 * 期待値はすべて現在の実装が返した実際の文字列。仕様書はないので、
 * いまどう振るまうかを固定することが目的。
 */
/* 検証対象。static 関数を呼ぶために実体ごと取りこむ。config.h /
 * constant.h / types.h も store2.c が連れてくる（types.h に多重取りこみの
 * 番人が無いので、テスト側から重ねて include できない）。
 * store2.c の sprintf に -Wformat-overflow の警告が出るが、本体側の
 * 既存の事情なので makefile.test 側で 1 つだけ警告を落としている。 */
#include "store2.c"

#include "fixture.h"

/* store2.c が参照するが、代役にも本物にも無いシンボル。
 * prt_comment2 / prt_comment3 はどれも呼ばないので、
 * 「呼ばれたら何もしない／固定値を返す」で足りる。
 * 一覧はリンカに出させたもので、手で数えあげたわけではない。 */
char doing_inven = 0;
/* msg_flag はもう要らない。メッセージの状態は messages.c が持つので、
 * 代役ではなく本物をリンクしている（makefile.test 参照）。 */
/* turn は src/progress.c の static である（#19B で store2.c が progress_turn()
 * 越しに読むようになり、#19C1 で実体もそこへ移った）。 */

void move_cursor(int row, int col) { (void)row; (void)col; }
void inven_command(char command) { (void)command; }
int get_item(int *cn, const char *pmt, int i, int j, const char *m, const char *p) {
    (void)cn; (void)pmt; (void)i; (void)j; (void)m; (void)p;
    return 0;
}
int32_t item_value(inven_type *i_ptr) { (void)i_ptr; return 0; }
int32_t sell_price(int n, int32_t *g, int32_t *v, inven_type *i_ptr) {
    (void)n; (void)g; (void)v; (void)i_ptr;
    return 0;
}
bool store_check_num(inven_type *i_ptr, int n) { (void)i_ptr; (void)n; return false; }
void store_carry(int n, int *pos, inven_type *i_ptr) {
    (void)n; (void)pos; (void)i_ptr;
}
void store_destroy(int n, int pos, int one) { (void)n; (void)pos; (void)one; }
bool noneedtobargain(int n, int32_t min) { (void)n; (void)min; return false; }
void updatebargain(int n, int32_t price, int32_t min) {
    (void)n; (void)price; (void)min;
}

/* 各テストの前に必ず呼ばれる。記録した msg_print の内容が毎回消える。 */
#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* 直前に msg_print へ渡された 1 通目のメッセージ。 */
#define LAST_MESSAGE fixture_message_text(0)

/* ------------------------------------------------------------------
 * 差異 3 の保護: %A1 / %A2 に入る引数の対応
 *
 * 2 つの引数に必ず違う値（111 と 222）を渡す。取りちがえれば
 * 文字列が入れかわるので必ずレッドになる。
 *
 * comment2b[1] = "%A1 is an insult!  Try %A2 gold pieces."
 *   → 第 1 引数が先、第 2 引数が後に出る。
 * comment3b[0] = "%A2 for that piece of junk?  No more than %A1."
 *   → 第 2 引数が先、第 1 引数が後に出る。
 * ------------------------------------------------------------------ */

TEST(prt_comment2_puts_first_argument_into_A1_and_second_into_A2)
{
    fixture_set_randint(2); /* comment2b[1]: %A1 が先、%A2 が後 */
    prt_comment2(111, 222, 0);
    ASSERT_EQ_STR(LAST_MESSAGE, "111 is an insult!  Try 222 gold pieces.");
}

TEST(prt_comment3_puts_second_argument_into_A2_and_first_into_A1)
{
    fixture_set_randint(1); /* comment3b[0]: %A2 が先、%A1 が後 */
    prt_comment3(111, 222, 0);
    ASSERT_EQ_STR(LAST_MESSAGE,
                  "222 for that piece of junk?  No more than 111.");
}

/* 引数を入れかえて呼べば結果も入れかわる。差異 3 の取りちがえは
 * この 1 件と上の 1 件の組で必ず検出される。 */
TEST(prt_comment2_swapping_arguments_swaps_the_displayed_numbers)
{
    fixture_set_randint(2);
    prt_comment2(222, 111, 0);
    ASSERT_EQ_STR(LAST_MESSAGE, "222 is an insult!  Try 111 gold pieces.");
}

/* ------------------------------------------------------------------
 * 分岐の保護: final > 0 なら a 系、そうでなければ b 系
 *
 * a 系（comment2a / comment3a）は「最終提示」の 3 件だけの短い配列。
 * final の符号で使う配列が切りかわる。境界は 0 で、0 は b 系に入る。
 * ------------------------------------------------------------------ */

TEST(prt_comment2_with_positive_final_uses_comment2a)
{
    fixture_set_randint(1);
    prt_comment2(111, 222, 1);
    ASSERT_EQ_STR(LAST_MESSAGE, "222 is my final offer; take it or leave it.");
}

TEST(prt_comment2_with_zero_final_uses_comment2b)
{
    fixture_set_randint(1);
    prt_comment2(111, 222, 0);
    ASSERT_EQ_STR(LAST_MESSAGE,
                  "111 for such a fine item?  HA!  No less than 222.");
}

/* final は int なので負も渡せる。> 0 の条件なので負は b 系。 */
TEST(prt_comment2_with_negative_final_uses_comment2b)
{
    fixture_set_randint(1);
    prt_comment2(111, 222, -1);
    ASSERT_EQ_STR(LAST_MESSAGE,
                  "111 for such a fine item?  HA!  No less than 222.");
}

TEST(prt_comment3_with_positive_final_uses_comment3a)
{
    fixture_set_randint(1);
    prt_comment3(111, 222, 1);
    ASSERT_EQ_STR(LAST_MESSAGE, "I'll pay no more than 111; take it or leave it.");
}

TEST(prt_comment3_with_zero_final_uses_comment3b)
{
    fixture_set_randint(1);
    prt_comment3(111, 222, 0);
    ASSERT_EQ_STR(LAST_MESSAGE,
                  "222 for that piece of junk?  No more than 111.");
}

/* ------------------------------------------------------------------
 * 差異 2 の保護: 配列の要素数と randint の上限
 *
 * randint(N) - 1 が添字なので、randint が 1 を返せば添字 0（先頭）、
 * N を返せば添字 N-1（末尾）。b 系の N は comment2b が 16、comment3b が
 * 15 で異なる。ステップ B で統合するときここを取りちがえると、
 * comment3b に 16 を渡して配列外アクセスになる。
 * 末尾の要素を選ぶ値を必ず固定しておく。
 *
 * SUSPICIOUS: randint の上限は配列の実際の要素数と一致している
 * （store2.c:34-78 の定義を確認: comment2a[3] / comment2b[16] /
 * comment3a[3] / comment3b[15]）。配列外アクセスのバグは無い。
 * ただし要素数がリテラルで 2 箇所（配列宣言と randint の引数）に
 * 書かれており、片方だけ増減させると壊れる。直していない。
 * ------------------------------------------------------------------ */

TEST(prt_comment2b_index_sixteen_selects_the_last_element)
{
    fixture_set_randint(16); /* comment2b[15]、要素数 16 の末尾 */
    prt_comment2(111, 222, 0);
    ASSERT_EQ_STR(LAST_MESSAGE, "Your mother was a Troll!  222 or I'll tell.");
}

TEST(prt_comment3b_index_fifteen_selects_the_last_element)
{
    fixture_set_randint(15); /* comment3b[14]、要素数 15 の末尾 */
    prt_comment3(111, 222, 0);
    ASSERT_EQ_STR(LAST_MESSAGE, "222 is too much, let us say 111 gold.");
}

/* comment3b は 15 件しかないので、16 番目は comment2b の末尾とは
 * 別の文字列になる。統合時に上限を取りちがえたことを検出する。 */
TEST(prt_comment2b_element_fifteen_differs_from_its_last_element)
{
    fixture_set_randint(15); /* comment2b[14] */
    prt_comment2(111, 222, 0);
    ASSERT_EQ_STR(LAST_MESSAGE, "May the Balrog find you tasty!  222 gold pieces?");
}

TEST(prt_comment2a_index_three_selects_the_last_element)
{
    fixture_set_randint(3); /* comment2a[2]、要素数 3 の末尾 */
    prt_comment2(111, 222, 1);
    ASSERT_EQ_STR(LAST_MESSAGE, "My patience grows thin.  222 is final.");
}

TEST(prt_comment3a_index_three_selects_the_last_element)
{
    fixture_set_randint(3); /* comment3a[2]、要素数 3 の末尾 */
    prt_comment3(111, 222, 1);
    ASSERT_EQ_STR(LAST_MESSAGE, "111 and that's final.");
}

/* ------------------------------------------------------------------
 * insert_lnum の置換が起きない場合
 *
 * どの配列も全要素に %A1 と %A2 の両方が入っているわけではない。
 * 置換対象が無ければ insert_lnum は何もせず（misc3.c:1907、strchr が
 * 0 を返した経路で string が NULL になり最後の if を通らない）、
 * 渡した数値は黙って捨てられる。
 *
 * SUSPICIOUS: 置換に失敗しても呼びだし側に何も伝わらない。a 系は
 * comment2a の 3 件すべてが %A1 を持たず、comment3a の 3 件すべてが
 * %A2 を持たないので、片方の引数はつねに捨てられる。プレースホルダの
 * 綴りを間違えても静かに数値が消えるだけで気づけない。直していない。
 * ------------------------------------------------------------------ */

TEST(prt_comment2a_discards_the_offer_because_it_has_no_A1_placeholder)
{
    fixture_set_randint(2); /* comment2a[1] = "I'll give you no more than %A2." */
    prt_comment2(111, 222, 1);
    ASSERT_EQ_STR(LAST_MESSAGE, "I'll give you no more than 222.");
}

TEST(prt_comment3a_discards_the_asking_because_it_has_no_A2_placeholder)
{
    fixture_set_randint(2); /* comment3a[1] = "You'll get no more than %A1 from me." */
    prt_comment3(111, 222, 1);
    ASSERT_EQ_STR(LAST_MESSAGE, "You'll get no more than 111 from me.");
}

TEST(prt_comment2b_element_three_discards_the_asking_having_only_A1)
{
    fixture_set_randint(3); /* comment2b[2]: %A2 が無い */
    prt_comment2(111, 222, 0);
    ASSERT_EQ_STR(LAST_MESSAGE, "111?!?  You would rob my poor starving children?");
}

TEST(prt_comment3b_element_four_discards_the_asking_having_only_A1)
{
    fixture_set_randint(4); /* comment3b[3]: %A2 が無い */
    prt_comment3(111, 222, 0);
    ASSERT_EQ_STR(LAST_MESSAGE, "Let's be reasonable. How about 111 gold pieces?");
}

/* insert_lnum は show_sign = false で呼ばれるので、正の数に + は付かない。
 * 負の数は %d がそのまま符号を出す。 */
TEST(prt_comment2_prints_a_negative_offer_without_a_plus_sign)
{
    fixture_set_randint(2);
    prt_comment2(-5, 222, 0);
    ASSERT_EQ_STR(LAST_MESSAGE, "-5 is an insult!  Try 222 gold pieces.");
}

/* ------------------------------------------------------------------
 * 表示の回数と桁数
 *
 * SUSPICIOUS: comment は vtype（char[80]）だが、テンプレートの最長は 51 文字
 * （comment2b[10]）で、%A1 と %A2 の両方を持つ最長は 48 文字
 * （comment2b[9]）。int32_t は符号つき 11 文字まで伸びるので最悪
 * 48 - 6 + 22 = 64 文字。80 には収まる。ただし境界の検査はどこにも無く、
 * テンプレートを 1 つ長くしただけで溢れる。直していない。
 * ------------------------------------------------------------------ */

TEST(prt_comment2_calls_msg_print_exactly_once)
{
    fixture_set_randint(1);
    prt_comment2(111, 222, 0);
    ASSERT_EQ_INT(fixture_message_count(), 1);
}

TEST(prt_comment3_calls_msg_print_exactly_once)
{
    fixture_set_randint(1);
    prt_comment3(111, 222, 0);
    ASSERT_EQ_INT(fixture_message_count(), 1);
}

/* int32_t の最大値でも vtype に収まる。最長のテンプレートで確認する。 */
TEST(prt_comment2_fits_int32_max_on_both_placeholders_within_vtype)
{
    fixture_set_randint(10); /* comment2b[9] = %A1 と %A2 を持つ最長 */
    prt_comment2(2147483647, 2147483647, 0);
    ASSERT_EQ_STR(LAST_MESSAGE,
                  "As scrap this would bring 2147483647.  "
                  "Try 2147483647 in gold.");
}

/* ------------------------------------------------------------------
 * randint に渡す上限が配列の要素数と一致しているか
 *
 * 添字だけを固定するテストでは、上限の取りちがえを検出できない。
 * 代役の randint は maxval を無視して固定値を返すので、
 * comment2b（16 要素）に 15 を渡しても返る値は同じだからだ。
 * 上限がずれると配列外を読むので、上限そのものを観測して固定する。
 * ------------------------------------------------------------------ */

/* 購入・通常時は comment2b（16 要素）から選ぶ。 */
TEST(prt_comment2_asks_randint_for_sixteen_when_not_final)
{
    prt_comment2(111, 222, 0);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), 16);
}

/* 売却・通常時は comment3b（15 要素）から選ぶ。購入と 1 つ違う。 */
TEST(prt_comment3_asks_randint_for_fifteen_when_not_final)
{
    prt_comment3(111, 222, 0);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), 15);
}

/* 最終提示時は a 系（どちらも 3 要素）から選ぶ。 */
TEST(prt_comment2_asks_randint_for_three_when_final)
{
    prt_comment2(111, 222, 1);
    ASSERT_EQ_INT(fixture_randint_last_maxval(), 3);
}

/* randint の呼びだしは 1 回だけ。回数が変わると乱数列がずれ、
 * ゲーム全体のふるまいが変わる。 */
TEST(prt_comment2_calls_randint_exactly_once)
{
    prt_comment2(111, 222, 0);
    ASSERT_EQ_INT(fixture_randint_call_count(), 1);
}

int main(void)
{
    RUN_TEST(prt_comment2_puts_first_argument_into_A1_and_second_into_A2);
    RUN_TEST(prt_comment3_puts_second_argument_into_A2_and_first_into_A1);
    RUN_TEST(prt_comment2_swapping_arguments_swaps_the_displayed_numbers);
    RUN_TEST(prt_comment2_with_positive_final_uses_comment2a);
    RUN_TEST(prt_comment2_with_zero_final_uses_comment2b);
    RUN_TEST(prt_comment2_with_negative_final_uses_comment2b);
    RUN_TEST(prt_comment3_with_positive_final_uses_comment3a);
    RUN_TEST(prt_comment3_with_zero_final_uses_comment3b);
    RUN_TEST(prt_comment2b_index_sixteen_selects_the_last_element);
    RUN_TEST(prt_comment3b_index_fifteen_selects_the_last_element);
    RUN_TEST(prt_comment2b_element_fifteen_differs_from_its_last_element);
    RUN_TEST(prt_comment2a_index_three_selects_the_last_element);
    RUN_TEST(prt_comment3a_index_three_selects_the_last_element);
    RUN_TEST(prt_comment2a_discards_the_offer_because_it_has_no_A1_placeholder);
    RUN_TEST(prt_comment3a_discards_the_asking_because_it_has_no_A2_placeholder);
    RUN_TEST(prt_comment2b_element_three_discards_the_asking_having_only_A1);
    RUN_TEST(prt_comment3b_element_four_discards_the_asking_having_only_A1);
    RUN_TEST(prt_comment2_prints_a_negative_offer_without_a_plus_sign);
    RUN_TEST(prt_comment2_calls_msg_print_exactly_once);
    RUN_TEST(prt_comment3_calls_msg_print_exactly_once);
    RUN_TEST(prt_comment2_fits_int32_max_on_both_placeholders_within_vtype);
    RUN_TEST(prt_comment2_asks_randint_for_sixteen_when_not_final);
    RUN_TEST(prt_comment3_asks_randint_for_fifteen_when_not_final);
    RUN_TEST(prt_comment2_asks_randint_for_three_when_final);
    RUN_TEST(prt_comment2_calls_randint_exactly_once);
    return TEST_SUMMARY();
}
