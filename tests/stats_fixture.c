// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* stats_fixture.c -- src/player/stats.c をリンクするための足場
 *
 * src/player/stats.c を切りだしたあと、リンカに未解決シンボルを列挙させたら 2 個
 * しか残らなかった:
 *   gcc -std=c17 -Isrc -Itests -c -o /tmp/stats.o src/stats.c
 *   gcc -o /tmp/t tests/stat_bonus_test.c /tmp/stats.o -Isrc -Itests \
 *     2>&1 | grep 'undefined reference'
 *   -> fixture_reset だけ
 *
 * fixture_reset は本体には存在しないテスト専用の関数（fixture.h の窓口）。
 *
 * fixture.c ではなくこれを使う理由: fixture.c は
 * 画面描画・乱数・インベントリの代役や実体を抱えていて、能力値の補正表とは
 * 何の関係もない module（desc.c / tables.c / treasure.c / player.c）を
 * 芋づるで引っぱってくる。補正表が読むのは使う能力値（player_stat_use）だけなので、
 * それだけを用意した足場のほうが「この module は何に依存しているか」を
 * 正しく表す。
 *
 * ここには代役（スタブ）が 1 つもない。stats.c が誰も呼ばないからで、
 * それが切りだしの成果そのものである。
 */
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"
#include "stats.h"
#include "stats_reset.h"

/* テスト専用。オリジナルには存在しない。
 * setUp から呼ぶことで、先行テストの影響を受けない条件を作る。 */
void fixture_reset(void)
{
    stats_reset();
}
