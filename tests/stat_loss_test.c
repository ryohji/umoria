// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 能力値を 1 点失う lose_stat（src/world/player/stat_loss.c）のテスト
 *
 * 能力値ごとに違うのはメッセージ 2 つだけ。期待するメッセージは、1 本に
 * まとめる前の 6 本（lose_str〜lose_chr）の字面そのまま（STR と CON は同じ文）。
 *
 * dec_stat() は代役で、渡された能力値を記録する（本物の stat_ops.c は
 * calc_bonuses などを巻きこむ）。維持の有無は本物の player_abilities.c に、
 * 装備の p1 を渡して立てる。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "fixture.h"
#include "player_stats.h"
#include "shared_stubs.h"
#include "stat_loss.h"

#define MU_SETUP()                                                             \
    do {                                                                       \
        shared_stubs_reset();                                                  \
        player_abilities_forget_all();                                         \
        dec_stat_calls = 0;                                                    \
        dec_stat_last = -1;                                                    \
    } while (0)

static int dec_stat_calls = 0;
static int dec_stat_last = -1;

bool dec_stat(int stat)
{
    dec_stat_calls++;
    dec_stat_last = stat;
    return true;
}

#include "minunit.h"

static const char *const lost[6] = {
    "You feel very sick.",  "You become very dizzy.", "You feel very naive.",
    "You feel very sore.",  "You feel very sick.",    "Your skin starts to itch.",
};

static const char *const sustained[6] = {
    "You feel sick for a moment,  it passes.",
    "You become dizzy for a moment,  it passes.",
    "You feel naive for a moment,  it passes.",
    "You feel sore for a moment,  it passes.",
    "You feel sick for a moment,  it passes.",
    "Your skin starts to itch, but feels better now.",
};

/* 装備の p1 のうち、stat を維持させるもの（1〜6 を試す）。無ければ 0 */
static int sustain_p1_for(int stat)
{
    for (int p1 = 1; p1 <= 6; p1++) {
        player_abilities_forget_all();
        player_abilities_note_sustain(p1);
        if (player_stat_sustained(stat)) {
            return p1;
        }
    }
    return 0;
}

/* 維持していなければ、その能力値を 1 度だけ下げて、失った文を出す */
TEST(lose_stat_lowers_the_stat_and_says_so)
{
    int mismatch = 0;
    for (int stat = A_STR; stat <= A_CHR; stat++) {
        shared_stubs_reset();
        dec_stat_calls = 0;
        dec_stat_last = -1;
        lose_stat(stat);
        if (dec_stat_calls != 1 || dec_stat_last != stat || fixture_message_count() != 1 ||
            strcmp(fixture_message_text(0), lost[stat]) != 0) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 維持していれば下げず、すぐ治った文を出す */
TEST(lose_stat_spares_a_sustained_stat)
{
    int mismatch = 0;
    for (int stat = A_STR; stat <= A_CHR; stat++) {
        int p1 = sustain_p1_for(stat);
        player_abilities_forget_all();
        player_abilities_note_sustain(p1);
        shared_stubs_reset();
        dec_stat_calls = 0;
        lose_stat(stat);
        if (p1 == 0 || dec_stat_calls != 0 || fixture_message_count() != 1 ||
            strcmp(fixture_message_text(0), sustained[stat]) != 0) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* ほかの能力値の維持は効かない（腕力だけ維持していて知力を失う） */
TEST(lose_stat_ignores_the_sustain_of_another_stat)
{
    player_abilities_note_sustain(sustain_p1_for(A_STR));
    lose_stat(A_INT);
    ASSERT_TRUE(dec_stat_calls == 1 && dec_stat_last == A_INT);
}

int main(void)
{
    RUN_TEST(lose_stat_lowers_the_stat_and_says_so);
    RUN_TEST(lose_stat_spares_a_sustained_stat);
    RUN_TEST(lose_stat_ignores_the_sustain_of_another_stat);
    return TEST_SUMMARY();
}
