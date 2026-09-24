/* 呪文を覚えるとき（misc3.c:1374 の gain_spells）のテスト
 *
 * プレイヤーが学ぶ（`G` コマンド、dungeon.c:1294）と呼ばれる。#18-8 で窓口
 * `src/spells_known.h` に預けた 4 個のうち、**覚えた印（`spell_learned`）と
 * 覚えた順（`spell_order`）に書き足すのはここだけ**なのに、単体テストが
 * 1 件も届いていなかった。窓口を通す書きかえ（ステップ B）の前に押さえた。
 * いまは読み書きとも窓口越し（#18-8-B2）。
 *
 * 戻り値は無く、結果は
 *
 *   ・覚えた印と覚えた順（`spell_learned` と `spell_order`）
 *   ・メッセージ（msg_print）
 *   ・`py.flags.new_spells`（まだ学べる数）と PY_STUDY
 *   ・魔力（`py.misc.mana`。最初の 1 つを覚えたときだけ計算される）
 *   ・断ったときの `free_turn_flag`（手番を使わない）
 *
 * にしか現れない。
 *
 * 保護したい仕掛けは 5 つ。
 *
 *   1. **覚えた印と覚えた順は必ず対で動く。** 印だけ立てて順に積み忘れると、
 *      あとで忘れる／思いだすときの順序が狂う（calc_spells が読む並び）。
 *      積む位置は「99 でない要素の次」——**覚えた数を数える別の変数は無い**。
 *
 *   2. **MAGE 系は本が要る。** 持っている魔法書の flags の論理和だけが候補で、
 *      足りなければ「本が見つからない」と言い、**学べる数を減らさない**
 *      （本を買ってから学べるように、差は py.flags.new_spells に残す）。
 *
 *   3. **PRIEST 系は本が要らず、選べもしない。** 候補から randint で 1 つ
 *      引いて、覚えたことを告げる（MAGE 系は画面に出すだけで何も言わない）。
 *
 *   4. **学べる数が 0 なら断る。** そのとき手番を使わない（free_turn_flag）。
 *
 *   5. **最初の 1 つで魔力が生える。** py.misc.mana が 0 のときだけ
 *      calc_mana を呼ぶ（レベル 1 のキャラクタが呪文を覚えた瞬間）。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、いまどう
 * 振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"
#include "inventory.h"
#include "spells_known.h"

extern player_type py;
extern bool free_turn_flag;

/* 検証対象（src/misc3.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
void gain_spells(void);

#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* ------------------------------------------------------------------
 * 条件づくりの補助関数
 * ------------------------------------------------------------------ */

/* fixture_reset() は py を 0 で埋めるが、呪文の 4 個は消さない
 * （置き場は src/spells_known.c で、代役の管轄ではない）。覚えた順を
 * SPELL_NONE で埋めるのは main.c:256 がゲーム開始時にするのと同じこと。
 * free_turn_flag も代役側の器なので自分で戻す。 */
static void given_no_spells_known(void) {
    spells_set_learned_bits(0);
    spells_set_worked_bits(0);
    spells_set_forgotten_bits(0);
    spell_order_forget_all();
    free_turn_flag = false;
}

/* 学べる数がある魔法使い（レベル 1・知力 18）。魔力は 1 にしておく
 * （0 だと最初の 1 つで calc_mana が走るので、その仕掛けは別の件で見る）。 */
static void given_a_mage_who_can_learn(int spells_to_learn) {
    py.misc.pclass = 1; /* class[1] は Mage（MAGE 系） */
    py.misc.lev = 1;
    py.stats.use_stat[A_INT] = 18;
    py.misc.mana = 1;
    py.flags.new_spells = (uint8_t)spells_to_learn;
}

/* 学べる数がある僧侶（レベル 1・賢さ 18）。 */
static void given_a_priest_who_can_learn(int spells_to_learn) {
    py.misc.pclass = 2; /* class[2] は Priest（PRIEST 系） */
    py.misc.lev = 1;
    py.stats.use_stat[A_WIS] = 18;
    py.misc.mana = 1;
    py.flags.new_spells = (uint8_t)spells_to_learn;
}

/* 持ち物に魔法書 1 冊。flags のビットが「この本に載っている呪文」。 */
/* すでに覚えている呪文を 1 つ足す（覚えた順の末尾に積むところまで窓口が
 * やる。gain_spells が「印だけ立てて順に積み忘れない」ことの前提になる）。 */
static void given_a_learned_spell(int spell) { spell_learn(spell); }

static void given_a_spell_book_containing(uint32_t spells) {
    inven_type *i_ptr = inventory_at(0);
    i_ptr->tval = TV_MAGIC_BOOK;
    i_ptr->flags = spells;
    i_ptr->number = 1;
    inventory_set_count(1);
}

static bool the_spell_is_learned(int spell) { return spell_is_learned(spell); }

/* 覚えた順の n 番目。SPELL_NONE なら「まだ」。 */
static int the_nth_spell_learned(int n) { return spell_learned_nth(n); }

/* ------------------------------------------------------------------
 * 断るとき
 * ------------------------------------------------------------------ */

/* 学べる数が 0 なら断り、手番を使わない。 */
TEST(a_character_with_nothing_left_to_learn_is_refused) {
    given_no_spells_known();
    given_a_mage_who_can_learn(0);

    gain_spells();

    ASSERT_EQ_STR("You can't learn any new spells!", fixture_message_text(0));
    ASSERT_TRUE(free_turn_flag);
    ASSERT_TRUE(!any_spell_learned());
}

/* 僧侶には "prayer" で断る。 */
TEST(a_priest_with_nothing_left_to_learn_is_refused_in_their_own_words) {
    given_no_spells_known();
    given_a_priest_who_can_learn(0);

    gain_spells();

    ASSERT_EQ_STR("You can't learn any new prayers!", fixture_message_text(0));
    ASSERT_TRUE(free_turn_flag);
}

/* 混乱していると学べない（職業を問わず、いちばん先に見る）。 */
TEST(a_confused_character_cannot_learn) {
    given_no_spells_known();
    given_a_priest_who_can_learn(1);
    py.flags.confused = 1;

    gain_spells();

    ASSERT_EQ_STR("You are too confused.", fixture_message_text(0));
    ASSERT_TRUE(!any_spell_learned());
}

/* MAGE 系は本を読むので、盲だと学べない。**僧侶は神から授かるので盲でも
 * 学べる**（この違いがあるので、判定は職業を見たあとに置かれている）。 */
TEST(a_blind_mage_cannot_read_their_book) {
    given_no_spells_known();
    given_a_mage_who_can_learn(1);
    py.flags.blind = 1;

    gain_spells();

    ASSERT_EQ_STR("You can't see to read your spell book!",
                  fixture_message_text(0));
    ASSERT_TRUE(!any_spell_learned());
}

/* ------------------------------------------------------------------
 * 僧侶（授かる側）
 *
 * 候補はレベル内の祈り全部（本は要らない）。レベル 1 の僧侶なら番号 0〜3 が
 * 候補で、randint を 1 に固定すると先頭（0 = Detect Evil）が引かれる。
 * ------------------------------------------------------------------ */

/* 1 つ授かる。印が立ち、覚えた順の先頭に積まれ、告げられる。 */
TEST(a_priest_is_granted_a_prayer_at_random) {
    given_no_spells_known();
    given_a_priest_who_can_learn(1);
    fixture_set_randint(1); /* 候補の先頭を引く */

    gain_spells();

    ASSERT_TRUE(the_spell_is_learned(0));
    ASSERT_EQ_INT(0, the_nth_spell_learned(0));
    ASSERT_EQ_STR("You have learned the prayer of Detect Evil.",
                  fixture_message_text(0));
    ASSERT_EQ_INT(0, (int)py.flags.new_spells);
    ASSERT_TRUE((py.flags.status & PY_STUDY) != 0);
}

/* 2 つ授かると、覚えた順に**積み重なる**。1 つめを候補から外すので、
 * 同じ乱数でも 2 つめは次の祈り（1 = Cure Light Wounds）になる。 */
TEST(two_prayers_are_appended_in_the_order_they_were_granted) {
    given_no_spells_known();
    given_a_priest_who_can_learn(2);
    fixture_set_randint(1);

    gain_spells();

    ASSERT_TRUE(the_spell_is_learned(0));
    ASSERT_TRUE(the_spell_is_learned(1));
    ASSERT_EQ_INT(0, the_nth_spell_learned(0));
    ASSERT_EQ_INT(1, the_nth_spell_learned(1));
    ASSERT_EQ_INT(SPELL_NONE, the_nth_spell_learned(2));
}

/* 魔力が 0 のときだけ、覚えたあとに魔力を計算する（レベル 1 の 1 つめ）。 */
TEST(the_first_prayer_brings_the_mana_with_it) {
    given_no_spells_known();
    given_a_priest_who_can_learn(1);
    py.misc.mana = 0;
    fixture_set_randint(1);

    gain_spells();

    ASSERT_TRUE(py.misc.mana > 0);
}

/* ------------------------------------------------------------------
 * 魔法使い（選ぶ側）
 *
 * 候補は持っている魔法書に載っている呪文だけ。どれを覚えるかはキーで選ぶ
 * （代役の get_com にテストから並べたキーを返させる）。
 * ------------------------------------------------------------------ */

/* 本に載っている呪文を 1 つ選んで覚える。印と覚えた順が対で動き、
 * **何も言わない**（画面の一覧に出るだけ）。 */
TEST(a_mage_learns_the_spell_they_choose_from_their_book) {
    given_no_spells_known();
    given_a_mage_who_can_learn(1);
    given_a_spell_book_containing(0xF); /* 番号 0〜3 が載っている */
    fixture_set_get_com_keys("a");      /* 一覧の 1 番目 */

    gain_spells();

    ASSERT_TRUE(the_spell_is_learned(0));
    ASSERT_EQ_INT(0, the_nth_spell_learned(0));
    ASSERT_EQ_INT(0, fixture_message_count());
    ASSERT_EQ_INT(0, (int)py.flags.new_spells);
}

/* 覚えた順は**すでに覚えた分の次**に積む（数えるための別の変数は無く、
 * 99 を探して位置を決める）。番号 5 を覚えている魔法使いが 1 つ足すと、
 * 並びの 2 番目に入る。 */
TEST(a_new_spell_is_appended_after_the_ones_already_known) {
    given_no_spells_known();
    given_a_mage_who_can_learn(1);
    given_a_learned_spell(5);
    given_a_spell_book_containing(0xF);
    fixture_set_get_com_keys("a");

    gain_spells();

    ASSERT_EQ_INT(5, the_nth_spell_learned(0));
    ASSERT_EQ_INT(0, the_nth_spell_learned(1));
    ASSERT_EQ_INT(SPELL_NONE, the_nth_spell_learned(2));
}

/* すでに覚えた呪文は候補から外す。番号 0 を覚えている魔法使いが 0〜3 の載った
 * 本で学ぶと、一覧の 1 番目は番号 1 になる（外さないと、同じ呪文をもう一度
 * 覚えられて覚えた順に二重に積まれる）。 */
TEST(a_spell_already_known_is_left_out_of_the_choices) {
    given_no_spells_known();
    given_a_mage_who_can_learn(1);
    given_a_learned_spell(0);
    given_a_spell_book_containing(0xF);
    fixture_set_get_com_keys("a");

    gain_spells();

    ASSERT_TRUE(the_spell_is_learned(1));
    ASSERT_EQ_INT(0, the_nth_spell_learned(0));
    ASSERT_EQ_INT(1, the_nth_spell_learned(1));
}

/* 本に載っている数が足りなければ「本が見つからない」と言い、**差を残す**。
 * 2 つ学べるのに 1 つしか載っていない本なら、1 つ覚えたあとも
 * py.flags.new_spells は 1 のまま（本を買えば残りを学べる）。 */
TEST(a_mage_short_of_books_keeps_what_they_could_not_learn) {
    given_no_spells_known();
    given_a_mage_who_can_learn(2);
    given_a_spell_book_containing(0x1); /* 番号 0 だけ */
    fixture_set_get_com_keys("a");

    gain_spells();

    ASSERT_EQ_STR("You seem to be missing a book.", fixture_message_text(0));
    ASSERT_TRUE(the_spell_is_learned(0));
    ASSERT_EQ_INT(1, (int)py.flags.new_spells);
}

/* 本を 1 冊も持っていなければ何も覚えられない（候補が 0 個）。 */
TEST(a_mage_without_a_book_learns_nothing) {
    given_no_spells_known();
    given_a_mage_who_can_learn(1);
    fixture_set_get_com_keys("a");

    gain_spells();

    ASSERT_EQ_STR("You seem to be missing a book.", fixture_message_text(0));
    ASSERT_TRUE(!any_spell_learned());
    ASSERT_EQ_INT(1, (int)py.flags.new_spells);
}

/* 一覧にない文字を押しても何も起きない（鈴を鳴らすだけ）。
 * キーを使いきると繰りかえしが終わるので、学べる数はそのまま残る。 */
TEST(a_key_outside_the_list_learns_nothing) {
    given_no_spells_known();
    given_a_mage_who_can_learn(1);
    given_a_spell_book_containing(0xF);
    fixture_set_get_com_keys("z"); /* 候補は 4 つなので範囲外 */

    gain_spells();

    ASSERT_TRUE(!any_spell_learned());
    ASSERT_EQ_INT(1, (int)py.flags.new_spells);
}

int main(void) {
    RUN_TEST(a_character_with_nothing_left_to_learn_is_refused);
    RUN_TEST(a_priest_with_nothing_left_to_learn_is_refused_in_their_own_words);
    RUN_TEST(a_confused_character_cannot_learn);
    RUN_TEST(a_blind_mage_cannot_read_their_book);

    RUN_TEST(a_priest_is_granted_a_prayer_at_random);
    RUN_TEST(two_prayers_are_appended_in_the_order_they_were_granted);
    RUN_TEST(the_first_prayer_brings_the_mana_with_it);

    RUN_TEST(a_mage_learns_the_spell_they_choose_from_their_book);
    RUN_TEST(a_new_spell_is_appended_after_the_ones_already_known);
    RUN_TEST(a_spell_already_known_is_left_out_of_the_choices);
    RUN_TEST(a_mage_short_of_books_keeps_what_they_could_not_learn);
    RUN_TEST(a_mage_without_a_book_learns_nothing);
    RUN_TEST(a_key_outside_the_list_learns_nothing);

    return TEST_SUMMARY();
}
