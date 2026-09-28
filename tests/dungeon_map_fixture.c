// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* マスの表の置き場（テスト用の足場）
 *
 * #18-14-8A/B のあいだ、表の置き場は src/variable.c:138 にあって、
 * src/dungeon_map.c が `extern` 1 行で見にいく。テストは variable.c を
 * リンクしない —— externs.h のグローバルが丸ごと付いてくる —— ので、
 * 同じ名前をここに置いて代わりにする。
 *
 * **この 1 行は #18-14-8C で消える。** 置き場が src/dungeon_map.c の
 * static に入ると、ここに代役を置いても窓口に届かない別の表になるだけ
 * （#18-14-4・7 の足場が C で縮んだのと同じ）。**しかもこの問いでは
 * 足場が 0 になる** —— module が外に持つ依存が 1 つも無いので、7 問めの
 * invcopy() のように残るものが無い。ファイルごと消える。
 *
 * **なぜ A で置き場を動かさなかったか。** #18-14-1〜3・5・6 は A の段で
 * 置き場を module の static に入れた（所見 52）。あの形が使えるのは
 * 「B を 1 コミットで通しきれる」ときだけで、この問いは 15 ファイル
 * 258 参照なので B をファイルごとに割る。割るあいだ器が 2 つあると
 * **半分が古い地図を、半分が新しい地図を歩く**ので、置き場は最後まで
 * 1 つにしておく。この区分でこの形は 3 度め（4 問め・7 問め・この問い）。
 *
 * **走りだしの値をここで決めてはいけない。** テストの 1 件は
 * dungeon_map_reset() を呼ぶ前の表を見て「ゼロ初期化されている」ことを
 * 押さえる（tests/dungeon_map_test.c の最初の件）。ここで何か書きこむと
 * その件が意味を失うので、宣言だけにしてある。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

cave_type cave[MAX_HEIGHT][MAX_WIDTH];
