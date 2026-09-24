/* 「この階は終わったか」の旗と、今いる深さの置き場（テスト用の足場）
 *
 * 旗のほうはステップ A の間だけ要る。実体は本番では variable.c の
 * new_level_flag で、src/level_exit.c の窓口はそれを指している。テストは
 * variable.c をリンクしない（externs.h の世界が丸ごと付いてくる）ので、同じ
 * 名前をここに 1 つ置いて代わりにする。
 *
 * **#18-11-4C で旗の実体が level_exit.c の static に入ったら、new_level_flag の
 * ほうは消す。** 残すと「窓口越しの読み書きが届かない器」が生き残り、テストは
 * 緑のまま保護が抜ける（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずき）。
 *
 * dun_level は話が別で、**C の後も残る**。これは別の区分（地下そのもの。
 * 44 参照）の global で、この単位で閉じる相手ではない。level_exit.c が触るのは
 * 「階を出る」が深さの置きかえと対で動くから（src/level_exit.h に書いてある）。
 * dun_level を閉じる人がこの行を引きとる。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は variable.c:100 に合わせる（あちらは初期値なしの bool なので false）。 */
bool new_level_flag = false;

/* 初期値は variable.c:63 に合わせる（= 0、つまり町）。 */
int16_t dun_level = 0;
