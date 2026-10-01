// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

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

#include <stdbool.h>

struct treasure_type;

/* グローバル状態をまっさらに戻す。各テストの前に呼ぶ。 */
void fixture_reset(void);

/* randint() が返す値を固定する。乱数に依存するコードを
 * 再現可能にするため。fixture_reset() では変更しないので、
 * 値を変えたテストは自分で戻すか、次のテストで再設定する。 */
void fixture_set_randint(int value);

/* 直前の randint() に渡された上限を読みとる。
 * 戻り値を固定するだけでは、呼びだし側が渡した上限が正しいかを検証できない。
 * 配列の要素数を取りちがえても、返る値が同じなら気づけないので、
 * 上限そのものを観測できるようにしてある。 */
int fixture_randint_last_maxval(void);

/* randint() が呼ばれた回数。リファクタリングの前後で変わってはいけない
 * （回数が変わると乱数列がずれ、ゲーム全体のふるまいが変わる）。 */
int fixture_randint_call_count(void);

/* put_buffer() が書いた文字を読みとる。tests/shared_stubs.c が
 * 提供する（fixture.c にはない）。画面に書くだけの関数のふるまいを、
 * 本体を変えずに観測するための窓口。fixture_reset() で記録は消える。 */
const char *fixture_screen_text(int row, int col);

/* msg_print() に渡された文字列を読みとる。tests/shared_stubs.c が
 * 提供する（fixture.c にはない）。メッセージを表示するだけの関数の
 * ふるまいを、本体を変えずに観測するための窓口。fixture_reset() で
 * 記録は消える。 */
const char *fixture_message_text(int index);

/* msg_print() が呼ばれた回数。fixture_reset() で 0 に戻る。 */
int fixture_message_count(void);

/* 最後に change_speed() へ渡された段数の差。tests/shared_stubs.c が
 * 提供する（fixture.c にはない）。check_strength() は速度を
 * change_speed(新しい段数 − 覚えた段数) という**差**で動かすので、
 * 符号と大きさが観測できなければ「遅くなった」と「速くなった」を
 * 区別できない。fixture_reset() で 0 に戻る。 */
int fixture_speed_change_last_steps(void);

/* change_speed() が呼ばれた回数。差が 0 のときに呼ばないことを見るために要る
 * （最後の値だけでは、呼ばれていないのと 0 を渡されたのが区別できない）。 */
int fixture_speed_change_count(void);

/* calc_bonuses() が呼ばれた回数。武器が重すぎるかどうかが変わったときに
 * 能力の再計算が走ることを見る。tests/shared_stubs.c が提供する。 */
int fixture_calc_bonuses_count(void);

/* get_com() が返すキーを前もって並べておく。tests/shared_stubs.c が
 * 提供する（fixture.c にはない）。既定では get_com() は
 * 「押されなかった」（0）を返すので、キーを待つ繰りかえし
 * （gain_spells の「どの呪文を学ぶ？」）に入れない。
 * 並べたキーを使いきると 0 に戻るので、繰りかえしはそこで終わる。
 * fixture_reset() で並びは空になる。 */
void fixture_set_get_com_keys(const char *keys);

/* set_large() が返す答えを前もって並べておく（'y' で大きい、ほかは
 * 大きくない）。使いきると「大きくない」に戻る。文字列は写さないので、
 * 使い終わるまで残るもの（文字列リテラル）を渡す。tests/shared_stubs.c が
 * 提供する。fixture_reset() で並びは空になる。 */
void fixture_set_large_answers(const char *answers);

/* set_large() が呼ばれた回数と、最後に訊かれた品物。fixture_reset() で
 * 0・NULL に戻る。 */
int fixture_set_large_call_count(void);
const struct treasure_type *fixture_set_large_last_item(void);

/* no_light() の答え（true で明かりが無い）。tests/shared_stubs.c が提供する。
 * fixture_reset() で明るいに戻る。 */
void fixture_set_no_light(bool dark);

#endif /* FIXTURE_H */
