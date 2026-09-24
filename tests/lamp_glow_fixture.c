/* 明かりの輪が描かれているかの旗の置き場（テスト用の足場）
 *
 * ステップ A の間だけ要る。実体は本番では variable.c の light_flag で、
 * src/player_light.c の窓口はそれを指している。player_light_test は
 * variable.c をリンクしない（externs.h の世界が丸ごと付いてくる）ので、
 * 同じ名前をここに 1 つ置いて代わりにする。
 *
 * **#18-11-1C で実体が player_light.c の static に入ったら、このファイルは
 * 消す。** 残すと「窓口越しの読み書きが届かない器」が生き残り、テストは
 * 緑のまま保護が抜ける（HANDOVER.md 第 7 節。#18-6C2 以来くりかえしている
 * つまずき）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は書かない。variable.c の light_flag も書いていない（false から
 * 始まり、最初の move_light() が入れなおす）。 */
bool light_flag;
