/* score_death_fixture.c -- src/score_death.c をリンクするための足場
 *
 * src/score_death.c は窓口だけを持ち、記録の実体はまだ src/variable.c にある
 * （ステップ A なので呼びだし側を 1 箇所も変えていない＝ふるまい不変）。
 * リンカに聞くと未解決シンボルはその 4 個だけ:
 *   gcc -std=c17 -Isrc -Itests -c -o /tmp/sd.o src/score_death.c
 *   gcc -o /tmp/t tests/score_death_test.c /tmp/sd.o -Isrc -Itests \
 *     2>&1 | grep 'undefined reference'
 *   -> death / died_from / birth_date / noscore
 *
 * variable.c そのものはリンクできない（global 92 個ぶんの実体を抱えていて
 * 芋づるで大半の module を引く）。4 個だけをここで定義する。初期値は
 * variable.c:42-43,60,71 と同じにしてある——走りだしの状態を見るテストが
 * あるので、ここがずれると本体と違うものを固定してしまう。
 *
 * この足場はステップ C で消える予定（progress_fixture.c と同じ）。消し忘れると
 * 「窓口越しの読み書きが届かない代役」が生き残るので注意（HANDOVER.md 第 7 節）。
 *
 * ここには代役（スタブ）が 1 つもない。score_death.c が誰も呼ばないからで、
 * それが切りだしの成果そのものである。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 本体では variable.c。初期値もそこに合わせる。 */
bool death = false;  /* variable.c:60 */
vtype died_from;     /* variable.c:42 初期化なし＝全バイト 0 */
int32_t birth_date;  /* variable.c:43 初期化なし＝0 */
int16_t noscore = 0; /* variable.c:71 */
