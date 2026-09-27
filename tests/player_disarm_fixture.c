/* 罠と鍵をはずす腕の置き場（テスト用の足場）
 *
 * src/player_disarm.c が触るのは py.misc.disarm の short 1 本だけだが、
 * 置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、
 * ここにも同じ形で置く。テストは variable.c も player.c もリンクしない ——
 * どちらも externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-20C でこのファイルは消える。** 1 本が src/player_disarm.c の
 * static に入ったら、ここで定義しても窓口には届かない別の器になるだけ
 * （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずきで、
 * #18-12-1C から #18-12-19C まで 18 回続けて同じ形の足場を消している）。
 *
 * ただし py 自身は C の後も残る —— `struct misc` は 16 から 15 フィールドに
 * 減るだけで、`struct stats` はそのまま。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **腕 0 は「まだ種族を選んでいない」でもあり、ありうる答えでもある** ——
 * Human の種族の土台はちょうど 0 で、DEX が 8〜12 なら創成時の下駄も 0 なので、
 * 階級を選ぶ前の Human は本当に 0。 */
player_type py;
