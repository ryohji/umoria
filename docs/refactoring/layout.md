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
- `get_panel` → `ui/panel.c`
- `loc_symbol`・`test_light`・`prt_map` → `ui/map_view.c`
- `compact_monsters`・`popm`・`place_monster`・`place_win_monster`・
  `get_mons_num`・`alloc_monster`・`summon*` → `monster/monster_place.c`
- `compact_objects`・`popt`・`pusht` → `dungeon/object_place.c`
- `m_bonus` → `item/item_enchant.c`
- `add_food` → `eat.c` か `player_food.c`。どちらにするかは呼び手を測ってから
  決める。

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

## combat/

`combat/` には新しく作るファイルだけを置く。どれも番号つきのファイルか、
大きなファイルから切り出す。

| ファイル | 中身（いまの場所） | 目安 |
|---|---|---|
| `hit_rolls.c` | `test_hit`（moria1）、`attack_blows`・`tot_dam`・`critical_blow`（misc3） | 約 150 行。副作用が無いので、乱数の代役を当てれば単体でテストできる |
| `player_melee.c` | `py_attack`（moria3）、`py_bash`（moria4） | |
| `monster_melee.c` | `make_attack`（`creature.c`。棚上げ #25）を中は割らずに移す | 約 600 行 |
| `player_damage.c` | `take_hit`（moria1）、`minus_ac`・`*_gas`・`*_dam`（moria2）、`player_saves`（misc3） | |
| `monster_damage.c` | `mon_take_hit`（moria3） | |
| `projectiles.c` | `get_flags`・`fire_bolt`・`fire_ball`・`breath`（`spells.c`） | |
| `throw.c` | `inven_throw`・`facts`・`drop_throw`・`throw_object`（moria4） | |

- 重複の #51（光る手）と #29（飛翔のループ）の 2 組が、同じディレクトリーに
  並ぶ。どちらもまとめはしない（棚上げの理由は変わらない）が、見比べやすくなる。
- `creature.c`（1651 行 → 約 1050 行）と `spells.c`（2141 行 → 約 1790 行）も
  触ることになる。そのため、番号つきのファイルが片づいたあとに別のブランチで行う。

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
| L1〜L3 | 未着手 | |
| D0 | 未着手 | |
| D | 未着手 | |
| R（misc4 → moria4） | 未着手 | |
| combat の残り | 未着手 | |
| 後始末 | 未着手 | |
