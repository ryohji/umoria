// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 未鑑定アイテムの名前組み立てのテスト -- 現在の実装を保護する
 *
 * desc.c の objdes() は「& %s Amulet」のような雛形（basenm）に材質や色
 * （modstr）を差しこんで名前を作る。差しこみは書式を変数で渡す sprintf で
 * 書かれていて、書式と引数の対応をコンパイラが検査できない
 * （-Wformat-nonliteral）。差しこみを自分で書く形へ変えるので、その前後で
 * 出力が 1 文字も変わらないことをここで押さえる。
 *
 * 未鑑定（品目ごとの覚えのビットが立っていない）だと objdes() の modify が
 * 真になり、雛形を使う経路に入る。fixture_reset() が覚えを全消しするので、
 * 各テストは未鑑定から始まる。
 *
 * 差しこむ語（amulets[] や colors[] のどれになるか）は magic_init() が
 * randint() で並べかえて決める。randint() は代役が固定値を返すので、
 * 実行するたび同じ語になる。並べかえは 1 度きりなので main で呼ぶ。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、いま
 * どう振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"

/* 検証に使う本物（src/desc.c, src/treasure.c）。externs.h は本体の宣言を
 * まるごと引きこむので、必要なものだけをここに書く。 */
extern treasure_type object_list[];
void invcopy(inven_type *, int);
void objdes(char *, inven_type *, int);
void known1(inven_type *);
void sample(inven_type *);
void store_bought(inven_type *);
void magic_init(void);

/* 各テストの前に必ず呼ばれる。品目ごとの覚えが毎回まっさらに戻る。 */
#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* object_list から、指定した tval の最初の品物の番号を探す。
 * 番号を直接書くと treasure.c の並びが変わったとき黙って別の品物を見る
 * ことになるので、探させる。 */
static int first_object_of_tval(int tval)
{
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (object_list[i].tval == tval) {
            return i;
        }
    }
    return -1;
}

/* テストの条件づくり。指定した tval の最初の品物を未鑑定のまま組みたて、
 * その名前を返す。分岐をテスト本体に持ちこまないため、品物が見つからない
 * 場合はここで空文字列を返す（期待値と一致しないので落ちる）。
 * pref = 0 は冠詞（a / an / the）を付けない指定。 */
static const char *name_of_first_unknown(int tval)
{
    static bigvtype name;
    name[0] = '\0';

    int index = first_object_of_tval(tval);
    if (index < 0) {
        return name;
    }

    inven_type item;
    invcopy(&item, index);
    objdes(name, &item, 0);

    return name;
}

/* 同じく、鑑定済みにしてからの名前。雛形を通らない経路の観測用。 */
static const char *name_of_first_known(int tval)
{
    static bigvtype name;
    name[0] = '\0';

    int index = first_object_of_tval(tval);
    if (index < 0) {
        return name;
    }

    inven_type item;
    invcopy(&item, index);
    known1(&item);
    objdes(name, &item, 0);

    return name;
}


/* 「試した」の表示を見るための条件づくり。desc.c:583-591 が品目ごとの覚えを
 * 引くのは冠詞つき（pref != 0）の経路だけなので、上の 2 つと違って pref に
 * 1 を渡す。marks に渡した関数で品物に印をつけてから名前を作る
 * （分岐をテスト本体に持ちこまないため、印のつけかたを引数にする）。 */
static const char *full_name_of_first(int tval, void (*mark)(inven_type *))
{
    static bigvtype name;
    name[0] = '\0';

    int index = first_object_of_tval(tval);
    if (index < 0) {
        return name;
    }

    inven_type item;
    invcopy(&item, index);
    if (mark != NULL) {
        mark(&item);
    }
    objdes(name, &item, 1);

    return name;
}


/* 同じ品目を 2 つ作り、1 つめを店で買った品にしたあと 2 つめを試す。
 * 枠は品目ごとに 1 つなので「試した」印は 2 つに共通で立つ。返すのは
 * 店で買ったほう（1 つめ）の名前。 */
static const char *name_of_store_bought_after_trying_the_same_kind(int tval)
{
    static bigvtype name;
    name[0] = '\0';

    int index = first_object_of_tval(tval);
    if (index < 0) {
        return name;
    }

    inven_type bought;
    invcopy(&bought, index);
    store_bought(&bought);

    inven_type found;
    invcopy(&found, index);
    sample(&found);

    objdes(name, &bought, 1);

    return name;
}

/* --- 雛形に材質・色を差しこむ経路（modstr != CNIL）--- */

TEST(unknown_amulet_is_named_by_its_material)
{
    ASSERT_EQ_STR(name_of_first_unknown(TV_AMULET), "Tortoise Shell Amulet");
}

TEST(unknown_ring_is_named_by_its_stone)
{
    ASSERT_EQ_STR(name_of_first_unknown(TV_RING), "Zircon Ring");
}

TEST(unknown_staff_is_named_by_its_wood)
{
    ASSERT_EQ_STR(name_of_first_unknown(TV_STAFF), "Walnut Staff");
}

TEST(unknown_wand_is_named_by_its_metal)
{
    ASSERT_EQ_STR(name_of_first_unknown(TV_WAND), "Zinc-Plated Wand");
}

TEST(unknown_scroll_is_named_by_its_title)
{
    /* 雛形が "& Scroll~ titled \"%s\"" なので、差しこみ位置が文字列の
     * 末尾ではなく途中にある。前後を取りちがえるとここで落ちる。 */
    ASSERT_EQ_STR(name_of_first_unknown(TV_SCROLL1), "Scroll titled \"a a\"");
}

TEST(unknown_potion_is_named_by_its_color)
{
    ASSERT_EQ_STR(name_of_first_unknown(TV_POTION1), "Icky Green Potion");
}

TEST(unknown_food_is_named_by_its_mushroom_color)
{
    ASSERT_EQ_STR(name_of_first_unknown(TV_FOOD), "Yellow Mushroom");
}

TEST(magic_book_name_is_inserted_at_the_end_of_the_template)
{
    /* 呪文書は modstr が object_list の名前そのもので、雛形
     * "& Book~ of Magic Spells %s" の末尾に差しこむ。鑑定の有無で
     * 変わらない。 */
    ASSERT_EQ_STR(name_of_first_unknown(TV_MAGIC_BOOK),
                  "Book of Magic Spells [Beginners-Magick]");
}

TEST(prayer_book_name_is_inserted_at_the_end_of_the_template)
{
    ASSERT_EQ_STR(name_of_first_unknown(TV_PRAYER_BOOK),
                  "Holy Book of Prayers [Beginners Handbook]");
}

/* --- 雛形を通らない経路（modstr == CNIL）--- */

TEST(known_amulet_uses_its_real_name)
{
    ASSERT_EQ_STR(name_of_first_known(TV_AMULET), "Amulet of Wisdom");
}

TEST(known_ring_uses_its_real_name)
{
    ASSERT_EQ_STR(name_of_first_known(TV_RING), "Ring of Strength");
}

TEST(known_staff_uses_its_real_name)
{
    ASSERT_EQ_STR(name_of_first_known(TV_STAFF), "Staff of Light");
}

TEST(known_wand_uses_its_real_name)
{
    ASSERT_EQ_STR(name_of_first_known(TV_WAND), "Wand of Light");
}

TEST(known_scroll_uses_its_real_name)
{
    ASSERT_EQ_STR(name_of_first_known(TV_SCROLL1),
                  "Scroll of Enchant Weapon To-Hit");
}

TEST(known_potion_uses_its_real_name)
{
    ASSERT_EQ_STR(name_of_first_known(TV_POTION1),
                  "Potion of Slime Mold Juice");
}

TEST(known_food_uses_its_real_name)
{
    ASSERT_EQ_STR(name_of_first_known(TV_FOOD), "Mushroom of Poison");
}


/* --- 「試した」の表示（品目ごとの覚えの OD_TRIED を引く 6 か所め）--- */

/* 試した品物は説明に "tried" が出る。desc.c:583-591 は #18-9-B より前は
 * object_offset() から添字を組みたてて表を引いていた、最後の 1 か所。 */
TEST(a_tried_item_says_tried_in_its_description)
{
    ASSERT_EQ_STR(full_name_of_first(TV_POTION1, sample),
                  "an Icky Green Potion {tried}.");
}

/* 三角測量：印をつけなければ出ない。上との差は sample() を呼んだかだけ。 */
TEST(an_untried_item_does_not_say_tried)
{
    ASSERT_EQ_STR(full_name_of_first(TV_POTION1, NULL),
                  "an Icky Green Potion.");
}

/* 鑑定すると "tried" は消える（known1 が同じ枠の OD_TRIED を落とすので、
 * 本当の名前で呼ばれるうえに印も残らない）。 */
TEST(a_known_item_does_not_say_tried)
{
    ASSERT_EQ_STR(full_name_of_first(TV_POTION1, known1),
                  "a Potion of Slime Mold Juice.");
}

/* 店で買うと「試した」印そのものが落ちる。store_bought() は known2() を
 * 呼び、その中の unsample() が枠の OD_TRIED を降ろすため（品名を言って
 * 売るので「試した」を覚えておく意味がない）。 */
TEST(buying_an_item_in_a_store_clears_that_the_kind_was_tried)
{
    ASSERT_EQ_STR(full_name_of_first(TV_POTION1, store_bought),
                  "a Potion of Slime Mold Juice.");
}

/* 店で買った品物は、**そのあとで同じ品目を試しても** "tried" と言わない
 * ——desc.c:589 が表の印と store_bought_p() を併せて見るのがこのため。
 * 枠は品目ごとに 1 つなので、ダンジョンで拾った同じ薬を飲んでみると印は
 * 店で買ったほうにも立つ。上のテストだけでは**この併せ見を観測できない**
 * （店で買った時点で印が落ちるので、印が立った店買いの品を作れない）。 */
TEST(a_store_bought_item_does_not_say_tried_even_after_the_kind_is_tried)
{
    ASSERT_EQ_STR(name_of_store_bought_after_trying_the_same_kind(TV_POTION1),
                  "a Potion of Slime Mold Juice.");
}

int main(void)
{
    /* 巻物の題名と薬の色の割りあてはここで 1 度だけ決まる。 */
    magic_init();

    RUN_TEST(unknown_amulet_is_named_by_its_material);
    RUN_TEST(unknown_ring_is_named_by_its_stone);
    RUN_TEST(unknown_staff_is_named_by_its_wood);
    RUN_TEST(unknown_wand_is_named_by_its_metal);
    RUN_TEST(unknown_scroll_is_named_by_its_title);
    RUN_TEST(unknown_potion_is_named_by_its_color);
    RUN_TEST(unknown_food_is_named_by_its_mushroom_color);
    RUN_TEST(magic_book_name_is_inserted_at_the_end_of_the_template);
    RUN_TEST(prayer_book_name_is_inserted_at_the_end_of_the_template);
    RUN_TEST(known_amulet_uses_its_real_name);
    RUN_TEST(known_ring_uses_its_real_name);
    RUN_TEST(known_staff_uses_its_real_name);
    RUN_TEST(known_wand_uses_its_real_name);
    RUN_TEST(known_scroll_uses_its_real_name);
    RUN_TEST(known_potion_uses_its_real_name);
    RUN_TEST(known_food_uses_its_real_name);

    RUN_TEST(a_tried_item_says_tried_in_its_description);
    RUN_TEST(an_untried_item_does_not_say_tried);
    RUN_TEST(a_known_item_does_not_say_tried);
    RUN_TEST(buying_an_item_in_a_store_clears_that_the_kind_was_tried);
    RUN_TEST(a_store_bought_item_does_not_say_tried_even_after_the_kind_is_tried);
    return TEST_SUMMARY();
}
