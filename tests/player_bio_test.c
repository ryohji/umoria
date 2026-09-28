// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「人物の身上書き」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 25 つめの問いで、**`struct misc` から出る 11 つめ**
 * （財布・どこまで来たか・どこまで潜ったか・体力の骰子・守りの点数・
 * 素の命中力・罠と鍵をはずす腕・抵抗・どの種族か・体の重さ・命中と打撃の
 * 下駄・どれくらい探すかにつづく）。答えは **6 つ** ——
 * `name`・`male`・`age`・`ht`・`sc`・`history`。**6 フィールドが一度に出る**
 * （これまでの最多は 5 つの `#18-12-6`）。
 *
 * **6 つの答えで module は 1 つ。束ねる規準（片方だけを読む場所があるか）では
 * 割れない** —— 6 つとも別々に読まれ、**2 つを足したり比べたりする場所は
 * 1 つも無い**。それでも 1 本にするのは 2 つの性質から:
 *
 *   1. **創成が一度置いたら誰も動かさない** —— 足す場所が 1 つも無いので
 *      **`_adjust` の窓口が 1 本も無い**（22 つめの体の重さが初めての例で、
 *      ここはそれが 6 つ並ぶ）。例外は `name` だけで、`change_name()` が
 *      **置きなおす**（足すのではない）。
 *   2. **訊く者が同じ 4 か所に集まっている** —— 人物画面・持ち出しファイル・
 *      セーブの書きと読み。
 *
 * 外に残すものは 5 つ（player_bio.h に書いてある）。`male` の規則 3 つ
 * （王か女王か・最初の持ち金の +50・どちらの身長の表を引くか）は
 * **どれも呼び手**で、下の 3 件がそれを固定する。
 *
 * **窓口は 13 本**（これまでの最多は 18 つめの 7 本）—— 読み 6・置く 6・
 * 消す 1。**`_adjust` は 1 本も無い。**
 *
 * テストは 1 プロセスで状態を共有するので、MU_SETUP が毎件 6 つとも白紙に
 * 戻す（`player_search_skill_test` は各件が自分で 2 つを置いていたが、
 * 6 つあると数が多いので setUp にまとめる）。
 */
/* externs.h は要らない。窓口 13 本と、型のための 3 つ、それに写しの memcpy。 */
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_bio.h"

/* 白紙の人物 —— **6 つとも 0 が走りだし**（名前は空・男ではない・年齢と身長と
 * 階層は 0・生い立ちは 4 行とも空）。`py` を消すのではなく窓口ごしに戻すのは、
 * #18-12-26C で置き場が static に移っても同じ 1 行で足りるから
 * （足場 `player_bio_fixture.c` はそのとき消える）。 */
static void given_a_blank_record(void) {
    player_name_set("");
    player_set_male(false);
    player_age_set(0);
    player_height_set(0);
    player_social_class_set(0);
    player_history_clear();
}

/* 創成が 6 つを置いたところから始める（create.c の並びは
 * 性別 `:301`〜`:305` → 生い立ち `:273` → 階層 `:285` → 年齢 `:319` →
 * 身長 `:321`/`:326` で、名前は `get_name()` が別に訊く）。 */
static void given_a_character(const char *name, bool male, int age, int height,
                              int social_class) {
    player_name_set(name);
    player_set_male(male);
    player_age_set(age);
    player_height_set(height);
    player_social_class_set(social_class);
}

/* 長さ `length` の行を作る。'a' から順に並べるので、切られた場所が目で見える。 */
static void build_line(char *buffer, int length) {
    for (int i = 0; i < length; i++) {
        buffer[i] = (char)('a' + i % 26);
    }
    buffer[length] = '\0';
}

/* `from` から `to` の手前までが 0 で埋まっているか。終端より先のバイトを
 * 見るのは、`get_string()` の中断の経路がそこを読むから（下の 2 件）。 */
static int tail_is_all_zero(const char *buffer, int from, int to) {
    for (int i = from; i < to; i++) {
        if (buffer[i] != '\0') {
            return 0;
        }
    }
    return 1;
}

#define MU_SETUP() given_a_blank_record()
#include "minunit.h"

/* ------------------------------------------------------------------
 * 6 つの答え -- 別々に読め、互いに移らない
 * ------------------------------------------------------------------ */

TEST(the_name_is_what_the_player_typed) {
    given_a_character("Aragorn", true, 87, 72, 45);

    ASSERT_EQ_STR(player_name(), "Aragorn");
}

TEST(the_sex_is_a_separate_answer_from_the_name) {
    given_a_character("Arwen", false, 2901, 66, 71);

    ASSERT_FALSE(player_is_male());
}

TEST(the_age_is_what_creation_rolled) {
    given_a_character("Aragorn", true, 87, 72, 45);

    ASSERT_EQ_INT(player_age(), 87);
}

TEST(the_height_is_a_different_number_from_the_age) {
    given_a_character("Aragorn", true, 87, 72, 45);

    ASSERT_EQ_INT(player_height(), 72);
}

/* **同じ幅・同じ並びの落とし穴**（→ 所見 46）。`age` と `ht` は
 * **どちらも `uint16_t`** で、`struct misc` でもセーブファイルでも
 * **隣りあっている**（save.c:163 が `age`、`:164` が `ht`）——
 * 置き場を入れかえても書きと読みが同じ嘘をつくので、**この 1 件が無いと
 * 原理的に見えない**。24 つめの `srh`/`fos` で同じことが起きたので、
 * **今回は最初から書いた**。 */
TEST(the_age_comes_before_the_height_the_way_the_saved_file_holds_it) {
    given_a_blank_record();

    player_age_set(25);
    player_height_set(72);

    ASSERT_EQ_INT(player_age(), 25);
    ASSERT_EQ_INT(player_height(), 72);
}

TEST(the_social_class_is_what_the_life_story_added_up_to) {
    given_a_character("Aragorn", true, 87, 72, 45);

    ASSERT_EQ_INT(player_social_class(), 45);
}

TEST(each_story_line_is_read_by_its_own_number) {
    player_history_line_set(0, "You are one of several children");
    player_history_line_set(1, "of a Serf.  You are a credit to");
    player_history_line_set(2, "the family.");

    ASSERT_EQ_STR(player_history_line(1), "of a Serf.  You are a credit to");
}

/* **6 つとも 0 が走りだし** —— 創成が振るまで人物は名前も年齢も持たない
 * （`py` が全域 0 で始まるのと同じところ）。 */
TEST(a_character_who_has_not_been_rolled_has_nothing) {
    given_a_blank_record();

    ASSERT_EQ_STR(player_name(), "");
    ASSERT_FALSE(player_is_male());
    ASSERT_EQ_INT(player_age(), 0);
    ASSERT_EQ_INT(player_height(), 0);
    ASSERT_EQ_INT(player_social_class(), 0);
    ASSERT_EQ_STR(player_history_line(0), "");
}

/* **6 つは互いに移らない。** 1 つを置きなおしても他の 5 つは動かない ——
 * `struct misc` で隣りあっていたものが 1 本の module に入っても、
 * 答えは 6 つのままだということ。 */
TEST(setting_one_answer_leaves_the_other_five_alone) {
    given_a_character("Aragorn", true, 87, 72, 45);
    player_history_line_set(0, "of a Serf.");

    player_age_set(21);

    ASSERT_EQ_STR(player_name(), "Aragorn");
    ASSERT_TRUE(player_is_male());
    ASSERT_EQ_INT(player_age(), 21);
    ASSERT_EQ_INT(player_height(), 72);
    ASSERT_EQ_INT(player_social_class(), 45);
    ASSERT_EQ_STR(player_history_line(0), "of a Serf.");
}

/* 置きなおすと**あとに置いたほうが残る**。足すのではない ——
 * **この問いには足す窓口が 1 本も無い**（年齢は上がらない）。 */
TEST(setting_again_replaces_rather_than_adds) {
    player_age_set(87);

    player_age_set(21);

    ASSERT_EQ_INT(player_age(), 21);
}

/* ------------------------------------------------------------------
 * 名前 -- 番地を渡していた 4 か所のぶん
 * ------------------------------------------------------------------ */

/* 器は `PLAYER_NAME_SIZE`（27）—— 終端を入れて 26 字まで入る。
 * `get_string(py.misc.name, 2, 15, 23)` が打たせるのは 23 字なので、
 * **打って入れられる名前はどれも余裕で入る**。 */
TEST(a_name_of_twenty_six_characters_fits) {
    char name[PLAYER_NAME_SIZE];
    build_line(name, PLAYER_NAME_SIZE - 1);

    player_name_set(name);

    ASSERT_EQ_STR(player_name(), name);
}

/* **27 字以上は器が止まっていたところで切れる。** フィールドのころ、
 * ここに 27 字を書けたのは `user_name()` だけで（`strcpy` で
 * `getlogin()` を丸ごと写す）、**27 字以上のログイン名はフィールドの外に
 * 書いていた** —— 隣の `male` と `age` に。窓口を通すと切れる。 */
TEST(a_longer_name_is_cut_where_the_record_stopped) {
    char name[PLAYER_NAME_SIZE + 10];
    build_line(name, PLAYER_NAME_SIZE + 9);

    player_name_set(name);

    ASSERT_EQ_INT((int)strlen(player_name()), PLAYER_NAME_SIZE - 1);
}

/* `get_name()`（misc3.c:817）は **`name[0] == 0` を見て**、空なら
 * `user_name()` に既定を入れさせる。空が読めることがその判断の土台。 */
TEST(an_empty_name_is_what_the_prompt_asks_about) {
    player_name_set("Aragorn");

    player_name_set("");

    ASSERT_EQ_STR(player_name(), "");
}

/* **終端より先はぜんぶ 0** という置き場の約束。長い名前のあとに短い名前を
 * 置いても、長いほうの尻尾は残らない。 */
TEST(a_shorter_name_leaves_no_tail_of_the_longer_one) {
    player_name_set("Aragorn");

    player_name_set("Bob");

    char whole[PLAYER_NAME_SIZE];
    memcpy(whole, player_name(), PLAYER_NAME_SIZE);
    ASSERT_TRUE(tail_is_all_zero(whole, 3, PLAYER_NAME_SIZE));
}

/* **`get_string()` の中断の経路。** io.c:325 は **ESC のとき終端を書かずに
 * 返す** —— 打った字だけが器に入り、その先は前のまま残る。`get_name()` は
 * そのあと `name[0] == 0` を見るので、**中断の経路は終端より先のバイトを
 * 読んでいる**。B ではこの形になる: 窓口から **27 バイト全部を** 局所に
 * `memcpy` で写し（`strcpy` では終端までしか来ない）、`get_string()` に
 * 書かせ、終わってから `player_name_set()` で置く。
 *
 * **上の約束（終端より先は 0）があるから、写しても前と同じものが読める。** */
TEST(the_escape_path_of_the_name_prompt_reads_past_the_terminator) {
    player_name_set("Bob");

    char typed[PLAYER_NAME_SIZE];
    memcpy(typed, player_name(), PLAYER_NAME_SIZE);
    /* ESC の枝 —— 5 字打ったところで中断。終端は書かれない。 */
    typed[0] = 'A';
    typed[1] = 'r';
    typed[2] = 'w';
    typed[3] = 'e';
    typed[4] = 'n';

    ASSERT_EQ_STR(typed, "Arwen");
}

/* 読みもどしは器の番地に読んでいた（`rd_string(m_ptr->name)`）。
 * B で局所の `char[PLAYER_NAME_SIZE]` に受けてから置く形になる
 * （save.c:618）—— **置きなおす窓口は名前を訊く側と同じ 1 本**。 */
TEST(loading_a_saved_name_uses_the_very_same_window) {
    given_a_blank_record();

    char name_from_the_file[PLAYER_NAME_SIZE] = "Aragorn";
    player_name_set(name_from_the_file);

    ASSERT_EQ_STR(player_name(), "Aragorn");
}

/* ------------------------------------------------------------------
 * 性別 -- 8 か所ぜんぶが真偽で読む 1 バイト
 * ------------------------------------------------------------------ */

TEST(the_sex_goes_back_and_forth) {
    player_set_male(false);

    player_set_male(true);

    ASSERT_TRUE(player_is_male());
}

/* **窓口は `bool` だがフィールドは `uint8_t` だった。** 手で 2 を書きこんだ
 * セーブファイルのバイトは、いまは `wr_byte(m_ptr->male)` がそのまま書き
 * もどすので 2 のまま往復する。B では `rd_byte(&raw)` の raw を
 * `player_set_male(raw != 0)` に渡すので **1 になる** —— 遊びの上の
 * ふるまいは変わらない（2 も真）。**0 でなければ男**を固定する。 */
TEST(a_hand_edited_byte_of_two_is_still_male) {
    uint8_t byte_from_the_file = 2;

    player_set_male(byte_from_the_file != 0);

    ASSERT_TRUE(player_is_male());
}

/* ------------------------------------------------------------------
 * 幅と留め -- 窓口は何も断らない（所見 24）
 * ------------------------------------------------------------------ */

/* 年齢の置き場は 2 バイトの**符号なし**のまま（`py.misc.age` は `uint16_t`）。
 * `race[i].b_age + randint(race[i].m_age)` が切り落としていたのと同じ
 * ところで折りかえす。 */
TEST(the_age_is_two_bytes_wide_and_unsigned_the_way_the_field_was) {
    player_age_set(65536);

    ASSERT_EQ_INT(player_age(), 0);
}

/* **身長も符号なし 2 バイト**。`randnor()` は負を返すことがあるので
 * （`m_b_ht` の下に振れる）、そのときフィールドは 65535 側に回っていた ——
 * 窓口も同じところで回る。**留めない**（所見 24）。 */
TEST(a_negative_height_wraps_because_randnor_can_go_below_zero) {
    player_height_set(-1);

    ASSERT_EQ_INT(player_height(), 65535);
}

/* **1..100 の留めは `get_history()` の規則で、窓口の規則ではない**
 * （create.c:278〜:283）。窓口は 200 でも 0 でもそのまま返す。 */
TEST(the_social_class_keeps_the_one_to_hundred_clamp_out_of_this_module) {
    player_social_class_set(200);

    ASSERT_EQ_INT(player_social_class(), 200);
}

/* 階層だけは**符号つき** 2 バイト（`int16_t`。他の 2 つと違う）。 */
TEST(the_social_class_is_signed_so_thirty_two_thousand_seven_hundred_sixty_eight_wraps) {
    player_social_class_set(32768);

    ASSERT_EQ_INT(player_social_class(), -32768);
}

/* ------------------------------------------------------------------
 * 生い立ち 4 行 -- 60 字ちょうどが入る（B21 のぶん）
 * ------------------------------------------------------------------ */

/* **60 字ちょうどの行が入ることがこの単位の要**。`get_history()` は 60 字で
 * 折りかえすので、**330,984 通りのうち 8,057 通りが 60 字ちょうどの行を持つ**。
 * だから置き場は 60 字 ＋ 終端で 61 バイト（`#18-12-26A` でフィールドを
 * 1 バイト広げた。59 字に切る道は選ばない —— それは 8,057 通りで最後の
 * 1 字を落とす、本当に見えるふるまいの変更になる）。 */
TEST(a_story_line_of_exactly_sixty_characters_keeps_all_sixty) {
    char line[PLAYER_HISTORY_LINE_SIZE];
    build_line(line, PLAYER_HISTORY_LINE_SIZE - 1);

    player_history_line_set(2, line);

    ASSERT_EQ_STR(player_history_line(2), line);
}

TEST(a_story_line_longer_than_sixty_is_cut_at_sixty) {
    char line[PLAYER_HISTORY_LINE_SIZE + 10];
    build_line(line, PLAYER_HISTORY_LINE_SIZE + 9);

    player_history_line_set(0, line);

    ASSERT_EQ_INT((int)strlen(player_history_line(0)),
                  PLAYER_HISTORY_LINE_SIZE - 1);
}

/* **4 行とも 60 字ちょうどでも、行が互いに食いこまない。** フィールドの
 * ころ `py.misc.history[line][60] = '\0'` は**次の行の先頭**に書いていた
 * （4 行めなら `struct misc` の外）。**到達しなかったので壊れてはいなかった**
 * が、置き場を 61 バイトにしたいまは自分の行の中に収まる。 */
TEST(sixty_character_lines_in_all_four_rows_do_not_run_into_each_other) {
    char line[PLAYER_HISTORY_LINE_SIZE];
    build_line(line, PLAYER_HISTORY_LINE_SIZE - 1);

    for (int row = 0; row < PLAYER_HISTORY_LINES; row++) {
        player_history_line_set(row, line);
    }

    for (int row = 0; row < PLAYER_HISTORY_LINES; row++) {
        ASSERT_EQ_STR(player_history_line(row), line);
    }
}

TEST(setting_one_story_line_leaves_the_other_three_alone) {
    player_history_line_set(0, "You are one of several children");
    player_history_line_set(1, "of a Serf.");

    player_history_line_set(1, "of a Landed Knight.");

    ASSERT_EQ_STR(player_history_line(0), "You are one of several children");
    ASSERT_EQ_STR(player_history_line(1), "of a Landed Knight.");
    ASSERT_EQ_STR(player_history_line(2), "");
    ASSERT_EQ_STR(player_history_line(3), "");
}

/* `get_history()`（create.c:237〜:239）は 4 行を消してから詰める。
 * **4 行いっぺんに消す窓口 1 本**で、フィールドのころの `for` 4 回まわしが
 * 1 呼びになる。 */
TEST(clearing_empties_all_four_lines) {
    for (int row = 0; row < PLAYER_HISTORY_LINES; row++) {
        player_history_line_set(row, "of a Serf.");
    }

    player_history_clear();

    for (int row = 0; row < PLAYER_HISTORY_LINES; row++) {
        ASSERT_EQ_STR(player_history_line(row), "");
    }
}

TEST(clearing_the_story_leaves_the_other_five_answers_alone) {
    given_a_character("Aragorn", true, 87, 72, 45);
    player_history_line_set(0, "of a Serf.");

    player_history_clear();

    ASSERT_EQ_STR(player_name(), "Aragorn");
    ASSERT_TRUE(player_is_male());
    ASSERT_EQ_INT(player_age(), 87);
    ASSERT_EQ_INT(player_height(), 72);
    ASSERT_EQ_INT(player_social_class(), 45);
}

/* 読みもどしも行ごと（`rd_string(m_ptr->history[i])` が局所に読んでから
 * 置く形になる。save.c:754）—— **4 行が書いた順に戻る**。 */
TEST(the_story_line_the_save_file_reads_back_is_the_one_it_wrote) {
    const char *lines_from_the_file[PLAYER_HISTORY_LINES] = {
        "You are one of several children", "of a Serf.  You are a credit to",
        "the family.", ""};

    for (int row = 0; row < PLAYER_HISTORY_LINES; row++) {
        player_history_line_set(row, lines_from_the_file[row]);
    }

    ASSERT_EQ_STR(player_history_line(2), "the family.");
}

/* ------------------------------------------------------------------
 * 呼び手の規則 -- 性別が持っている 3 つは module の外
 * ------------------------------------------------------------------ */

/* **王か女王か**（misc3.c:291 の `**KING**`/`**QUEEN**`・death.c:154 の
 * `*King*`/`*Queen*`・death.c:454 の `All Hail the Mighty King!`）——
 * **同じ分かれめが 3 か所に書いてあって、文字列は 3 通りとも違う**。
 * 窓口は真偽を返すだけで、どの語を出すかを知らない。 */
TEST(the_king_or_queen_fork_is_the_callers_rule) {
    player_set_male(false);

    const char *title = player_is_male() ? "**KING**" : "**QUEEN**";

    ASSERT_EQ_STR(title, "**QUEEN**");
}

/* **最初の持ち金**（create.c:462 の `sc * 6 + randint(25) + 325` と
 * `:467` の `!male` なら +50。"She charmed the banker into it! -CJS-"）——
 * **階層と性別はお金の計算の項**であって、この問いの答えではない。
 * 乱数を抜いた骨（`sc * 6 + 325 + 50`）を写して、窓口が 2 つの数を
 * 渡すだけだということを固定する。 */
TEST(the_starting_purse_is_the_callers_arithmetic) {
    given_a_character("Arwen", false, 2901, 66, 45);

    int gold = player_social_class() * 6 + 325;
    if (!player_is_male()) {
        gold += 50;
    }

    ASSERT_EQ_INT(gold, 45 * 6 + 325 + 50);
}

/* **どちらの身長の表を引くか**（create.c:320〜:326。男は `m_b_ht`/`m_m_ht`、
 * 女は `f_b_ht`/`f_m_ht`）—— 22 つめの体重で同じ形を呼び手に残してある。
 * 窓口は**振った結果を受けとるだけ**で、どの表から来たかを知らない。 */
TEST(which_height_table_to_roll_against_is_the_callers_choice) {
    player_set_male(false);

    /* create.c と同じ枝。randnor() の代わりに表の中央値を置く。 */
    if (player_is_male()) {
        player_height_set(72);
    } else {
        player_height_set(66);
    }

    ASSERT_EQ_INT(player_height(), 66);
}

/* 得点表の 1 字（death.c:273 の `py.misc.male ? 'M' : 'F'`）も呼び手の側。
 * **同じ分かれめの 4 度めで、ここだけ語ではなく 1 文字**。 */
TEST(the_high_score_entrys_letter_is_the_callers_too) {
    player_set_male(true);

    char sex = player_is_male() ? 'M' : 'F';

    ASSERT_EQ_INT(sex, 'M');
}

int main(void) {
    RUN_TEST(the_name_is_what_the_player_typed);
    RUN_TEST(the_sex_is_a_separate_answer_from_the_name);
    RUN_TEST(the_age_is_what_creation_rolled);
    RUN_TEST(the_height_is_a_different_number_from_the_age);
    RUN_TEST(the_age_comes_before_the_height_the_way_the_saved_file_holds_it);
    RUN_TEST(the_social_class_is_what_the_life_story_added_up_to);
    RUN_TEST(each_story_line_is_read_by_its_own_number);
    RUN_TEST(a_character_who_has_not_been_rolled_has_nothing);
    RUN_TEST(setting_one_answer_leaves_the_other_five_alone);
    RUN_TEST(setting_again_replaces_rather_than_adds);

    RUN_TEST(a_name_of_twenty_six_characters_fits);
    RUN_TEST(a_longer_name_is_cut_where_the_record_stopped);
    RUN_TEST(an_empty_name_is_what_the_prompt_asks_about);
    RUN_TEST(a_shorter_name_leaves_no_tail_of_the_longer_one);
    RUN_TEST(the_escape_path_of_the_name_prompt_reads_past_the_terminator);
    RUN_TEST(loading_a_saved_name_uses_the_very_same_window);

    RUN_TEST(the_sex_goes_back_and_forth);
    RUN_TEST(a_hand_edited_byte_of_two_is_still_male);

    RUN_TEST(the_age_is_two_bytes_wide_and_unsigned_the_way_the_field_was);
    RUN_TEST(a_negative_height_wraps_because_randnor_can_go_below_zero);
    RUN_TEST(the_social_class_keeps_the_one_to_hundred_clamp_out_of_this_module);
    RUN_TEST(the_social_class_is_signed_so_thirty_two_thousand_seven_hundred_sixty_eight_wraps);

    RUN_TEST(a_story_line_of_exactly_sixty_characters_keeps_all_sixty);
    RUN_TEST(a_story_line_longer_than_sixty_is_cut_at_sixty);
    RUN_TEST(sixty_character_lines_in_all_four_rows_do_not_run_into_each_other);
    RUN_TEST(setting_one_story_line_leaves_the_other_three_alone);
    RUN_TEST(clearing_empties_all_four_lines);
    RUN_TEST(clearing_the_story_leaves_the_other_five_answers_alone);
    RUN_TEST(the_story_line_the_save_file_reads_back_is_the_one_it_wrote);

    RUN_TEST(the_king_or_queen_fork_is_the_callers_rule);
    RUN_TEST(the_starting_purse_is_the_callers_arithmetic);
    RUN_TEST(which_height_table_to_roll_against_is_the_callers_choice);
    RUN_TEST(the_high_score_entrys_letter_is_the_callers_too);

    return TEST_SUMMARY();
}
