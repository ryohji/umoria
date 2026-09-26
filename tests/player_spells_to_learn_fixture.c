/* あと何個呪文を覚えられるかの置き場（テスト用の足場）
 *
 * src/player_spells_to_learn.c が触るのは py.flags.new_spells の 1 バイトだけ
 * だが、置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、
 * ここにも同じ形で置く。テストは variable.c も player.c もリンクしない ——
 * どちらも externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-14C でこのファイルは消える。** 1 バイトが
 * src/player_spells_to_learn.c の static に入ったら、ここで定義しても窓口には
 * 届かない別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずきで、#18-12-1C から #18-12-13C まで 13 回続けて同じ形の足場を
 * 消している）。
 *
 * ただし py 自身は C の後も残る —— `struct flags` は 3 フィールドに減るだけで
 * （しかも残る 3 つはどれも死んでいる）、`struct misc` と `struct stats` は
 * そのまま。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **0 は「いま覚えられる呪文は無い」** —— 戦士はずっと 0 で、魔法使いも
 * calc_spells() が最初のレベルで数を置くまで 0。 */
player_type py;
