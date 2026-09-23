/* burden_test の足場 -- #18-7-4 のあいだだけ要るもの
 *
 * weapon_heavy と pack_heavy の実体はまだ src/variable.c にある（ステップ A は
 * ふるまいを変えないので実体を動かさない）。src/variable.c を引くと 73 個の
 * グローバル一式を抱えこむので、この 2 行だけを置く。初期値も variable.c と
 * 同じものを書く（あちらは false と 0 を明示している）。
 *
 * ステップ C で実体が src/burden.c の static になったら、このファイルは用が
 * 済む。**残すと窓口越しの読み書きが届かない器が生き残る**ので必ず消す
 * （HANDOVER.md 第 7 節。#18-6C2・#18-7-1C2・#18-7-2C2・#18-7-3C2 と同じ）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

bool weapon_heavy = false;
int pack_heavy = 0;
