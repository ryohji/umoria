/* 未鑑定アイテムの名前組み立てのテスト -- 現在の実装を保護する
 *
 * desc.c の objdes() は「& %s Amulet」のような雛形（basenm）に材質や色
 * （modstr）を差しこんで名前を作る。差しこみは書式を変数で渡す sprintf で
 * 書かれていて、書式と引数の対応をコンパイラが検査できない
 * （-Wformat-nonliteral）。差しこみを自分で書く形へ変えるので、その前後で
 * 出力が 1 文字も変わらないことをここで押さえる。
 *
 * 未鑑定（object_ident のビットが立っていない）だと objdes() の modify が
 * 真になり、雛形を使う経路に入る。fixture_reset() が object_ident を全消し
 * するので、各テストは未鑑定から始まる。
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
void magic_init(void);

/* 各テストの前に必ず呼ばれる。object_ident が毎回まっさらに戻る。 */
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
    return TEST_SUMMARY();
}
