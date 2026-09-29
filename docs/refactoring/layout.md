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
| D | 済み（2026-09-29、マージ待ち）。D0 の表のとおり `.c` 96 本・`.h` 70 本を 10 個のディレクトリーへ（`combat/` は 0 本なのでまだ無い）。どのコミットでも本体の `objdump -d` が変更前と一致 | `refactor/53-directories`、`68dbae9`〜`cfaadc4`（13 コミット。makefile の仕組み 1・`layer_deps.py --matrix` 1・`git mv` 10・コメント 1。`worklog.md`） |
| R（misc4 → moria4） | 未着手 | |
| combat の残り | 未着手 | |
| 後始末 | 未着手 | |
