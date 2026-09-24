/* 飛び道具の通し番号の置き場（テスト用の足場）
 *
 * ステップ A の間だけ要る。実体は本番では variable.c の missile_ctr で、
 * src/missile_serial.c の窓口はそれを指している。missile_serial_test は
 * variable.c をリンクしない（externs.h の世界が丸ごと付いてくる）ので、
 * 同じ名前をここに 1 つ置いて代わりにする。
 *
 * **#18-11-2C で実体が missile_serial.c の static に入ったら、このファイルは
 * 消す。** 残すと「窓口越しの読み書きが届かない器」が生き残り、テストは
 * 緑のまま保護が抜ける（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずき）。消す本数は「missile_serial.c をリンクしている実行形式の数」で、
 * いまは 1 本だけ（#18-11-1 では 2 本あった）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は variable.c:64 に合わせる（あちらは = 0 と書いてある）。 */
int16_t missile_ctr = 0;
