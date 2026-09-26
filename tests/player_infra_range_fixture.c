/* 赤外視の届く距離の置き場（テスト用の足場）
 *
 * src/player_infra_range.c が触るのは py.flags.see_infra の 1 個の short だけだが、
 * 置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、ここにも
 * 同じ形で置く。テストは variable.c も player.c もリンクしない —— どちらも
 * externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-12C でこのファイルは消える。** 1 個が src/player_infra_range.c の
 * static に入ったら、ここで定義しても窓口には届かない別の器になるだけ
 * （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずきで、#18-12-1C から
 * #18-12-11C まで 11 回続けて同じ形の足場を消している）。
 *
 * ただし py 自身は C の後も残る —— `struct flags` は 5 フィールドに減るだけで、
 * `struct misc` と `struct stats` はそのまま。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **0 ます（赤外視なし）は Human の走りだし**だが、**この問いでは 0 が
 * 「まだ決まっていない」ではない** —— 種族を選んだ時点で create.c が
 * r_ptr->infra を置く（Dwarf なら 5）。本物の値はそこか、装備の
 * py_bonuses()、時限の赤外視（dungeon.c の ±1）、セーブファイルの読みが置く。 */
player_type py;
