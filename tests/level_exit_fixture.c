// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 今いる深さの置き場（テスト用の足場）
 *
 * **これは「この階は終わったか」の旗の足場ではない。** 旗の実体は
 * #18-11-4C で src/level_exit.c の static に入ったので、ここには無い
 * （ここで定義しても窓口には届かない別の器になるだけ。HANDOVER.md 第 7 節。
 * #18-6C2 以来くりかえしているつまずき）。
 *
 * 残っているのは dun_level だけで、こちらは **C の後も残る**。別の区分
 * （地下そのもの。44 参照）の global で、この単位で閉じる相手ではない。
 * src/level_exit.c がこれを触るのは、「階を出る」が深さの置きかえと対で動く
 * から（src/level_exit.h に書いてある）。テストは variable.c をリンクしない
 * （externs.h の世界が丸ごと付いてくる）ので、同じ名前をここに 1 つ置いて
 * 代わりにする。
 *
 * **dun_level を閉じる人がこのファイルを消す。**
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は variable.c:63 に合わせる（= 0、つまり町）。 */
int16_t dun_level = 0;
