# リファクタリング計画：umoria

台帳。開いている項目は 1 項目 1 行、済んだ項目は末尾の索引に 1 行ずつ。
2026-10-01 に整理し、2026-10-23 に詰めなおした。2026-10-01 の整理の前の全文（前提・検出結果の
内訳・済んだ行・#50 の経過）は [done/plan-2026-10-01.md](docs/refactoring/done/plan-2026-10-01.md)、
2026-10-23 の前の全文は [done/plan-2026-10-23.md](docs/refactoring/done/plan-2026-10-23.md)。

- 着手時：`488db72`（2026-08-26）、テスト 0 件、62 ファイル・約 30,000 行・C17。
- いま：テスト 1850 件・89 本と golden 10 本（と `objdes_all`）、ヘッダの global 39 個（→ `HANDOVER.md` 第 0 節）。
- 番号は着手時の検出に振ったもので、振りなおさない。場所は 2026-10-01 の名前と行。
  判断（着手しない理由）は整理の前と同じで、字を詰めただけ。
- #58〜#76 は 2026-10-02 の再検出（4 つの担当がディレクトリーごとに調べた）で足した。
  場所は 2026-10-02 の行。同じ日に、開いている行の場所と数を今のコードに合わせた。
- #77〜#79 は 2026-10-10 の下調べ（どのテストも引かない `.o` と、使い手の少ない global）で足し、同じ日に済んだ。
- #80〜#88 は 2026-10-14 のモジュール化の列で足し、同じ日に済んだ。#89〜#93 は 2026-10-15 に HANDOVER 第 6 節の候補から足し、同じ日に済んだ。#94・#95 は 2026-10-24〜25。開いている行の場所は、
  このとき新しいディレクトリーの名前に書きかえた（行番号は 2026-10-01 のまま）。

## 開いている項目

### P1（次に着手する候補。効果が大きく、作業が軽い）

いまは無い。2026-10-02 に足した #58〜#61 は済んだ（経過は
[done/p1-2026-10.md](docs/refactoring/done/p1-2026-10.md)）。

### P2（時間・合意しだい）

いまは無い。#13 は 2026-10-14 に #75 と一緒に済んだ。

### P3（記録のみ。着手しない）

将来その場所を触るときの手掛かり。

| # | 分類 | 場所 | 内容 | 着手しない理由 |
|---|---|---|---|---|
| 19 | 重複コード | world/item/spells.c の `light_line` `disarm_all` `hp_monster` `drain_life` `speed_monster` `confuse_monster` `sleep_monster` `wall_to_mud` `td_destroy2` `poly_monster` `build_wall` `clone_monster` `teleport_monster`、world/combat/projectiles.c:69-141 `fire_bolt`・:144-268 `fire_ball` | 「方向に飛ばして最初の対象を探す」骨格の写し。うち 5 関数はほぼ行単位で一致 | 終了条件に微妙な差がある（`fire_ball` は `fval` の判定を else 側に置く）。テストの保護なしの抽出は危険 |
| 21 | 複雑な条件分岐 | world/monster/creature.c:471-679 `mon_cast_spell`（switch は :534） | switch 20 ケース。ケース番号がビット位置の生の数で、魔法 ID が前段の分岐に直書き | 変更頻度が低く、テストのコストが高い |
| 50 | （テストの穴） | world/player/player_bonuses.c:94-200 `calc_bonuses`、command/create.c:422-423、app/dungeon.c:466, 473, 485, 493、app/save.c:734-745 | **窓口を呼ぶ順番と条件（呼び手に残した規則）を見るテストが無い。** 窓口の中は `player_display_numbers_test` が守るが、呼び手は守られていない | もとから空いていた穴。呼び手を単体で呼べる足場が要る。経過は下の「#50 の族」 |
| 51 | 重複コード | world/combat/monster_melee.c:601-619（モンスターの攻撃）と world/combat/player_melee.c:96-113（プレイヤーの攻撃） | 光る手が当たったときの約 18 行がほぼ写し。差は 6 つ：(1) 前者だけ `adesc != 99` を見る、(2) 大文字化の手が違う（`monster_name()` か、`monster_name_lower()` のあとの `toupper`）、(3) 記憶を足す条件が `visible && !player_is_dead()` か `m_ptr->ml` か、(4) `msg_print("Your hands stop glowing.")` と `player_glowing_hands_spend()` の順が逆（monster_melee.c:602-603 と player_melee.c:97-98）、(5) 混乱に抵抗するかの判定で、前者は `randint(MAX_MONS_LEVEL)` を先に引き、後者は `CD_NO_SLEEP` を先に見て短絡する（眠らない相手には乱数を引かない）、(6) 前者の `"is unaffected."` は頭の空白が無い（バグ候補 B24）。(5)・(6) は 2026-10-29 に見つけた。golden は両方の `if` の行まで来るが、中身はどちらも通らない（2026-10-29、gcov） | (1) は意図がコメントにある。(2) は出る文が同じ。(4) は `spend` が充填を 0 にするだけなので見分けられないと読める（確かめていない）。(3)・(5)・(6) は意図か分からず、畳むと決めた時点でふるまいが変わる（(5) は乱数の並び）。まず「どちらが正しいか」の判断が要る |
| 72 | 長すぎるメソッド | command/inven_menu.c:394-970 `inven_command`（577 行）、command/recall.c:163-655 `roff_recall`（493 行）、app/save.c:524-1259 `get_char`（736 行）、command/potions.c `quaff`・command/eat.c `eat` | 台帳の #22・#25〜#27 に無い長い関数。`get_char` は版の分岐と goto が入りまじる | どれも externs.h・画面・乱数に全面的に依存する。`get_char` は #49 と同じ足場が要る |

### #50 の族（要約）

#18 の単位を閉じるたびに呼び手側を壊して走らせ、12 度のうち捕まったのは 3 度・4 つだけだった（5 度め 1・9 度め 1・11 度め 2）。
分かったことは 1 つ：**穴は「窓口の外だから届かない」のではなく、呼び手を単体で呼べる
足場があるかどうかで決まる。** 捕まったのは足場のある呼び手（`calc_hitpoints_test`・
`put_misc3_test`・`calc_spells_test`・`gain_spells_test`）だけで、画面に出ないもの・
ファイルに書くもの・`dungeon.c` の中のものは素通りした。**#44・#49 も同じ族**（#49 は 2026-10-05 に埋めた）。
窓口を 2 本に分けると順番の約束が生まれ、その約束にはテストが届かない
（`calc_bonuses()`・`beginning` → `count_down`・休息の残りターン）。
回ごとの壊しかたと件数は [done/plan-2026-10-01.md](docs/refactoring/done/plan-2026-10-01.md)
の P3 の節と `findings.md`。

### 棚上げ（やらない）

| # | 分類 | 場所 | 内容 | 棚上げの理由 |
|---|---|---|---|---|
| 25 | 長すぎるメソッド | world/combat/monster_melee.c:30-629 `make_attack` | 600 行・89 ケース。switch が 4 段 | 保護するテストのコストが極端に高い。global と乱数への依存が全面的。#57 でも中を割らずに移した |
| 26 | 長すぎるメソッド | world/item/item_enchant.c:28-858 `magic_treasure` | 831 行・switch 10・119 ケース。品物の生成の中枢 | 同上。付呪の規則の網羅テストは現実的でない |
| 27 | 長すぎるメソッド | command/scrolls.c:28-486 `read_scroll` | 459 行・40 ケース | 同上。局所変数を用途を変えて使いまわしており、分割の前に変数の整理が要る |
| 28 | 重複コード | command/magic.c:26-212 と command/prayer.c:26-209 | `cast()` と `pray()` が構造ごと同型 | 分岐の本体・メッセージ・階級の判定の 3 点が違う。差 3 つ以上は抽出しない |
| 29 | 重複コード | world/combat/projectiles.c:69-141 `fire_bolt` と world/combat/throw.c:181-285 `throw_object` | 飛ぶ道のりのループを 2 か所で書いている | 命中・終了条件・外れの 3 点が違う。描画だけ切りだせるが効果が小さい |
| 30 | 重複コード | world/store/store_haggle.c:316-469 `purchase_haggle` と :472-674 `sell_haggle` | 値切りが買いと売りで並行して書かれている | 値段の向き・侮辱の上限・成立条件が逆。骨格が似ているだけで意味は反転している |
| 31 | データの散在 | 19 ファイル・70 定義の `(int y, int x)` | 座標が 4 つの形で表され、Coord 型が無い | 置きかえが広すぎる。入れるならテストを揃えてから |
| 32 | 強すぎる依存関係 | model/data/player.c | 524 行・関数 0。`py.` の直参照は、2026-10-02 には `py.stats` の 58 行・17 ファイルだけ（→ #67） | 構造の設計変更で、このリファクタリングの範囲を超える |
| 34 | 未接続の基盤 | ui/view_observer.c（宣言は ui/internal/ui_internal.h）（296 行） | Observer の基盤。参照は宣言と定義だけ | 残骸ではなく先に用意された基盤（導入コミットによる）。消さない。接続の予定がなくなったら再評価 |
| 35 | 未接続の基盤 | app/game_state.c（宣言は app/internal/app_internal.h）（328 行） | GameState 構造体。呼ばれていない（同じファイルの `init_seeds()` だけは main.c:117 から呼ばれる生きたコード） | 同上。ただし「写した瞬間に元と離れる」設計なので、つなぐときに見なおす（→ バグ候補 B3） |
| 36 | 未接続の基盤 | platform/backend.h（RenderBackend の `get_char` / `check_input`）、platform/render_ncurses.c:55-56, 82-83, 228-262 | `get_char` / `check_input` が InputBackend と役割が重なる。参照 0。同じ基盤（`2cd0529c`）の `render_present`（ui/render.c:95）・`render_get_size`（:119）も参照 0 | 同上。3 つのうち唯一「重複」の性が強い。input 側への統合が確かめられれば削除の候補に戻せる |

## 済んだ項目の索引

行の字面と経過は [done/plan-2026-10-01.md](docs/refactoring/done/plan-2026-10-01.md)。
2026-09-14 より前の SHA は履歴の書きかえで残っていない（題名で探す）。

| # | 何をしたか | マージ・記録 |
|---|---|---|
| 0 | テストの makefile を整備 | [done/p1.md](docs/refactoring/done/p1.md)、[done/worklog-to-18.md](docs/refactoring/done/worklog-to-18.md) |
| 1 | `distance` にテスト（第一号） | 同上 |
| 2 | `io.c` の不要な `curses.h` などを外す | 同上 |
| 6 | 参照 0 の関数を消す | 同上 |
| 7 | 魔法道具の成功判定を `device_use_chance()` に | 同上 |
| 8・8b | 鑑定のブロック 5 か所を `learn_item_effect()` に | 同上 |
| 9 | `suspend()` の端末設定の写しを抽出 | 同上 |
| 10 | スタックの判定を `items_can_stack()` に（B8・B9 を発見） | 同上 |
| 11 | 能力値の 9 式を `calc_abilities()` に | 同上 |
| 12 | `prt_comment2/3` を統合 | 同上 |
| 14 | `movement_rate` を `moves_this_turn` に | 同上 |
| 15 | ncurses の backend の宣言を一元化 | 同上 |
| 16 | `wait_for_more_confirmation` の goto を整理 | 同上 |
| 17 | 能力値の閾値を表に | 同上 |
| 18 | global 112 個 → 42 個（最後は `415f4ab`） | `done/18-*.md`、`globals_inventory.md` |
| 40 | 能力値の補正表を `player/stats.c` に（`1d29550`） | [done/33-41-misc3.md](docs/refactoring/done/33-41-misc3.md) |
| 41 | `insert_str` / `insert_lnum` を `core/str_insert.c` に | 同上 |
| 42 | misc3.c を 13 の行き先へ（`cb5279a`） | [done/layout.md](docs/refactoring/done/layout.md) |
| 47 | `concat` にテストを足して `core/str_insert.c` へ（`dcb4f9f`・`cdd5bf8`） | 同上 |
| 52 | テストを `libcore.a` から引く形に（`5e0cd6e`） | 同上 |
| 53 | `src/` をサブディレクトリーへ（`99fe489`。層の規則 `d6b95e3`） | 同上 |
| 54 | misc1・misc2・misc4 を分ける（`1647652`・`d437df0`・`76ced6f`） | 同上 |
| 55 | store1・store2 を 4 本に（`59e0c92`・`ced26a4`） | 同上 |
| 56 | moria1〜4 を分ける（`c0fe8d3`・`7a2bc35`・`ce9251e`・`3787dc4`） | 同上 |
| 57 | `combat/` に寄せる（`0416bb0`） | 同上 |
| 37 | 代役の重複 12 本を `shared_stubs.c` に（`df6de6d`） | 同上 |
| 38 | テストのリンクを 1 実行形式 1 対象に（`48af494`・`65773e1`） | 同上 |
| 44 | 引数を見ない代役：届かないものは止め、安いものは本物に（`264d258`・`230ddcc`・`c2a97d5`） | 同上 |
| 58 | 本体のヘッダ依存を `-MMD -MP` に（`a6b8535`） | [done/p1-2026-10.md](docs/refactoring/done/p1-2026-10.md) |
| 59 | `distance()` を `core/` へ、geometry に本物のテスト（`8afcf63`） | 同上 |
| 60 | 生の数 4 族に名前（`b4a84c7`。(d) は作りなおした） | 同上 |
| 61 | 注釈の経緯・作業番号・日本語を外し、手書きの宣言を `externs.h` の include に（`f9d283b`・`babd322`・`01f0a98`） | 同上 |
| 62 | 方向 → コマンド文字の switch 3 本を `ui/direction_keys.c` の表に（`8a65714`） | [done/p2-2026-10.md](docs/refactoring/done/p2-2026-10.md) |
| 63 | `build_type2` の秘密の扉 4 か所を static 関数に（`9642d4b`） | 同上 |
| 64 | `mon_move` の乱数 5 か所と気まぐれな移動 3 分岐を static 関数に（`e111365`） | 同上 |
| 65 | `recall_*` を `monster/monster_recall.c` へ、テストを足す（`a8dc7df`） | 同上 |
| 66 | `fixture.c`・`save_stubs.c` の重複した代役を外す（`3327c22`） | 同上 |
| 68 | 着ている防具を選ぶ部分を `item/armor_selection.c` に（`f227fc7`） | 同上 |
| 70・45 | `globals.py` に「コード」と「単項」の列（`5e09ed2`） | 同上 |
| 69 | `spells.c` の壁に埋まる処理・`cure_*` 4 本・`detect_*` 5 本を static 関数に（`c9807f7`） | 同上 |
| 67 | `py` を消す：能力値の 4 配列を `player/stats.c` の static に、窓口 9 本（`cdefbe0`。global 42 → 41） | 同上 |
| 49 | セーブファイルの並びを表と往復で守る `savefile_layout_test`（`35b59f1`） | [done/49-savefile-layout.md](docs/refactoring/done/49-savefile-layout.md) |
| 48 | `insert_str` の「見つかったか」を変数 `found` に（`f6f8e7d`） | [done/p3-48-39-24.md](docs/refactoring/done/p3-48-39-24.md) |
| 39・24 | 鑑定の述語を `bool` に、`ident` の生のビット操作を `item/item_flags.c` の窓口に（`f4f2453`） | 同上 |
| 77 | 乱数 `core/rnd.c` にテスト、使われていない自己検査 `TEST_RNG` を外す | [done/2026-10-10.md](docs/refactoring/done/2026-10-10.md) |
| 78 | global 41 → 38：`days` の宣言・`copyright` の表を消し、`normal_table` を `rnd.c` の `static const` に（ユーザーの判断） | 同上 |
| 79 | `combat/hit_rolls.c` と `store/store_price.c` にテスト | 同上 |
| 71（一部） | ウィザード用の switch を `ui/wizard.c` の `wizard_command` に、食料の枝の必ず真の判定を `else` に。行は開いたまま | 同上 |
| 20 | `lose_str`〜`lose_chr` の 6 本を `player/stat_loss.c` の `lose_stat` 1 本と表に | 同上 |
| 73 | 杖と魔法棒の骨格を `item/device_use.c` の `use_charged_device` に | 同上 |
| 80 | `src/` を層（core < model < platform < ui < world < command < app）× 名詞のディレクトリーに | [done/modules-2026-10-14.md](docs/refactoring/done/modules-2026-10-14.md) |
| 81 | `layer_deps.py` で層の順を守る。例外の一覧は 0 本 | 同上 |
| 82 | model → world の 2 本（`is_a_vowel` を core へ、`invcopy` を model/item へ） | 同上 |
| 83 | world → command の 6 本（`world/item/item_picker.c`、店の入口は登録する手） | 同上 |
| 84・75 | platform・ui → 上の層（シグナルの処理と `platform_init` を app へ、入力の終わりを登録する手に） | 同上 |
| 13 | SIGTSTP の登録を `platform/signals.c` に | 同上 |
| 87 | 層ごとの静的ライブラリー 11 本を層の順にリンク | 同上 |
| 85・76 | model/player の約 37 module を 6 つに、注釈を `docs/modules/player.md` へ | 同上 |
| 88 | model/game・monster・dungeon・item の module をまとめる、`docs/modules/model.md` | 同上 |
| 86 | `externs.h` を消し、公開ヘッダと `internal/` の内部ヘッダに。どのヘッダも単体でコンパイルできる | 同上 |
| 89 | 定義したファイルの外で使われない名前 9 個を static に（global 45 → 40） | [done/section6-2026-10-15.md](docs/refactoring/done/section6-2026-10-15.md) |
| 90 | ui/ の入力の状態 3 本を `ui/input_state.c` に | 同上 |
| 91 | `constant.h` の定数 174 個を使う場所へ、`types.h` の型 2 つを使う場所へ、core/ が src/ 直下のヘッダを読まないように | 同上 |
| 92 | world/ のループを 4 段（表示 < 規則 < 床と生死 < 行動）に並べ、`layer_deps.py` の `WORLD_TIERS` で守る（ディレクトリーは名詞のまま。ユーザーの判断） | 同上 |
| 93 | どこからも使われない `SCARE_MONSTER` を消す（表の一部の 15 個は残す。ユーザーの判断） | 同上 |
| 94 | 端末なしで本体を通す golden（`tests/headless_backend.c`・`tests/golden/`・`make -f makefile.test golden`）。8 本で本体の行の 44.5% を通る。`scripts/check.sh` | [done/golden-2026-10-25.md](docs/refactoring/done/golden-2026-10-25.md) |
| 95 | モンスター表を先頭からたどる iterator（`monster_creature_begin` ほか・`advance`・`distance`）を足し、セーブで記憶を書く走査と `summon_undead` の走査をそれに（ユーザーの依頼）。`summon_undead_test` を足した | 同上 |
| 96 | handle と iterator を 1 つの型 `creature_iterator`（添字を持つ構造体）にまとめる。`monster_get_creature`・`make_creature_handle`・`get_creature_handle` を消す（ユーザーの依頼） | 同上 |
| 22 | `objdes`（309 行）を `base_name` と段ごとの static 関数 6 本に（17 行）。総当たり `objdes_all` を golden に常設 | [done/worklog-2026-10-23-30.md](docs/refactoring/done/worklog-2026-10-23-30.md)（2026-10-25） |
| 71（一部） | `dungeon()` の時間で切れる効果 18 種を `tick_<効果>()` に（行は開いたまま） | 同上 |
| 74 | `free_turn_flag` を戻り値に（ユーザーの判断）。コマンドの関数が「turn を使ったか」を `bool` で、取りけせる効果 3 つが `effect_outcome` で返す。global 40 → 39。保護は golden の台本 `free_turn`（turn を記録）と `glowing_hands`（#51） | [done/worklog-2026-10-23-30.md](docs/refactoring/done/worklog-2026-10-23-30.md)（2026-10-29〜30）、所見 75・76 |
| 71 | `dungeon()` 732 → 207 行（効果 18 種・明かり・空腹と回復・休息・状態行・付呪の感知・コマンドの読みとりを static 関数に）、`do_command` 404 → 227 行（長い 5 つの case を関数に、方向ごとの 24 の case を `command_key_direction()` の逆引きに）。確かめは `scripts/inline_back.py` と golden の台本 `free_turn`（5 つの run） | [done/worklog-2026-10-23-30.md](docs/refactoring/done/worklog-2026-10-23-30.md)（2026-10-25・30）、所見 77 |
| キーマップ | `original_commands` の switch（約 150 行）を `ui/keymap.c` の原文の鍵の組の表に（15 行に）、コマンドを名前 `player_command`（遊び 48・wizard 14）にして `do_command`・`wizard_command` を名前で分ける。rogue-like の組の表 `rogue_key_command()`。`keymap_test` が書きかえる前の 3 つの switch と 256 通りのキーで突きあわせる（ユーザーの依頼。名前と方向の構造体、wizard も名前に、はユーザーの判断） | [done/worklog-2026-10-23-30.md](docs/refactoring/done/worklog-2026-10-23-30.md)（2026-10-30） |

**ほかの作業で消えた項目：** #23（描画と計算の同居。#42 の分割で `ui/status_line.c` に分かれた）、
#33（misc3.c。#42 で無くなった）、#43（番号つきファイル。#54・#56 で無くなった）、
#46（`makefile.win` の `SRCS` の抜け。#52 で `sources.mk` を読むようになった。Windows では未確認）。
#3〜#5 は棚上げの #34〜#36 に移した番号。

## 記録の置き場

- バグ候補 → [docs/refactoring/bugs.md](docs/refactoring/bugs.md)
- 所見（わかったこと）→ [docs/refactoring/findings.md](docs/refactoring/findings.md)
- #18 の棚おろしと区分ごとの見立て → [docs/refactoring/globals_inventory.md](docs/refactoring/globals_inventory.md)
- 作業ログ → [docs/refactoring/worklog.md](docs/refactoring/worklog.md)
- 済んだ列の経過（整理の前の台帳を含む）→ [docs/refactoring/done/](docs/refactoring/done/)
