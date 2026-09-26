/* 装備で決まる耐性・能力の置き場（テスト用の足場）
 *
 * src/player_abilities.c が触るのは py.flags の 17 バイトだけだが、置き場の
 * 実体は player_type の構造体まるごと（src/player.c:17）なので、ここにも同じ
 * 形で置く。テストは variable.c も player.c もリンクしない —— どちらも
 * externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-8C でこのファイルは消える。** 17 個が src/player_abilities.c の
 * static に入ったら、ここで定義しても窓口には届かない別の器になるだけ
 * （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずきで、#18-12-1C から
 * #18-12-7C まで 7 回続けて同じ形の足場を消している）。
 *
 * ただし py 自身は C の後も残る —— 問いはまだ 60 個以上あって、この単位で
 * 閉じる相手は「装備で決まる耐性・能力」17 個だけ。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **何もできない・何にも耐えないのが人物の走りだし**で、本物の値は装備を
 * 身につけたときに calc_bonuses() が、あるいは save.c がファイルから
 * 17 バイト読んで置く。 */
player_type py;
