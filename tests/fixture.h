/* fixture.h -- テスト用のグローバル状態を操作する関数
 *
 * umoria は状態をグローバル変数で持つので、テストごとに条件を揃えないと
 * 先行テストの影響を受けて実行順で結果が変わる。ここで宣言する関数は
 * fixture.c にあり、本体（src/）には存在しない。
 *
 * 使い方: MU_SETUP から fixture_reset() を呼ぶ。
 *   #define MU_SETUP() fixture_reset()
 *   #include "minunit.h"
 */
#ifndef FIXTURE_H
#define FIXTURE_H

/* グローバル状態をまっさらに戻す。各テストの前に呼ぶ。 */
void fixture_reset(void);

/* randint() が返す値を固定する。乱数に依存するコードを
 * 再現可能にするため。fixture_reset() では変更しないので、
 * 値を変えたテストは自分で戻すか、次のテストで再設定する。 */
void fixture_set_randint(int value);

/* 直前の randint() に渡された上限を読みとる。misc3_stubs.c だけが提供する。
 * 戻り値を固定するだけでは、呼びだし側が渡した上限が正しいかを検証できない。
 * 配列の要素数を取りちがえても、返る値が同じなら気づけないので、
 * 上限そのものを観測できるようにしてある。 */
int fixture_randint_last_maxval(void);

/* randint() が呼ばれた回数。リファクタリングの前後で変わってはいけない
 * （回数が変わると乱数列がずれ、ゲーム全体のふるまいが変わる）。 */
int fixture_randint_call_count(void);

/* put_buffer() が書いた文字を読みとる。misc3_stubs.c だけが提供する
 * （fixture.c にはない）。画面に書くだけの関数のふるまいを、本体を
 * 変えずに観測するための窓口。fixture_reset() で記録は消える。 */
const char *fixture_screen_text(int row, int col);

/* msg_print() に渡された文字列を読みとる。misc3_stubs.c だけが提供する
 * （fixture.c にはない）。メッセージを表示するだけの関数のふるまいを、
 * 本体を変えずに観測するための窓口。fixture_reset() で記録は消える。 */
const char *fixture_message_text(int index);

/* msg_print() が呼ばれた回数。fixture_reset() で 0 に戻る。 */
int fixture_message_count(void);

#endif /* FIXTURE_H */
