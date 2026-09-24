/* 覚えている呪文の置き場（src/spells_known.c）のテスト
 *
 * ここに「何を覚えていられるか」の計算は無い。数えなおしは calc_spells()
 * （misc3.c:1220）、どれを覚えるかは gain_spells()（misc3.c:1374）にあり、
 * この module が預かるのは**その結果の覚え**だけ。保護するのは「置き場と
 * してのふるまい」で、意味のある性質は 6 つある。
 *
 *   1. 3 つの印は**別の記録**であること。覚えている・一度成功した・忘れた
 *      は同じ呪文について同時に意味を持つ（忘れた呪文も「成功したことが
 *      ある」ままで、思いだしたときに経験値を二度払わない）。
 *
 *   2. 覚えた印と覚えた順は**対で動く**こと（spell_learn）。印だけ立てて
 *      順に積み忘れると、忘れる／思いだす順が狂う。積む位置は「まだ覚えて
 *      いない印（SPELL_NONE）の最初」——数えるための変数は無い。
 *
 *   3. 忘れる／思いだすは**互いの鏡**であること（spell_forget /
 *      spell_remember）。覚えた印と忘れた印が片方ずつ動くと、一覧の表示
 *      （" forgotten"）と数えなおしが食いちがう。
 *
 *   4. 忘れた呪文も**覚えた順には残る**こと。順は履歴で、いま唱えられる
 *      呪文の一覧ではない。calc_spells() が「どれを先に返すか」をここから
 *      読む。
 *
 *   5. SPELL_NONE（99）で**ビットを計算しない**こと。`1L << 99` は結果が
 *      決まらない。窓口を呼ぶ側は覚えた順をそのまま渡してくるので、印が
 *      混ざった並びでも答えが定まることが要る。
 *
 *   6. 範囲の検査をする側と**しない側**があること。番号は検査するが、
 *      セーブファイル用の生の窓口（spells_set_*_bits）は渡された 32 ビット
 *      をそのまま覚える（古い rd_long も検査していなかった）。
 *
 * 走りだしの状態は見ない。4 つの実体はまだ src/player.c にあり、ゲームの
 * 開始時に main.c:256 が覚えた順を SPELL_NONE で埋める（ステップ C で実体が
 * この module に移ったら、そのときに初期値を固定する）。
 */
/* externs.h は要らない。窓口と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "spells_known.h"

/* テストは 1 プロセスで状態を共有するので、毎回まっさらから始める。
 * 生の窓口を使うのは、ここが「置き場を空にする」唯一の入口だから。 */
#define MU_SETUP() given_nothing_known()

#include "minunit.h"

static void given_nothing_known(void) {
    spells_set_learned_bits(0);
    spells_set_worked_bits(0);
    spells_set_forgotten_bits(0);
    spell_order_forget_all();
}

/* --- 3 つの印は別の記録 -------------------------------------------------- */

TEST(a_spell_that_has_been_learned_is_known) {
    spell_learn(0);
    ASSERT_TRUE(spell_is_learned(0));
}

TEST(learning_one_spell_says_nothing_about_another) {
    spell_learn(0);
    ASSERT_TRUE(!spell_is_learned(1));
}

/* 番号 31 は 32 ビットの最上位。ここが落ちると、いちばん上の呪文
 * （魔法使いの番号 30 の隣）だけ覚えられない置き場になる。 */
TEST(the_highest_numbered_spell_fits_in_the_record) {
    spell_learn(31);
    ASSERT_TRUE(spell_is_learned(31));
}

/* 成功した印は覚えた印とは別の記録。覚えただけでは立たない。 */
TEST(a_newly_learned_spell_has_not_worked_yet) {
    spell_learn(0);
    ASSERT_TRUE(!spell_has_worked(0));
}

TEST(a_spell_that_has_worked_is_remembered_as_having_worked) {
    spell_mark_worked(0);
    ASSERT_TRUE(spell_has_worked(0));
}

/* 成功した印は忘れても残る（思いだしたときに経験値を二度払わないため）。 */
TEST(forgetting_a_spell_does_not_undo_that_it_once_worked) {
    spell_learn(0);
    spell_mark_worked(0);
    spell_forget(0);
    ASSERT_TRUE(spell_has_worked(0));
}

/* --- 覚えた印と覚えた順は対で動く --------------------------------------- */

TEST(the_first_spell_learned_stands_at_the_head_of_the_order) {
    spell_learn(7);
    ASSERT_EQ_INT(7, spell_learned_nth(0));
}

/* 積む位置は「まだ覚えていない印の最初」。数えるための変数は無い。 */
TEST(spells_are_appended_in_the_order_they_were_learned) {
    spell_learn(7);
    spell_learn(2);
    spell_learn(5);
    ASSERT_EQ_INT(2, spell_learned_nth(1));
}

TEST(the_order_ends_with_the_not_yet_learned_marker) {
    spell_learn(7);
    ASSERT_EQ_INT(SPELL_NONE, spell_learned_nth(1));
}

/* 履歴が満杯なら、覚えた印だけ立てて順への積みを落とす（配列の外に書くのが
 * 古い形だった）。1 職業に呪文は 31 個で枠は 32 なので、まともなゲームでは
 * どちらも起きないが、置き場としてはここで止まる。 */
TEST(a_full_order_drops_the_append_instead_of_running_off_the_end) {
    for (int spell = 0; spell < 32; spell++) {
        spell_learn(spell);
    }
    spell_learn(5); /* 33 個め */
    ASSERT_EQ_INT(31, spell_learned_nth(31));
}

/* --- 忘れる／思いだすは互いの鏡 ----------------------------------------- */

TEST(a_forgotten_spell_is_no_longer_known) {
    spell_learn(3);
    spell_forget(3);
    ASSERT_TRUE(!spell_is_learned(3));
}

TEST(a_forgotten_spell_is_recorded_as_forgotten) {
    spell_learn(3);
    spell_forget(3);
    ASSERT_TRUE(spell_is_forgotten(3));
}

TEST(a_remembered_spell_is_known_again) {
    spell_learn(3);
    spell_forget(3);
    spell_remember(3);
    ASSERT_TRUE(spell_is_learned(3));
}

TEST(a_remembered_spell_is_no_longer_recorded_as_forgotten) {
    spell_learn(3);
    spell_forget(3);
    spell_remember(3);
    ASSERT_TRUE(!spell_is_forgotten(3));
}

/* 忘れさせるのは 1 つだけ。まとめて落ちると、レベルが 1 つ下がっただけで
 * 全部忘れる置き場になる。 */
TEST(forgetting_one_spell_leaves_the_others_known) {
    spell_learn(3);
    spell_learn(4);
    spell_forget(3);
    ASSERT_TRUE(spell_is_learned(4));
}

/* --- 忘れた呪文も覚えた順には残る --------------------------------------- */

/* 履歴なので消えない。これが消えると calc_spells() は「どれを先に返すか」を
 * 知る手だてを失う。 */
TEST(a_forgotten_spell_keeps_its_place_in_the_order) {
    spell_learn(7);
    spell_forget(7);
    ASSERT_EQ_INT(7, spell_learned_nth(0));
}

/* 思いだしても履歴は動かない（覚えなおしではなく、同じ 1 回の学びの続き）。 */
TEST(remembering_a_spell_does_not_append_it_again) {
    spell_learn(7);
    spell_forget(7);
    spell_remember(7);
    ASSERT_EQ_INT(SPELL_NONE, spell_learned_nth(1));
}

/* --- SPELL_NONE ではビットを計算しない ---------------------------------- */

/* 覚えた順をそのまま渡されたときに答えが定まること。99 は「まだ」なので、
 * 覚えてはいない。丸めて `1L << (99 & 31)` にすると呪文 3 の答えを返す。 */
TEST(the_not_yet_learned_marker_names_no_spell_that_is_known) {
    spell_learn(3); /* 99 & 31 == 3 */
    ASSERT_TRUE(!spell_is_learned(SPELL_NONE));
}

TEST(the_not_yet_learned_marker_names_no_spell_that_is_forgotten) {
    spell_learn(3);
    spell_forget(3);
    ASSERT_TRUE(!spell_is_forgotten(SPELL_NONE));
}

TEST(the_not_yet_learned_marker_names_no_spell_that_has_worked) {
    spell_mark_worked(3);
    ASSERT_TRUE(!spell_has_worked(SPELL_NONE));
}

/* 履歴の外を訊かれたら「まだ」と答える。忘れる走査は 31 から下りてくるので、
 * 印と実体が食いちがったときにだけ負の番号が来る（古い形は配列の手前を
 * 読んでいた）。 */
TEST(asking_past_the_end_of_the_order_answers_not_yet) {
    spell_learn(7);
    ASSERT_EQ_INT(SPELL_NONE, spell_learned_nth(32));
}

TEST(asking_before_the_start_of_the_order_answers_not_yet) {
    spell_learn(7);
    ASSERT_EQ_INT(SPELL_NONE, spell_learned_nth(-1));
}

/* --- 数と有無 ----------------------------------------------------------- */

TEST(a_character_who_knows_nothing_has_learned_no_spell) {
    ASSERT_TRUE(!any_spell_learned());
}

TEST(a_character_who_knows_one_spell_has_learned_a_spell) {
    spell_learn(0);
    ASSERT_TRUE(any_spell_learned());
}

/* 最後の 1 つを忘れたら「1 つも覚えていない」に戻る（魔力がここで消える）。 */
TEST(forgetting_the_last_spell_leaves_nothing_learned) {
    spell_learn(0);
    spell_forget(0);
    ASSERT_TRUE(!any_spell_learned());
}

TEST(nothing_is_forgotten_before_anything_is_forgotten) {
    spell_learn(0);
    ASSERT_TRUE(!any_spell_forgotten());
}

TEST(a_forgotten_spell_makes_something_to_remember) {
    spell_learn(0);
    spell_forget(0);
    ASSERT_TRUE(any_spell_forgotten());
}

TEST(nothing_learned_counts_as_zero) {
    ASSERT_EQ_INT(0, learned_spell_count());
}

/* 数えるのは覚えている呪文だけ。忘れたぶんは入らない（覚えていられる数と
 * 比べるための数なので）。 */
TEST(the_count_leaves_out_the_spells_that_have_been_forgotten) {
    spell_learn(0);
    spell_learn(1);
    spell_learn(2);
    spell_forget(1);
    ASSERT_EQ_INT(2, learned_spell_count());
}

/* いちばん上のビットも数える（31 個覚えた魔法使いが 30 個と数えられない）。 */
TEST(the_count_includes_the_highest_numbered_spell) {
    spell_learn(31);
    ASSERT_EQ_INT(1, learned_spell_count());
}

/* --- 呪文の組との照らしあわせ ------------------------------------------- */

/* 魔法書に載っている呪文の組（inven_type.flags）のうち唱えられるぶん。 */
TEST(only_the_known_spells_of_a_book_can_be_cast) {
    spell_learn(0);
    spell_learn(2);
    ASSERT_EQ_INT(0x5, (int)spells_learned_among(0xF));
}

/* 本に載っていない呪文は、覚えていても混ざらない。 */
TEST(a_known_spell_outside_the_book_is_not_among_its_spells) {
    spell_learn(4);
    ASSERT_EQ_INT(0, (int)spells_learned_among(0xF));
}

/* 学ぶときの候補は、載っているうち**まだ覚えていない**ぶん。 */
TEST(the_candidates_to_learn_are_the_spells_not_yet_known) {
    spell_learn(0);
    ASSERT_EQ_INT(0xE, (int)spells_not_learned_among(0xF));
}

/* 忘れた呪文は候補に**戻る**（覚えた印が降りているので、覚えていない呪文と
 * 同じに見える）。ゲームでそうならないのは、忘れた呪文をレベルの余裕で先に
 * 思いださせるのが calc_spells の仕事だから——置き場の側の性質ではない。 */
TEST(a_forgotten_spell_is_a_candidate_to_learn_again) {
    spell_learn(0);
    spell_forget(0);
    ASSERT_EQ_INT(0xF, (int)spells_not_learned_among(0xF));
}

/* --- セーブファイル用の生の窓口 ----------------------------------------- */

/* 3 つのビット列は、書いたとおりに読みだせる（セーブして読みこんだ
 * キャラクタが同じ呪文を覚えているために要る）。 */
TEST(the_learned_bits_are_written_and_read_back_unchanged) {
    spells_set_learned_bits(0xDEADBEEF);
    ASSERT_EQ_INT(0xDEADBEEF, (long)spells_learned_bits());
}

TEST(the_worked_bits_are_written_and_read_back_unchanged) {
    spells_set_worked_bits(0xDEADBEEF);
    ASSERT_EQ_INT(0xDEADBEEF, (long)spells_worked_bits());
}

TEST(the_forgotten_bits_are_written_and_read_back_unchanged) {
    spells_set_forgotten_bits(0xDEADBEEF);
    ASSERT_EQ_INT(0xDEADBEEF, (long)spells_forgotten_bits());
}

/* 3 つは別の記録なので、1 つ読みこんでも他は動かない
 * （セーブファイルは 3 語を続けて書く）。 */
TEST(reading_the_learned_bits_leaves_the_worked_bits_alone) {
    spells_set_worked_bits(0x1);
    spells_set_learned_bits(0xDEADBEEF);
    ASSERT_EQ_INT(0x1, (long)spells_worked_bits());
}

/* 生の窓口は番号ごとの答えと同じ置き場を見る（読みこんだ直後に
 * 「唱えられるか」を訊かれる）。 */
TEST(spells_read_in_from_a_save_file_are_known_by_number) {
    spells_set_learned_bits((uint32_t)(1L << 5));
    ASSERT_TRUE(spell_is_learned(5));
}

/* 覚えた順は 32 バイトのまま渡す（wr_bytes / rd_bytes がそれを取る）。
 * 渡した先が本物の置き場であること——別の配列の複製だと、読みこんだ
 * 覚えた順が誰にも見えない。 */
TEST(the_order_bytes_are_the_order_itself) {
    spell_order_bytes()[0] = 5;
    ASSERT_EQ_INT(5, spell_learned_nth(0));
}

/* --- 履歴を空にする ----------------------------------------------------- */

TEST(forgetting_the_whole_order_leaves_nothing_learned_anywhere_in_it) {
    spell_learn(7);
    spell_learn(2);
    spell_order_forget_all();
    ASSERT_EQ_INT(SPELL_NONE, spell_learned_nth(0));
}

/* 端まで埋める（main.c:256 が開始時に呼ぶ。1 つでも残ると、新しい
 * キャラクタが前のキャラクタの履歴を引きつぐ）。 */
TEST(forgetting_the_whole_order_reaches_the_last_place) {
    spell_order_bytes()[31] = 5;
    spell_order_forget_all();
    ASSERT_EQ_INT(SPELL_NONE, spell_learned_nth(31));
}

/* 消すのは履歴だけ。覚えた印は別の記録（main.c は 3 つのビット列を
 * それぞれ 0 にする）。 */
TEST(forgetting_the_whole_order_does_not_touch_what_is_known) {
    spell_learn(7);
    spell_order_forget_all();
    ASSERT_TRUE(spell_is_learned(7));
}

int main(void) {
    RUN_TEST(a_spell_that_has_been_learned_is_known);
    RUN_TEST(learning_one_spell_says_nothing_about_another);
    RUN_TEST(the_highest_numbered_spell_fits_in_the_record);
    RUN_TEST(a_newly_learned_spell_has_not_worked_yet);
    RUN_TEST(a_spell_that_has_worked_is_remembered_as_having_worked);
    RUN_TEST(forgetting_a_spell_does_not_undo_that_it_once_worked);

    RUN_TEST(the_first_spell_learned_stands_at_the_head_of_the_order);
    RUN_TEST(spells_are_appended_in_the_order_they_were_learned);
    RUN_TEST(the_order_ends_with_the_not_yet_learned_marker);
    RUN_TEST(a_full_order_drops_the_append_instead_of_running_off_the_end);

    RUN_TEST(a_forgotten_spell_is_no_longer_known);
    RUN_TEST(a_forgotten_spell_is_recorded_as_forgotten);
    RUN_TEST(a_remembered_spell_is_known_again);
    RUN_TEST(a_remembered_spell_is_no_longer_recorded_as_forgotten);
    RUN_TEST(forgetting_one_spell_leaves_the_others_known);

    RUN_TEST(a_forgotten_spell_keeps_its_place_in_the_order);
    RUN_TEST(remembering_a_spell_does_not_append_it_again);

    RUN_TEST(the_not_yet_learned_marker_names_no_spell_that_is_known);
    RUN_TEST(the_not_yet_learned_marker_names_no_spell_that_is_forgotten);
    RUN_TEST(the_not_yet_learned_marker_names_no_spell_that_has_worked);
    RUN_TEST(asking_past_the_end_of_the_order_answers_not_yet);
    RUN_TEST(asking_before_the_start_of_the_order_answers_not_yet);

    RUN_TEST(a_character_who_knows_nothing_has_learned_no_spell);
    RUN_TEST(a_character_who_knows_one_spell_has_learned_a_spell);
    RUN_TEST(forgetting_the_last_spell_leaves_nothing_learned);
    RUN_TEST(nothing_is_forgotten_before_anything_is_forgotten);
    RUN_TEST(a_forgotten_spell_makes_something_to_remember);
    RUN_TEST(nothing_learned_counts_as_zero);
    RUN_TEST(the_count_leaves_out_the_spells_that_have_been_forgotten);
    RUN_TEST(the_count_includes_the_highest_numbered_spell);

    RUN_TEST(only_the_known_spells_of_a_book_can_be_cast);
    RUN_TEST(a_known_spell_outside_the_book_is_not_among_its_spells);
    RUN_TEST(the_candidates_to_learn_are_the_spells_not_yet_known);
    RUN_TEST(a_forgotten_spell_is_a_candidate_to_learn_again);

    RUN_TEST(the_learned_bits_are_written_and_read_back_unchanged);
    RUN_TEST(the_worked_bits_are_written_and_read_back_unchanged);
    RUN_TEST(the_forgotten_bits_are_written_and_read_back_unchanged);
    RUN_TEST(reading_the_learned_bits_leaves_the_worked_bits_alone);
    RUN_TEST(spells_read_in_from_a_save_file_are_known_by_number);
    RUN_TEST(the_order_bytes_are_the_order_itself);

    RUN_TEST(forgetting_the_whole_order_leaves_nothing_learned_anywhere_in_it);
    RUN_TEST(forgetting_the_whole_order_reaches_the_last_place);
    RUN_TEST(forgetting_the_whole_order_does_not_touch_what_is_known);

    return TEST_SUMMARY();
}
