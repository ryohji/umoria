/* 「入力が尽きたか」の数の置き場（テスト用の足場）
 *
 * ステップ A の段では、実体はまだ src/variable.c にある。テストは variable.c を
 * リンクしない（externs.h の世界が丸ごと付いてくる）ので、同じ名前をここに
 * 1 つ置いて代わりにする。
 *
 * **#18-11-5C で実体が src/input_ended.c の static に入ったら、このファイルは
 * 消す。** 残すと、窓口越しの読み書きがこちらの器に当たってすり抜ける
 * （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずき）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は variable.c:103 に合わせる（= 0、つまりまだ尽きていない）。 */
int eof_flag = 0;
