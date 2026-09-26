/* 守りの点数の置き場（テスト用の足場）
 *
 * src/player_armour_class.c が触るのは py.misc.pac と py.misc.ptoac の
 * short 2 本だけだが、置き場の実体は player_type の構造体まるごと
 * （src/player.c:17）なので、ここにも同じ形で置く。テストは variable.c も
 * player.c もリンクしない —— どちらも externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-18C でこのファイルは消える。** 2 本が
 * src/player_armour_class.c の static に入ったら、ここで定義しても窓口には
 * 届かない別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずきで、#18-12-1C から #18-12-17C まで 16 回続けて同じ形の足場を
 * 消している）。
 *
 * ただし py 自身は C の後も残る —— `struct misc` は 20 から 18 フィールドに
 * 減るだけで、`struct stats` はそのまま。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **守りの点数 0 は遊びのなかにありうる** —— 何も着ていない敏捷さ 7〜14 の
 * 人物がちょうど 0 で、この問いは 0 が「まだ決まっていない」を意味しない
 * 初めての単位（骰子の 0 面や深さの 0 階とちがう）。 */
player_type py;
