/* 値段表の置き場（テスト用の足場）
 *
 * **#18-12-6C で py が要らなくなった。** 階級と経験値の 5 つは
 * src/player_level.c の static になったので、ここで py を定義しても窓口には
 * 届かない別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2 以来くりかえして
 * いるつまずきで、#18-12-1C から #18-12-5C まで 5 回続けて同じ形の足場を
 * 消している）。テストは player_set_*() で 5 つを置く。
 *
 * **残るのは値段表 player_exp[] 1 つだけ。** 表は module の中に入れなかった
 * —— externs.h の「定数表（読みとり専用データ）」20 個の 1 つで、
 * player_title・race・class・class_level_adj と同じ区分にあり、その区分は
 * #18 の対象外（`const` 化のみ。GLOBALS_INVENTORY.md:748）。入れれば台帳の
 * global は 53 → 52 になるが、**本体の誰も書かない表に setter を付けること
 * になる** —— hp_table.c の表は人物を作るときに書くので setter に呼び手が
 * いるが、こちらは呼び手が 0 になり、誰も呼ばない窓口が 1 つ増える。
 *
 * だから本体では src/player.c:94 が 40 個の実の値を持ったままで、
 * この足場は src/player.c をリンクしないテストのために同じ器を 1 つ置く
 * （テストは段ごとに欲しい値を自分で入れるので、初期値は空のまま）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

uint32_t player_exp[MAX_PLAYER_LEVEL];
