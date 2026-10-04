// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* stats_reset.h -- 能力値の 4 配列（src/player/stats.c の static）を窓口ごしに 0 へ戻す。
 * 足場の fixture_reset() から呼ぶ。先に config.h・constant.h・types.h を include すること。 */
#ifndef STATS_RESET_H
#define STATS_RESET_H

#include "stats.h"

static inline void stats_reset(void)
{
    for (int i = 0; i < 6; i++) {
        player_stat_set_max(i, 0);
        player_stat_set_cur(i, 0);
        player_stat_set_mod(i, 0);
        player_stat_set_use(i, 0);
    }
}

#endif /* STATS_RESET_H */
