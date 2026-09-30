# 実装の再配置の計画

台帳の #52〜#57（#42・#43・#38 を束ねる）の計画。2026-09-29、`develop` の
`994818a` の時点で書いた。行数や本数はその時点の実測値。

## ねらい

1. **番号つきのファイルをなくす。** `misc1〜4`・`store1/2`・`moria1〜4` は
   責務ではなくコンパイル単位で割った名残り（#43）。中身の名前のファイルへ分ける。
2. **`src/` をサブディレクトリーに分ける。** `src/*.c` は 109 本がひと並び。
   どこに何があるかをディレクトリーの名前で示す。
3. **テストのビルドをライブラリー化する。** `makefile.test` の手書きの recipe
   （2335 行、`src/*.c` の名前が 1415 回出てくる）をやめる。module を 1 本足す
   たびに多くの recipe へ名前を足して回る手間（#38 の「税」）をなくす。

ふるまいは変えない。どの段も「修正前後で結果が変わらないこと」をテストで守り、
毎ステップの終わりに `HANDOVER.md` 第 2 節の確認 5 つをする。

## ディレクトリー

| ディレクトリー | 置くもの | いまのファイル（例） |
|---|---|---|
| `core/` | どこにも依存しない道具 | `rnd.c`・`str_insert.c` |
| `data/` | 表と置き場（中身の決まった定数表、起動時に一度書く表） | `tables.c`・`treasure.c`・`monsters.c`・`player.c`・`variable.c` |
| `player/` | 人物の状態の module | `player_*.c`・`stats.c`・`abilities.c`・`hp_table.c`・`burden.c` |
| `monster/` | モンスターの状態と行動 | `creature.c`・`monster_*.c` |
| `dungeon/` | 地図と階の生成 | `dungeon_map.c`・`dungeon_size.c`・`dungeon_level.c`・`generate.c`・`floor_items.c` |
| `item/` | 品物（鑑定・名前・使うコマンド） | `item_ident.c`・`object_levels.c`・`desc.c`・`inventory.c`・`eat.c`・`potions.c`・`scrolls.c`・`staffs.c`・`wands.c` |
| `store/` | 店 | `stores.c`（と store1/2 の分割先） |
| `combat/` | 命中・傷・飛び道具（下記） | 新しく作るファイルだけ |
| `ui/` | 画面と入力のうち描画側・メニュー | `io.c`・`messages.c`・`panel.c`・`help.c`・`render.c`・`view_observer.c` |
| `save/` | セーブとスコア | `save.c`・`save_state.c`・`death.c`・`score_death.c` |
| `platform/` | OS と端末 | `platform.c`・`signals.c`・`render_ncurses.c`・`input_ncurses.c` |

- `main.c` だけ `src/` に残す。
- 109 本ぜんぶの行き先の表は D の段の最初（D0）に作り、ユーザーに見てもらって
  から動かす。上の「例」は置き場の考えかたを示すもので、確定の一覧ではない。
- **`#include` は書きかえない。** `src/` と `tests/` の `#include "…"` 1394 行は
  そのまま。3 つの makefile で、ディレクトリーごとに `-I` を足す。
  **D の後は**、ディレクトリーの一覧は `sources.mk` の `SRC_SUBDIRS` の 1 か所で、
  3 つの makefile の `VPATH`・`-I` はそこから作る。`.c` を足すのは `SRCS` に
  `player/foo.c` の形で 1 語。

### 置き場の規則

**葉の「状態」module（`inventory.c`・`player_level.c`・`stats.c` など）に、UI や
外への依存を持つ code を足さない。** 足すなら兄弟のファイルを作る
（例：`inventory.c` の隣に `inven_ops.c`）。葉のテストが単独でリンクできる
ことが、ここまでの窓口化の成果だから。

## テストのライブラリー化（L）

**試し（2026-09-29、`/tmp` で）。**
- `main`・`render_ncurses`・`input_ncurses` を除く `src/` の `.o` 103 本を 1 つの
  `libcore.a` にまとめた。
- 各テストを「`test.c` ＋そのテストの `tests/` のファイル ＋ `libcore.a`」で
  リンクした。
- 結果：74 本すべてがリンクした。1662 件が通り、失敗 0。多重定義も 0。
- archive から引かれた `.o` は手書きの一覧と一致した（例：`put_misc3_test` は
  recipe の 45 本に対し、引かれたのも 45 本）。

**段どり。**

1. **L1**：`sources.mk` に `.c` の一覧を 1 つだけ書く。`makefile`・
   `makefile.win`・`makefile.test` がそれを読む。
2. **L2**：`makefile.test` を、型の規則と「実行形式ごとに足す `tests/` の
   ファイル」の表だけにする。**74 本すべてで、`-Wl,-Map` に出る `.o` が旧 recipe
   と一致すること**を確かめてから、旧 recipe を消す。
3. **L3**：道具を 2 つ置く。
   - `scripts/link_units.py`：実行形式ごとに、archive から引かれた `.o` を出す。
   - `nm` を使って、層をまたぐ依存（たとえば `core/` から `ui/`）が無いことを
     見る検査。

**落とし穴 2 つ。**

- **代役を消しても赤くならない。** 手書きの recipe なら `undefined reference`
  で止まったところを、archive は黙って本物で埋める。代役を消したら、
  `link_units.py` で引かれた `.o` を見る。
- **archive は 1 つにする。** 領域ごとに分けると互いに呼びあって、リンクの
  順番が壊れる。層の境目は、archive を分けるかわりに L3 の検査で守る。

## 番号つきファイルの分割（R）

新しいファイルは最初から行き先のディレクトリーに作る。分け終わった番号つきの
ファイルは消す。1 つの塊を 1 コミットで移す。

**`misc4.c`（114 行）**
- `scribe_object`・`add_inscribe`・`inscribe` → `item/inscription.c`
- `check_view` → `ui/map_view.c`
- `concat` → `core/str_insert.c`。**先にテストを足してから移す**（#47。保護が
  ゼロのため）。

**`misc1.c`（824 行）**
- `init_seeds`・`set_seed`・`reset_seed`・`randint`・`randnor`・`magik` →
  `core/rnd.c` に寄せる
- `damroll`・`pdamroll`・`max_hp` → `core/dice.c`
- `bit_pos` → `core/bits.c`
- `in_bounds`・`distance`・`los`・`next_to_walls`・`next_to_corr` →
  `dungeon/geometry.c`
- `get_panel` → `ui/map_view.c`（2026-09-29 に `ui/panel.c` から改めた。下の「下調べ」）
- `loc_symbol`・`test_light`・`prt_map` → `ui/map_view.c`
- `compact_monsters`・`popm`・`place_monster`・`place_win_monster`・
  `get_mons_num`・`alloc_monster`・`summon*` → `monster/monster_place.c`
- `compact_objects`・`popt`・`pusht` → `dungeon/object_place.c`
- `m_bonus` → `item/item_enchant.c`
- `add_food` → `item/eat.c` か、兄弟の `player/food_ops.c`。**`player_food.c` は
  不可**（測った。下の「下調べ」）。どちらにするかは misc1 の段で決める。

**misc1 で決まったこと（2026-09-29、ユーザーの判断）。** 下調べの「core/ の規則に
合わないもの」と `add_food` について。
- `init_seeds`（`progress.c` の窓口を呼ぶ）→ `src/game_state.c`。core には入れない。
- `randnor` → `core/rnd.c`。読む `normal_table` も `data/tables.c` から**表ごと**移す
  （読み手は `randnor` だけ。中身は変えない）。
- `add_food` → 新しい `player/food_ops.c`。
- `damroll`・`pdamroll`・`max_hp` は `randint` を呼ぶので、`randint` が `core/rnd.c` に
  入ってから `core/dice.c` へ移す。

### misc4・misc1 の下調べ（2026-09-29、`aefc979` の時点）

読みとりだけで測った。関数は `misc1.c` に 33 個（`static` 3）、`misc4.c` に 5 個で、
全部に上の行き先がある。

- **上の表に無かったもの。** `misc1.c:60` の `static uint32_t old_seed`。
  `set_seed`・`reset_seed` と同じファイルへ移す。`static` の `get_mons_num`・
  `summon`・`compact_objects` も、呼び手と同じファイルでないとリンクできない。
- **テストへの影響は代役からだけ来る。** `src/misc1.c`・`src/misc4.c` を旧 recipe で
  リンクしていたテストは 0 本、`#include "misc1.c"` なども 0。同じ名前の代役は
  `tests/creature_stubs.c` に 11 個、`tests/misc3_stubs.c` に 9 個、
  `tests/fixture.c` に 4 個（`add_inscribe` の第 2 引数が `int` で、本物の
  `uint8_t` とずれている）、`tests/save_stubs.c`・`tests/device_chance_test.c` に
  `randint` が 1 個ずつ。`distance` の写しは 3 つ（`distance_test.c` の `static`、
  2 つの代役。#37）。
- **`get_panel` は `panel.c` に入れない。** `find_bound` と `end_find()`（moria2。
  行き先は `player/run_path.c`）を読むので、依存ゼロの `panel.c` が依存を持つ。
  呼び手 4 か所と `check_view` はどれも直後に `prt_map` を呼ぶので、
  `ui/map_view.c` が自然。
- **`add_food` は `player_food.c` に入れない。** `msg_print` と
  `player_timed_add` を呼ぶ。`player_food.h:33` も「食べすぎの罰は画面に出すので
  外に置く」と書いている。呼び手は `eat.c:189` と `potions.c:316`。
- **`core/` の規則に合わないものがある。** `init_seeds` は `progress.c` の窓口を、
  `randnor` は `tables.c` の `normal_table` を呼ぶ。`core/rnd.c` に入れるなら
  この 2 つは別の置き場を考える（`magik` は `misc2` だけが呼ぶので
  `item/item_enchant.c` の案もある）。
- **下の層から上の層を呼ぶものがある。** `compact_objects`（`dungeon/`）が
  `prt_map`（`ui/`）と `msg_print` を、`compact_monsters` が `msg_print` と
  moria3 の `delete_monster` を呼ぶ。L3 の層の検査の規則を決めるときの材料になる。
- **archive の中の `.o` の粒度が効く。** 代役が一部だけを覆っている `.o`
  （例：`geometry.o` の `in_bounds`・`los`・`distance` は `creature_stubs.c` が
  代役にしている）は、テストがその `.o` の別の関数を 1 つでも要ると丸ごと
  引かれて多重定義になる。**分けるたびに `scripts/link_units.py` で引かれた
  `.o` を見る。** #37 の `distance` の写しは、`geometry.c` をリンクするだけでは
  消せない（代役の `in_bounds`（いつも真）と `los`（いつも偽）まで本物に替わり、
  テストのふるまいが変わる）。
- **`concat` のテスト（#47）。** 直接の呼び手は 0 で、`CONCAT` マクロ
  （`externs.h:174`）越しに 88 か所（creature 40・spells 34・moria3 7・moria4 7）。
  本物を通るテストは 0 件。`misc4.o` を archive から引くと `scribe_object` が
  `get_item`（moria1。libcore に入っていない）を要るので、**先に inscription と
  `check_view` を出して `misc4.c` を `concat` だけにしてから**テストを足すのが
  楽。見るのは「戻り値は第 1 引数」「NULL だけなら空文字列」「順につなぐ」
  「79 文字ちょうど」。長さは見ない関数なので、あふれは試さない。
- **`moria1.c` と `dungeon.c` は libcore に入っていない**（#52 の L で外した）。
  テストのフラグ（`-O2` が無い）だと `-Wformat-overflow` が `moria1.c` に 4 件
  （499・711・975・1074 行の `sprintf`）、`dungeon.c` に 1 件（1317 行）出るため。
  **moria1 を割ると、その行を持つ塊が警告を連れて libcore に入る。** 警告を
  消すにはコードを変えることになるので、その段でユーザーに問い合わせる
  （本当にあふれうるかは測っていない。バグ候補にはまだ入れていない）。
- **古い行番号のコメントがある**（`distance_test.c` の「misc1.c:217」、
  `panel_test.c` の「misc1.c:165-200」、台帳 #47 の「misc4.c:95-108」など）。
  移すときに直す（コメントだけの別コミット）。

**順番の提案（影響の小さい順）。** ① misc4 の inscription と `check_view` →
② `concat`（テストを足してから `core/str_insert.c` へ）→ ③ misc1 のうち代役が
0 のもの（bits・`max_hp`/`pdamroll`・`m_bonus`・`add_food`・`next_to_*`・
object_place）→ ④ map_view と monster_place → ⑤ 最後に rnd と geometry
（代役と写しが最も多く絡む）。

**`misc2.c`（914 行）**
- `magic_treasure`（23〜853 行。棚上げ #26）→ そのまま `item/item_enchant.c` へ。
  中は割らない。
- `set_options` → `ui/options_menu.c`

**`store1.c`（396 行）＋ `store2.c`（1085 行）**
- 品ぞろえ → `store/store_stock.c`：`store_check_num`・`insert_store`・
  `store_carry`・`store_destroy`・`store_init`・`store_create`・`store_maint`
- 値段 → `store/store_price.c`：`item_value`・`sell_price`・
  `noneedtobargain`・`updatebargain`
- 値切り → `store/store_haggle.c`：`prt_comment*`・`*_insults`・`get_haggle`・
  `receive_offer`・`purchase_haggle`・`sell_haggle`（#12・#30 の 2 組がここに並ぶ）
- 画面 → `store/store_ui.c`：`display_*`・`store_prt_gold`・`get_store_item`・
  `store_purchase`・`store_sell`・`enter_store`

### store1・store2 の下調べ（2026-09-30、`f39aae5` の時点）

読みとりと、別の worktree でのビルドだけで測った。関数は `store1.c` に 11 個（`static` 2）、
`store2.c` に 26 個（`static` 25。外に出ているのは `enter_store` だけ）、`static` の表と
変数は `store2.c` に 10 個（`comment1`〜`comment6` の表 9 つと `last_store_inc`）。

- **上の表に無かったもの。** `haggle_commands`（呼び手は `purchase_haggle`・`sell_haggle`
  だけ）、`prt_haggle_comment` と `comment_count`（#12-B）、表 9 つと `last_store_inc`
  （読み手は値切りの 3 本だけ）→ どれも `store_haggle.c`。`insert_store`・`store_create`
  の前方宣言（`store1.c:21-22`）は本体と一緒に `store_stock.c`。
- **`static` と呼び手が割れる。** 画面の `store_purchase`・`store_sell` が値切りの
  `purchase_haggle`・`sell_haggle`・`increase_insults` を呼ぶ。`decrease_insults` と
  `prt_comment1`（＋表 `comment1`）の呼び手は**画面の側だけ**。案のままなら外に出す名前は
  5 つ、`decrease_insults` と `prt_comment1` を `store_ui.c` に置けば 3 つ。
- **store2 の確かめ方は、テストと差分の読みだけにする**（2026-09-30、ユーザーの判断）。
- **`prt_comment1` は画面なので `store_ui.c`、`decrease_insults` は `insult_cur` を書くので
  `store_haggle.c`**（2026-09-30、ユーザーの判断）。外に出した名前は 4 つ（`store_haggle.h`）。
- **store2 は機械語の一致では確かめられない。** 本体の `-O2` では `store2.o` に記号が
  残るのは 8 本だけで（`nm`）、`purchase_haggle`・`sell_haggle`・`prt_comment1〜6`・
  `display_store` など 15 本は呼び手の中に溶けこんでいる。値切りを別のファイルに出すと
  溶けこまなくなり、`store_purchase`・`store_sell` の機械語が変わる。`store1` は
  値段と品ぞろえのあいだの呼びだしがいまも関数呼びだしのままで（溶けているのは
  `store_maint` の中の `store_create` だけで、同じ行き先）、一致が見込める。
- **外への依存。** 値段と品ぞろえは画面も入力も呼ばない（値段は `desc`・`stores`・
  `tables`・`treasure`・`player_race` だけ）。値切りは `ui/io.c` を直に呼ぶ、対話つきの
  処理。画面は `moria1.c` の `get_item`・`inven_command` を呼ぶ（`moria1` は libcore に
  入っていない）。`dungeon/generate.c` → `store_maint` と、品ぞろえ → `floor_item_at`・
  `popt` で、`dungeon/` と `store/` が互いに依存する。
- **テスト。** `store1.o`・`store2.o` を引くテストは 0 本。本物に届くのは
  `haggle_comment_test`（`#include "store2.c"`）の `prt_comment2`・`prt_comment3`・
  `prt_haggle_comment` と表 4 つだけで、**残りの 30 個余りは壊しても赤くならない。**
  値段の 4 本は依存が小さく、`concat`（#47）と同じく先にテストを足せそう。
  `haggle_comment_test` の `#include` は値切りを移すコミットで付けかえる（付けかえると、
  代役 10 個のうち 6 個が要らなくなり、`draw_cave` 経由で引いていた `misc3.o` などの
  49 本も減るはず）。同じテストの 43-44 行の「-Wformat-overflow を落としている」は古い。
- **字面を変えずに運ぶところ。** #12 の `prt_haggle_comment` と表（要素数と引数の順は
  25 件が固定）、#30 の `purchase_haggle`・`sell_haggle`（統合しない）。B22
  （`store_sell` の `mask`）。ほかに、`int16_t last_store_inc` に `int32_t` を入れる
  8 か所と、`display_cost` と `display_inventory` の書式の違いがある（どれも直さない）。
- **古い行番号のコメント。** `store1.c`・`store2.c` の名前は外の 18 ファイル・39 行に出る。
  すでにずれている行番号（`player_race.h:94` の「store1.c:139」、`str_insert_test.c` の
  「store2.c:145-146」など）もある。`check_strength_test.c:390` の「store1.c の買い物」は
  もとから誤り（`store_purchase` は `store2.c`）。

**順番の提案。** ① 値段 → `store_price.c`（`static` 0。先にテストを足す案もある）→
② 品ぞろえ → `store_stock.c`（`store1.c` が消える。misc1 の `popt`・`pusht`・`randint`
の行き先に依存するので misc1 のマージの後）→ ③ 画面を先に `store_ui.c` へ出して
`store2.c` を値切りだけにする → ④ 値切り → `store_haggle.c`（`store2.c` が消える。
`#include` の付けかえは 1 度で済む）→ ⑤ コメント。③④ の確かめかたはユーザーに相談する。

**`misc3.c`（1899 行）**：#42 の 10 塊。行き先は
[done/33-41-misc3.md](done/33-41-misc3.md) の「#33 の精査」の表。戦闘の塊
（`attack_blows`・`tot_dam`・`critical_blow`）は `combat/hit_rolls.c` へ。

**`moria1.c`〜`moria4.c`（1711・607・1063・1064 行）**

| 行き先 | 関数（いまの場所） |
|---|---|
| `player/player_bonuses.c` | `change_speed`・`py_bonuses`・`calc_bonuses`（1） |
| `ui/inven_menu.c` | `show_inven`・`describe_use`・`show_equip`・`takeoff`・`verify`・`inven_screen`・`inven_command`・`get_item`（1） |
| `ui/direction.c` | `map_roguedir`・`get_dir`・`get_alldir`（1） |
| `dungeon/lighting.c` | `no_light`・`move_rec`・`light_room`・`lite_spot`・`sub*_move_light`・`move_light`（1） |
| `player/rest_command.c` | `disturb`・`search_on`・`search_off`・`rest`・`rest_off`（1） |
| `dungeon/search.c` | `change_trap`・`search`（2） |
| `player/run_path.c` | `find_init`・`find_run`・`end_find`・`see_wall`・`see_nothing`・`area_affect`（2） |
| `dungeon/traps.c` | `hit_trap`・`chest_trap`（3）、`disarm_trap`（4） |
| `item/spellbook.c` | `cast_spell`（3）。misc3 の呪文の塊と並べる |
| `player/player_move.c` | `move_char`・`carry`（3）。misc3 の移動の塊と並べる |
| `monster/monster_death.c` | `delete_monster`・`fix*_delete_monster`・`summon_object`・`delete_object`・`monster_death`（3） |
| `dungeon/terrain_commands.c` | `openobject`・`closeobject`・`twall`（3）、`tunnel`・`bash`（4） |
| `ui/look.c` | `look`・`look_ray`・`look_see`（4） |
| `combat/` | `test_hit`・`take_hit`（1）、`minus_ac`・`*_gas`・`*_dam`（2）、`mon_take_hit`・`py_attack`（3）、`inven_throw`・`facts`・`drop_throw`・`throw_object`・`py_bash`（4）。行き先は下記 |

**順番**：`misc4` → `misc1` → `misc2` → `store1/2` → `misc3`（#42）→
`moria1〜4`。小さいものから始め、`misc3_stubs.c` の代役が多く当たるものほど
後にする。

### misc3 の下調べ（2026-09-30、`59e0c92` の時点）

読みとりと、/tmp の写しでのビルドだけで測った（コミットは 0）。この時点の `store2.c` は、
いまの `store/store_haggle.c`（#55）。

- **状態:** `src/misc3.c` は 1899 行・77 関数（static 7）。#33 の 87 関数から #40・#41 の 10 を引いた数。include は 35 本で 10-51 行。static のデータが 2 つ。
  - `stat_names` は B だけが使う。
  - `blank_string` と `#define BLANK_LENGTH 24` は B と D の `get_name`（:820）が使う。
- **塊の数え直し（関数の数は #33 の表とすべて一致）:**

| 塊 | 関数 | 今の行数 | #33 表 | 表の行き先 | 今の事情 |
|---|---:|---:|---:|---|---|
| A 配置 | 7 | 160 | 161 | `place.c` | `dungeon/object_place.c`（popt ほか）が既にある |
| B 描画 | 35 | 468 | 475 | `char_screen.c`（dir 無し） | static の prt_lnum/prt_7lnum/prt_num/prt_long/prt_int を持つ |
| C' 能力値の変更 | 6 | 124 | 115 | `stats.c` | `player/stats.c` は葉モジュール。C' は prt_stat・calc_bonuses・calc_* を呼ぶので、置き場の規則に反する |
| D 名前 | 2 | 59 | 50 | char_screen に同居させるか別 | B の display_char と blank_string を使う |
| E 持ち物 | 10 | 219 | 210 | `inventory.c` | `item/inventory.c` は葉の状態モジュール。規則上は兄弟ファイルになる |
| F 呪文 | 6 | 483 | 525 | spellbook.c | layout.md では `item/spellbook.c` |
| G 経験値 | 3 | 64 | 82 | `player_level.c` | `player/player_level.c` は葉モジュールで、規則とぶつかる |
| I wizard | 1 | 18 | 18 | wizard.c | `ui/wizard.c` |
| J 打撃 | 4 | 123 | 120 | `combat.c` | layout.md では `combat/hit_rolls.c` と `combat/player_damage.c`（player_saves） |
| K 移動 | 3 | 118 | 118 | `movement.c` | layout.md では `player/player_move.c` |

  - 合計は 1836 行で、冒頭の 63 行を足すと 1899 行。
  - C' は 498-620 行で、B の中に挟まっている。J と K は交互に並ぶ（critical_blow と player_saves のあいだに mmove）。
- **K の中身は性質が 3 通り:**
  - find_range は持ち物への問い合わせ。呼び手は 10 ファイル（creature dungeon eat magic potions prayer scrolls spells staffs wands）。
  - mmove は幾何に近い。呼び手は 7 ファイル（creature dungeon generate moria2-4 spells）。
  - teleport は移動。
- layout.md が hit_rolls を「副作用が無い」とするのは不正確。tot_dam は recall_update_characteristics を、critical_blow は msg_print を呼ぶ。
- 外から呼ばれていないのに static でない関数が 3 つある: modify_stat、prt_field、spell_chance。
- **外向きの依存（-O0 の再配置を定義元の .o に対応づけた）:**
  - A: desc dungeon_level dungeon_map dungeon_size floor_items geometry io item_enchant object_levels object_place player_pos rnd sets treasure
  - B: abilities command_state dungeon_level io map_view player、player_* 約 15 本、progress save_state score_death variable。B からほかの塊は呼ばない。
  - C': moria1（calc_bonuses）player_class player_status_flags rnd py。塊をまたいで B（prt_stat）、F（calc_spells・calc_mana）、G（calc_hitpoints）を呼ぶ。
  - D: files player_bio io、B の display_char、blank_string
  - E: burden desc dungeon_map floor_items inventory io item_ident moria1（calc_bonuses change_speed takeoff）moria3（delete_object）object_place（popt）player_body_weight player_pos player_status_flags rnd
  - F: inventory io moria1（no_light）player player_class player_level player_mana player_spells_to_learn player_status_flags player_timed_effects rnd spells_known stats variable
  - G: hp_table io player_class player_hp player_level player_status_flags stats。塊をまたいで F（calc_mana calc_spells）と B（prt_level prt_title、static の prt_long）を呼ぶ。
  - I: io progress score_death
  - J: io monsters player player_class player_level player_saving_throw rnd stats tables variable
  - K: creature dungeon_map dungeon_size geometry inventory map_view moria1（lite_spot move_rec）pending_teleport player_pos rnd
- **層の規則:** `layer_deps.py --check` は 827 辺・違反 0。規則があるのは core/ だけで、core/ へ行く塊は無いので、どの塊も規則を破らない。combat/ は LAYER_ORDER には既にあるが、ディレクトリはまだ無い。作るなら sources.mk の SRC_SUBDIRS に足す。
- **ビルド定義:** externs.h:316-376 に「// misc3.c」の見出しと 70 宣言。ほかに makefile:187（`misc3.o: burden.h $(HEADERS_FULL)`）、makefile.win:105、sources.mk:22。
- **テスト:** `--who misc3.o` は 9 本で、EXTRA = misc3_stubs.c の 9 本とちょうど一致する。

| テスト | misc3.o を引く関数 | 件数 |
|---|---|---:|
| calc_hitpoints | calc_hitpoints | 13 |
| calc_spells | calc_spells | 17 |
| check_strength | weight_limit | 24 |
| gain_spells | gain_spells | 13 |
| haggle_comment | draw_cave（`#include "store2.c"` 経由） | 25 |
| inven_stack | inven_check_num | 40 |
| objdes | prt_experience（item_ident.o 経由） | 21 |
| object_levels | get_obj_num | 18 |
| put_misc3 | put_misc3 | 24 |

  - `--count` は各 45 メンバー、haggle だけ 49。
  - `#include "misc3.c"` するテストは 0 本。REFACTORING_PLAN:262 の「calc_spells_test・gain_spells_test が include する」は古い。
  - `tests/misc3_stubs.c` は 516 行・大域シンボル 55 個。冒頭コメントは今も「2212 行・6 責務」。
- **`--shadows`（9 本とも）:** 代役が本物を覆うメンバーは 13 個（creature files geometry inscription io item_enchant map_view monsters moria3 object_place rnd sets variable）。moria1 系の代役は moria1 が libcore に無いので表に出ない。
- **misc3.o を引かずに misc3 の名前を代役にしているテスト:**
  - item_ident_test（fixture.c）: prt_experience
  - movement_rate_test（creature_stubs.c）: dec_stat find_range inven_destroy mmove player_saves prt_cmana prt_experience prt_gold
  - save_bool_test・store_save_test（save_stubs.c）: check_strength prt_experience
- **本物に届くテスト（gcov、全 1671 件）:** 77 関数のうち通るのは 15 で、**残る 62 は 0%**。

| テスト | 通る関数（行カバレッジ） |
|---|---|
| calc_hitpoints_test | calc_hitpoints 100% |
| calc_spells_test | calc_spells 95.65% |
| check_strength_test | check_strength 100%、inven_check_weight 100%、weight_limit 80% |
| gain_spells_test | gain_spells 97.14%、calc_mana 39.39%、print_spells 64.29%、spell_chance 71.43% |
| inven_stack_test | inven_carry、inven_check_num、items_can_stack（いずれも 100%） |
| object_levels_test | get_obj_num 91.67% |
| put_misc3_test | put_misc3 100%、likert 88.89% |

  - haggle_comment と objdes は misc3 のコードを 1 行も通らない。
  - 塊で見ると、A・B・E・F・G は一部が保護されている。C'・D・I・J・K は保護が 0。
- **-O2 で溶けるもの（nm）:** 非 static の 70 関数は全部 T で残る。static 7 つはすべて溶けている。clone が 3 つ: calc_mana.part.0、prt_state.part.0、get_obj_num.part.0。
  - 塊をまたぐインライン展開:
    - inc_stat・dec_stat・res_stat が prt_stat（B）を展開
    - change_name が display_char（B）を展開
    - prt_experience の中の gain_level が calc_mana・calc_spells（F）、prt_level・prt_title（B）、prt_long を展開
  - 塊の中だけの展開: A の alloc_object が place_rubble・place_trap を、E が items_can_stack・weight_limit を展開する。B の中にも多数。
  - -O2 -fno-inline でも ICF が prt_int を prt_long に、prt_num を prt_lnum に畳む。prt_7lnum と prt_long は .constprop.0 になる。
- **試しの移動（/tmp/m3s の写し。塊ごとに新ファイル src/trial_X.c へ出し、畳まれない 73 関数を dis_compare）:**
  - **-O2 -fno-inline:** B と G の試しで prt_experience だけが DIFFERENT、ほかは全部 same。prt_long が 2 ファイルに複写されるため、ipa-cp の定数伝播（`mov $0xe,%esi` が消える）と ICF の別名が変わる。
    - `-fno-ipa-cp` を足しても `jmp <prt_int>` と `<prt_long>` の差が残る。
    - `-fno-ipa-icf` を足すと prt_gold と prt_experience が DIFFERENT。
    - **`-O2 -fno-inline -fno-ipa-cp -fno-ipa-icf` で B も G も 73/73 same。**
  - **素の -O2:** A・E・I・J・K は 70/70 same。変わるもの:
    - B: inc/dec/res_stat、change_name、prt_experience
    - C': set_use_stat、inc/dec/res_stat
    - D: change_name
    - F・G: それぞれ set_use_stat、gain_spells、calc_mana、prt_experience
  - 健全性の確認: 移した critical_blow の文字列を 1 つ変えると DIFFERENT になった。
  - **既存ファイルへの合流（-O2 -fno-inline）:** I → ui/wizard.c は 4/4 same、A → dungeon/object_place.c は 10/10 same。**ただし A を合流させると object_levels_test がリンクで落ちる（`multiple definition of 'popt'`）。** misc3_stubs.c の popt と、get_obj_num が引きこむ本物の object_place.o がぶつかる。
  - **新ファイルへの試し（10 塊すべて）は GREEN:** 1671 件、失敗 0、multiple definition も undefined reference も 0。
    - 新しい .o を引くテスト: trial_A は object_levels、trial_B・F・G は 9 本すべて、trial_E は check_strength・haggle・inven_stack。trial_C'・D・I・J・K は 0 本。
    - 最後まで misc3.o は 9 本とも引かれたまま。
- **字を変えずに運ぶもの:**
  - B1 / #10: items_can_stack と「must agree」のコメント（970-982）
  - #39: `known1_p(existing) == known1_p(incoming)` は今は :982（計画の :1052 は古い）
  - B8: inven_carry の `for (locn = 0;; locn++)`（:1070）
  - B9: inven_check_num の inventory_slot_count の判定と inven_carry の食いちがい
  - B18: prt_winner の「Duplicate」の分岐（`& 0x4`、:491）には到達しない。0x2 は enter_wiz_mode（:1652）が立てる。
- **番号の衝突:** gain_spells の :1410 のコメントにある「バグ候補 B22」（magic_spell[-1] のアドレスを職業の判定より先に作る）は worklog #18-12-28（2026-09-28）で付いた名前で、bugs.md には入っていない。bugs.md の B22（b62de9df）は store_sell のマスクで、別物。calc_spells:1262 も同じアドレスを作る。
- **行番号がもう合わない記述:** bugs.md の B10・B11 が指す「misc3.c:973/977」のコメントは src のどこにも無い。
- **日本語のコメント:** 424-426、624-625、643-646、667-671、822-826、957-959、1292-1293、1379-1380、1404-1406、1410-1412、1828-1830。
- **孤立したコメント:** :1659 の「// Weapon weight VS strength and dexterity -RAK-」は、attack_blows と空行 1 行で離れている。
- **古い行番号のコメント:** src と tests で「misc3.c」を名指しするのは 52 ファイル・135 行（misc3.c 自身を除く）。「misc3.c:N」の形は 24 ファイルに 46 箇所。
  - 関数を名指しする 23 箇所のうち、21 箇所がずれていて、2 箇所はほぼ合う（calc_hitpoints_test:17 の 1582 は今 1579、player_bio_test:221 の 817 は今 818）。残りの 23 箇所は関数を名指ししない。
  - 例: calc_hitpoints_test「1624」（今 1616）、fixture.c と item_ident_test「1838」（prt_experience は今 1603）、misc3_stubs「2103」（teleport は今 1873）、player_class_test:280「1404」、src/player/player_body_weight.h:64,74、player_class.h:88,89,115。
  - 文書側も古い行を指す: REFACTORING_PLAN #10 #11 #17 #23 #39 #40 #41、bugs.md。
- **順番の案（推測）:**
  1. I: 18 行・届くテスト 0・wizard.c への合流で same
  2. J: combat/ を今作るなら SRC_SUBDIRS を足す
  3. K: 3 関数の置き場を決めてから
  4. A: object_place.c には合流させず新ファイルへ。合流させるなら先に popt の代役を片づける
  5. E: inventory.c の兄弟ファイル
  6. D と B: static の補助関数と blank_string の扱いを決めてから
  7. C'・F・G は最後にまとめて: 互いに呼び合い、-O2 で互いに展開される。B と G は `-fno-ipa-cp -fno-ipa-icf` で比べる

  A・E・I・J・K は素の -O2 でも same なので、比べやすい順でもある。

### 迷いどころ

1. C'・E・G の行き先。表どおり stats.c・inventory.c・player_level.c に入れると葉モジュールの規則に反する。兄弟ファイル（例: stat_ops.c・inven_ops.c・level_ops.c）にするか。
2. B を 1 ファイル（char_screen）にするか、ステータス行とキャラ画面に割るか。置くディレクトリ（ui/ か）。
3. D を B と同じファイルに置くか。
4. K の 3 関数を 1 ファイルに置くか。find_range は持ち物への問い合わせ、mmove は幾何、teleport は移動。
5. A を object_place.c に合流させるか（先に misc3_stubs の popt を片づける）、新ファイルにするか。
6. combat/ を今作るか。hit_rolls を「副作用が無い」とする layout.md の記述を直すか。
7. prt_long・blank_string・stat_names を複写するか、公開するか、B と一緒に動かすか。
8. 比べるフラグ。B・G は `-O2 -fno-inline -fno-ipa-cp -fno-ipa-icf`、その他は -O2 -fno-inline か素の -O2 で足りる。
9. 「B22」の番号の衝突（gain_spells:1410 のコメントと bugs.md）。バグ候補に新しい番号を振るか。
10. 古い行番号のコメント（24 ファイル・46 箇所）と日本語のコメント（11 箇所）を移動と一緒に直すか、別のコミットにするか。
11. 移動の前に、届くテストが 0 の 62 関数（とくに C'・D・I・J・K）へステップ A として保護を足すか。
12. 外から呼ばれない modify_stat・prt_field・spell_chance を、このとき static にするか（ふるまいは変わらないが、純粋な移動ではなくなる）。
13. misc3_stubs.c の冒頭コメントの数字（2212 行・6 責務）と、REFACTORING_PLAN:262 の #include の記述が古い。直す時期をいつにするか。

### misc3 で決まったこと・やったこと（2026-09-30）

上の 13 点は、7 を除いて案のとおりにした（ユーザーの判断）。

1. C′・E・G は兄弟のファイル: `player/stat_ops.c`・`item/inven_ops.c`・`player/level_ops.c`。
2. B はステータス行 `ui/status_line.c` と人物画面 `ui/char_screen.c` の 2 本に割った。
3. D（`get_name`・`change_name`）は人物画面と同じ `ui/char_screen.c`。
4. K は 3 か所へ: `mmove` → `dungeon/geometry.c`、`find_range` → `item/inven_ops.c`、
   `teleport` → 新しい `player/player_move.c`。
5. A は新しい `dungeon/object_alloc.c`（object_place.c に合流させない）。
6. `combat/` を作った（`hit_rolls.c`・`player_damage.c`）。上の表の「副作用が無い」も直した。
7. **ユーザーの変更:** `prt_long`・`blank_string`・`stat_names` は複写も公開もせず、
   **UI 層の操作として呼べるようにした**（ヘッダーに実装は書かない）。`ui/screen_fields.c` /
   `.h` に 4 本: `erase_field(width, row, column)`（空白の表の後ろから width 字）・
   `prt_stat_name(stat, row, column)`・`prt_num`・`prt_long`。2 つの表は `static` のまま。
   B を割ると `prt_num` も 2 本に跨がるので、同じ扱いにした。先に misc3.c の中で
   呼び手 12 か所を 2 つの操作越しにし（#42-10、素の `-O2` で呼び手 13 本が同じ）、
   それから移した（#42-11）。
8. 比べるフラグは案のとおり。どの移動も、動かした関数と同じファイルに残る関数の機械語が
   直前のコミットと一致した。例外は `static` を外した #42-11 だけで、
   `-fno-ipa-cp -fno-ipa-icf` では呼び手 27 本が同じ、4 本は頭の `endbr64` だけが違う。
   `-O2 -fno-inline` では `prt_long.constprop`（列 6 を焼きこんだもの）が無くなり、
   呼び手が列を自分で渡すようになった。
9. `gain_spells` のコメントの「B22」は **B23** に付けなおした（bugs.md）。
10. コメントは移動ごとに別のコミットで直した。移った関数の中の日本語のコメントは英語に。
11. テストは足していない（どの移動も字を変えない運びで、機械語で確かめた）。
12. `modify_stat`・`prt_field`・`spell_chance` の `static` 化は後始末で決める。
13. `misc3_stubs.c` の冒頭と REFACTORING_PLAN:262 は最後のコミット（#42-19）で直した。

- **順番:** I → J → K → A → E → （表の下ごしらえ）→ B → D → C′ → F → G。最後の G は
  `misc3.c` を `git mv` で `player/level_ops.c` にした。**misc3.c は無くなった。**
- **include:** 新しいファイルは misc3.c の include を全部写してから、1 つずつ外して警告の
  増えないものを落とした。makefile と makefile.win の依存にも同じものを書いた。
- **数:** テスト 1671 件・75 本 GREEN、警告 0、globals 42、層 963 本・128 単位で違反 0、起動 1
  （#42 の前は 834 本・118 単位）。

### moria1〜4 で決まったこと・やったこと（2026-10-01）

行き先は上の表のとおり。迷いどころ 8 点はすべて案のとおり（ユーザーの判断）。

1. 各ファイルの最後の塊は `git mv` で残した：moria1 → `ui/inven_menu.c`、moria2 →
   `player/run_path.c`、moria3 → `monster/monster_death.c`、moria4 → `ui/look.c`。
2. makefile.test の `LIB_EXCLUDE` は moria1.c を `inven_menu.c` に置きかえただけ。
3. ファイルをまたいで呼ばれるようになった `hit_trap`・`py_bash` の `static` を外した
   （違いは頭の `endbr64` だけ）。
4. 宣言は externs.h の新しいファイルの見出しの下へ。
5. 2 列で並べた：moria2 → moria1 と moria4 → moria3。
6. 確かめは `-O2 -fno-inline` と `-O2 -fno-inline -fno-ipa-cp -fno-ipa-icf` の機械語。テストは足していない。
7. コメントは別のコミット。移った関数の中の日本語のコメントは英語に。
8. `verify`・`minus_ac` の `static` 化は後始末で決める。

- **数:** テスト 1671 件・75 本 GREEN、警告 0、globals 42、層 1104 本・138 単位で違反 0、起動 1。
- **後始末に回したもの:** `verify`・`minus_ac`・`modify_stat`・`prt_field`・`spell_chance` の
  `static` 化。`inven_menu.c` に残る -RAK- の「town level code」のコメント。
  `player_search_skill.h:104`（search() の場所が誤り）、`move_char` の上の「pre-declared」、
  `fix1_delete_monster` の呼び手の説明。
- **代役の名前の重なり:** `misc3_stubs.c`・`save_stubs.c` の `calc_bonuses`・`change_speed` は
  libcore の `player_bonuses.o` と同じ名前。いまはどのテストもそのメンバーを引かないので
  リンクが通る。`py_bonuses` を要るテストができると二重定義で落ちる。

### combat の残りで決まったこと・やったこと（2026-10-01）

迷いどころ 5 点はすべて案のとおり（ユーザーの判断）。

1. `make_attack` → 新しい `combat/monster_melee.c`（中は割らない。#25 の棚上げはそのまま）。
   呼び手が creature.c の `make_move` なので `static` を外した（違いは頭の `endbr64` だけ）。
2. `get_flags`・`fire_bolt`・`fire_ball`・`breath` → 新しい `combat/projectiles.c`。
   spells.c に残る `hp_monster` の後ろ向きの jmp 1 つが 2 バイトの形から 5 バイトの形に
   なった（飛び先と命令は同じ。spells.o の中の位置が変わったため）。
3. **movement_rate_test は変えない。** creature.c を丸ごと取りこむこのテストは、libcore から
   本物の `monster_melee.o` を引くようになった（テストは呼ばない）。代償は引かれるメンバーが
   増えること。`creature_stubs.c` の代役と同じ名前を引いたメンバーが定義すると二重定義で落ちる。
4. `get_flags` の `static` 化は後始末で決める。
5. コメントは別のコミット。移動でずれた creature.c・spells.c の行番号は落とした。

- **数:** テスト 1671 件・75 本 GREEN、警告 0、globals 42、層 1139 本・140 単位で違反 0、起動 1。
- **後始末に回したもの:** `creature_stubs.c` の代役のうち 20 本（`test_hit`・`*_dam`・
  `monster_attack_*` など）は creature.c からは引かれず、`monster_melee.o` の穴を埋めている。
  冒頭の組み立ての例の `src/creature.c` と「本物のソースを 1 つも足さない」の説明が古い。
  `movement_rate_test.c` の SUSPICIOUS の注記は、creature.c のコメントが直ったことを
  反映していない。`tables.c:89`・`save_bool_test.c:38` の externs.h の行番号が古い。

### 後始末で決まったこと・やったこと（2026-10-01）

進めかたは案のとおり（ユーザーの判断）。経緯だけのコメントは消す、#37・#44 は棚上げのまま。

1. **12 本を `static` に。** externs.h に宣言があるのに定義したファイルの中でしか使われて
   いなかった `sleep_in_seconds`・`tilde`（io.c）、`popm`、`m_bonus`、`minus_ac`、`prt_field`、
   `modify_stat`、`spell_chance`・`get_spell`（spellbook.c）、`verify`、`get_rnd_seed`、`get_flags`。
   1 ファイル 1 コミット。定義より前で使う 3 本には static の前方宣言を置いた（`tilde` の
   ものは定義と同じ `#ifndef _WIN32` の中）。
   - `-O2 -fno-inline -fno-ipa-cp -fno-ipa-icf` では違うのは 12 本だけ。`endbr64` と nop を除き、
     飛び先を命令の番号に置きかえると並びは同じ。
   - `-O2 -fno-inline` では `minus_ac`・`prt_field` に `.constprop.0` ができ、呼ぶ側 4 本
     （`acid_dam`・`corrode_gas`・`prt_stat_block`・`prt_title`）が定数の引数を渡さなくなった。
2. **使われていない代役 13 本を消した。** `misc3_stubs.c` の `check_view`・`creatures`・`distance`・
   `lite_spot`・`monster_get_creature`・`move_rec`・`recall_update_characteristics`、`save_stubs.c` の
   店の買いとり判定 6 本。リンクする各テストで `ld --cref` に参照が無いことを確かめた。
   各テストのリンクマップに出るメンバーは変わらず、実行形式から消えたのは 13 個の名前だけ。
3. **古いコメントを直した**（moria1〜4 と combat の残りの節の「後始末に回したもの」のうち、
   static 化を除く全部）。
4. **経緯だけのコメントを消した。** src と makefile の「Moved out of misc1.c unchanged (#54)」
   「came from misc3.c (#42)」、externs.h の「were misc1.c until #54」など。同じ段落の説明は残した。
   機械語は全関数で 1 の先端と同じ。

- **数:** テスト 1671 件・75 本 GREEN、警告 0、globals 42、層 1139 本・140 単位で違反 0、起動 1。
- **残したもの:**
  - 窓口の理由を「前はどこに散らばっていたか」で説明している段落 13 か所（`player_speed.h`・
    `panel.h` など）。
  - tests のコメントの由来の記述。テストの成り立ちの説明と一体なので切り分けなかった。
  - コメント中の `file.c:NNN` の行番号（約 280 か所）。機械では正しさを確かめられないので、
    見つかったものだけ直した。
  - `str_insert_test.c:40` の「#41 の移動が済むまで」（もう古い）。`fixture.c` の `py` もどのテストからも
    参照されていない。
- **#37・#44 は #38 まで棚上げ。** `creature_stubs.c` と `misc3_stubs.c` の重なりは 18 本
  （`distance` を消した後）。代役が libcore の本物と同じ名前を持つのはこの組み立て全体の性質で
  （`misc3_stubs.c` は 55 本中 43 本）、上の `calc_bonuses` だけの話ではない。引いたメンバーが
  同じ名前を定義すると二重定義で落ちる。

### #38 で決まったこと・やったこと（2026-10-01）

進めかたは案のとおり（ユーザーの判断）。

- **「1 実行形式 1 対象 module」の意味：** 実行形式が引くのは対象の module と、対象（と
  テスト本体）が本当に呼ぶものだけ。共有の足場がつれてくる単位は 0。
- **段階 A（テストだけ）を済ませた。** `misc3_stubs.c` を使っていた 9 本を、対象 module ごとの
  足場 7 つ（`level_ops`・`spellbook`・`inven_ops`・`desc`・`object_levels`・`char_screen`・
  `store_haggle` の `_fixture.c`）に移し、`misc3_stubs.c` を消した。
  - `fixture_reset()` は、そのテストにリンクされる窓口だけを 0 に戻す。外したのは、ほかに誰も
    リンクしていない窓口の分だけ（読む者がいないので、ふるまいは変わらない）。
  - 代役は `tests/shared_stubs.c` 1 つにまとめた。libcore の中を呼ばないので、
    使わないテストにリンクしても単位は増えない。記録は `shared_stubs_reset()` で消え、
    各足場の `fixture_reset()` が呼ぶ。
  - 足場を分けた直後は写しが 44 本・226 定義に増えたので、まとめてからマージした
    （ユーザーの判断）。重なりは develop と同じ 32 本・76 定義。

| テスト | 前 | 後 |
|---|---|---|
| haggle_comment | 40 | 7 |
| calc_spells・gain_spells | 39 | 12 |
| calc_hitpoints | 39 | 27 |
| put_misc3・objdes | 43・39 | 32 |
| check_strength・inven_stack | 44 | 38 |
| object_levels | 45 | 38 |

- **残る芋づるの根（段階 B の候補、未着手）：** `item_ident.o` が品物を覚えたときに経験値を
  足して `prt_experience()`（`level_ops.o`）を呼び、そこから `status_line.o`（窓口 12 個ほど）と
  `spellbook.o` が付いてくる。これを切ると objdes 32→9、check_strength・inven_stack・
  object_levels 38→16 になる見こみ（`nm` での試算）。src の設計変更なので、やるかは
  ユーザーと決める。
- **変えなかったもの：** save_bool・store_save（51。save.c がもともと全部の状態に触れる）、
  movement_rate（24。creature.c を代役 28 本で囲って切りはなしている）、item_ident（7）。
- **#37：** 重なりは `creature_stubs.c` と `shared_stubs.c` のあいだの 12 本（定義を `nm` で
  突きあわせた数。`fixture_reset` を除く）。前に書いた「18 本」は、後始末で消した 7 本の
  うち `distance` しか引いていなかった。
- **#44：** 定数を返す代役は中身を変えずに運んだ。
- **気づいたこと：** 古い `fixture_reset()` には、赤外視の距離を 0 に戻す理由のコメントだけが
  あり、呼びだしは無かった。新しい足場にも無い。いまのテストは自分で値を置いてから読むので
  影響はない。

## combat/

`combat/` には新しく作るファイルだけを置く。どれも番号つきのファイルか、
大きなファイルから切り出す。

| ファイル | 中身（いまの場所） | 目安 |
|---|---|---|
| `hit_rolls.c` | `test_hit`（moria1。#56 で移した）、`attack_blows`・`tot_dam`・`critical_blow`（misc3。#42 で移した） | 約 150 行。副作用が無いわけではない —— `tot_dam` は思い出（`recall_update_characteristics`）を書き、`critical_blow` は `msg_print` を呼ぶ。乱数と画面の代役を当てれば単体でテストできる |
| `player_melee.c` | `py_attack`（moria3）、`py_bash`（moria4）。#56 で移した | |
| `monster_melee.c` | `make_attack`（`creature.c`。棚上げ #25）を中は割らずに移す。#57 で移した | 約 600 行 |
| `player_damage.c` | `take_hit`（moria1）、`minus_ac`・`*_gas`・`*_dam`（moria2。#56 で移した）、`player_saves`（misc3。#42 で移した） | |
| `monster_damage.c` | `mon_take_hit`（moria3。#56 で移した） | |
| `projectiles.c` | `get_flags`・`fire_bolt`・`fire_ball`・`breath`（`spells.c`）。#57 で移した | |
| `throw.c` | `inven_throw`・`facts`・`drop_throw`・`throw_object`（moria4。#56 で移した） | |

- 重複の #51（光る手）と #29（飛翔のループ）の 2 組が、同じディレクトリーに
  並ぶ。どちらもまとめはしない（棚上げの理由は変わらない）が、見比べやすくなる。
- `creature.c`（1651 行 → 約 1050 行）と `spells.c`（2141 行 → 約 1790 行）も
  触ることになる。そのため、番号つきのファイルが片づいたあとに別のブランチで行う。

## D0：行き先の表

2026-09-29、`68c7ad0` の時点で測った。**まだ 1 本も動かしていない。** ユーザーに
見てもらい、末尾の「決まったこと」のとおりに決まった。

**数えた本数。** `src/*.c` は **109 本**（上の「109 本」と一致）、`src/*.h` は
**76 本**。`.h` のうち 68 本は同じ名前の `.c` と対で、`.c` に付いていく。
残りの 8 本はヘッダだけのもの（`backend_ncurses.h`・`config.h`・`constant.h`・
`curses.h`・`equipment.h`・`externs.h`・`headers.h`・`types.h`）。

**見たもの。** 各ファイルの冒頭の注釈と関数の一覧、`#include` の並び、それに
`.o` を 1 本ずつ組んで `nm -u` に出る名前（libc を除く）。ncurses を取りこむのは
`#include` で見ると `curses.h`・`platform.c`（`backend_ncurses.h` 経由）・
`render_ncurses.c`・`input_ncurses.c` だけ。`externs.h` はもう何も include
しない（`types.h` が `headers.h` を引くだけ）。

**本数のまとめ。**

| 行き先 | `.c` | `.h` | `.c` の行数 |
|---|---|---|---|
| `core/` | 2 | 1 | 210 |
| `data/` | 8 | 2 | 2731 |
| `player/` | 37 | 36 | 3549 |
| `monster/` | 5 | 4 | 1917 |
| `dungeon/` | 6 | 5 | 1437 |
| `item/` | 14 | 6 | 4927 |
| `store/` | 1 | 1 | 28 |
| `combat/` | 0 | 0 | 0 |
| `ui/` | 14 | 9 | 3208 |
| `save/` | 4 | 2 | 2281 |
| `platform/` | 5 | 4 | 755 |
| `src/` に残す | 3 | 6 | 2347 |
| R で分割（D では動かさない） | 10 | 0 | 9677 |
| 計 | 109 | 76 | |

- `combat/` は 0 本。上の「combat/」の節のとおり、R と「combat の残り」で
  新しく作るファイルだけが入る。
- `src/` に残す `.c` は `main.c` のほかに 2 本（`dungeon.c`・`game_state.c`）。
  上の「`main.c` だけ `src/` に残す」から外れるので、下で訊く。

### R で分割（D では動かさない）

`misc1.c`（824）・`misc2.c`（914）・`misc3.c`（1899）・`misc4.c`（114）・
`store1.c`（396）・`store2.c`（1085）・`moria1.c`（1711）・`moria2.c`（607）・
`moria3.c`（1063）・`moria4.c`（1064）。10 本、どれも対の `.h` は無い。
D のあいだは `src/` に置いたまま。分け先は上の「番号つきファイルの分割（R）」。

### core/（2 本）

| ファイル | 行数 | 理由 |
|---|---|---|
| `rnd.c` | 116 | 乱数の本体。`nm -u` に出る名前が 0。`externs.h` を include するのは自分の宣言のためだけ |
| `str_insert.c`・`.h` | 94・38 | 文字列の道具。`nm -u` が 0、`externs.h` も引かない |

- `rnd.c` の `externs.h` は、include の上では `core/` から `src/` の全域ヘッダへの
  依存になる。リンクの上では何にも頼らない。L3 の検査は `nm` で見るので当たらない。

### data/（8 本）

| ファイル | 行数 | 理由 |
|---|---|---|
| `tables.c` | 217 | 店主・品ぞろえ・品物の見た目の名前（色・木・金属など）・打撃の回数・正規分布の定数表 |
| `treasure.c` | 572 | 品物の定義表 |
| `monsters.c` | 800 | モンスターの定義表 |
| `player.c` | 524 | 人物の表（`py`・種族・職業・称号） |
| `variable.c` | 193 | 全域変数の置き場 |
| `sets.c` | 280 | `set_*` の述語と店の買い取り表。`tables.c` の店の表がこの 6 本を指す |
| `options.c`・`.h` | 66・50 | 設定の旗の表。UI の呼び出しは無く、`save.c` が読む |
| `progress.c`・`.h` | 79・54 | 手番・種・wizard の旗の置き場。依存 0 |

- **`monsters.c`**：表のほかに名前の関数（`monster_name*`）と攻撃の窓口を持つ。
  `is_a_vowel`（`desc.c`）を呼ぶので `data/` → `item/` の依存になる。
  候補は `data/`（表が主）と `monster/`（関数が主）。表の行数が多いので `data/`。
- **`variable.c`**：残りの全域変数と、モンスターの思い出（`recall_*`）の窓口。
  候補は `data/` と `monster/`。まだ全域変数が主なので `data/`。
- **`sets.c`**：候補は `data/` と `item/`（品物の述語が主）。`tables.c` から
  呼ばれるので、同じディレクトリーにそろえた。
- **`options.c`**：候補は `data/` と `ui/`（R で作る `ui/options_menu.c` の隣）。
  `ui/` に置くと `save/` → `ui/` の依存ができるので `data/`。`find_*` などの
  旗は `variable.c` にある。
- **`progress.c`**：候補は `data/` と `save/`（兄弟の `save_state.c`・
  `score_death.c` と同じ区分から出た）。中身は手番の数なので `data/`。

### player/（37 本）

`player_*.c` の 29 本と、その `.h` 29 本はそのまま。どれも `externs.h` を引かない
葉の状態 module。ほかに次の 8 本。

| ファイル | 行数 | 理由 |
|---|---|---|
| `abilities.c`・`.h` | 84・40 | 能力値の計算。`player_*`・`stats.c` と `player.c` の表（`class_level_adj`）を読む |
| `burden.c`・`.h` | 44・60 | 荷の重さ。依存 0 |
| `hp_table.c`・`.h` | 40・42 | レベルごとの HP の表。依存 0 |
| `stats.c`・`.h` | 143・45 | 能力値から修正値への表。`py` だけを読む |
| `spells_known.c`・`.h` | 163・132 | 覚えた呪文。依存 0 |
| `running.c`・`.h` | 61・65 | 走っているかの状態。R の `player/run_path.c` と並べる |
| `pending_teleport.c`・`.h` | 41・38 | 予約された瞬間移動。依存 0 |
| `create.c` | 538 | 人物を作る流れ。prompt を出す（`inkey`・`clear_from` など） |

- **`create.c`**：候補は `player/`（作るのは人物）と `ui/`（画面で訊く流れ）。
  葉ではないので、置いても規則には当たらない。`player/`。

### monster/（5 本）

| ファイル | 行数 | 理由 |
|---|---|---|
| `creature.c` | 1651 | モンスターの行動。上の計画どおり。`make_attack` は後で `combat/` へ |
| `monster_breeding.c`・`.h` | 65・78 | 増殖の予算。依存 0 |
| `monster_levels.c`・`.h` | 75・44 | レベルごとの索引。依存 0 |
| `monster_list.c`・`.h` | 72・86 | 階にいるモンスターの表。依存 0 |
| `monster_turn.c`・`.h` | 54・76 | 手番のモンスター。依存 0 |

### dungeon/（6 本）

| ファイル | 行数 | 理由 |
|---|---|---|
| `dungeon_map.c`・`.h` | 36・145 | 床の表 |
| `dungeon_size.c`・`.h` | 37・86 | 階の広さ |
| `dungeon_level.c`・`.h` | 34・77 | いま何階か |
| `floor_items.c`・`.h` | 77・125 | 床に載っているもの |
| `level_exit.c`・`.h` | 53・53 | 階を出る印。`set_dungeon_level` だけを呼ぶ |
| `generate.c` | 1200 | 階と町の生成 |

- `generate.c` は `panel.h` を引く（`ui/`）。上の計画で `panel.c` は `ui/` なので、
  `dungeon/` → `ui/` の依存が 1 本できる。

### item/（14 本）

| ファイル | 行数 | 理由 |
|---|---|---|
| `inventory.c`・`.h` | 86・55 | 持ち物。葉 |
| `equipment.h` | 45 | 装備の窓口。ヘッダだけ。`inventory.c` に実体があるので隣へ |
| `item_ident.c`・`.h` | 152・66 | 鑑定の記録 |
| `object_levels.c`・`.h` | 80・41 | レベルごとの品物の索引 |
| `desc.c` | 609 | 品物の名前 |
| `device.c`・`.h` | 45・28 | 杖と棒の成功率。`nm -u` が 0 |
| `missile_serial.c`・`.h` | 45・32 | 矢の束の番号。依存 0 |
| `eat.c`・`potions.c`・`scrolls.c`・`staffs.c`・`wands.c` | 195・320・486・174・173 | 品物を使うコマンド |
| `magic.c` | 212 | `cast`（魔法を唱えるコマンド） |
| `prayer.c` | 209 | `pray`（祈るコマンド） |
| `spells.c` | 2141 | 呪文と品物の効き目（68 関数） |

- **`magic.c`・`prayer.c`**：候補は `item/`（R の `item/spellbook.c` の
  `cast_spell` と並べる、と上に書いた）と `player/`。本を使うコマンドで、
  `eat.c` などと同じ形なので `item/`。
- **`spells.c`**：いちばん迷う。中身は探知（`detect_*`）、地形（`earthquake`・
  `destroy_area`・`door_creation`）、モンスターへの効き目、人物への効き目
  （`cure_*`・`lose_*`）、飛び道具（`fire_bolt` など。後で `combat/`）が混ざる。
  呼び手は `magic.c`・`prayer.c` と品物のコマンド 5 本。候補は `item/`
  （呼び手の隣）、`combat/`、新しいディレクトリー（たとえば `magic/`）の 3 つ。
  11 個の中からなら `item/`。

### store/（1 本）

| ファイル | 行数 | 理由 |
|---|---|---|
| `stores.c`・`.h` | 28・33 | 店の表の窓口。依存 0。`store1/2` の分け先はここに増える |

### ui/（14 本）

| ファイル | 行数 | 理由 |
|---|---|---|
| `io.c` | 617 | 画面と入力の古い窓口（`prt`・`msg_print`・`inkey` など） |
| `messages.c`・`.h` | 98・67 | 上の行の履歴 |
| `panel.c`・`.h` | 145・77 | 画面が映している範囲 |
| `render.c`・`.h` | 127・100 | 描画の前面。backend を差しかえる口で、ncurses は引かない |
| `input.c`・`.h` | 70・104 | 入力の前面。`render.c` と対 |
| `view_observer.c`・`.h` | 150・146 | 表示の通知 |
| `input_ended.c`・`.h` | 47・57 | 入力が尽きた回数。`io.c` が読む |
| `command_state.c`・`.h` | 89・101 | 繰り返しの回数と、覚えた向き |
| `inven_command_state.c`・`.h` | 43・43 | 持ち物画面の途中の状態。`screen_touched.c` を呼ぶ |
| `screen_touched.c`・`.h` | 37・35 | 画面を流したかの印 |
| `help.c` | 323 | `ident_char`（記号を訊く） |
| `recall.c` | 683 | モンスターの思い出を画面に出す（`prt`・`inkey`） |
| `files.c` | 355 | ファイルを見せる・書き出す（下記） |
| `wizard.c` | 424 | wizard のコマンド（下記） |

- **`io.c`**：`shell_out`・`user_name`・`topen`・`tilde` の 4 本は OS 寄りで、
  候補は `platform/`。D では割らずに `ui/` へ運ぶ。
- **`render.c`・`input.c`**：候補は `ui/`（上の計画の例）と `platform/`（backend と
  並べる）。backend に依存しないので `ui/`。
- **`panel.c`**：葉の状態 module だが、`generate.c`・`creature.c`・`save.c` など
  12 本が引く。候補は `ui/`（上の計画の例）と `dungeon/`。
- **`command_state.c`・`input_ended.c`**：候補は `ui/` と `player/`（または
  `platform/`）。どちらも入力の状態なので `ui/`。
- **`recall.c`**：候補は `ui/`（画面に出す）と `monster/`（思い出の中身）。
  `prt`・`inkey` を呼ぶので `ui/`。
- **`files.c`**：`init_scorefile`（得点ファイルを開く。B19）・`read_times`・
  `helpfile`・`print_objects`（wizard）・`file_character`（人物の書き出し）の
  5 本。候補は `ui/` と `save/`。`get_string`・`msg_print` を呼ぶ関数が多いので
  `ui/`。いずれ割る対象。
- **`wizard.c`**：`wizard_light`・`change_character`・`wizard_create`。
  候補は `ui/`（prompt と `prt_*`）と `player/`（人物を書きかえる）、それに
  12 個めのディレクトリー（`debug/` など）。11 個の中からなら `ui/`。

### save/（4 本）

| ファイル | 行数 | 理由 |
|---|---|---|
| `save.c` | 1593 | セーブの書きと読み |
| `save_state.c`・`.h` | 84・75 | セーブの状態の置き場 |
| `score_death.c`・`.h` | 98・77 | 死んだか・勝ったかの置き場 |
| `death.c` | 506 | 墓と得点表 |

- **`death.c`**：`display_scores`・`print_tomb` は画面を描くので、候補は `save/`
  （上の計画の例）と `ui/`。得点ファイルの読み書き（`highscores`）を持つので
  `save/`。

### platform/（5 本）

| ファイル | 行数 | 理由 |
|---|---|---|
| `platform.c`・`.h` | 39・22 | 描画と入力の backend をつなぐ |
| `render_ncurses.c` | 302 | ncurses の描画 backend |
| `input_ncurses.c` | 132 | ncurses の入力 backend |
| `backend_ncurses.h` | 22 | 上の 2 本の宣言。ヘッダだけ |
| `curses.h` | 16 | `<ncurses.h>` を包む。ヘッダだけ（下記） |
| `signals.c` | 228 | signal の受け手 |
| `signal_flags.c`・`.h` | 54・45 | signal と本体のあいだの旗。依存 0 |

- `signals.c` は `prt`・`save_char` を呼ぶ。`platform/` は葉ではないので規則には
  当たらない。

### src/ に残す（`.c` 3 本・`.h` 6 本）

| ファイル | 行数 | 理由 |
|---|---|---|
| `main.c` | 287 | 上の計画どおり |
| `dungeon.c` | 1904 | 遊びの主ループとコマンドの振り分け（下記） |
| `game_state.c`・`.h` | 156・148 | 誰も呼ばない写しとり（B3。下記） |
| `config.h`・`constant.h`・`types.h`・`headers.h`・`externs.h` | 26・600・351・54・646 | 全域ヘッダ。ほぼすべての `.c` が引く |

- **`dungeon.c`**：名前は「ダンジョン」だが、中身は `dungeon()`（主ループ）・
  `do_command`・`original_commands` と、再生（`regenhp`・`regenmana`）、階段
  （`go_up`・`go_down`）、`jamdoor`・`refill_lamp`・`examine_book`。
  呼び手は `main.c` だけで、`#include` は 42 行。候補は `src/`（`main.c` の隣。
  組みたての頂上）と `dungeon/`。`dungeon/` に置くと「地図と階」が
  いちばん広い依存を抱えるので、`src/` を案にした。いずれ割る対象。
- **`game_state.c`**：`game_state_init()` を誰も呼ばない（B3）。約 50 個の
  全域変数を写すので、どこへ置いても全方向に依存する。消すかどうかは B3 の
  判断なので、それまで `src/` に置く案。
- **全域ヘッダ 5 本**：`core/` に置く案もあるが、`externs.h` はすべての
  ディレクトリーの名前を宣言するので、`core/` の「どこにも依存しない」に合わない。
  `-Isrc` は残るので、`src/` のままで `#include` は変わらない。

### 同じ名前のヘッダ

**`-I` を足す方式で気をつけるのは 2 つ。**

1. **ディレクトリーをまたいだ同名。** いまはすべて `src/` の 1 か所にあるので、
   同名は 1 つも無い。`tests/` のヘッダ（`fixture.h`・`minunit.h`）とも重ならない。
   `src/` と `tests/` のファイル名の重なりも 0。R で作る予定の名前
   （上の R と `combat/` の節の 33 本）も、いまの `src/` とは重ならない。
   → 同名が無いので、`-I` の順番で答えが変わるところは無い。**新しいファイルを
   足すときは、ほかのディレクトリーとの同名を検める**（`ls src/*/ | sort | uniq -d`）。
2. **システムのヘッダとの同名。** `/usr/include` と重なるのは 2 本。
   - `curses.h`：`#include "curses.h"` は `render_ncurses.c`・`input_ncurses.c` の
     2 本だけで、どちらも `platform/` に行く。同じディレクトリーの `curses.h` が
     先に見つかるので、ふるまいは変わらない。`<curses.h>` を書くファイルは 0。
   - **`panel.h`**：ncurses の panel ライブラリーの `<panel.h>` と同名。
     `"panel.h"` を引くのは 13 本（`src/` 12・`tests/` 1）、`<panel.h>` は 0。
     いまも `-Isrc` で横取りしうる形で、`ui/` に移っても変わらない。
   - R で作る予定の `dungeon/search.c` の `search.h` は、`/usr/include/search.h`
     （`hsearch` など）と同名になる。`src/strings.h` と同じ理由で避けたい。
     名前を変える案：`dungeon/secret_search.c`。
- **`.c` を include するテストが 4 本ある**（`#include "store2.c"`・
  `"creature.c"`・`"save.c"` 2 本）。`-I` で探すので、`makefile.test` には
  `monster/`・`save/` の `-I` も要る。

### 決まったこと（2026-09-29、ユーザーの判断）

D0 の案のうち迷いどころ 10 点を問い合わせ、**すべて上の表の案のとおり**と決まった。

- `src/` に残すのは `main.c`・`dungeon.c`（主ループ）・`game_state.c`（B3。
  判断が出るまで）と、全域ヘッダ 5 本（`config.h`・`constant.h`・`types.h`・
  `headers.h`・`externs.h`）。
- ディレクトリーは 11 個のまま。`spells.c` は `item/`、`wizard.c` は `ui/`。
  `magic/`・`debug/` は作らない（`spells.c` の飛び道具は #57 で `combat/` へ）。
- `monsters.c`・`variable.c`・`options.c`・`progress.c`・`sets.c` は `data/`。
- `panel.c`・`render.c`・`input.c`・`files.c`・`recall.c`・`help.c` は `ui/`。
  `create.c` は `player/`、`magic.c`・`prayer.c` は `item/`、`death.c` は `save/`。
- R で作る `dungeon/search.c` は `<search.h>` と重ならない名前にする
  （`secret_search.c` など。名前は R の段で決める）。

## 順番

**L → D → R → 後始末。** 段ごとにブランチを分け、段ごとにマージの許可を取る。

1. **L**：L1 → L2 → L3。
2. **D**：D0 で行き先の表を作って見てもらう。そのあと、すでにきれいな module を
   `git mv` で移す（1 ディレクトリー 1 コミット、中身は変えない）。
3. **R**：上の順番で分割する。1 塊ごとに A/B/C の要領で進める。まず新しいファイルと
   テストを置き、次に番号つきのファイルから消し、最後に呼び手と代役を整える。
4. **combat の残り**：`creature.c`・`spells.c` から切り出す。
5. **後始末**：代役の重複（#37）と、定数を返すだけの代役（#44）を見なおす。

## 進みぐあい

| 段 | 状態 | ブランチ・コミット |
|---|---|---|
| L1〜L3 | 済み（2026-09-29、`develop` へマージ `5e0cd6e`） | `refactor/52-test-library`、`e7c3a9c`〜`c188e58`（7 コミット）。74 本の Map は 72 本が旧 recipe と一致、残る 2 本は旧 recipe が誰も参照しない `tables.c`・`treasure.c` を並べていた差（`worklog.md`） |
| D0 | 済み（2026-09-29、迷いどころ 10 点はすべて案のとおり。マージ `111d2e7`） | `docs/53-d0-destinations`、`2dfa7b8`・`89395b0` |
| D | 済み（2026-09-29、`develop` へマージ `99fe489`）。D0 の表のとおり `.c` 96 本・`.h` 70 本を 10 個のディレクトリーへ（`combat/` は 0 本なのでまだ無い）。どのコミットでも本体の `objdump -d` が変更前と一致 | `refactor/53-directories`、`68dbae9`〜`cfaadc4`（13 コミット。makefile の仕組み 1・`layer_deps.py --matrix` 1・`git mv` 10・コメント 1。`worklog.md`） |
| R（misc4 → moria4） | misc4・misc2 済み（2026-09-29、マージ `d437df0`・`1647652`）。misc1 済み（2026-09-30、マージ `76ced6f` と `refactor/54-misc1-rnd`）。store1/2 済み（2026-09-30、マージ `59e0c92`・`ced26a4`）。misc3 済み（2026-09-30、マージ `cb5279a`）。moria1〜4 済み（2026-10-01、マージ `c0fe8d3`・`7a2bc35`・`ce9251e`・`3787dc4`）。**R は終わった** | `refactor/54-misc4`（`427a390`〜`4d4b515`、5 コミット）、`refactor/54-misc2`（`0baa1af`〜`7479a8d`、3 コミット）、`refactor/54-misc1`（`f107639`〜`a74e12c`、11 コミット）と `refactor/54-misc1-rnd`（`c2fb744`〜`196a837`、4 コミット）、`refactor/55-store-price`（`855c998`・`36ae8e9`）、`refactor/55-store-stock`（`8351ce2`・`0ea41fd`）と `refactor/55-store2`（`a341211`〜`3eaa18b`、3 コミット）、`refactor/42-misc3`（`b3c0ad6`〜`38325a1`、19 コミット）、`refactor/56-moria2`（`b9bc379`〜`8c06bfa`、5 コミット）と `refactor/56-moria1`（`012cb46`〜`ae2f40a`、8 コミット）、`refactor/56-moria4`（`0a4ce56`〜`0105184`、7 コミット）と `refactor/56-moria3`（`0083e5e`〜`27e871c`、9 コミット） |
| combat の残り | 済み（2026-10-01、マージ `0416bb0`） | `refactor/57-combat`（`6b23689`〜`0a04327`、4 コミット） |
| 後始末 | 済み（2026-10-01、マージ `acdcd51`・`77c8182`・`082b0e4`）。#37・#44 は #38 まで棚上げ | `refactor/cleanup-static`（`6a6e343`〜`996f256`、10 コミット）、`refactor/cleanup-stubs`（`688e0a5`・`e63445c`）、`refactor/cleanup-comments`（`b698337`〜`40811f7`、4 コミット） |
| #38 段階 A | 済み（2026-10-01、マージ `48af494`）。段階 B は未定 | `refactor/38-fixtures`（`61b98ee`〜`cfbb67b`、17 コミット） |
