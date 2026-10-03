// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* テスト用のグローバルデータと初期化関数
 * 本物のデータ（object_list, colors 等）は tables.c / treasure.c を
 * そのままリンクして使う。ここに置くのは、本体では別ファイルに
 * 散っていて一緒にリンクできないものだけ。 */
#include <string.h>
#include "config.h"
#include "constant.h"
#include "types.h"

#include "inventory.h"
#include "item_ident.h"
#include "player_level.h"
#include "shared_stubs.h"

player_type py;         /* 本体では player.c（530行の巨大データと同居） */
/* 階級の値段表。#18-12-6A で src/player/player_level.c がリンクされる全ての実行形式に
 * 要る。src/data/player.c をリンクする足場（tests/level_ops_fixture.c
 * などを使う側）は本物の 40 個を持っているが、この足場は py を自分で
 * 定義する = player.c と一緒にはリンクされないので、ここにも空の表を置く。 */
uint32_t player_exp[MAX_PLAYER_LEVEL];
/* 持ち物（inventory / inven_ctr / inven_weight / equip_ctr）はここでは定義
 * しない。#18-5C で src/item/inventory.c が static で持つようになったので、
 * 消しかたも窓口（src/item/inventory.h）越しになる。 */

/* テスト専用。オリジナルには存在しない。
 * setUp から呼ぶことで、先行テストの影響を受けない条件を作る。 */
void fixture_reset(void)
{
    /* 品目ごとの覚えは #18-9-C で src/item/item_ident.c が static で持つように
     * なったので、セーブファイル用の生の窓口越しに消す。 */
    memset(item_kind_record_bytes(), 0, (size_t)item_kind_record_count());
    /* 持ち物と装備は 1 本の配列なので、跨ぎの窓口で全域を消す。 */
    memset(inventory_and_equipment_at(0), 0,
           sizeof(inven_type) * (size_t)inventory_and_equipment_slot_count());
    memset(&py, 0, sizeof py);
    /* 階級と経験値の 5 つは #18-12-6C で src/player/player_level.c が static で
     * 持つようになったので、py を消しても届かない。窓口越しに 0 へ戻す
     * （実体は初期化子なしの static なので、走りだしの値は 0 のまま）。 */
    player_set_level(0);
    player_set_experience(0);
    player_set_max_experience(0);
    player_set_experience_fraction(0);
    player_set_experience_factor(0);
    inventory_set_count(0);
    shared_stubs_reset();
}

/* --- スタブ ---
 * テスト対象が呼ぶが、テストしたいふるまいには関係しない関数。
 * 本物をリンクすると画面や乱数への依存が芋づるで付いてくるので、
 * ここで最小限の代役を置く。
 *
 * shared_stubs.c と重複していた代役（msg_print, randint, set_seed,
 * reset_seed, add_inscribe）は #66 で削除し、shared_stubs.c に統一した。 */

/* 経験値の表示。本物（level_ops.c:55）は表示のついでに上限の打ち切りと
 * レベルアップ判定（gain_level）も行うので、リンクすると画面・呪文・
 * HP 計算まで芋づるで付いてくる。経験値の加算式を見たいテストには
 * 不要なので捨てる。 */
void prt_experience(void) {}

/* 文字列組み立て。desc.c 内の別関数用で、今回の対象は呼ばない */
void insert_str(char *o, const char *m, const char *i) { (void)o; (void)m; (void)i; }
