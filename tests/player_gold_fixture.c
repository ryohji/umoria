/* 持ちものの記録の置き場（テスト用の足場）
 *
 * src/player_gold.c が触るのは py.misc.au の 1 つだけだが、実体は
 * player_type の構造体まるごと（src/player.c:17）なので、ここにも同じ形で
 * 1 つ置く。テストは variable.c も player.c もリンクしない —— どちらも
 * externs.h の世界か大きな表が丸ごと付いてくる。
 *
 * **#18-12-1C でこのファイルは消える。** 金の置き場が
 * src/player_gold.c の static に入ったら、ここで定義しても窓口には届かない
 * 別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずき）。
 *
 * ただし py 自身は C の後も残る —— 問いはまだ 80 個以上あって、この単位で
 * 閉じる相手は「持っている金」だけ。**次の問いを切る人がこの足場を使い、
 * py が空になった人がこのファイルを消す。**
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0。
 * 金も 0 で、走りだしの財布は空）。 */
player_type py;
