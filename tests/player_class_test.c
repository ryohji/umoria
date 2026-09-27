/* 「どの階級か」のテスト -- 現在のふるまいを保護する
 *
 * py から出す **27 つめで最後の問い**で、**`struct misc` から出る 13 つめ**
 * （財布・どこまで来たか・どこまで潜ったか・体力の骰子・守りの点数・
 * 素の命中力・罠と鍵をはずす腕・抵抗・どの種族か・体の重さ・
 * 命中と打撃の下駄・どれくらい探すか・人物の身上書き 6 つ・足音の静かさに
 * つづく）。答えは 1 バイト —— もとは py.misc.pclass で、16 ファイルから
 * 55 か所が名ざしていた。**#18-12-28C でフィールドではなく `struct misc` が
 * struct ごと消えた**（残りが無かったので）。
 *
 * **この問いは「数」ではない**（21 つめ「どの種族か」と同じ形で、
 * この道で 2 つめ）。55 か所のうち**数として比べている場所も足している場所も
 * 1 つも無く、ぜんぶ表の添字か、そのまま渡すバイト**。だから `_adjust` の
 * 窓口は無く、これからも無い（**人物はローグに「なっていく」ものではない**）。
 *
 * **書かれるのは人物の一生で 2 回だけ** —— 階級を選んだとき（create.c）と
 * セーブを読みもどしたとき（save.c）。**3 つめの書きはメニューの前の `= 0`**
 * で、あのループは答えずに出られないので、その 0 が人物の答えになることは
 * 無い。
 *
 * 行番号と表の中身:
 *   0 Warrior   1 Mage   2 Priest   3 Rogue   4 Ranger   5 Paladin
 * （src/player.c:283 の class[MAX_CLASS]）
 *
 * **呼びは 55 で、この道でいちばん多い**（前の最多は 25 つめの身上書きの 44。
 * 前の単位は 9）。**答えが難しいからではなく、この 1 バイトで引く定数表が
 * 4 枚あるから** —— class[] そのもの・class_level_adj[][]・magic_spell[][]・
 * player_title[][]（＋起動時の player_init[][]）。
 *
 * **表に届く窓口は 2 本だけ**（`player_class_title()` と
 * `player_class_spell_type()`）。表そのものは module の外に置いたまま
 * extern 1 行で引く —— src/player_race.c が race[] に、src/player_level.c が
 * player_exp[] にしているのと同じ形。**だから足場は C でも消えなかった**
 * （消えたのは py の器だけ。tests/player_class_fixture.c に書いてある）。
 *
 * class_level_adj の 16 か所・magic_spell の 10 か所（`- 1` は**表の並びの
 * 知識**）・階級が何をくれるか・階層ごとの称号・魔法系か祈り系かで何が
 * 変わるか・得点表の `.class` は **どれも module の外**（player_class.h に
 * 6 つ書いてある）。
 *
 * **同じ幅の隣り（所見 46）は、この道でいちばん危ない並び。** セーブは
 * `stl`（short）→ **`pclass`（byte）→ `prace`（byte）→ `hitdie`（byte）→
 * `expfact`（byte）** で **byte が 4 本つづく**ので、save.c で 2 本の窓口を
 * 入れちがえても幅では気づけない —— **4 本が 4 つの別の module にあるので、
 * 並びを固定する 1 件はここには書けない**。**網は人物画面のほう**:
 * `pclass` と `prace` が入れかわると `Class` 行と `Race` 行が同時にずれ、
 * `hitdie` と入れかわると体力の上限が変わる。B の往復確認でそこを読む。
 *
 * テストは 1 プロセスで状態を共有するので、MU_SETUP が毎件 0 に戻す。
 */
/* externs.h は要らない。窓口 4 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_class.h"

/* 白紙は 0 —— 階級を選ぶ前の人物（`py` が全域 0 で始まるのと同じところ）で、
 * しかも Warrior の行番号でもある。 */
#define MU_SETUP() player_class_set(0)
#include "minunit.h"

/* 表は足場が空で持っている（tests/player_class_fixture.c）。名前と系を読む件が
 * 自分で入れる —— 本物の 6 行を写してしまうと、src/player.c が変わったときに
 * 足場だけが古くなって気づけない。 */
extern class_type class[MAX_CLASS];

/* 階級の欄に答えたところから始める（create.c:387 と同じ形）。
 * 引数は class[] の行番号ただ 1 つ。 */
static void given_the_menu_answered(int row) { player_class_set(row); }

/* 表の 6 行に名前を入れる。本物と同じ綴りを使うのは**この関数の中だけ**で、
 * 件はどれも「行 n の名前が返る」ことだけを見る。 */
static void given_the_table_has_the_six_names(void) {
    class[0].title = "Warrior";
    class[1].title = "Mage";
    class[2].title = "Priest";
    class[3].title = "Rogue";
    class[4].title = "Ranger";
    class[5].title = "Paladin";
}

/* 表の 6 行に系を入れる。**本物の並びと同じ** —— 戦士だけが NONE で、
 * 盗人と狩人は魔法系、聖人と騎士は祈り系（src/player.c:285 の Spell 欄）。 */
static void given_the_table_has_the_six_schools(void) {
    class[0].spell = NONE;
    class[1].spell = MAGE;
    class[2].spell = PRIEST;
    class[3].spell = MAGE;
    class[4].spell = MAGE;
    class[5].spell = PRIEST;
}

/* ------------------------------------------------------------------
 * 行番号そのもの -- 1 バイト、1 つの答え、足すものは無い
 * ------------------------------------------------------------------ */

TEST(the_row_is_whatever_the_menu_answered) {
    given_the_menu_answered(3); /* Rogue */

    ASSERT_EQ_INT(player_class(), 3);
}

/* 0 は「まだ階級を選んでいない」でもあり、ありうる答えでもある ——
 * Warrior の行番号が 0（種族の Human と同じ形）。 */
TEST(zero_is_both_the_starting_state_and_a_real_answer) {
    given_the_menu_answered(0); /* Warrior */

    ASSERT_EQ_INT(player_class(), 0);
}

TEST(setting_again_replaces_rather_than_adds) {
    given_the_menu_answered(5); /* Paladin */

    given_the_menu_answered(1); /* Mage */

    ASSERT_EQ_INT(player_class(), 1);
}

/* **メニューが出せる 6 つぜんぶが往復する。** 種族の表が階級を絞るので、
 * どの行が出るかは種族しだい —— だから 6 つとも通ることを見る。 */
TEST(every_row_the_menu_can_produce_comes_back) {
    for (int row = 0; row < MAX_CLASS; row++) {
        given_the_menu_answered(row);

        ASSERT_EQ_INT(player_class(), row);
    }
}

TEST(reading_the_row_twice_gives_the_same_answer) {
    given_the_menu_answered(4); /* Ranger */

    ASSERT_EQ_INT(player_class(), 4);
    ASSERT_EQ_INT(player_class(), 4);
}

/* ------------------------------------------------------------------
 * 窓口は何も断らない（所見 24）
 * ------------------------------------------------------------------ */

/* 0〜5 はメニューと表の規則で、窓口の規則ではない。 */
TEST(the_window_takes_a_row_past_the_end_of_the_table) {
    player_class_set(MAX_CLASS);

    ASSERT_EQ_INT(player_class(), MAX_CLASS);
}

/* 置き場は 1 バイトの**符号なし**のまま（`py.misc.pclass` は uint8_t）。 */
TEST(the_row_is_one_byte_wide_the_way_the_field_was) {
    player_class_set(256);

    ASSERT_EQ_INT(player_class(), 0);
}

TEST(a_negative_row_lands_where_the_byte_puts_it) {
    player_class_set(-1);

    ASSERT_EQ_INT(player_class(), 255);
}

/* **セーブファイルの 1 バイトはそのまま行番号に戻る。** save.c は
 * `wr_byte()` で書き `rd_byte()` で読む —— B では読んだバイトを窓口に
 * 渡す形になるので、往復で変わらないことをここで固定する。 */
TEST(the_saved_files_byte_comes_back_as_the_same_row) {
    uint8_t what_the_file_holds = 2; /* Priest */

    player_class_set(what_the_file_holds);

    ASSERT_EQ_INT(player_class(), 2);
}

/* ------------------------------------------------------------------
 * 階級の名前 -- player_race_name() の鏡像
 * ------------------------------------------------------------------ */

TEST(the_title_is_the_name_in_that_row_of_the_table) {
    given_the_table_has_the_six_names();
    given_the_menu_answered(2); /* Priest */

    ASSERT_EQ_STR(player_class_title(), "Priest");
}

TEST(the_title_follows_the_row) {
    given_the_table_has_the_six_names();
    given_the_menu_answered(0);

    given_the_menu_answered(4); /* Ranger */

    ASSERT_EQ_STR(player_class_title(), "Ranger");
}

/* **表に何も入っていなければ何も返らない。** 窓口は行を確かめない
 * （所見 24）—— `class[py.misc.pclass].title` がそうだったとおり。 */
TEST(the_title_window_hands_back_whatever_the_table_holds) {
    class[3].title = (const char *)0;
    given_the_menu_answered(3);

    ASSERT_TRUE(player_class_title() == (const char *)0);

    given_the_table_has_the_six_names(); /* あとの件のために戻す */
}

/* ------------------------------------------------------------------
 * 魔法系か祈り系か -- 3 つの答えが 1 つの数
 * ------------------------------------------------------------------ */

TEST(a_mage_class_casts_spells) {
    given_the_table_has_the_six_schools();
    given_the_menu_answered(1); /* Mage */

    ASSERT_EQ_INT(player_class_spell_type(), MAGE);
}

TEST(a_priest_class_says_prayers) {
    given_the_table_has_the_six_schools();
    given_the_menu_answered(2); /* Priest */

    ASSERT_EQ_INT(player_class_spell_type(), PRIEST);
}

/* **戦士には系が無い。** NONE は 0 なので、白紙の人物と同じ答えが返る ——
 * **どちらも本当に「魔法も祈りもしない」**（constant.h:262）。 */
TEST(a_warrior_has_no_school_at_all) {
    given_the_table_has_the_six_schools();
    given_the_menu_answered(0); /* Warrior */

    ASSERT_EQ_INT(player_class_spell_type(), NONE);
}

/* **魔法系は 3 つある**（Mage・Rogue・Ranger）—— 呼び手が `== MAGE` で
 * 訊いているのは「この階級が本を読むか」で、「Mage かどうか」ではない。 */
TEST(three_of_the_six_classes_are_on_the_mage_side) {
    given_the_table_has_the_six_schools();
    int how_many = 0;

    for (int row = 0; row < MAX_CLASS; row++) {
        given_the_menu_answered(row);
        if (player_class_spell_type() == MAGE) {
            how_many++;
        }
    }

    ASSERT_EQ_INT(how_many, 3);
}

TEST(the_school_follows_the_row) {
    given_the_table_has_the_six_schools();
    given_the_menu_answered(1);

    given_the_menu_answered(5); /* Paladin */

    ASSERT_EQ_INT(player_class_spell_type(), PRIEST);
}

/* ------------------------------------------------------------------
 * 呼び手の規則 -- 窓口は行番号を覚えるだけ
 * ------------------------------------------------------------------ */

/* **呪文の表の行は階級より 1 つ小さい**（`magic_spell[pclass - 1]`）。
 * ここに写しがあるのは **この module の外の規則**を 2 件で固定するためで、
 * 窓口の仕事ではない（`- 1` は「戦士に行が無い」という**表の並びの知識**で、
 * src/player.c:307 のコメントがこの道より古い）。 */
TEST(the_spell_tables_row_is_one_less_than_the_class) {
    given_the_menu_answered(2); /* Priest */

    int which_row_of_the_spell_table = player_class() - 1;

    ASSERT_EQ_INT(which_row_of_the_spell_table, 1);
}

/* **戦士には行が無い。** だから 10 か所の呼び手はどれも先に系を訊く ——
 * 訊かずに引くと `magic_spell[-1]` になる（バグ候補 B22 は
 * `misc3.c:1404` がまさにその番地を**確かめる前に**作っているところで、
 * 読まないので届かない）。 */
TEST(a_warrior_would_index_before_the_start_of_the_spell_table) {
    given_the_table_has_the_six_schools();
    given_the_menu_answered(0); /* Warrior */

    ASSERT_EQ_INT(player_class() - 1, -1);
    ASSERT_EQ_INT(player_class_spell_type(), NONE); /* だから先にこれを訊く */
}

/* **`class_level_adj[pclass][列]` の 16 か所は外に残る**（所見 42 の借りは
 * この単位で解禁するだけで、畳まない）。窓口が返すのはその添字だけ ——
 * 列の意味も掛ける階層も呼び手のもの。 */
TEST(the_row_is_the_subscript_the_skill_table_wants) {
    given_the_menu_answered(3); /* Rogue */

    int16_t the_skill_table[MAX_CLASS][MAX_LEV_ADJ];
    the_skill_table[3][CLA_DISARM] = 4;

    ASSERT_EQ_INT(the_skill_table[player_class()][CLA_DISARM], 4);
}

int main(void) {
    RUN_TEST(the_row_is_whatever_the_menu_answered);
    RUN_TEST(zero_is_both_the_starting_state_and_a_real_answer);
    RUN_TEST(setting_again_replaces_rather_than_adds);
    RUN_TEST(every_row_the_menu_can_produce_comes_back);
    RUN_TEST(reading_the_row_twice_gives_the_same_answer);

    RUN_TEST(the_window_takes_a_row_past_the_end_of_the_table);
    RUN_TEST(the_row_is_one_byte_wide_the_way_the_field_was);
    RUN_TEST(a_negative_row_lands_where_the_byte_puts_it);
    RUN_TEST(the_saved_files_byte_comes_back_as_the_same_row);

    RUN_TEST(the_title_is_the_name_in_that_row_of_the_table);
    RUN_TEST(the_title_follows_the_row);
    RUN_TEST(the_title_window_hands_back_whatever_the_table_holds);

    RUN_TEST(a_mage_class_casts_spells);
    RUN_TEST(a_priest_class_says_prayers);
    RUN_TEST(a_warrior_has_no_school_at_all);
    RUN_TEST(three_of_the_six_classes_are_on_the_mage_side);
    RUN_TEST(the_school_follows_the_row);

    RUN_TEST(the_spell_tables_row_is_one_less_than_the_class);
    RUN_TEST(a_warrior_would_index_before_the_start_of_the_spell_table);
    RUN_TEST(the_row_is_the_subscript_the_skill_table_wants);

    return TEST_SUMMARY();
}
