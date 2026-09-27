/* 人物の器と階級の表（テスト用の足場）
 *
 * **この足場は 2 つのものを持っていて、#18-12-28C で片方だけが消える見こみ**
 * —— 22 つめ（どの種族か）の足場とまったく同じ形。
 *
 * 1. py —— **#18-12-28C で消える。** A のあいだは置き場が `py.misc.pclass` の
 *    ままなので実体が要るが、1 バイトが `src/player_class.c` の static に
 *    入ったあとは、ここで定義しても窓口には届かない別の器になるだけ
 *    （HANDOVER.md 第 7 節。#18-6C2 以来くりかえしているつまずきで、
 *    #18-12-1C から #18-12-27C まで 24 回続けて同じ形の足場を消している）。
 *    **`struct misc` はこの単位で struct ごと消える**ので、C のあとは
 *    `player_type` に `misc` という入れ物じたいが無い。
 *
 * 2. class[] —— **こちらは残る。** 表は module の中に入れなかった
 *    （externs.h の「定数表（読みとり専用データ）」20 個の 1 つで、その区分は
 *    #18 の対象外。`const` 化のみ。GLOBALS_INVENTORY.md:748）。本体では
 *    `src/player.c:283` が 6 行の実の値を持ったままで、`src/player_class.c` は
 *    extern 1 行で届く —— `src/player_race.c` が `race[]` に対して、
 *    `src/player_level.c` が `player_exp[]` に対してしているのと同じ形。
 *
 * 表は空のまま置く（初期化子なし = `title` が全部 NULL・`spell` が全部 0）。
 * **名前や系を読む件は自分で入れてから読む** —— 本物の 6 行を写すと、
 * `src/player.c` が変わったときに足場だけが古くなって気づけない
 * （22 つめの足場に書いた理由をそのまま当てる）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

player_type py;

class_type class[MAX_CLASS];
