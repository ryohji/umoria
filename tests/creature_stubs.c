// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* creature.c をテストに取りこむための代役
 *
 * movement_rate() の依存は turn（src/data/progress.c）と休息の残りターン
 * （src/player/player_resting.c。#18-12-10C）の 2 つだけだが、
 * static 関数なので外から呼べない。実体を検証するにはテスト側が
 * src/monster/creature.c を #include して翻訳単位ごと取りこむしかなく、そうすると
 * creature.c 全体（モンスターの移動と呪文）が持ちこまれ、
 * 70 個のシンボルが未解決になる。
 *
 * ここに置くのはその代役。movement_rate() はどれも呼ばないので、すべて
 * 「呼ばれたら何もしない／固定値を返す」で足りる。一覧はリンカに出させた
 * もので、手で数えあげたわけではない:
 *   gcc -std=c17 -Isrc -c -o /tmp/c.o src/creature.c
 *   gcc -o /tmp/t probe.c /tmp/c.o \
 *     2>&1 | grep 'undefined reference' | sed 's/.*to //' | sort -u
 *
 * misc3_stubs.c / fixture.c と分けている理由: どちらも creature.c が要求する
 * シンボルの一部（msg_print・randint・py）しか持たず、逆に creature.c 側と
 * 重複するものも持つ。creature.c だけをリンクするなら本物のソースを 1 つも
 * 足す必要がないので、この 1 ファイルで完結させるのが最も小さい構成になる。
 * 窓口の名前は fixture.h と同じにそろえてある。
 */
#include <stddef.h>
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_map.h"
#include "fixture.h"
#include "player_glowing_hands.h"
#include "player_infra_range.h"
#include "player_resting.h"
#include "player_status_flags.h"

/* --- グローバル状態 ---
 * turn と wizard はここに無い。#19B で creature.c が progress_turn() /
 * progress_wizard_mode() 越しに読むようになったので、実体は
 * src/data/progress.c の static である（#19C1。ここで定義しても窓口には届かない
 * 別の器になるだけ）。テストは progress_set_turn() で turn を動かす。 */

player_type py;
/* 階級の値段表。#18-12-6A で src/player/player_level.c がリンクされる全ての実行形式に
 * 要る。この足場は py を自分で定義する = src/data/player.c（本物の 40 個の持ち主）と
 * 一緒にはリンクされないので、ここにも空の表を置く。 */
uint32_t player_exp[MAX_PLAYER_LEVEL];
/* マスの表（cave）もここに無い。#18-14-8C で置き場が src/dungeon/dungeon_map.c の
 * static に入ったので、代役を置くと窓口に届かない別の表になるだけ
 * （m_list の 2 行が #18-14-4C で消えたのと同じ理由）。この実行形式は
 * src/dungeon/dungeon_map.c をリンクしていて、下の階を白紙に戻す 1 行はその窓口を
 * 呼ぶ。creature.c は square_at(y, x) で 1 マスを取る。 */
/* m_list もここに無い。#18-14-4C で src/monster/monster_list.c が static で持つように
 * なったので、代役を置くと窓口に届かない別の表になるだけ
 * （movement_rate_test はその monster_list.c をリンクしている）。creature.c は
 * monster_list_at() で行を取り、monster_list_used() を数えおろしの上限にする。 */
/* 白紙のモンスター 1 体。#18-14-4B で src/monster/monster_list.c をリンクするように
 * なったので要る（階の頭で全行に書き、返した行を白紙に戻すのに使う）。
 * 本物は monsters.c の定義表のとなりで、あちらはこの実行形式に来ない。
 * m_list / mfptr の 2 行は #18-14-4C で消えたが、**この 1 行は残った** ——
 * blank_monster はこの問いの主題ではなく定数表の側。 */
monster_type blank_monster;
/* 床に落ちているものの置き場（t_list / tcptr）もここに無い。#18-14-7C で
 * src/dungeon/floor_items.c が static で持つようになったので、代役を置くと窓口に
 * 届かない別の表になるだけ（この実行形式はその floor_items.c をリンクして
 * いる）。creature.c は floor_item_at() で行を取る。表の 1 行は長く
 * ここにあった —— creature.c が `t_list[c_ptr->tptr]` と書いていたから。
 * invcopy() の代役は下に残る —— 空の行は定義表の写しで、あれは主題の外。 */
/* 居場所の 2 個（char_row / char_col）もここに無い。#18-6C1 で
 * src/player/player_pos.c が static で持つようになったので、代役を置くと窓口越しの
 * 読み書きが届かない別の器になるだけ。creature.c は player_row() /
 * player_col() 越しに読み、テストが位置を動かすなら player_place() を使う。 */
/* 持ち物の 4 個（inventory / inven_ctr / inven_weight / equip_ctr）はここに
 * 無い。#18-5C で src/item/inventory.c が static で持つようになったので、代役を
 * 置く必要が無くなった（置くと inventory.c の分と別の器になり、窓口越しの
 * 読み書きが別の場所に当たる）。 */
/* mfptr もここに無い（上の m_list と一緒に #18-14-4C で出ていった）。 */
/* mon_tot_mult もここに無い。#18-14-3B で src/monster/monster_breeding.c が static で
 * 持つようになったので、代役を置くと窓口に届かない別の器になる
 * （movement_rate_test はその monster_breeding.c をリンクしている）。
 * creature.c は monster_breeding_allowed() で訊き、
 * monster_breeding_note_birth() で数える。 */
/* find_flag もここに無い。#18-11-6C で src/player/running.c が static で持つように
 * なったので、代役を置くと窓口に届かない別の器になる（movement_rate_test は
 * その running.c をリンクしている）。creature.c は player_is_running() 越しに
 * 訊く。 */
/* hack_monptr もここに無い。#18-14-1B で src/monster/monster_turn.c が static で持つ
 * ようになったので、代役を置くと窓口に届かない別の器になる（movement_rate_test
 * はその monster_turn.c をリンクしている）。creature.c は monster_turn_begin() /
 * monster_turn_end() 越しに開け閉めする。**この 1 行は #18-14-1C で消し忘れて
 * いた** —— 置き場が A の段で module に入る形（→ 所見 52）だと、代役は B の
 * 時点で誰も見ない死んだ定義になるが、リンクは通るので何も知らせない。 */
/* death もここに無い。#19B2 で creature.c が player_is_dead() 越しに読む
 * ようになったので、実体は src/save/score_death.c の static である。 */
/* player_light もここに無い。#18-7-3C1 で src/player/player_light.c が static で
 * 持つようになったので、代役を置くと窓口に届かない別の器になる。 */
/* screen_change もここに無い。#18-11-3C で src/ui/screen_touched.c が static で
 * 持つようになったので、代役を置くと窓口に届かない別の器になる。 */
/* total_winner と max_score もここに無い。#18-7-1C1 で src/save/score_death.c が
 * static で持つようになったので、代役を置くと窓口に届かない別の器になる。 */

/* --- 画面出力・メッセージ --- */
void msg_print(const char *str) { (void)str; }
void lite_spot(int y, int x) { (void)y; (void)x; }
void disturb(int a, int b) { (void)a; (void)b; }
void prt_cmana(void) {}
void prt_experience(void) {}
void prt_gold(void) {}

/* --- 乱数。テストから制御できるように固定値を返す --- */
static int fixture_randint_value = 1;
static int fixture_randint_last_max = 0;
static int fixture_randint_calls = 0;

int randint(int maxval)
{
    fixture_randint_last_max = maxval;
    fixture_randint_calls++;
    return fixture_randint_value;
}

void fixture_set_randint(int value) { fixture_randint_value = value; }
int fixture_randint_last_maxval(void) { return fixture_randint_last_max; }
int fixture_randint_call_count(void) { return fixture_randint_calls; }

int damroll(int num, int sides) { (void)num; (void)sides; return 0; }

/* --- ダンジョン・座標 --- */
bool in_bounds(int y, int x) { (void)y; (void)x; return true; }
bool panel_contains(int y, int x) { (void)y; (void)x; return true; }
bool los(int a, int b, int c, int d) { (void)a; (void)b; (void)c; (void)d; return false; }
/* distance は代役にしない。creature.c が `m_ptr->cdis` に
 * 代入しており、常に 0 を返すと「全モンスターが隣接している」状態に
 * なる。純粋関数なので もと misc1.c:210（いまは dungeon/geometry.c）の実装を写す（misc1.c 全体を
 * リンクすると依存が芋づるで付くため）。tests/distance_test.c が
 * 本物のふるまいを固定しているので、乖離すればそちらで気づける。 */
int distance(int y1, int x1, int y2, int x2) {
    int dy = y1 - y2;
    if (dy < 0) {
        dy = -dy;
    }

    int dx = x1 - x2;
    if (dx < 0) {
        dx = -dx;
    }

    return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}
int mmove(int dir, int *y, int *x) { (void)dir; (void)y; (void)x; return 0; }
void move_rec(int y1, int x1, int y2, int x2) { (void)y1; (void)x1; (void)y2; (void)x2; }
int twall(int y, int x, int t, int d) { (void)y; (void)x; (void)t; (void)d; return 0; }
int find_range(int a, int b, int *lo, int *hi) { (void)a; (void)b; (void)lo; (void)hi; return 0; }

/* --- モンスター --- */
void delete_monster(int m) { (void)m; }
void fix1_delete_monster(int m) { (void)m; }
void fix2_delete_monster(int m) { (void)m; }
int mon_take_hit(int m, int dam) { (void)m; (void)dam; return 0; }
bool place_monster(int y, int x, creature_handle h, int slp) {
    (void)y; (void)x; (void)h; (void)slp; return false;
}
bool summon_monster(int *y, int *x, int slp) { (void)y; (void)x; (void)slp; return false; }
bool summon_undead(int *y, int *x) { (void)y; (void)x; return false; }
int aggravate_monster(int d) { (void)d; return 0; }
creature_type *monster_get_creature(creature_handle h) { (void)h; return NULL; }
const char *monster_name(vtype buf, const monster_type *m) { (void)buf; (void)m; return ""; }
const char *monster_name_indefinite(vtype buf, const creature_type *c) {
    (void)buf; (void)c; return "";
}
bool monster_attack_is_null(attack_handle h) { (void)h; return true; }
uint8_t monster_attack_get_type(attack_handle h) { (void)h; return 0; }
uint8_t monster_attack_get_desc(attack_handle h) { (void)h; return 0; }
uint8_t monster_attack_get_dice(attack_handle h) { (void)h; return 0; }
uint8_t monster_attack_get_sides(attack_handle h) { (void)h; return 0; }

/* --- モンスター記録（recall） --- */
static recall_type fixture_recall;
recall_type *recall_get(creature_handle h) { (void)h; return &fixture_recall; }
void recall_update_characteristics(creature_handle h, int defence) { (void)h; (void)defence; }
void recall_update_move(creature_handle h, int move) { (void)h; (void)move; }
void recall_update_spell(creature_handle h, uint32_t type) { (void)h; (void)type; }
void recall_increment_spell_chance(creature_handle h) { (void)h; }
void recall_increment_death(creature_handle h) { (void)h; }

/* --- プレイヤーへの被害・状態 --- */
void take_hit(int dam, const char *from) { (void)dam; (void)from; }
void acid_dam(int dam, const char *from) { (void)dam; (void)from; }
void cold_dam(int dam, char *from) { (void)dam; (void)from; }
void fire_dam(int dam, const char *from) { (void)dam; (void)from; }
void light_dam(int dam, char *from) { (void)dam; (void)from; }
void corrode_gas(const char *from) { (void)from; }
void breath(int t, int y, int x, int dam, char *dsc, int m) {
    (void)t; (void)y; (void)x; (void)dam; (void)dsc; (void)m;
}
bool test_hit(int a, int b, int c, int d, int e) {
    (void)a; (void)b; (void)c; (void)d; (void)e; return false;
}
bool dec_stat(int s) { (void)s; return false; }
void lose_exp(int32_t amount) { (void)amount; }
bool player_saves(void) { return false; }
void calc_bonuses(void) {}
void teleport_away(int m, int d) { (void)m; (void)d; }
void teleport_to(int y, int x) { (void)y; (void)x; }

/* --- 持ち物 --- */
int delete_object(int y, int x) { (void)y; (void)x; return 0; }
void invcopy(inven_type *i, int id) { (void)i; (void)id; }
void inven_destroy(int item) { (void)item; }
int known2_p(inven_type *i) { (void)i; return 0; }
void add_inscribe(inven_type *i, uint8_t flag) { (void)i; (void)flag; }

/* --- ビット操作・文字列 --- */
int bit_pos(uint32_t *test) { (void)test; return 0; }
char *concat(char *buffer, ...) { return buffer; }

/* --- テスト専用の初期化。本体（src/）には存在しない。
 * MU_SETUP から呼ぶことで、先行テストの影響を受けない条件を作る。
 * turn はここでは触らない。テストごとに progress_set_turn() で制御するため
 * （fixture_reset が値を決めると、テストの前提が見えなくなる）。 --- */
void fixture_reset(void)
{
    memset(&py, 0, sizeof py);
    /* 状態の旗は #18-12-7C で src/player/player_status_flags.c の static に入ったので、
     * py を消しても届かない。セーブファイル用の窓口で 30 bit まとめて降ろす
     * （creature.c が読むのは PY_BLIND と PY_SEARCH あたり）。 */
    player_set_status_word(0);
    /* 休息の残りターンも #18-12-10C で src/player/player_resting.c の static へ移った。
     * **movement_rate_test の 3 件がこの 0 戻しに頼っている** —— 1 件が
     * player_rest_set(1) を置き、次の件は「休んでいない」前提で始まる
     * （py を memset しても module の static には届かない）。 */
    /* 赤外視の距離も #18-12-12C で src/player/player_infra_range.c の static へ移った。
     * creature.c の update_mon() が「距離のうちで、しかも温かいか」でモンスターを
     * 見せるかを決めるので、残った距離が次の件に漏れないように 0 に戻す
     * （**0 は Human の走りだし** = 温かい血だけではモンスターは見えない）。
     * 休息の 1 行とちがって**いまは頼っている件が無い**（外してもグリーンのまま。
     * 実際に外して確かめた）。約束のために足しておく。 */
    player_infra_range_set(0);
    /* 光る手も #18-12-13C で src/player/player_glowing_hands.c の static へ移った。
     * monster_melee.c の make_attack() が「手が光っていて、しかもはじかれて
     * いない攻撃か」でモンスターを混乱させるかを決めるので、残った蓄えが
     * 次の件に漏れないように 0 に戻す（**この蓄えはターンで減らない** ——
     * 1 撃が当たるまで光ったままなので、消し忘れると次の件まで持ちこす）。
     * 赤外視の 1 行と同じで**いまは頼っている件が無い**（外してもグリーンの
     * まま。実際に外して確かめた）。約束のために足しておく。 */
    player_glowing_hands_restore(0);
    /* 階を白紙に戻す。#18-14-8B までは memset 1 行だったが、それは
     * generate.c の blank_cave() の写しだった —— いまは本物と同じ窓口を
     * 呼ぶ（上の置き場を掃く）。 */
    dungeon_map_reset();
    fixture_randint_last_max = 0;
    fixture_randint_calls = 0;
}

/* fixture.h の窓口のうち、creature.c 側では観測に使わないもの。
 * 宣言があるので定義だけそろえておく。 */
const char *fixture_screen_text(int row, int col) { (void)row; (void)col; return ""; }
const char *fixture_message_text(int index) { (void)index; return ""; }
int fixture_message_count(void) { return 0; }
