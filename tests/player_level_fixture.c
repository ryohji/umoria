/* 階級と経験値の置き場（テスト用の足場）
 *
 * src/player_level.c が触るのは py.misc.lev・exp・max_exp・exp_frac・
 * expfact の 5 つと、値段表 player_exp[] だが、置き場の実体は
 * player_type の構造体まるごと（src/player.c:17）と 40 個の表
 * （src/player.c:94）なので、ここにも同じ形で置く。テストは
 * variable.c も player.c もリンクしない —— どちらも externs.h の世界か
 * 大きな表が丸ごと付いてくる。
 *
 * **#18-12-6C でこのファイルは消える。** 5 つの置き場が
 * src/player_level.c の static に入ったら、ここで定義しても窓口には届かない
 * 別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずきで、#18-12-1C から #18-12-5C まで 5 回続けて同じ形の足場を
 * 消している）。
 *
 * **値段表 player_exp[] をどうするかは C の段で決める。** 読み手 5 か所が
 * すべてこの問いの中にあるので、`src/hp_table.c` がレベルごとの HP 表を
 * 引きとったのと同じように module の中へ入れられる（台帳の global が
 * 53 → 52 になる、この道では初めての動き）。A の段では本体の
 * `src/player.c` が持っているままなので、ここでも同じ形で 1 つ置く。
 *
 * ただし py 自身は C の後も残る —— 問いはまだ 70 個以上あって、この単位で
 * 閉じる相手は「階級と経験値」だけ。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は src/player.c:17 に合わせる（初期化子なし = 全部 0。5 つとも 0 で、
 * 本物の値は create.c が人物を作るときに、あるいは save.c がファイルから
 * 読んで置く）。**階級 0 は人物がまだ居ないことの印**で、そのあいだは
 * 値段の計算（player_exp[lev - 1]）も経験値の分けまえ（÷ lev）も
 * 成りたたない —— 本体でも create.c が lev = 1 を入れるまで誰も呼ばない。 */
player_type py;

/* 値段表。本体では src/player.c:94 に 40 個の実の値が入っているが、
 * テストは段ごとに欲しい値を自分で置くので、ここでは空（全部 0）にする。 */
uint32_t player_exp[MAX_PLAYER_LEVEL];
