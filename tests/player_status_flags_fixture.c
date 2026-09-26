/* 状態の旗の置き場（テスト用の足場）
 *
 * src/player_status_flags.c が触るのは py.flags.status の 1 語だけだが、
 * 置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、
 * ここにも同じ形で置く。テストは variable.c も player.c もリンクしない
 * —— どちらも externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-7C でこのファイルは消える。** 1 語が
 * src/player_status_flags.c の static に入ったら、ここで定義しても窓口には
 * 届かない別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2 以来くりかえして
 * いるつまずきで、#18-12-1C から #18-12-6C まで 6 回続けて同じ形の足場を
 * 消している）。
 *
 * ただし py 自身は C の後も残る —— 問いはまだ 70 個近くあって、この単位で
 * 閉じる相手は「状態の旗」だけ。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **旗が 1 つも立っていない状態が人物の走りだし**で、本物の値は create.c が
 * 人物を作るときに、あるいは save.c がファイルから 1 語まるごと読んで置く。 */
player_type py;
