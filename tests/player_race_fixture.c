/* 種族の置き場と、種族の表（テスト用の足場）
 *
 * **この足場は 2 つのものを持つ。C で消えるのは片方だけ。**
 *
 * 1. py —— src/player_race.c が触るのは py.misc.prace の 1 バイトだけだが、
 *    置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、
 *    ここにも同じ形で置く。テストは variable.c も player.c もリンクしない ——
 *    どちらも externs.h の世界が丸ごと付いてくる。
 *    **#18-12-22C でこの器は消える。** 1 バイトが src/player_race.c の static に
 *    入ったら、ここで定義しても窓口には届かない別の器になるだけ
 *    （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずきで、
 *    #18-12-1C から #18-12-21C まで 20 回続けて同じ形の足場を消している）。
 *
 * 2. race[] —— **こちらは C の後も残る。** 表は module の中に入れなかった
 *    （externs.h の「定数表（読みとり専用データ）」20 個の 1 つで、その区分は
 *    #18 の対象外。`const` 化のみ。GLOBALS_INVENTORY.md:748）。本体では
 *    src/player.c:107 が 8 行の実の値を持ったままで、src/player_race.c は
 *    extern 1 行で届く —— src/player_level.c が player_exp[] に対してしている
 *    のと同じ形で、その足場（tests/player_level_fixture.c）も C の後は表だけを
 *    残している。
 *
 * **つまりこの単位は「C で足場ごと消えない」最初の単位。** 16 つめから
 * 21 つめまでの 6 つは足場がまるごと消えていた。
 *
 * 表は空のまま置く（初期化子なし = trace が全部 NULL）。名前を読む件は
 * 自分で trace を入れてから読む —— 本物の 8 つの名前を写すと、
 * src/player.c が変わったときに足場だけが古くなって気づけない。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **行番号 0 は「まだ選んでいない」でもあり、ありうる答えでもある** ——
 * race[] の 0 行めは Human なので、創成前の器は「Human」と読める。
 * 抵抗（#18-12-21）の 0 と同じ性質で、赤外視（#18-12-12）とは違う。 */
player_type py;

/* 名前だけを入れて使う（値は件ごとに入れる）。 */
race_type race[MAX_RACES];
