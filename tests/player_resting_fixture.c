/* 休息の残りターンの置き場（テスト用の足場）
 *
 * src/player_resting.c が触るのは py.flags.rest の 1 個の short だけだが、
 * 置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、ここにも
 * 同じ形で置く。テストは variable.c も player.c もリンクしない —— どちらも
 * externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-10C でこのファイルは消える。** 1 個が src/player_resting.c の
 * static に入ったら、ここで定義しても窓口には届かない別の器になるだけ
 * （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずきで、#18-12-1C から
 * #18-12-9C まで 9 回続けて同じ形の足場を消している）。
 *
 * ただし py 自身は C の後も残る —— `struct flags` は 7 フィールドに減るだけで、
 * `struct misc` と `struct stats` はそのまま。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **休んでいないのが人物の走りだし**で、本物の値は `R` コマンド（moria1.c の
 * rest()）か save.c がファイルから読んだ short が置く。 */
player_type py;
