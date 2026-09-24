/* 「再開する持ち物コマンド」の置き場（テスト用の足場）
 *
 * ステップ A の間だけ要る。実体は本番では variable.c の doing_inven で、
 * src/inven_command_state.c の窓口はそれを指している。テストは variable.c を
 * リンクしない（externs.h の世界が丸ごと付いてくる）ので、同じ名前をここに
 * 1 つ置いて代わりにする。
 *
 * **#18-11-3C で実体が inven_command_state.c の static に入ったら、この
 * ファイルは消す。** 残すと「窓口越しの読み書きが届かない器」が生き残り、
 * テストは緑のまま保護が抜ける（HANDOVER.md 第 7 節）。
 *
 * なお tests/haggle_comment_test.c も同じ名前を定義している（store2.c を
 * 丸ごと #include するため）。そちらも C で消す。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は variable.c:95 に合わせる（あちらは = 0 と書いてある）。 */
char doing_inven = 0;
