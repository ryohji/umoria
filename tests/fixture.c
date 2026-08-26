/* テスト用のグローバルデータと初期化関数
 * 本物のデータ（object_list, colors 等）は tables.c / treasure.c を
 * そのままリンクして使う。ここに置くのは、本体では別ファイルに
 * 散っていて一緒にリンクできないものだけ。 */
#include <string.h>
#include "config.h"
#include "constant.h"
#include "types.h"

player_type py;          /* 本体では player.c（530行の巨大データと同居） */
/* inven_ctr は treasure.c にあるのでここでは定義しない */

/* テスト専用。オリジナルには存在しない。
 * setUp から呼ぶことで、先行テストの影響を受けない条件を作る。 */
/* 記録変数はこの下で定義するので、先に宣言だけしておく。 */
static void fixture_clear_randint_record(void);

void fixture_reset(void)
{
    extern uint8_t object_ident[];
    extern inven_type inventory[];
    memset(object_ident, 0, OBJECT_IDENT_SIZE);
    memset(inventory, 0, sizeof(inven_type) * INVEN_ARRAY_SIZE);
    memset(&py, 0, sizeof py);
    { extern int16_t inven_ctr; inven_ctr = 0; }
    fixture_clear_randint_record();
}

/* --- スタブ ---
 * テスト対象が呼ぶが、テストしたいふるまいには関係しない関数。
 * 本物をリンクすると画面や乱数への依存が芋づるで付いてくるので、
 * ここで最小限の代役を置く。 */

/* 画面出力。テストでは捨てる */
void msg_print(char *str) { (void)str; }

/* 経験値の表示。本物（misc3.c:1838）は表示のついでに上限の打ち切りと
 * レベルアップ判定（gain_level）も行うので、リンクすると画面・呪文・
 * HP 計算まで芋づるで付いてくる。経験値の加算式を見たいテストには
 * 不要なので捨てる。 */
void prt_experience(void) {}

/* 乱数。テストから制御できるように固定値を返す。
 * 値を変えたいテストは fixture_set_randint() で差しかえる。 */
static int fixture_randint_value = 1;
/* 戻り値を固定するだけでは、呼びだし側が渡した上限が正しいかを検証できない。
 * 配列の要素数を取りちがえても返る値が同じで気づけないので、上限と
 * 呼びだし回数も記録する。misc3_stubs.c と同じ窓口を提供する。 */
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

static void fixture_clear_randint_record(void)
{
    fixture_randint_last_max = 0;
    fixture_randint_calls = 0;
}

void set_seed(uint32_t seed) { (void)seed; }
void reset_seed(void) {}
uint32_t randes_seed = 0;

/* 文字列組み立て。desc.c 内の別関数用で、今回の対象は呼ばない */
void insert_str(char *o, char *m, char *i) { (void)o; (void)m; (void)i; }
void add_inscribe(inven_type *i, int flag) { (void)i; (void)flag; }

/* 店舗の商品判定。desc.c の store_bought_p が参照する */
bool general_store(int t) { (void)t; return false; }
bool armory(int t) { (void)t; return false; }
bool weaponsmith(int t) { (void)t; return false; }
bool temple(int t) { (void)t; return false; }
bool alchemist(int t) { (void)t; return false; }
bool magic_shop(int t) { (void)t; return false; }
