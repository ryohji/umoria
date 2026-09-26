/* 素の命中力の置き場（テスト用の足場）
 *
 * src/player_base_to_hit.c が触るのは py.misc.bth と py.misc.bthb の
 * short 2 本だけだが、置き場の実体は player_type の構造体まるごと
 * （src/player.c:17）なので、ここにも同じ形で置く。テストは variable.c も
 * player.c もリンクしない —— どちらも externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-19C でこのファイルは消える。** 2 本が
 * src/player_base_to_hit.c の static に入ったら、ここで定義しても窓口には
 * 届かない別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずきで、#18-12-1C から #18-12-18C まで 17 回続けて同じ形の足場を
 * 消している）。
 *
 * ただし py 自身は C の後も残る —— `struct misc` は 18 から 16 フィールドに
 * 減るだけで、`struct stats` はそのまま。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **素の命中力 0 は「まだ決まっていない」でもあり、ありうる答えでもある** ——
 * 種族を選ぶまでは 0 で、Human の種族の土台もちょうど 0（階級を選ぶと 70 や
 * 34 が足される）。守りの点数と同じで、0 に特別な意味は無い。 */
player_type py;
