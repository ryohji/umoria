/* player_light_test の足場 -- #18-7-3 のあいだだけ要るもの
 *
 * player_light の実体はまだ src/variable.c にある（ステップ A はふるまいを
 * 変えないので実体を動かさない）。src/variable.c を引くと 74 個のグローバル
 * 一式を抱えこむので、この 1 行だけを置く。
 *
 * ステップ C で実体が src/player_light.c の static になったら、このファイルは
 * 用が済む。**残すと窓口越しの読み書きが届かない器が生き残る**ので必ず消す
 * （HANDOVER.md 第 7 節。#18-6C2・#18-7-1C2・#18-7-2C2 と同じ）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

bool player_light;
