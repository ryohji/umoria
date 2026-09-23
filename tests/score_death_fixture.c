/* score_death_test の足場 -- #18-7-1 のあいだだけ要るもの
 *
 * total_winner と max_score の実体はまだ src/variable.c にある（ステップ A は
 * ふるまいを変えないので実体を動かさない）。src/variable.c を引くとテストが
 * externs.h ごと全部を抱えこむので、この 2 行だけを置く。
 *
 * ステップ C で実体が src/score_death.c の static になったら、このファイルは
 * 用が済む。**残すと窓口越しの読み書きが届かない器が生き残る**ので必ず消す
 * （HANDOVER.md 第 7 節。#18-6C2 で player_pos_fixture.c を消したのと同じ）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

bool total_winner = false;
int32_t max_score = 0;
