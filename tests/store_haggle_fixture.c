// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* store_haggle_fixture.c -- src/store/store_haggle.c を試すテストの足場
 * （haggle_comment_test）
 *
 * fixture_reset() は、リンクされる窓口の状態と、代役（shared_stubs.c）の
 * 記録を 0 に戻す。
 */

#include <stddef.h>
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"
#include "shared_stubs.h"
#include "player_race.h"
#include "stats_reset.h"

/* 各テストの前に呼ぶ。 */
void fixture_reset(void)
{
    stats_reset();
    player_race_set(0);
    shared_stubs_reset();
}
