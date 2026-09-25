/* 画面に出す 4 つの数の置き場（テスト用の足場）
 *
 * src/player_display_numbers.c が触るのは py.misc.dis_th・dis_td・dis_tac・
 * dis_ac の 4 つだけだが、実体は player_type の構造体まるごと
 * （src/player.c:17）なので、ここにも同じ形で 1 つ置く。テストは variable.c も
 * player.c もリンクしない —— どちらも externs.h の世界か大きな表が丸ごと
 * 付いてくる。
 *
 * **#18-12-3C でこのファイルは消える。** 4 つの置き場が
 * src/player_display_numbers.c の static に入ったら、ここで定義しても窓口には
 * 届かない別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずき。#18-12-1C・#18-12-2C でも同じ形の足場を消している）。
 *
 * ただし py 自身は C の後も残る —— 問いはまだ 80 個近くあって、この単位で
 * 閉じる相手は「画面に出す数字」だけ。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0。
 * 4 つとも 0 で、本物の値は create.c が人物を作りおわってから、
 * あるいは save.c がファイルから読んで置く）。 */
player_type py;
