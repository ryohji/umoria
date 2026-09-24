/* 覚えている呪文の出し入れ（misc3.c:1220 の calc_spells）のテスト
 *
 * calc_spells() は「いまのレベルと能力値で覚えていられる呪文の数」を数えなおし、
 * 多すぎれば忘れさせ、余裕ができれば忘れた呪文を思いださせる唯一の場所。
 * #18-8 で窓口（`src/spells_known.h`）に預けた 4 つの global
 * （`spell_learned` `spell_worked` `spell_forgotten` `spell_order`）のうち
 * 3 つを書きかえるのはここだけなので、窓口を通す書きかえ（ステップ B）の前に
 * ここで押さえた。いまは読み書きとも窓口越し（#18-8-B2）。
 *
 * 戻り値は無く、結果は
 *
 *   ・覚えた／忘れた印（`spell_learned` と `spell_forgotten`）
 *   ・メッセージ（msg_print で「忘れた」「思いだした」「学べる」）
 *   ・`py.flags.new_spells`（あと何個学べるか）と PY_STUDY
 *
 * にしか現れない。
 *
 * 保護したい仕掛けは 5 つ。
 *
 *   1. **レベルを超えた呪文は忘れる。** 上の位（31）から下りていき、レベル内の
 *      呪文に当たったところで**止まる**（呪文の番号は難度の順に並んでいるので、
 *      それより下は全部レベル内）。
 *
 *   2. **覚えていられる数は能力値の段 × レベル数。** `stat_adj()` の段
 *      （0〜7）で 0・1・1.5・2・2.5 倍に変わる。数が足りていれば何も起きない。
 *
 *   3. **思いだす順は覚えた順、忘れる順はその逆。** `spell_order` が覚えた順の
 *      並びで、思いだすときは先頭から、忘れるときは末尾（添字 31）から見る。
 *
 *   4. **覚えた順に残る SPELL_NONE（99）は「まだ覚えていない」印。** 99 のまま
 *      `1L << 99` を計算すると結果が決まらない。#18-8-B2 からはその判断が
 *      窓口の内側にあり、ここは「印を渡しても答えが定まる」ことだけを見る。
 *
 *   5. **職業で言いかたが変わる。** MAGE 系は "spell"、PRIEST 系は "prayer"。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、いまどう
 * 振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"
#include "spells_known.h"

extern player_type py;

/* 検証対象（src/misc3.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
void calc_spells(int stat);

#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* ------------------------------------------------------------------
 * 条件づくりの補助関数
 *
 * 呪文の段（`magic_spell`、player.c:315）は本物をリンクする。MAGE の段は
 *   番号 0〜3 が レベル 1、4〜7 が 3、8〜11 が 5、12〜14 が 7、15〜18 が 9、
 *   19〜20 が 11、21 が 13、22 が 15、23 が 17 …… 30 が 37。
 * PRIEST の段は 0〜3 が 1、25 が 15。**番号が上がるほど高いレベルを要る**
 * という並びが、仕掛け 1 の「当たったら止まる」の前提になっている。
 *
 * 覚えていられる数 = stat_adj(能力値) の段 × (レベル − 職業の初級レベル + 1)。
 * MAGE も PRIEST も初級レベルは 1 なので、レベル数はそのままレベルになる。
 * `stat_adj` の段は 0〜7 が 0、8〜14 が 1、15〜17 が 2、18〜67 が 3、
 * 68〜87 が 4（ここから 1.5 倍）、88〜107 が 5、108〜117 が 6、118 以上が 7。
 * ------------------------------------------------------------------ */

/* fixture_reset() は py を 0 で埋めるが、呪文の 4 個は消さない
 * （置き場は src/spells_known.c で、代役の管轄ではない）。どのテストも同じ
 * 地点から始まるように、ここで 4 個とも自分で初期化する。覚えた順を
 * SPELL_NONE で埋めるのは main.c:256 がゲーム開始時にするのと同じこと。 */
static void given_no_spells_known(void) {
    spells_set_learned_bits(0);
    spells_set_worked_bits(0);
    spells_set_forgotten_bits(0);
    spell_order_forget_all();
}

/* 知力 18（段 3 = 1 倍）の魔法使い。レベル n なら n 個まで覚えていられる。 */
static void given_a_mage_of_level(int level) {
    py.misc.pclass = 1; /* class[1] は Mage（MAGE 系・初級レベル 1） */
    py.misc.lev = (uint16_t)level;
    py.stats.use_stat[A_INT] = 18;
}

/* 賢さ 18（段 3 = 1 倍）の僧侶。言いかたが "prayer" になる。 */
static void given_a_priest_of_level(int level) {
    py.misc.pclass = 2; /* class[2] は Priest（PRIEST 系・初級レベル 1） */
    py.misc.lev = (uint16_t)level;
    py.stats.use_stat[A_WIS] = 18;
}

/* 覚えている呪文を 1 つ足す（覚えた順の末尾に積むところまで窓口がやる）。 */
static void given_a_learned_spell(int spell) { spell_learn(spell); }

/* 忘れている呪文を 1 つ足す。**一度覚えてから忘れる**のがゲームでの唯一の
 * 道すじで、忘れた呪文も覚えた順には残る（だから思いだすときに順番が分かる）。 */
static void given_a_forgotten_spell(int spell) {
    spell_learn(spell);
    spell_forget(spell);
}

/* 覚えている印が立っているか。 */
static bool the_spell_is_learned(int spell) { return spell_is_learned(spell); }

/* 忘れた印が立っているか。 */
static bool the_spell_is_forgotten(int spell) {
    return spell_is_forgotten(spell);
}

/* ------------------------------------------------------------------
 * 数が足りているとき
 * ------------------------------------------------------------------ */

/* レベル 5・知力 18 なら 5 個まで。1 個しか覚えていなければ何も忘れない。
 * 余裕の 4 個は「学べる数」として py.flags.new_spells に入り、
 * 「学べるようになった」と告げられる。 */
TEST(a_character_with_room_to_spare_is_told_they_can_learn_more) {
    given_no_spells_known();
    given_a_mage_of_level(5);
    given_a_learned_spell(0); /* Magic Missile（レベル 1） */

    calc_spells(A_INT);

    ASSERT_TRUE(the_spell_is_learned(0));
    ASSERT_EQ_INT(4, (int)py.flags.new_spells);
    ASSERT_EQ_INT(1, fixture_message_count());
    ASSERT_EQ_STR("You can learn some new spells now.", fixture_message_text(0));
    ASSERT_TRUE((py.flags.status & PY_STUDY) != 0);
}

/* 覚えていられる数ぴったりなら、忘れも学びも起きない。 */
TEST(a_character_at_the_limit_neither_learns_nor_forgets) {
    given_no_spells_known();
    given_a_mage_of_level(2);
    given_a_learned_spell(0);
    given_a_learned_spell(1);

    calc_spells(A_INT);

    ASSERT_TRUE(the_spell_is_learned(0));
    ASSERT_TRUE(the_spell_is_learned(1));
    ASSERT_EQ_INT(0, (int)py.flags.new_spells);
    ASSERT_EQ_INT(0, fixture_message_count());
}

/* 能力値の段が 0（知力 7 以下）なら 1 つも覚えていられない。 */
TEST(a_character_without_the_wits_for_it_may_keep_no_spells) {
    given_no_spells_known();
    given_a_mage_of_level(5);
    py.stats.use_stat[A_INT] = 7; /* 段 0 */
    given_a_learned_spell(0);

    calc_spells(A_INT);

    ASSERT_TRUE(!the_spell_is_learned(0));
    ASSERT_TRUE(the_spell_is_forgotten(0));
    ASSERT_EQ_INT(0, (int)py.flags.new_spells);
}

/* 段が 4 以上だと 1.5 倍。レベル 4・知力 68 なら 6 個まで。 */
TEST(a_high_stat_allows_one_and_a_half_spells_per_level) {
    given_no_spells_known();
    given_a_mage_of_level(4);
    py.stats.use_stat[A_INT] = 68; /* 段 4 → 3 * 4 / 2 = 6 個 */

    calc_spells(A_INT);

    ASSERT_EQ_INT(6, (int)py.flags.new_spells);
}

/* ------------------------------------------------------------------
 * レベルを超えた呪文を落とす（上の位から下りて、当たったら止まる）
 * ------------------------------------------------------------------ */

/* レベル 1 の魔法使いが Fire Bolt（番号 22・レベル 15）を覚えていたら忘れる。
 * 忘れた印が立ち、覚えた印が降りる（**2 つは対で動く**）。 */
TEST(a_spell_above_the_characters_level_is_forgotten) {
    given_no_spells_known();
    given_a_mage_of_level(1);
    given_a_learned_spell(22);

    calc_spells(A_INT);

    ASSERT_TRUE(!the_spell_is_learned(22));
    ASSERT_TRUE(the_spell_is_forgotten(22));
    ASSERT_EQ_STR("You have forgotten the spell of Fire Bolt.",
                  fixture_message_text(0));
}

/* 上の位から下りる走査は、レベル内の呪文に**当たったところで止まる**。
 * 番号 22（レベル 15）と番号 0（レベル 1）を覚えているレベル 1 の魔法使いでは、
 * 22 を落として 0 で止まる。呪文の番号が難度の順に並んでいることが前提。 */
TEST(the_sweep_from_the_top_stops_at_the_first_spell_within_reach) {
    given_no_spells_known();
    given_a_mage_of_level(1);
    given_a_learned_spell(0);
    given_a_learned_spell(22);

    calc_spells(A_INT);

    ASSERT_TRUE(!the_spell_is_learned(22));
    ASSERT_TRUE(the_spell_is_learned(0));
}

/* 覚えていられる数を超えた分は、**覚えた順の逆から**忘れる。
 * 0 → 1 の順に覚えたレベル 1 の魔法使い（1 個まで）は、後に覚えた 1 を忘れる。 */
TEST(the_spell_learned_last_is_the_first_one_forgotten) {
    given_no_spells_known();
    given_a_mage_of_level(1);
    given_a_learned_spell(0);
    given_a_learned_spell(1);

    calc_spells(A_INT);

    ASSERT_TRUE(the_spell_is_learned(0));
    ASSERT_TRUE(!the_spell_is_learned(1));
    ASSERT_TRUE(the_spell_is_forgotten(1));
    ASSERT_EQ_STR("You have forgotten the spell of Detect Monsters.",
                  fixture_message_text(0));
}

/* ------------------------------------------------------------------
 * 忘れた呪文を思いだす（覚えた順に、前から）
 * ------------------------------------------------------------------ */

/* 余裕ができたら、忘れた呪文が戻る。忘れた印が降りて覚えた印が立つ。 */
TEST(a_forgotten_spell_comes_back_when_there_is_room) {
    given_no_spells_known();
    given_a_mage_of_level(1);
    given_a_forgotten_spell(0);

    calc_spells(A_INT);

    ASSERT_TRUE(the_spell_is_learned(0));
    ASSERT_TRUE(!the_spell_is_forgotten(0));
    ASSERT_EQ_STR("You have remembered the spell of Magic Missile.",
                  fixture_message_text(0));
}

/* 思いだす順は**覚えた順**。0 → 1 の順に覚えて両方忘れている 1 個ぶんの
 * 余裕しかない魔法使いは、先に覚えた 0 を思いだす。 */
TEST(the_spell_learned_first_is_the_first_one_remembered) {
    given_no_spells_known();
    given_a_mage_of_level(1);
    given_a_forgotten_spell(0);
    given_a_forgotten_spell(1);

    calc_spells(A_INT);

    ASSERT_TRUE(the_spell_is_learned(0));
    ASSERT_TRUE(!the_spell_is_learned(1));
    ASSERT_TRUE(the_spell_is_forgotten(1));
}

/* レベルが足りない呪文は思いだせない。**そのぶん枠を 1 つ足して**
 * 次に覚えた呪文を見にいく（さもないと高い呪文 1 つで列が詰まる）。 */
TEST(a_forgotten_spell_still_out_of_reach_does_not_block_the_next_one) {
    given_no_spells_known();
    given_a_mage_of_level(1);
    given_a_forgotten_spell(22); /* レベル 15。まだ思いだせない */
    given_a_forgotten_spell(0);  /* レベル 1。こちらが戻る */

    calc_spells(A_INT);

    ASSERT_TRUE(!the_spell_is_learned(22));
    ASSERT_TRUE(the_spell_is_learned(0));
}

/* 覚えた順の並びに残る 99（まだ覚えていない印）では遮蔽を 0 にする。
 * 99 のまま `1L << 99` を計算すると結果が決まらないため。**ここが崩れると
 * 未定義動作**なので、印が残っている並びでも落ちないことを見る。
 *
 * ただし読み飛ばすわけではない —— 走査は「覚えていられる数」の回数しか
 * 回らないので、**99 が 1 つあると 1 つぶん枠を食う**。レベル 1（枠 1）で
 * 先頭が 99 なら、その次にある忘れた呪文までは届かない。 */
TEST(the_not_yet_learned_marker_uses_up_one_turn_of_the_scan) {
    given_no_spells_known();
    given_a_mage_of_level(1);
    spells_set_forgotten_bits((uint32_t)(1L << 0));
    spell_order_bytes()[1] = 0;

    calc_spells(A_INT);

    ASSERT_TRUE(!the_spell_is_learned(0));
    ASSERT_TRUE(the_spell_is_forgotten(0));
}

/* 99 は**どの呪文の代わりにもならない**。遮蔽を 0 にせず `1L << (99 & 31)`
 * のように丸めると、99 が呪文 3（Light Area）を指してしまう。忘れているのが
 * 呪文 3 だけで、覚えた順の並びが 99 のままなら、何も思いだしてはいけない。
 *
 * レベルを 7 にしてあるのは、この違いが見えるようにするため。丸めた添字
 * （99）で段の表を引くと表の外の値（Ranger の 7 番目・段 7）を拾うので、
 * レベル 7 以上でなければ「まだ届かない」側に落ちて差が出ない。 */
TEST(the_marker_does_not_stand_in_for_the_spell_its_low_bits_name) {
    given_no_spells_known();
    given_a_mage_of_level(7);
    spells_set_forgotten_bits((uint32_t)(1L << 3)); /* 99 & 31 == 3 */

    calc_spells(A_INT);

    ASSERT_TRUE(!the_spell_is_learned(3));
    ASSERT_TRUE(the_spell_is_forgotten(3));
    /* 出るのは枠の知らせだけ。「思いだした」は 1 件も出ない。 */
    ASSERT_EQ_STR("You can learn some new spells now.", fixture_message_text(0));
    ASSERT_EQ_INT(1, fixture_message_count());
}

/* 枠が 2 つ（レベル 2）あれば、99 の次にある忘れた呪文まで届く。
 * 99 そのものでは何も起きない（遮蔽が 0 なので、どの呪文にも当たらない）。 */
TEST(a_forgotten_spell_after_the_marker_is_reached_when_the_scan_is_longer) {
    given_no_spells_known();
    given_a_mage_of_level(2);
    spells_set_forgotten_bits((uint32_t)(1L << 0));
    spell_order_bytes()[1] = 0;

    calc_spells(A_INT);

    ASSERT_TRUE(the_spell_is_learned(0));
    ASSERT_TRUE(!the_spell_is_forgotten(0));
}

/* 学べる数は「いま手の届く呪文の数」で打ち切られる。レベル 2・知力 118
 * （段 7 = 2.5 倍）なら枠は 5 つだが、レベル 2 までの呪文は 4 つしかない
 * ので 4 で止まる（さもないと「学べる」と言われつづける）。 */
TEST(the_number_to_learn_is_capped_by_the_spells_within_reach) {
    given_no_spells_known();
    given_a_mage_of_level(2);
    py.stats.use_stat[A_INT] = 118; /* 段 7 → 5 * 2 / 2 = 5 枠 */

    calc_spells(A_INT);

    ASSERT_EQ_INT(4, (int)py.flags.new_spells);
}

/* 学べる数が前と同じなら黙っている（PY_STUDY も立てない）。
 * calc_spells はレベルが上がるたびに呼ばれるので、ここで毎回知らせると
 * 1 歩ごとに「学べる」と言うことになる。 */
TEST(nothing_is_said_when_the_number_to_learn_has_not_changed) {
    given_no_spells_known();
    given_a_mage_of_level(1);
    py.flags.new_spells = 1; /* すでに 1 つ学べると知っている */

    calc_spells(A_INT);

    ASSERT_EQ_INT(1, (int)py.flags.new_spells);
    ASSERT_EQ_INT(0, fixture_message_count());
    ASSERT_EQ_INT(0, (int)(py.flags.status & PY_STUDY));
}

/* ------------------------------------------------------------------
 * 職業で言いかたが変わる
 * ------------------------------------------------------------------ */

/* PRIEST 系は "spell" ではなく "prayer"。名前の表も 31 個ずれた側を読む
 * （僧侶の番号 25 は Prayer・レベル 15）。 */
TEST(a_priest_forgets_prayers_not_spells) {
    given_no_spells_known();
    given_a_priest_of_level(1);
    given_a_learned_spell(25);

    calc_spells(A_WIS);

    ASSERT_TRUE(the_spell_is_forgotten(25));
    ASSERT_EQ_STR("You have forgotten the prayer of Prayer.",
                  fixture_message_text(0));
}

/* 学べるようになった知らせも "prayer" で言う。 */
TEST(a_priest_is_told_about_new_prayers) {
    given_no_spells_known();
    given_a_priest_of_level(1);

    calc_spells(A_WIS);

    ASSERT_EQ_INT(1, (int)py.flags.new_spells);
    ASSERT_EQ_STR("You can learn some new prayers now.",
                  fixture_message_text(0));
}

int main(void) {
    RUN_TEST(a_character_with_room_to_spare_is_told_they_can_learn_more);
    RUN_TEST(a_character_at_the_limit_neither_learns_nor_forgets);
    RUN_TEST(a_character_without_the_wits_for_it_may_keep_no_spells);
    RUN_TEST(a_high_stat_allows_one_and_a_half_spells_per_level);

    RUN_TEST(a_spell_above_the_characters_level_is_forgotten);
    RUN_TEST(the_sweep_from_the_top_stops_at_the_first_spell_within_reach);
    RUN_TEST(the_spell_learned_last_is_the_first_one_forgotten);

    RUN_TEST(a_forgotten_spell_comes_back_when_there_is_room);
    RUN_TEST(the_spell_learned_first_is_the_first_one_remembered);
    RUN_TEST(a_forgotten_spell_still_out_of_reach_does_not_block_the_next_one);
    RUN_TEST(the_not_yet_learned_marker_uses_up_one_turn_of_the_scan);
    RUN_TEST(the_marker_does_not_stand_in_for_the_spell_its_low_bits_name);
    RUN_TEST(a_forgotten_spell_after_the_marker_is_reached_when_the_scan_is_longer);

    RUN_TEST(the_number_to_learn_is_capped_by_the_spells_within_reach);
    RUN_TEST(nothing_is_said_when_the_number_to_learn_has_not_changed);

    RUN_TEST(a_priest_forgets_prayers_not_spells);
    RUN_TEST(a_priest_is_told_about_new_prayers);

    return TEST_SUMMARY();
}
