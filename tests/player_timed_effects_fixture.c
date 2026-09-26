/* 一時的な状態の数えおとしの置き場（テスト用の足場）
 *
 * src/player_timed_effects.c が触るのは py.flags の 18 個の short だけだが、
 * 置き場の実体は player_type の構造体まるごと（src/player.c:17）なので、ここにも
 * 同じ形で置く。テストは variable.c も player.c もリンクしない —— どちらも
 * externs.h の世界が丸ごと付いてくる。
 *
 * **#18-12-9C でこのファイルは消える。** 18 個が src/player_timed_effects.c の
 * static に入ったら、ここで定義しても窓口には届かない別の器になるだけ
 * （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずきで、#18-12-1C から
 * #18-12-8C まで 8 回続けて同じ形の足場を消している）。
 *
 * ただし py 自身は C の後も残る —— `struct flags` は空になるが `struct misc` と
 * `struct stats` はそのままで、問いはまだ 20 個以上ある。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0）。
 * **どの一時状態も効いていないのが人物の走りだし**で、本物の値は薬や罠や
 * モンスターが足すか、save.c がファイルから 18 個の short を読んで置く。 */
player_type py;
