/* ステップ A のあいだの足場
 *
 * src/spells_known.c の窓口は、まだ player.c にある 4 つのグローバル
 * （spell_learned・spell_worked・spell_forgotten・spell_order）を指している。
 * テストで src/player.c をリンクすると、そちらは呪文の 4 個だけでなく
 * 段の表・名前の表・キャラクタ作成の初期値まで抱えているので、対象 1 本を
 * 見るテストには重すぎる（hp_table_fixture.c・burden_fixture.c と同じ理由）。
 *
 * ステップ C で実体が src/spells_known.c の static に移ったら、この足場は消す。
 * 消し忘れると「足場を壊してもグリーン」になるので、C の検証で実体の側を
 * 壊してレッドになることを確かめる。
 */
#include <stdint.h>

uint32_t spell_learned = 0;
uint32_t spell_worked = 0;
uint32_t spell_forgotten = 0;
uint8_t spell_order[32];
