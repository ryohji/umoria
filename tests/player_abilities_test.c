/* 「装備で決まる耐性・能力」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 8 つめの問い（1 つめは持っている金、2 つめは腹の具合、3 つめは
 * 画面に出す数字、4 つめは魔力、5 つめは体力、6 つめは階級と経験値、7 つめは
 * 状態の旗）。答えは 17 個 —— もとは py.flags.see_inv から py.flags.sustain_chr
 * まで、8 ファイルから 106 か所が触っていた —— で、#18-12-8C からは
 * src/player_abilities.c の中の配列 1 つだけが持っている。
 *
 * **この単位で新しいのは「答えが覚えてあるのではなく毎回導かれる」こと。**
 * ここまでの 7 つは何かを覚えていたが、この 17 個は calc_bonuses()
 * （moria1.c）が**装備から作りなおす**。だからテストの重心は 3 つ:
 *
 *   1. **バイトの並びはセーブファイルの書式**（save.c が 17 バイトを 1 続きで
 *      書く）。だから **どの位置がどの能力かを名指しで釘打つ** —— module の
 *      enum の 2 行を入れかえても、1 つずつ立てて 1 つずつ読むだけのテストでは
 *      気づけない。入れかえは「セーブファイルの読みちがえ」そのもの。
 *   2. **どの窓口も自分の 1 個以外を触らない。** 17 個は 1 つの配列に並んで
 *      いるので、添字の取りちがえは隣を巻きこむ。**1 個だけ立てて 12 の
 *      読み窓口ぜんぶを見る**形をくりかえす。
 *   3. **能力値の並びが 2 つある。** 品物の p1 は 1=str 2=int 3=wis **4=con
 *      5=dex** 6=chr、ゲームの側は A_STR A_INT A_WIS **A_DEX A_CON** A_CHR。
 *      **4 番と 5 番が入れちがっている**ので、`sustained[p1 - 1]` と書くと
 *      黙って耐久と敏捷が入れかわる。そこに 2 本の釘を打つ。
 *
 * 17 個は 4 つの種類に分かれる（src/player_abilities.h の頭に書いた） ——
 * されない事（麻痺しない・能力値 6 つを吸われない）／痛くない事（火・冷気・
 * 酸・光・落下）／体のはたらき（透明が見える・回復が速い・腹が減りにくい・
 * 勝手に飛ぶ）／モンスターが気づく事（怒らせる）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に 17 バイトを置きなおす。
 */
/* externs.h は要らない。窓口と、TR_*・A_* のための constant.h・types.h だけで
 * 足りる。**TR_* と A_* をここで名指しするのは意図的** —— 呼び手からは消すが、
 * 「どの品の旗がどの能力になるか」を守るテストはどこかで名前を言わなければ
 * ならない。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_abilities.h"

#include "minunit.h"

/* セーブファイルに並ぶ 17 バイトの位置。**この並びが書式**で、save.c:199-215 の
 * wr_byte() の順そのもの。module 側の enum と二重に書いてあるのが釘で、
 * どちらかを動かせば下の 17 件が落ちる。 */
enum {
    SAVED_SEE_INVISIBLE,
    SAVED_TELEPORT,
    SAVED_NEVER_PARALYZED,
    SAVED_SLOW_DIGESTION,
    SAVED_AGGRAVATE,
    SAVED_RESIST_FIRE,
    SAVED_RESIST_COLD,
    SAVED_RESIST_ACID,
    SAVED_REGENERATE,
    SAVED_RESIST_LIGHT,
    SAVED_NO_FALLING_DAMAGE,
    SAVED_SUSTAIN_STR,
    SAVED_SUSTAIN_INT,
    SAVED_SUSTAIN_WIS,
    SAVED_SUSTAIN_CON,
    SAVED_SUSTAIN_DEX,
    SAVED_SUSTAIN_CHR
};

#define ONLY(position) (1L << (position))
#define ALL_SEVENTEEN ((1L << PLAYER_ABILITIES_SAVED_BYTES) - 1)

/* 12 の読み窓口ぜんぶに訊いて、答えをセーブの並びに詰めなおしたもの。
 * **これが「位置 ↔ 能力」の釘打ちの道具** —— 能力値の 6 つは A_* で訊くので、
 * 並びの入れちがいはここで見える（位置 14 は耐久、A_CON で訊く）。 */
static long what_the_windows_answer(void) {
    long answers = 0;

    if (player_can_see_invisible()) {
        answers |= ONLY(SAVED_SEE_INVISIBLE);
    }
    if (player_teleports_randomly()) {
        answers |= ONLY(SAVED_TELEPORT);
    }
    if (player_never_paralyzed()) {
        answers |= ONLY(SAVED_NEVER_PARALYZED);
    }
    if (player_has_slow_digestion()) {
        answers |= ONLY(SAVED_SLOW_DIGESTION);
    }
    if (player_aggravates_monsters()) {
        answers |= ONLY(SAVED_AGGRAVATE);
    }
    if (player_resists_fire()) {
        answers |= ONLY(SAVED_RESIST_FIRE);
    }
    if (player_resists_cold()) {
        answers |= ONLY(SAVED_RESIST_COLD);
    }
    if (player_resists_acid()) {
        answers |= ONLY(SAVED_RESIST_ACID);
    }
    if (player_regenerates()) {
        answers |= ONLY(SAVED_REGENERATE);
    }
    if (player_resists_light()) {
        answers |= ONLY(SAVED_RESIST_LIGHT);
    }
    if (player_takes_no_falling_damage()) {
        answers |= ONLY(SAVED_NO_FALLING_DAMAGE);
    }
    if (player_stat_sustained(A_STR)) {
        answers |= ONLY(SAVED_SUSTAIN_STR);
    }
    if (player_stat_sustained(A_INT)) {
        answers |= ONLY(SAVED_SUSTAIN_INT);
    }
    if (player_stat_sustained(A_WIS)) {
        answers |= ONLY(SAVED_SUSTAIN_WIS);
    }
    if (player_stat_sustained(A_CON)) {
        answers |= ONLY(SAVED_SUSTAIN_CON);
    }
    if (player_stat_sustained(A_DEX)) {
        answers |= ONLY(SAVED_SUSTAIN_DEX);
    }
    if (player_stat_sustained(A_CHR)) {
        answers |= ONLY(SAVED_SUSTAIN_CHR);
    }

    return answers;
}

/* セーブに出ていく 17 バイトを同じ形に詰めたもの。 */
static long what_the_save_file_gets(void) {
    long bytes = 0;

    for (int position = 0; position < PLAYER_ABILITIES_SAVED_BYTES; position++) {
        if (player_abilities_saved_byte(position) != 0) {
            bytes |= ONLY(position);
        }
    }

    return bytes;
}

/* 17 個を 1 つずつ置く。**forget_all は使わない** —— それ自身を試す件が
 * 独立していてほしいから。 */
static void given_nothing_granted(void) {
    for (int position = 0; position < PLAYER_ABILITIES_SAVED_BYTES; position++) {
        player_abilities_restore_byte(position, 0);
    }
}

static void given_everything_granted(void) {
    for (int position = 0; position < PLAYER_ABILITIES_SAVED_BYTES; position++) {
        player_abilities_restore_byte(position, 1);
    }
}

/* セーブファイルの 1 バイトだけが立っている状態を作って、窓口の答えを見る。 */
static long answers_when_only(int position) {
    given_nothing_granted();
    player_abilities_restore_byte(position, 1);

    return what_the_windows_answer();
}

/* 品物の旗 1 つだけを身につけた状態を作って、窓口の答えを見る。 */
static long answers_when_wearing(uint32_t item_flags) {
    given_nothing_granted();
    player_abilities_note_item_flags(item_flags);

    return what_the_windows_answer();
}

/* 能力値を保つ品物 1 つだけを身につけた状態を作って、窓口の答えを見る。 */
static long answers_when_sustaining(int item_p1) {
    given_nothing_granted();
    player_abilities_note_sustain(item_p1);

    return what_the_windows_answer();
}

/* --- 位置 ↔ 能力（セーブファイルの書式） ------------------------------- */

TEST(the_first_saved_byte_is_seeing_the_invisible) {
    ASSERT_EQ_INT(answers_when_only(SAVED_SEE_INVISIBLE), ONLY(SAVED_SEE_INVISIBLE));
}

TEST(the_second_saved_byte_is_teleporting_at_random) {
    ASSERT_EQ_INT(answers_when_only(SAVED_TELEPORT), ONLY(SAVED_TELEPORT));
}

TEST(the_third_saved_byte_is_never_being_paralyzed) {
    ASSERT_EQ_INT(answers_when_only(SAVED_NEVER_PARALYZED), ONLY(SAVED_NEVER_PARALYZED));
}

TEST(the_fourth_saved_byte_is_digesting_slowly) {
    ASSERT_EQ_INT(answers_when_only(SAVED_SLOW_DIGESTION), ONLY(SAVED_SLOW_DIGESTION));
}

TEST(the_fifth_saved_byte_is_aggravating_monsters) {
    ASSERT_EQ_INT(answers_when_only(SAVED_AGGRAVATE), ONLY(SAVED_AGGRAVATE));
}

TEST(the_sixth_saved_byte_is_resisting_fire) {
    ASSERT_EQ_INT(answers_when_only(SAVED_RESIST_FIRE), ONLY(SAVED_RESIST_FIRE));
}

TEST(the_seventh_saved_byte_is_resisting_cold) {
    ASSERT_EQ_INT(answers_when_only(SAVED_RESIST_COLD), ONLY(SAVED_RESIST_COLD));
}

TEST(the_eighth_saved_byte_is_resisting_acid) {
    ASSERT_EQ_INT(answers_when_only(SAVED_RESIST_ACID), ONLY(SAVED_RESIST_ACID));
}

TEST(the_ninth_saved_byte_is_regenerating) {
    ASSERT_EQ_INT(answers_when_only(SAVED_REGENERATE), ONLY(SAVED_REGENERATE));
}

TEST(the_tenth_saved_byte_is_resisting_light) {
    ASSERT_EQ_INT(answers_when_only(SAVED_RESIST_LIGHT), ONLY(SAVED_RESIST_LIGHT));
}

TEST(the_eleventh_saved_byte_is_taking_no_falling_damage) {
    ASSERT_EQ_INT(answers_when_only(SAVED_NO_FALLING_DAMAGE), ONLY(SAVED_NO_FALLING_DAMAGE));
}

TEST(the_twelfth_saved_byte_is_sustaining_the_strength) {
    ASSERT_EQ_INT(answers_when_only(SAVED_SUSTAIN_STR), ONLY(SAVED_SUSTAIN_STR));
}

TEST(the_thirteenth_saved_byte_is_sustaining_the_intelligence) {
    ASSERT_EQ_INT(answers_when_only(SAVED_SUSTAIN_INT), ONLY(SAVED_SUSTAIN_INT));
}

TEST(the_fourteenth_saved_byte_is_sustaining_the_wisdom) {
    ASSERT_EQ_INT(answers_when_only(SAVED_SUSTAIN_WIS), ONLY(SAVED_SUSTAIN_WIS));
}

/* **15 番めは耐久で、16 番めが敏捷。** ゲームの側の並び（A_DEX が A_CON より
 * 先）とは逆で、セーブファイルは品物の p1 の並びに従っている。 */
TEST(the_fifteenth_saved_byte_is_sustaining_the_constitution) {
    ASSERT_EQ_INT(answers_when_only(SAVED_SUSTAIN_CON), ONLY(SAVED_SUSTAIN_CON));
}

TEST(the_sixteenth_saved_byte_is_sustaining_the_dexterity) {
    ASSERT_EQ_INT(answers_when_only(SAVED_SUSTAIN_DEX), ONLY(SAVED_SUSTAIN_DEX));
}

TEST(the_seventeenth_saved_byte_is_sustaining_the_charisma) {
    ASSERT_EQ_INT(answers_when_only(SAVED_SUSTAIN_CHR), ONLY(SAVED_SUSTAIN_CHR));
}

/* --- セーブファイルの 17 バイト ---------------------------------------- */

TEST(the_run_in_the_save_file_is_seventeen_bytes) {
    ASSERT_EQ_INT(PLAYER_ABILITIES_SAVED_BYTES, 17);
}

TEST(what_the_windows_granted_is_what_the_save_file_gets) {
    given_nothing_granted();
    player_abilities_note_item_flags(TR_RES_FIRE | TR_FFALL);
    player_abilities_note_sustain(4);

    ASSERT_EQ_INT(what_the_save_file_gets(),
                  ONLY(SAVED_RESIST_FIRE) | ONLY(SAVED_NO_FALLING_DAMAGE) | ONLY(SAVED_SUSTAIN_CON));
}

/* 壊れたファイルの 5 を 1 に丸めない。もとの save.c は wr_byte(f_ptr->free_act)
 * でフィールドの中身をそのまま書いていた（だから置き場は bool ではなく
 * 1 バイト）。読んで書けば同じ値が出る。 */
TEST(a_restored_byte_comes_back_unchanged) {
    given_nothing_granted();
    player_abilities_restore_byte(SAVED_NEVER_PARALYZED, 5);

    ASSERT_EQ_INT(player_abilities_saved_byte(SAVED_NEVER_PARALYZED), 5);
}

TEST(any_byte_that_is_not_zero_means_the_ability_is_there) {
    given_nothing_granted();
    player_abilities_restore_byte(SAVED_REGENERATE, 5);

    ASSERT_TRUE(player_regenerates());
}

TEST(a_position_past_the_run_reads_as_zero) {
    given_everything_granted();

    ASSERT_EQ_INT(player_abilities_saved_byte(PLAYER_ABILITIES_SAVED_BYTES), 0);
}

TEST(a_position_past_the_run_changes_nothing) {
    given_everything_granted();
    player_abilities_restore_byte(PLAYER_ABILITIES_SAVED_BYTES, 0);

    ASSERT_EQ_INT(what_the_save_file_gets(), ALL_SEVENTEEN);
}

TEST(a_negative_position_reads_as_zero) {
    given_everything_granted();

    ASSERT_EQ_INT(player_abilities_saved_byte(-1), 0);
}

TEST(all_seventeen_can_stand_at_once) {
    given_everything_granted();

    ASSERT_EQ_INT(what_the_windows_answer(), ALL_SEVENTEEN);
}

/* --- 身につけているものから作りなおす ---------------------------------- */

TEST(nothing_worn_grants_nothing) {
    ASSERT_EQ_INT(answers_when_wearing(0), 0);
}

TEST(slow_digestion_comes_from_the_slow_digest_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_SLOW_DIGEST), ONLY(SAVED_SLOW_DIGESTION));
}

TEST(aggravation_comes_from_the_aggravate_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_AGGRAVATE), ONLY(SAVED_AGGRAVATE));
}

TEST(random_teleporting_comes_from_the_teleport_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_TELEPORT), ONLY(SAVED_TELEPORT));
}

TEST(regeneration_comes_from_the_regen_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_REGEN), ONLY(SAVED_REGENERATE));
}

TEST(resisting_fire_comes_from_the_res_fire_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_RES_FIRE), ONLY(SAVED_RESIST_FIRE));
}

TEST(resisting_acid_comes_from_the_res_acid_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_RES_ACID), ONLY(SAVED_RESIST_ACID));
}

TEST(resisting_cold_comes_from_the_res_cold_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_RES_COLD), ONLY(SAVED_RESIST_COLD));
}

TEST(never_being_paralyzed_comes_from_the_free_act_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_FREE_ACT), ONLY(SAVED_NEVER_PARALYZED));
}

TEST(seeing_the_invisible_comes_from_the_see_invis_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_SEE_INVIS), ONLY(SAVED_SEE_INVISIBLE));
}

TEST(resisting_light_comes_from_the_res_light_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_RES_LIGHT), ONLY(SAVED_RESIST_LIGHT));
}

TEST(taking_no_falling_damage_comes_from_the_ffall_flag) {
    ASSERT_EQ_INT(answers_when_wearing(TR_FFALL), ONLY(SAVED_NO_FALLING_DAMAGE));
}

/* calc_bonuses() は身につけているものぜんぶの旗を 1 語に畳んでから渡す。 */
TEST(the_flags_of_everything_worn_are_taken_together) {
    ASSERT_EQ_INT(answers_when_wearing(TR_RES_FIRE | TR_REGEN | TR_FFALL),
                  ONLY(SAVED_RESIST_FIRE) | ONLY(SAVED_REGENERATE) | ONLY(SAVED_NO_FALLING_DAMAGE));
}

/* **能力値を保つ旗だけは 1 語からは決まらない。** どの能力値かは品物ごとの p1 に
 * あるので、呼び手が品物 1 つずつ note_sustain() を呼ぶ。 */
TEST(the_sustain_flag_alone_grants_no_sustain) {
    ASSERT_EQ_INT(answers_when_wearing(TR_SUST_STAT), 0);
}

/* 能力値の上げ下げや探索・隠密・速さの旗はこの問いの答えではない
 * （calc_bonuses() の別の部分と py.misc が受けもつ）。 */
TEST(flags_that_are_not_abilities_grant_nothing) {
    ASSERT_EQ_INT(answers_when_wearing(TR_STR | TR_SEARCH | TR_STEALTH | TR_SPEED | TR_INFRA), 0);
}

/* 立てるだけ —— 消すのは forget_all() の仕事で、順番が入れかわったら
 * calc_bonuses() は装備を外した人に能力を残す。 */
TEST(worn_flags_never_take_an_ability_away) {
    given_everything_granted();
    player_abilities_note_item_flags(0);

    ASSERT_EQ_INT(what_the_windows_answer(), ALL_SEVENTEEN);
}

TEST(forgetting_leaves_nothing_behind) {
    given_everything_granted();
    player_abilities_forget_all();

    ASSERT_EQ_INT(what_the_windows_answer(), 0);
}

TEST(forgetting_empties_the_save_file_run_too) {
    given_everything_granted();
    player_abilities_forget_all();

    ASSERT_EQ_INT(what_the_save_file_gets(), 0);
}

/* --- 能力値を保つ品物（並びが 2 つある） -------------------------------- */

TEST(the_first_kind_of_sustain_item_keeps_the_strength) {
    ASSERT_EQ_INT(answers_when_sustaining(1), ONLY(SAVED_SUSTAIN_STR));
}

TEST(the_second_kind_of_sustain_item_keeps_the_intelligence) {
    ASSERT_EQ_INT(answers_when_sustaining(2), ONLY(SAVED_SUSTAIN_INT));
}

TEST(the_third_kind_of_sustain_item_keeps_the_wisdom) {
    ASSERT_EQ_INT(answers_when_sustaining(3), ONLY(SAVED_SUSTAIN_WIS));
}

TEST(the_fourth_kind_of_sustain_item_keeps_the_constitution) {
    ASSERT_EQ_INT(answers_when_sustaining(4), ONLY(SAVED_SUSTAIN_CON));
}

TEST(the_fifth_kind_of_sustain_item_keeps_the_dexterity) {
    ASSERT_EQ_INT(answers_when_sustaining(5), ONLY(SAVED_SUSTAIN_DEX));
}

TEST(the_sixth_kind_of_sustain_item_keeps_the_charisma) {
    ASSERT_EQ_INT(answers_when_sustaining(6), ONLY(SAVED_SUSTAIN_CHR));
}

/* **並びの入れちがいに打つ 2 本の釘。** 品物の 4 番は耐久だが、ゲームの側の
 * 4 番（A_DEX）は敏捷 —— `sustained[p1 - 1]` と書くとここが入れかわる。 */
TEST(the_fourth_kind_of_sustain_item_does_not_keep_the_dexterity) {
    given_nothing_granted();
    player_abilities_note_sustain(4);

    ASSERT_FALSE(player_stat_sustained(A_DEX));
}

TEST(the_fifth_kind_of_sustain_item_does_not_keep_the_constitution) {
    given_nothing_granted();
    player_abilities_note_sustain(5);

    ASSERT_FALSE(player_stat_sustained(A_CON));
}

/* moria1.c の switch には default: break; があった —— 1 から 6 の外は何も
 * 起こらない（TR_SUST_STAT を持つのに p1 が別の意味の品もある）。 */
TEST(a_sustain_item_numbered_zero_keeps_nothing) {
    ASSERT_EQ_INT(answers_when_sustaining(0), 0);
}

TEST(a_sustain_item_numbered_past_the_six_keeps_nothing) {
    ASSERT_EQ_INT(answers_when_sustaining(7), 0);
}

TEST(a_sustain_item_with_a_negative_number_keeps_nothing) {
    ASSERT_EQ_INT(answers_when_sustaining(-1), 0);
}

TEST(sustains_from_two_items_add_up) {
    given_nothing_granted();
    player_abilities_note_sustain(1);
    player_abilities_note_sustain(6);

    ASSERT_EQ_INT(what_the_windows_answer(), ONLY(SAVED_SUSTAIN_STR) | ONLY(SAVED_SUSTAIN_CHR));
}

TEST(asking_about_a_stat_past_the_six_answers_no) {
    given_everything_granted();

    ASSERT_FALSE(player_stat_sustained(6));
}

TEST(asking_about_a_negative_stat_answers_no) {
    given_everything_granted();

    ASSERT_FALSE(player_stat_sustained(-1));
}

/* --- 装備でない出どころ（時限の透明看破） ------------------------------- */

TEST(the_timed_potion_grants_seeing_the_invisible) {
    given_nothing_granted();
    player_grant_see_invisible();

    ASSERT_EQ_INT(what_the_windows_answer(), ONLY(SAVED_SEE_INVISIBLE));
}

TEST(granting_it_twice_is_the_same_as_once) {
    given_nothing_granted();
    player_grant_see_invisible();
    player_grant_see_invisible();

    ASSERT_EQ_INT(what_the_windows_answer(), ONLY(SAVED_SEE_INVISIBLE));
}

/* 2 つの出どころは同じ 1 つの答えになる（片方が消えてももう片方で立つ）。 */
TEST(an_item_and_the_potion_grant_the_same_one_answer) {
    given_nothing_granted();
    player_abilities_note_item_flags(TR_SEE_INVIS);
    player_grant_see_invisible();

    ASSERT_EQ_INT(what_the_windows_answer(), ONLY(SAVED_SEE_INVISIBLE));
}

int main(void) {
    RUN_TEST(the_first_saved_byte_is_seeing_the_invisible);
    RUN_TEST(the_second_saved_byte_is_teleporting_at_random);
    RUN_TEST(the_third_saved_byte_is_never_being_paralyzed);
    RUN_TEST(the_fourth_saved_byte_is_digesting_slowly);
    RUN_TEST(the_fifth_saved_byte_is_aggravating_monsters);
    RUN_TEST(the_sixth_saved_byte_is_resisting_fire);
    RUN_TEST(the_seventh_saved_byte_is_resisting_cold);
    RUN_TEST(the_eighth_saved_byte_is_resisting_acid);
    RUN_TEST(the_ninth_saved_byte_is_regenerating);
    RUN_TEST(the_tenth_saved_byte_is_resisting_light);
    RUN_TEST(the_eleventh_saved_byte_is_taking_no_falling_damage);
    RUN_TEST(the_twelfth_saved_byte_is_sustaining_the_strength);
    RUN_TEST(the_thirteenth_saved_byte_is_sustaining_the_intelligence);
    RUN_TEST(the_fourteenth_saved_byte_is_sustaining_the_wisdom);
    RUN_TEST(the_fifteenth_saved_byte_is_sustaining_the_constitution);
    RUN_TEST(the_sixteenth_saved_byte_is_sustaining_the_dexterity);
    RUN_TEST(the_seventeenth_saved_byte_is_sustaining_the_charisma);


    RUN_TEST(the_run_in_the_save_file_is_seventeen_bytes);
    RUN_TEST(what_the_windows_granted_is_what_the_save_file_gets);
    RUN_TEST(a_restored_byte_comes_back_unchanged);
    RUN_TEST(any_byte_that_is_not_zero_means_the_ability_is_there);
    RUN_TEST(a_position_past_the_run_reads_as_zero);
    RUN_TEST(a_position_past_the_run_changes_nothing);
    RUN_TEST(a_negative_position_reads_as_zero);
    RUN_TEST(all_seventeen_can_stand_at_once);

    RUN_TEST(nothing_worn_grants_nothing);
    RUN_TEST(slow_digestion_comes_from_the_slow_digest_flag);
    RUN_TEST(aggravation_comes_from_the_aggravate_flag);
    RUN_TEST(random_teleporting_comes_from_the_teleport_flag);
    RUN_TEST(regeneration_comes_from_the_regen_flag);
    RUN_TEST(resisting_fire_comes_from_the_res_fire_flag);
    RUN_TEST(resisting_acid_comes_from_the_res_acid_flag);
    RUN_TEST(resisting_cold_comes_from_the_res_cold_flag);
    RUN_TEST(never_being_paralyzed_comes_from_the_free_act_flag);
    RUN_TEST(seeing_the_invisible_comes_from_the_see_invis_flag);
    RUN_TEST(resisting_light_comes_from_the_res_light_flag);
    RUN_TEST(taking_no_falling_damage_comes_from_the_ffall_flag);
    RUN_TEST(the_flags_of_everything_worn_are_taken_together);
    RUN_TEST(the_sustain_flag_alone_grants_no_sustain);
    RUN_TEST(flags_that_are_not_abilities_grant_nothing);
    RUN_TEST(worn_flags_never_take_an_ability_away);
    RUN_TEST(forgetting_leaves_nothing_behind);
    RUN_TEST(forgetting_empties_the_save_file_run_too);

    RUN_TEST(the_first_kind_of_sustain_item_keeps_the_strength);
    RUN_TEST(the_second_kind_of_sustain_item_keeps_the_intelligence);
    RUN_TEST(the_third_kind_of_sustain_item_keeps_the_wisdom);
    RUN_TEST(the_fourth_kind_of_sustain_item_keeps_the_constitution);
    RUN_TEST(the_fifth_kind_of_sustain_item_keeps_the_dexterity);
    RUN_TEST(the_sixth_kind_of_sustain_item_keeps_the_charisma);
    RUN_TEST(the_fourth_kind_of_sustain_item_does_not_keep_the_dexterity);
    RUN_TEST(the_fifth_kind_of_sustain_item_does_not_keep_the_constitution);
    RUN_TEST(a_sustain_item_numbered_zero_keeps_nothing);
    RUN_TEST(a_sustain_item_numbered_past_the_six_keeps_nothing);
    RUN_TEST(a_sustain_item_with_a_negative_number_keeps_nothing);
    RUN_TEST(sustains_from_two_items_add_up);
    RUN_TEST(asking_about_a_stat_past_the_six_answers_no);
    RUN_TEST(asking_about_a_negative_stat_answers_no);

    RUN_TEST(the_timed_potion_grants_seeing_the_invisible);
    RUN_TEST(granting_it_twice_is_the_same_as_once);
    RUN_TEST(an_item_and_the_potion_grant_the_same_one_answer);

    return TEST_SUMMARY();
}
