// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 種族の表（テスト用の足場）
 *
 * **この足場は 2 つのものを持っていて、#18-12-22C で片方だけが消えた。**
 *
 * 1. py —— **消えた。** A のあいだは置き場が py.misc.prace のままだったので
 *    実体が要ったが、1 バイトが src/player_race.c の static に入った今は、
 *    ここで定義しても窓口には届かない別の器になるだけ
 *    （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずきで、
 *    #18-12-1C から #18-12-21C まで 20 回続けて同じ形の足場を消している）。
 *
 * 2. race[] —— **こちらは残る。** 表は module の中に入れなかった
 *    （externs.h の「定数表（読みとり専用データ）」20 個の 1 つで、その区分は
 *    #18 の対象外。`const` 化のみ。GLOBALS_INVENTORY.md:748）。本体では
 *    src/player.c:107 が 8 行の実の値を持ったままで、src/player_race.c は
 *    extern 1 行で届く —— src/player_level.c が player_exp[] に対してしている
 *    のと同じ形で、その足場（tests/player_level_fixture.c）も C の後は表だけを
 *    残している。
 *
 * **つまりこの単位は「C で足場ごと消えない」最初の単位になった。** 16 つめから
 * 21 つめまでの 6 つは足場がまるごと消えていて、makefile.test の recipe から
 * 行が 1 本消えるのが C の目印だった。**今回は recipe が 1 行も動かない** ——
 * 足場は同じ名前のまま中身が半分になる。
 *
 * 表は空のまま置く（初期化子なし = trace が全部 NULL）。名前を読む件は
 * 自分で trace を入れてから読む —— 本物の 8 つの名前を写すと、
 * src/player.c が変わったときに足場だけが古くなって気づけない。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* **#18-12-22C で py の器は消えた。** 1 バイトは src/player_race.c の static に
 * なったので、ここで定義しても窓口には届かない別の器になるだけ。
 * テストは player_race_set() で行番号を置く。
 *
 * **残るのは種族の表 race[] 1 つだけ** —— tests/player_level_fixture.c が
 * player_exp[] を残しているのとまったく同じ形。名前だけを入れて使う
 * （値は件ごとに入れる）。 */
race_type race[MAX_RACES];
