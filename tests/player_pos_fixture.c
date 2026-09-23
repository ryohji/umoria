/* player_pos_fixture.c -- src/player_pos.c をリンクするための足場
 *
 * src/player_pos.c に未解決シンボルを列挙させると 2 個だけ残る:
 *   gcc -std=c17 -Isrc -Itests -c -o /tmp/pp.o src/player_pos.c
 *   gcc -o /tmp/t tests/player_pos_test.c /tmp/pp.o -Isrc -Itests \
 *     2>&1 | grep 'undefined reference'
 *   -> char_row と char_col の 2 個
 *
 * 本体ではこの 2 つは player.c（530 行の定数表と同居）にあるので、そこだけを
 * リンクすることはできない。テスト側で定義する。
 *
 * これはステップ A のあいだだけ要る。実体が player_pos.c の static になった
 * 時点でこのファイルは消える（inventory_fixture.c が辿ったのと同じ道）。
 * 消し忘れると「窓口越しの読み書きが届かない代役」が生き残るので、
 * ステップ C ではこのファイルの削除まで含めて 1 つの変更にする。
 *
 * 代役（スタブ）は 1 つもない。player_pos.c が誰も呼ばないからで、
 * それが窓口の狭さそのものである。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

int16_t char_row; /* 本体では player.c */
int16_t char_col; /* 本体では player.c */
