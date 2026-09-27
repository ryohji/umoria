/* 人物の器（テスト用の足場）
 *
 * **#18-12-24A のあいだだけ要る。** 置き場がまだ `py.misc.ptohit` と
 * `py.misc.ptodam` なので実体が 1 つ必要で、**#18-12-24C でこのファイルごと
 * 消える** —— 2 つの short が src/player_attack_bonuses.c の static に入ると、
 * ここで定義しても窓口には届かない別の器になるだけ（HANDOVER.md 第 7 節。
 * #18-6C2 以来くりかえしているつまずきで、#18-12-1C から #18-12-23C まで
 * 21 回続けて同じ形の足場を消している。例外は 21 つめだけで、あれは定数表
 * race[] の器も持っていたから残った）。
 *
 * **この問いは表に届かない** —— 下駄のもとは src/stats.c の補正表だが、
 * それを読むのは呼び手（`tohit_adj()` / `todam_adj()`）で、できた数を窓口に
 * 渡す。だから足場は 21 回続いた形のほうで、C で消える。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

player_type py;
