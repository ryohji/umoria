// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* hit_rolls_test の足場：disturb() の代役
 *
 * 本物の disturb()（world/player/rest_command.c）は画面と走りの状態を巻きこむ。
 * test_hit() が毎回 1 度呼ぶことだけを見たいので、回数を数える。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "hit_rolls_fixture.h"

static int disturb_calls = 0;

void disturb(int stop_search, int flush_input)
{
    (void)stop_search;
    (void)flush_input;
    disturb_calls++;
}

int fixture_disturb_count(void) { return disturb_calls; }

void fixture_disturb_reset(void) { disturb_calls = 0; }
