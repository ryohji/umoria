/* 「teleport が待っているか」の旗の置き場（テスト用の足場）
 *
 * ステップ A の間だけ要る。実体は本番では variable.c の teleport_flag で、
 * src/pending_teleport.c の窓口はそれを指している。テストは variable.c を
 * リンクしない（externs.h の世界が丸ごと付いてくる）ので、同じ名前をここに
 * 1 つ置いて代わりにする。
 *
 * **#18-11-4C で実体が pending_teleport.c の static に入ったら、このファイルは
 * 消す。** 残すと「窓口越しの読み書きが届かない器」が生き残り、テストは緑の
 * まま保護が抜ける（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずき）。
 *
 * なお tests/misc3_stubs.c も同じ名前を定義している（misc3.c をリンクする
 * 実行形式が 9 本あり、teleport() が旗を消すため）。そちらも C で消す ——
 * 消す本数は「pending_teleport.c をリンクしている実行形式の数」で、B で
 * misc3.c が窓口を呼ぶようになると 9 本がそこに入ってくる。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は variable.c:101 に合わせる（あちらは初期値なしの bool なので false）。 */
bool teleport_flag = false;
