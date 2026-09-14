/* save_state_fixture.c -- src/save_state.c をリンクするための足場
 *
 * src/save_state.c は窓口だけを持ち、記録の実体はまだ src/variable.c にある
 * （ステップ A なので呼びだし側を 1 箇所も変えていない＝ふるまい不変）。
 * ここで定義するのは save_state.c 自身が要る 4 個だけ:
 *   savefile / character_generated / character_saved / panic_save
 *
 * 残りは progress_fixture.c と score_death_fixture.c が持っている。
 * save_state.c は 3 モジュールのうち唯一よその窓口を呼ぶ側なので
 * （save_state_has_live_character が score_death.h の player_is_dead を、
 * save_state_character_is_in_play が progress.h の progress_turn を読む）、
 * この実行形式は 3 つのモジュールと 3 つの足場をリンクする。足場を分けてある
 * ので同じ名前を二重に定義することはない。
 *
 * ステップ C で 13 個の実体が 3 つのモジュールの static になれば足場 3 つは
 * 消え、この実行形式はテスト 1 本＋モジュール 3 本の 4 単位・代役 0 になる。
 * それが有向依存の代金で、隠さずに数えておく。
 *
 * 初期値は variable.c:45,49,50,70 と同じにしてある。
 *
 * ここには代役（スタブ）が 1 つもない。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 本体では variable.c。初期値もそこに合わせる。 */
vtype savefile;                   /* variable.c:45 初期化なし＝全バイト 0 */
bool character_generated = false; /* variable.c:49 */
bool character_saved = false;     /* variable.c:50 */
bool panic_save = false;          /* variable.c:70 */
