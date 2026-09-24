/* 「画面が流されたか」の旗の置き場（テスト用の足場）
 *
 * ステップ A の間だけ要る。実体は本番では variable.c の screen_change で、
 * src/screen_touched.c の窓口はそれを指している。テストは variable.c を
 * リンクしない（externs.h の世界が丸ごと付いてくる）ので、同じ名前をここに
 * 1 つ置いて代わりにする。
 *
 * **#18-11-3C で実体が screen_touched.c の static に入ったら、このファイルは
 * 消す。** 残すと「窓口越しの読み書きが届かない器」が生き残り、テストは
 * 緑のまま保護が抜ける（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずき）。消す本数は「screen_touched.c をリンクしている実行形式の数」。
 *
 * なお tests/creature_stubs.c も同じ名前を定義している（movement_rate_test が
 * creature.c を丸ごと #include するため）。そちらも C で消す。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は variable.c:97 に合わせる（あちらは = false と書いてある）。 */
bool screen_change = false;
