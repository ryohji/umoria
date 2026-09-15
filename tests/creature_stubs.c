/* creature.c をテストに取りこむための代役
 *
 * movement_rate() の依存はグローバル turn と py.flags.rest の 2 つだけだが、
 * static 関数なので外から呼べない。実体を検証するにはテスト側が
 * src/creature.c を #include して翻訳単位ごと取りこむしかなく、そうすると
 * creature.c 全体（1609 行、モンスターの移動・攻撃・呪文）が持ちこまれ、
 * 76 個のシンボルが未解決になる。
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

#include "fixture.h"

/* --- グローバル状態 ---
 * turn と wizard はここに無い。#19B で creature.c が progress_turn() /
 * progress_wizard_mode() 越しに読むようになったので、実体は
 * tests/progress_fixture.c にある（両方で定義すると窓口越しの読み書きが
 * 別の器に当たる）。テストは progress_set_turn() で turn を動かす。 */

player_type py;
cave_type cave[MAX_HEIGHT][MAX_WIDTH];
monster_type m_list[MAX_MALLOC];
inven_type t_list[MAX_TALLOC];
int16_t char_row;
int16_t char_col;
/* 持ち物の 4 個（inventory / inven_ctr / inven_weight / equip_ctr）はここに
 * 無い。#18-5C で src/inventory.c が static で持つようになったので、代役を
 * 置く必要が無くなった（置くと inventory.c の分と別の器になり、窓口越しの
 * 読み書きが別の場所に当たる）。 */
int16_t mfptr;
int16_t mon_tot_mult;
int find_flag;
int hack_monptr;
/* death もここに無い。#19B2 で creature.c が player_is_dead() 越しに読む
 * ようになったので、実体は tests/score_death_fixture.c にある。 */
bool player_light;
bool screen_change;

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
/* distance は代役にしない。creature.c:1033,1532 が `m_ptr->cdis` に
 * 代入しており、常に 0 を返すと「全モンスターが隣接している」状態に
 * なる。純粋関数なので misc1.c:210 の実装を写す（misc1.c 全体を
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
 * turn はここでは触らない。テストごとに明示的に代入して制御するため
 * （fixture_reset が値を決めると、テストの前提が見えなくなる）。 --- */
void fixture_reset(void)
{
    memset(&py, 0, sizeof py);
    memset(cave, 0, sizeof cave);
    fixture_randint_last_max = 0;
    fixture_randint_calls = 0;
}

/* fixture.h の窓口のうち、creature.c 側では観測に使わないもの。
 * 宣言があるので定義だけそろえておく。 */
const char *fixture_screen_text(int row, int col) { (void)row; (void)col; return ""; }
const char *fixture_message_text(int index) { (void)index; return ""; }
int fixture_message_count(void) { return 0; }
