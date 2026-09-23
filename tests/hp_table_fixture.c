/* hp_table_test の足場 -- #18-7-2 のあいだだけ要るもの
 *
 * player_hp の実体はまだ src/player.c にある（ステップ A はふるまいを変えないので
 * 実体を動かさない）。src/player.c を引くと py ほか一式を抱えこむので、この 1 行
 * だけを置く。
 *
 * ステップ C で実体が src/hp_table.c の static になったら、このファイルは用が
 * 済む。**残すと窓口越しの読み書きが届かない器が生き残る**ので必ず消す
 * （HANDOVER.md 第 7 節。#18-6C2・#18-7-1C2 と同じ）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

uint16_t player_hp[MAX_PLAYER_LEVEL];
