/* 光る手の置き場（テスト用の足場）
 *
 * src/player_glowing_hands.c が触るのは py.flags.confuse_monster の 1 バイトだけ
 * だが、置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、
 * ここにも同じ形で置く。テストは variable.c も player.c もリンクしない ——
 * どちらも externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-13C でこのファイルは消える。** 1 バイトが
 * src/player_glowing_hands.c の static に入ったら、ここで定義しても窓口には
 * 届かない別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずきで、#18-12-1C から #18-12-12C まで 12 回続けて同じ形の足場を
 * 消している）。
 *
 * ただし py 自身は C の後も残る —— `struct flags` は 4 フィールドに減るだけで、
 * `struct misc` と `struct stats` はそのまま。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **0 は「手が光っていない」で、これが走りだし** —— 種族もセーブの読みも
 * 関係なく、巻物 11 を読むまで 0 のまま。 */
player_type py;
