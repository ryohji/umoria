// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 文字列をつなぐ concat のテスト -- 現在の実装を保護する（台帳 #47）
 *
 * concat は #54 で src/misc4.c から src/core/str_insert.c へ移した。移す前に
 * （#54-misc4-3A）ここでふるまいを押さえた。それまで本物の concat を通る
 * テストは 0 件だった（tests/creature_stubs.c に第 1 引数をそのまま返すだけの
 * 代役がある）。このテストが引く本体の .o は str_insert.o の 1 本だけ。
 *
 * 直接の呼び手は 0 で、呼ばれるのはいつも externs.h の CONCAT マクロ越し
 * （88 か所。creature 40・spells 34・moria3 7・moria4 7）:
 *   #define CONCAT(...) concat((vtype){0}, __VA_ARGS__, NULL)
 * だから第 1 引数はいつも vtype（80 バイト）で、最後はいつも NULL。
 *
 * 見るのは 5 つ: 戻り値は第 1 引数・NULL だけなら空文字列・順につなぐ・
 * 空文字列が混ざっても同じ・79 文字ちょうど（vtype に収まる最長）。
 * それに、呼び手の書く形（CONCAT を展開した複合リテラル）を 1 件。
 * concat は長さを見ない関数なので、あふれ（80 文字以上）は試さない。
 * 引数どうしや引数と第 1 引数の重なりも試さない（呼び手に無い形）。
 *
 * 期待値はすべて現在の実装が返した実際の値。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 検証対象。宣言は externs.h と str_insert.h にあるが、移す前後で同じ
 * テストを走らせるために、同じ形をここに書く（str_insert_test.c と同じ）。 */
char *concat(char *buffer, ...);

#include "minunit.h"

/* 前のテストの結果が残っていないことを見るため、書く先をいつも別の
 * 文字で埋めてから呼ぶ。 */
static vtype buffer;

static void fill_buffer(void)
{
    (void)memset(buffer, 'x', sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
}

/* --- 戻り値 --- */

/* CONCAT は戻り値をそのまま msg_print() などに渡すので、第 1 引数が
 * 返らないと複合リテラルの中身が読まれない。 */
TEST(returns_its_first_argument)
{
    fill_buffer();
    ASSERT_TRUE(concat(buffer, "You hit ", "the Orc", ".", NULL) == buffer);
}

/* --- NULL だけ --- */

/* 何もつながなくても、前に入っていた中身は消えて空文字列になる。 */
TEST(only_null_gives_the_empty_string)
{
    fill_buffer();
    ASSERT_EQ_STR(concat(buffer, NULL), "");
}

/* --- 順につなぐ --- */

TEST(a_single_string_is_copied_as_it_is)
{
    fill_buffer();
    ASSERT_EQ_STR(concat(buffer, "the Orc", NULL), "the Orc");
}

/* moria3.c の "You hit " cdesc "." の形。 */
TEST(three_strings_are_joined_in_order)
{
    fill_buffer();
    ASSERT_EQ_STR(concat(buffer, "You hit ", "the Orc", ".", NULL), "You hit the Orc.");
}

/* 2 つめ以降が前のものの終わりにつながる（上書きしない）ことを、
 * 長さの違う 5 つで見る。 */
TEST(five_strings_of_different_lengths_are_joined_in_order)
{
    fill_buffer();
    ASSERT_EQ_STR(concat(buffer, "a", "bcd", "ef", "ghijk", "l", NULL), "abcdefghijkl");
}

/* --- 空文字列が混ざる --- */

/* 空文字列は何も足さない。先頭・途中・末尾のどこにあっても同じ。 */
TEST(empty_strings_anywhere_add_nothing)
{
    fill_buffer();
    ASSERT_EQ_STR(concat(buffer, "", "The ", "", "Orc", "", NULL), "The Orc");
}

TEST(only_empty_strings_give_the_empty_string)
{
    fill_buffer();
    ASSERT_EQ_STR(concat(buffer, "", "", NULL), "");
}

/* --- 79 文字ちょうど --- */

/* vtype（80 バイト）に収まる最長。40 文字と 39 文字をつなぐ。 */
TEST(seventy_nine_characters_fit_exactly)
{
    char expected[80];

    (void)memset(expected, 'a', 40);
    (void)memset(expected + 40, 'b', 39);
    expected[79] = '\0';

    fill_buffer();
    ASSERT_EQ_STR(concat(buffer, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                         "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb", NULL),
                  expected);
}

/* --- CONCAT の形 --- */

/* 呼び手が書くのはいつもこの形（externs.h の CONCAT を展開したもの）。
 * 書く先はその場で作る複合リテラルで、中身は 0 で始まる。 */
TEST(the_concat_macro_form_joins_into_a_fresh_vtype)
{
    ASSERT_EQ_STR(concat((vtype){0}, "You have slain ", "the Orc", ".", NULL), "You have slain the Orc.");
}

int main(void)
{
    RUN_TEST(returns_its_first_argument);
    RUN_TEST(only_null_gives_the_empty_string);
    RUN_TEST(a_single_string_is_copied_as_it_is);
    RUN_TEST(three_strings_are_joined_in_order);
    RUN_TEST(five_strings_of_different_lengths_are_joined_in_order);
    RUN_TEST(empty_strings_anywhere_add_nothing);
    RUN_TEST(only_empty_strings_give_the_empty_string);
    RUN_TEST(seventy_nine_characters_fit_exactly);
    RUN_TEST(the_concat_macro_form_joins_into_a_fresh_vtype);
    return TEST_SUMMARY();
}
