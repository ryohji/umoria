/* キャラクターの能力値算出のテスト -- 現在の実装を保護する
 *
 * src/misc3.c:967-1010 の put_misc3()（画面表示版）と
 * src/files.c:232-249 の file_character()（ファイル出力版）に、
 * 9 つの式が丸ごと二重に書かれている（コメントまで完全一致で、
 * 差異は出力先が put_buffer か fprintf かだけ）。片方だけ直すと
 * 画面とファイルで数値が食いちがう。
 *
 * 9 式は put_misc3() の途中のローカル変数なので外から直接は見えない。
 * しかし put_misc3() は結果を put_buffer() で画面に書くので、
 * tests/misc3_stubs.c の代役が記録した内容を fixture_screen_text() で
 * 読みとれば観測できる。本体は一切変更していない。
 *
 * 写しにしなかった理由: ステップ B で式を 1 箇所に抽出するとき、
 * 写しでは抽出後の実体を検証できず保護にならない。
 *
 * 観測はすべて likert() を通した文字列になるので分解能は粗い。
 * 各テストは「likert の境界をまたぐかどうか」で式を固定している。
 * xstl だけは likert(xstl, 1) で除数が 1 なので 1 刻みで観測できる。
 *
 * likert(x, y) の対応（src/misc3.c:908）:
 *   x/y = -3..-1 "Very Bad", 0..1 "Bad", 2 "Poor", 3..4 "Fair",
 *         5 "Good", 6 "Very Good", 7..8 "Excellent", それ以上 "Superb"
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、
 * いまどう振るまうかを固定することが目的。
 */
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"

extern player_type py;

/* 検証対象（src/misc3.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
void put_misc3(void);

#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* ------------------------------------------------------------------
 * 観測の窓口
 *
 * put_misc3() が put_buffer() で書く座標（src/misc3.c:990-1010）。
 * 左列 15、中列 42、右列 69 に likert() の文字列が入る。
 * ------------------------------------------------------------------ */
#define AT_FIGHTING     fixture_screen_text(16, 15) /* xbth  / 12 */
#define AT_BOWS         fixture_screen_text(17, 15) /* xbthb / 12 */
#define AT_SAVING_THROW fixture_screen_text(18, 15) /* xsave / 6  */
#define AT_STEALTH      fixture_screen_text(16, 42) /* xstl  / 1  */
#define AT_DISARMING    fixture_screen_text(17, 42) /* xdis  / 8  */
#define AT_MAGIC_DEVICE fixture_screen_text(18, 42) /* xdev  / 6  */
#define AT_PERCEPTION   fixture_screen_text(16, 69) /* xfos  / 3  */
#define AT_SEARCHING    fixture_screen_text(17, 69) /* xsrh  / 6  */
#define AT_INFRA_VISION fixture_screen_text(18, 69) /* "%d feet"  */

/* ------------------------------------------------------------------
 * 条件づくりの補助関数
 *
 * fixture_reset() が py を丸ごと 0 にするので、pclass は 0（Warrior）、
 * lev も 0 から始まる。lev が 0 なら class_level_adj の項が消えるので、
 * 素の式だけを見たいテストは lev を触らない。
 * ------------------------------------------------------------------ */

/* class_level_adj の行と、掛かるレベルを決める。
 * class_level_adj（src/player.c:300）の列は
 *   bth, bthb, device, disarm, save の順。
 * 行は 0 Warrior {4,4,2,2,3}, 1 Mage {2,2,4,3,3}, 3 Rogue {3,4,3,4,3}。 */
static void given_class_and_level(int pclass, int lev)
{
    py.misc.pclass = (uint8_t)pclass;
    py.misc.lev = (uint16_t)lev;
}

/* stat_adj() / todis_adj() が見る能力値。A_INT と A_WIS に違う値を
 * 入れることで、式がどちらを見ているかを判別できるようにする。
 * stat_adj（src/misc3.c:256）は 7 以下 -> 0、8..14 -> 1、15..17 -> 2、
 * 18..67 -> 3、68..87 -> 4。 */
static void given_stat(int which, int value)
{
    py.stats.use_stat[which] = (uint8_t)value;
}

/* todis_adj()（src/misc3.c:753）は A_DEX を見る。既定の 0 では -8 を
 * 返して xdis に -16 が乗ってしまうので、xdis を見ないテストでも
 * 邪魔にならないよう「0 を返す値」を明示できるようにする。
 * A_DEX が 8..12 なら todis_adj() は 0。 */
static void given_neutral_dexterity(void)
{
    given_stat(A_DEX, 12);
}

/* stat_adj() の引数の取りちがえを検出するための条件。
 * A_WIS と A_INT に違う値を入れ、どちらを見ているかで結果が変わる
 * ようにする。stat_adj(A_WIS)=4、stat_adj(A_INT)=0 になる。 */
static void given_wisdom_high_and_intelligence_low(void)
{
    given_stat(A_WIS, 68);
    given_stat(A_INT, 7);
}

/* 上の逆。stat_adj(A_INT)=4、stat_adj(A_WIS)=0。 */
static void given_intelligence_high_and_wisdom_low(void)
{
    given_stat(A_INT, 68);
    given_stat(A_WIS, 7);
}

/* ------------------------------------------------------------------
 * xstl -- stl + 1
 * ------------------------------------------------------------------ */

TEST(xstl_is_stl_plus_one)
{
    py.misc.stl = 1;
    put_misc3();
    ASSERT_EQ_STR(AT_STEALTH, "Poor"); /* xstl=2 -> 2/1=2 */
}

/* SUSPICIOUS: 本体のコメント「this results in a range from 0 to 9」は
 * +1 しているので成りたたない。stl=0 でも xstl は 1 になる。 */
TEST(xstl_is_one_when_stl_is_zero)
{
    py.misc.stl = 0;
    put_misc3();
    ASSERT_EQ_STR(AT_STEALTH, "Bad"); /* xstl=1 -> 1/1=1 */
}

/* ------------------------------------------------------------------
 * xfos -- 40 - fos、負なら 0
 * ------------------------------------------------------------------ */

TEST(xfos_is_forty_minus_fos_when_fos_is_below_forty)
{
    py.misc.fos = 22;
    put_misc3();
    ASSERT_EQ_STR(AT_PERCEPTION, "Very Good"); /* xfos=18 -> 18/3=6 */
}

TEST(xfos_is_zero_when_fos_is_exactly_forty)
{
    py.misc.fos = 40;
    put_misc3();
    ASSERT_EQ_STR(AT_PERCEPTION, "Bad"); /* xfos=0 -> 0/3=0 */
}

/* クランプがなければ 40-52 = -12 で -12/3 = -4、
 * likert の default に落ちて "Superb" になるので判別できる。 */
TEST(xfos_is_clamped_to_zero_when_fos_exceeds_forty)
{
    py.misc.fos = 52;
    put_misc3();
    ASSERT_EQ_STR(AT_PERCEPTION, "Bad");
}

/* SUSPICIOUS: 本体のコメント「this results in a range from 0 to 29」は
 * fos が 11 以上でないと成りたたない。キャラクター作成時の fos は
 * class の mfos（16..38、src/player.c:288）に race の fos（-5..5、
 * src/player.c:110）を足した 11..43 なので上端 29 は合う。しかし
 * 探索つきの装備は fos を減らす（src/moria1.c:48 の fos -= amount）
 * ので、遊んでいる途中で fos は 11 を下まわり xfos は 29 を超える。
 * このテストはそれを固定する。 */
TEST(xfos_exceeds_twenty_nine_when_fos_falls_below_eleven)
{
    py.misc.fos = 2;
    put_misc3();
    ASSERT_EQ_STR(AT_PERCEPTION, "Superb"); /* xfos=38 -> 38/3=12 */
}

/* ------------------------------------------------------------------
 * xinfra -- see_infra * 10 を "%d feet"
 * ------------------------------------------------------------------ */

TEST(xinfra_is_see_infra_times_ten_in_feet)
{
    py.flags.see_infra = 3;
    put_misc3();
    ASSERT_EQ_STR(AT_INFRA_VISION, "30 feet");
}

/* ------------------------------------------------------------------
 * stat_adj() の引数 -- xsave は A_WIS、xdis と xdev は A_INT
 *
 * 抽出のときに最も起こりやすい間違いなので、A_WIS と A_INT に
 * 違う値を入れて、どちらを見ているかを判別できるようにしている。
 * ------------------------------------------------------------------ */

/* save=8 に stat_adj(A_WIS)=4 が乗って 12。A_INT を見ていたら
 * 8 のままで "Bad" になるので判別できる。 */
TEST(xsave_uses_wisdom_not_intelligence)
{
    py.misc.save = 8;
    given_wisdom_high_and_intelligence_low();
    put_misc3();
    ASSERT_EQ_STR(AT_SAVING_THROW, "Poor"); /* 12/6=2 */
}

/* 逆向き。A_INT だけ高くても xsave は上がらない。 */
TEST(xsave_is_unaffected_by_intelligence)
{
    py.misc.save = 8;
    given_intelligence_high_and_wisdom_low();
    put_misc3();
    ASSERT_EQ_STR(AT_SAVING_THROW, "Bad"); /* 8/6=1 */
}

/* xdev は xsave と同じ p_ptr->save を基にしつつ stat_adj は A_INT。 */
TEST(xdev_uses_intelligence_not_wisdom)
{
    py.misc.save = 8;
    given_intelligence_high_and_wisdom_low();
    put_misc3();
    ASSERT_EQ_STR(AT_MAGIC_DEVICE, "Poor"); /* 12/6=2 */
}

TEST(xdev_is_unaffected_by_wisdom)
{
    py.misc.save = 8;
    given_wisdom_high_and_intelligence_low();
    put_misc3();
    ASSERT_EQ_STR(AT_MAGIC_DEVICE, "Bad"); /* 8/6=1 */
}

/* SUSPICIOUS: xdev は disarm ではなく p_ptr->save を基にしている
 * （xsave と同じ）。意図的かどうかは判断できないので固定するだけ。 */
TEST(xdev_is_based_on_save_not_disarm)
{
    py.misc.save = 0;
    py.misc.disarm = 40;
    given_intelligence_high_and_wisdom_low();
    put_misc3();
    ASSERT_EQ_STR(AT_MAGIC_DEVICE, "Bad"); /* 0+4=4, 4/6=0 */
}

/* ------------------------------------------------------------------
 * class_level_adj の列 -- xdis は CLA_DISARM、xsave は CLA_SAVE、
 * xdev は CLA_DEVICE、xbth は CLA_BTH、xbthb は CLA_BTHB
 *
 * 列を取りちがえても値が同じになるクラスがあるので、テストごとに
 * 「その列だけが他と違う値を持つクラス」を選んでいる。
 *   Mage    {2,2,4,3,3} -- device 4 だけが他と違う
 *   Rogue   {3,4,3,4,3} -- disarm 4 と bthb 4、bth 3 が違う
 *   Warrior {4,4,2,2,3} -- save 3 だけが device/disarm 2 と違う
 * ------------------------------------------------------------------ */

/* Mage / lev=9。CLA_DEVICE=4 なので 4*9/3=12。CLA_SAVE か CLA_DISARM
 * （どちらも 3）を見ていたら 9 で "Bad" になるので判別できる。 */
TEST(xdev_uses_the_device_column_of_class_level_adj)
{
    given_class_and_level(1, 9);
    given_intelligence_high_and_wisdom_low();
    given_stat(A_INT, 7); /* stat_adj を 0 にして class の項だけ見る */
    put_misc3();
    ASSERT_EQ_STR(AT_MAGIC_DEVICE, "Poor"); /* 12/6=2 */
}

/* Rogue / lev=18。CLA_DISARM=4 なので 4*18/3=24。CLA_DEVICE か
 * CLA_SAVE（どちらも 3）なら 18 で "Poor" になる。 */
TEST(xdis_uses_the_disarm_column_of_class_level_adj)
{
    given_class_and_level(3, 18);
    given_neutral_dexterity();
    given_stat(A_INT, 7);
    put_misc3();
    ASSERT_EQ_STR(AT_DISARMING, "Fair"); /* 24/8=3 */
}

/* Warrior / lev=18。CLA_SAVE=3 なので 3*18/3=18。CLA_DEVICE か
 * CLA_DISARM（どちらも 2）なら 12 で "Poor" になる。 */
TEST(xsave_uses_the_save_column_of_class_level_adj)
{
    given_class_and_level(0, 18);
    given_stat(A_WIS, 7);
    put_misc3();
    ASSERT_EQ_STR(AT_SAVING_THROW, "Fair"); /* 18/6=3 */
}

/* Rogue / lev=20。CLA_BTH=3 なので 3*20=60（/3 はしない）。
 * CLA_BTHB=4 を見ていたら 80 で "Very Good" になる。 */
TEST(xbth_uses_the_bth_column_of_class_level_adj)
{
    given_class_and_level(3, 20);
    put_misc3();
    ASSERT_EQ_STR(AT_FIGHTING, "Good"); /* 60/12=5 */
}

/* 同じ条件で xbthb は CLA_BTHB=4 なので 4*20=80。 */
TEST(xbthb_uses_the_bthb_column_of_class_level_adj)
{
    given_class_and_level(3, 20);
    put_misc3();
    ASSERT_EQ_STR(AT_BOWS, "Very Good"); /* 80/12=6 */
}

/* ------------------------------------------------------------------
 * / 3 の切り捨て -- xdis / xsave / xdev だけが 3 で割る
 * ------------------------------------------------------------------ */

/* Warrior / lev=12。CLA_SAVE=3 なので 3*12=36、36/3=12（割り切れる）。 */
TEST(xsave_class_bonus_divides_evenly_when_product_is_multiple_of_three)
{
    given_class_and_level(0, 12);
    given_stat(A_WIS, 7);
    put_misc3();
    ASSERT_EQ_STR(AT_SAVING_THROW, "Poor"); /* 12/6=2 */
}

/* Mage / lev=5。CLA_DEVICE=4 なので 4*5=20、20/3=6 に切り捨てられる。
 * 切り捨てずに 6.67 を丸めていれば 7 になり "Bad" のままではない。 */
TEST(xdev_class_bonus_is_truncated_when_product_is_not_multiple_of_three)
{
    given_class_and_level(1, 5);
    given_stat(A_INT, 7);
    py.misc.save = 6;
    put_misc3();
    ASSERT_EQ_STR(AT_MAGIC_DEVICE, "Poor"); /* 6+6=12, 12/6=2 */
}

/* ------------------------------------------------------------------
 * xbth -- ptohit に BTH_PLUS_ADJ（3）が掛かる
 * ------------------------------------------------------------------ */

/* ptohit=0 なら bth がそのまま出る。次のテストの基準値。 */
TEST(xbth_is_bth_itself_when_ptohit_is_zero)
{
    py.misc.bth = 24;
    py.misc.ptohit = 0;
    put_misc3();
    ASSERT_EQ_STR(AT_FIGHTING, "Poor"); /* 24/12=2 */
}

/* ptohit=4 なら 4*3=12 が乗って 36。係数がなければ 28 で "Poor"。 */
TEST(xbth_adds_three_times_ptohit)
{
    py.misc.bth = 24;
    py.misc.ptohit = 4;
    put_misc3();
    ASSERT_EQ_STR(AT_FIGHTING, "Fair"); /* 36/12=3 */
}

/* xbthb も同じ係数を使う（基は bthb）。 */
TEST(xbthb_adds_three_times_ptohit_to_bthb)
{
    py.misc.bthb = 24;
    py.misc.ptohit = 4;
    put_misc3();
    ASSERT_EQ_STR(AT_BOWS, "Fair"); /* 36/12=3 */
}

/* ------------------------------------------------------------------
 * xsrh -- srh をそのまま出す（補正なし）
 * ------------------------------------------------------------------ */

TEST(xsrh_is_srh_without_any_adjustment)
{
    py.misc.srh = 32;
    given_class_and_level(1, 20); /* class の項が乗らないことも確かめる */
    put_misc3();
    ASSERT_EQ_STR(AT_SEARCHING, "Good"); /* 32/6=5 */
}

/* ------------------------------------------------------------------
 * xdis -- todis_adj() が 2 倍で乗る
 * ------------------------------------------------------------------ */

/* A_DEX=18 なら todis_adj()=4、2 倍で 8。disarm=32 に乗って 40。
 * 2 倍しなければ 36 で "Fair" になるので判別できる。 */
TEST(xdis_adds_twice_the_dexterity_adjustment)
{
    py.misc.disarm = 32;
    given_stat(A_DEX, 18);
    given_stat(A_INT, 7);
    put_misc3();
    ASSERT_EQ_STR(AT_DISARMING, "Good"); /* 40/8=5 */
}

int main(void)
{
    /* --- xstl: stl + 1。likert の除数が 1 なので 1 刻みで観測できる --- */

    /* stl=1 なら xstl=2 で "Poor"。+1 がなければ 1 で "Bad" になる。 */
    RUN_TEST(xstl_is_stl_plus_one);

    /* stl=0 でも xstl=1。本体のコメント「range from 0 to 9」は
     * +1 しているので成りたたない（最小は 1）。SUSPICIOUS。 */
    RUN_TEST(xstl_is_one_when_stl_is_zero);

    /* --- xfos: 40 - fos を 0 で下げどめる --- */

    RUN_TEST(xfos_is_forty_minus_fos_when_fos_is_below_forty);
    RUN_TEST(xfos_is_zero_when_fos_is_exactly_forty);
    RUN_TEST(xfos_is_clamped_to_zero_when_fos_exceeds_forty);
    RUN_TEST(xfos_exceeds_twenty_nine_when_fos_falls_below_eleven);

    /* --- xinfra: see_infra * 10 を "%d feet" で書く --- */

    RUN_TEST(xinfra_is_see_infra_times_ten_in_feet);

    /* --- stat_adj() の引数（A_WIS か A_INT か）を判別する --- */

    RUN_TEST(xsave_uses_wisdom_not_intelligence);
    RUN_TEST(xsave_is_unaffected_by_intelligence);
    RUN_TEST(xdev_uses_intelligence_not_wisdom);
    RUN_TEST(xdev_is_unaffected_by_wisdom);
    RUN_TEST(xdev_is_based_on_save_not_disarm);

    /* --- class_level_adj のどの列を見ているかを判別する --- */

    RUN_TEST(xdev_uses_the_device_column_of_class_level_adj);
    RUN_TEST(xdis_uses_the_disarm_column_of_class_level_adj);
    RUN_TEST(xsave_uses_the_save_column_of_class_level_adj);
    RUN_TEST(xbth_uses_the_bth_column_of_class_level_adj);
    RUN_TEST(xbthb_uses_the_bthb_column_of_class_level_adj);

    /* --- / 3 の切り捨て --- */

    RUN_TEST(xsave_class_bonus_divides_evenly_when_product_is_multiple_of_three);
    RUN_TEST(xdev_class_bonus_is_truncated_when_product_is_not_multiple_of_three);

    /* --- BTH_PLUS_ADJ の係数、xsrh の素通し、todis_adj の 2 倍 --- */

    RUN_TEST(xbth_is_bth_itself_when_ptohit_is_zero);
    RUN_TEST(xbth_adds_three_times_ptohit);
    RUN_TEST(xbthb_adds_three_times_ptohit_to_bthb);
    RUN_TEST(xsrh_is_srh_without_any_adjustment);
    RUN_TEST(xdis_adds_twice_the_dexterity_adjustment);

    return TEST_SUMMARY();
}
