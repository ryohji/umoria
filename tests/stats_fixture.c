/* stats_fixture.c -- src/stats.c をリンクするための足場
 *
 * src/stats.c を切りだしたあと、リンカに未解決シンボルを列挙させたら 2 個
 * しか残らなかった:
 *   gcc -std=c17 -Isrc -Itests -c -o /tmp/stats.o src/stats.c
 *   gcc -o /tmp/t tests/stat_bonus_test.c /tmp/stats.o -Isrc -Itests \
 *     2>&1 | grep 'undefined reference'
 *   -> py と fixture_reset の 2 個
 *
 * py は本体では player.c（530 行の巨大な定数表と同居）にあるので、そこだけを
 * リンクすることはできない。テスト側で定義する。
 * fixture_reset は本体には存在しないテスト専用の関数（fixture.h の窓口）。
 *
 * misc3_stubs.c / fixture.c ではなくこれを使う理由: どちらも py のほかに
 * 画面描画・乱数・インベントリの代役や実体を抱えていて、能力値の補正表とは
 * 何の関係もない module（desc.c / tables.c / treasure.c / player.c）を
 * 芋づるで引っぱってくる。補正表が読むのは py.stats.use_stat だけなので、
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

player_type py; /* 本体では player.c */

/* テスト専用。オリジナルには存在しない。
 * setUp から呼ぶことで、先行テストの影響を受けない条件を作る。 */
void fixture_reset(void)
{
    memset(&py, 0, sizeof py);
}
