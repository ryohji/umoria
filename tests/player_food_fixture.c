/* 持ちものの記録の置き場（テスト用の足場）
 *
 * src/player_food.c が触るのは py.flags.food と py.flags.food_digested の
 * 2 つだけだが、実体は player_type の構造体まるごと（src/player.c:17）なので、
 * ここにも同じ形で 1 つ置く。テストは variable.c も player.c もリンクしない
 * —— どちらも externs.h の世界か大きな表が丸ごと付いてくる。
 *
 * **#18-12-2C でこのファイルは消える。** 腹の置き場が src/player_food.c の
 * static に入ったら、ここで定義しても窓口には届かない別の器になるだけ
 * （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずき。#18-12-1C でも
 * 同じ形の tests/player_gold_fixture.c を消している）。
 *
 * ただし py 自身は C の後も残る —— 問いはまだ 80 個以上あって、この単位で
 * 閉じる相手は「腹の具合」だけ。**次の問いを切る人がこの足場を使い、
 * py が空になった人がこのファイルを消す。**
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0。
 * 腹も 0 で、消化の速さも 0。本物の 7500 と 2 は main.c が
 * 人物を作りおわってから置く）。 */
player_type py;
