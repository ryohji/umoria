/* 「打っているコマンドについて覚えていること」の置き場（テスト用の足場）
 *
 * ステップ A の段では、実体はまだ src/variable.c にある。テストは variable.c を
 * リンクしない（externs.h の世界が丸ごと付いてくる）ので、同じ名前をここに
 * 3 つ置いて代わりにする。
 *
 * **#18-11-7C で実体が src/command_state.c の static に入ったら、このファイルは
 * 消す。** 残すと、窓口越しの読み書きがこちらの器に当たってすり抜ける
 * （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずき）。
 * **同じ理由で tests/misc3_stubs.c の代役 3 個も C で消す**（misc3.c を引く
 * 9 本が src/command_state.c をリンクし、その 3 つを使っている）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は variable.c:75,76,99 に合わせる（繰りかえし無し・向きは訊く・
 * 前のコマンドは空白）。 */
int command_count;
bool default_dir = false;
char last_command = ' ';
