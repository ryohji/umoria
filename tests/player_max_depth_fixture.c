/* どこまで潜ったかの置き場（テスト用の足場）
 *
 * src/player_max_depth.c が触るのは py.misc.max_dlv の 1 つの short だけだが、
 * 置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、
 * ここにも同じ形で置く。テストは variable.c も player.c もリンクしない ——
 * どちらも externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-16C でこのファイルは消える。** short が
 * src/player_max_depth.c の static に入ったら、ここで定義しても窓口には
 * 届かない別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずきで、#18-12-1C から #18-12-14C まで 14 回続けて同じ形の足場を
 * 消している）。
 *
 * ただし py 自身は C の後も残る —— `struct misc` は 22 から 21 フィールドに
 * 減るだけで、`struct stats` はそのまま。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **0 は「町から下りたことがない」** —— 作られたばかりの人物はここにいる。 */
player_type py;
