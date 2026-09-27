/* 階級の表（テスト用の足場）
 *
 * **#18-12-28C で `py` の器が消えて、残ったのは表 1 つだけ** —— 22 つめ
 * （どの種族か）の足場とまったく同じ道すじで、そこでも `py` だけが消えて
 * `race[]` が残った。
 *
 * 1 バイトは `src/player_class.c` の static に入ったので、ここで `py` を
 * 定義しても窓口には届かない別の器になるだけ（HANDOVER.md 第 7 節。#18-6C2
 * 以来くりかえしているつまずきで、#18-12-1C から #18-12-28C まで **25 回**
 * 続けて同じ形の足場を消している）。**しかも今回は `struct misc` が struct
 * ごと消えた**ので、`player_type` に `misc` という入れ物じたいが無い。
 *
 * 表は残る —— externs.h の「定数表（読みとり専用データ）」20 個の 1 つで、
 * その区分は #18 の対象外（`const` 化のみ。GLOBALS_INVENTORY.md:748）。本体では
 * `src/player.c:283` が 6 行の実の値を持ったままで、`src/player_class.c` は
 * extern 1 行で届く —— `src/player_race.c` が `race[]` に対して、
 * `src/player_level.c` が `player_exp[]` に対してしているのと同じ形。
 *
 * 表は空のまま置く（初期化子なし = `title` が全部 NULL・`spell` が全部 0）。
 * **名前や系を読む件は自分で入れてから読む** —— 本物の 6 行を写すと、
 * `src/player.c` が変わったときに足場だけが古くなって気づけない
 * （22 つめの足場に書いた理由をそのまま当てる）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

class_type class[MAX_CLASS];
