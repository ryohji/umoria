/* progress_fixture.c -- src/progress.c をリンクするための足場
 *
 * src/progress.c は窓口だけを持ち、記録の実体はまだ src/variable.c にある
 * （ステップ A なので呼びだし側を 1 箇所も変えていない＝ふるまい不変）。
 * リンカに聞くと未解決シンボルはその 5 個だけ:
 *   gcc -std=c17 -Isrc -Itests -c -o /tmp/progress.o src/progress.c
 *   gcc -o /tmp/t tests/progress_test.c /tmp/progress.o -Isrc -Itests \
 *     2>&1 | grep 'undefined reference'
 *   -> turn / randes_seed / town_seed / wizard / to_be_wizard
 *
 * variable.c そのものはリンクできない（externs.h の global 92 個ぶんの実体を
 * 抱えていて、芋づるで大半の module を引く）。5 個だけをここで定義する。
 * 初期値は variable.c:67-69 と同じにしてある——走りだしの状態を見るテストが
 * あるので、ここがずれると本体と違うものを固定してしまう。
 *
 * この足場はステップ C で消える予定。実体を progress.c の static に引きこめば
 * 未解決シンボルが 0 になり、inventory_test と同じ 2 単位・代役 0 になる。
 * 消し忘れると「窓口越しの読み書きが届かない代役」が生き残るので注意
 * （HANDOVER.md 第 7 節）。
 *
 * ここには代役（スタブ）が 1 つもない。progress.c が誰も呼ばないからで、
 * それが切りだしの成果そのものである。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 本体では variable.c。初期値もそこに合わせる。 */
int32_t turn = -1;       /* variable.c:67 */
uint32_t randes_seed;    /* variable.c:52 初期化なし＝0 */
uint32_t town_seed;      /* variable.c:53 初期化なし＝0 */
bool wizard = false;     /* variable.c:68 */
bool to_be_wizard = false; /* variable.c:69 */
