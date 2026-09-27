/* 抵抗の置き場（テスト用の足場）
 *
 * src/player_saving_throw.c が触るのは py.misc.save の short 1 本だけだが、
 * 置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、
 * ここにも同じ形で置く。テストは variable.c も player.c もリンクしない ——
 * どちらも externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-21C でこのファイルは消える。** 1 本が src/player_saving_throw.c の
 * static に入ったら、ここで定義しても窓口には届かない別の器になるだけ
 * （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずきで、
 * #18-12-1C から #18-12-20C まで 19 回続けて同じ形の足場を消している）。
 *
 * ただし py 自身は C の後も残る —— `struct misc` は 15 から 14 フィールドに
 * 減るだけで、`struct stats` はそのまま。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **抵抗 0 は「まだ種族を選んでいない」でもあり、ありうる答えでもある** ——
 * Human の種族の土台はちょうど 0 なので、階級を選ぶ前の Human は本当に 0。
 * 罠と鍵をはずす腕（#18-12-20）と違い、創成時に足される下駄が無いので、
 * 「0 から始まる」と言えるのは種族が Human のときだけではなく置きなおす前の
 * どの人物でも同じ。 */
player_type py;
