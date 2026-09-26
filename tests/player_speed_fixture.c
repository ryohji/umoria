/* いまの速さの置き場（テスト用の足場）
 *
 * src/player_speed.c が触るのは py.flags.speed の 1 個の short だけだが、
 * 置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、ここにも
 * 同じ形で置く。テストは variable.c も player.c もリンクしない —— どちらも
 * externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-11C でこのファイルは消える。** 1 個が src/player_speed.c の static に
 * 入ったら、ここで定義しても窓口には届かない別の器になるだけ（HANDOVER.md
 * 第 7 節。#18-6C2 以来くりかえしているつまずきで、#18-12-1C から #18-12-10C まで
 * 10 回続けて同じ形の足場を消している）。
 *
 * ただし py 自身は C の後も残る —— `struct flags` は 6 フィールドに減るだけで、
 * `struct misc` と `struct stats` はそのまま。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **ふつうの速さ（0 段）が人物の走りだし**で、本物の値は change_speed()
 * （moria1.c）が足し引きするか、save.c がファイルから読んだ short が置く。 */
player_type py;
