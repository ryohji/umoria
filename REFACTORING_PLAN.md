# リファクタリング計画：umoria

台帳。**いまの姿を踏まえた計画**（候補を主題ごとに並べ、守りの手と前提を書く）と、済んだ項目の
索引。2026-10-30 に組みなおした。組みなおす前の全文（P1〜P3・棚上げの区分）は
[done/plan-2026-10-30.md](docs/refactoring/done/plan-2026-10-30.md)、それより前は
[done/plan-2026-10-23.md](docs/refactoring/done/plan-2026-10-23.md)・[done/plan-2026-10-01.md](docs/refactoring/done/plan-2026-10-01.md)。

- 着手時：`488db72`（2026-08-26）、テスト 0 件、62 ファイル・約 30,000 行・C17。
- いま：テスト 1865 件・91 本、golden の台本 11 本と総当たり 6 本（`objdes_all`・`recall_all`・
  `bolts_all`・`magic_all`・`melee_all`・`turns_all`）、本体の行の 51.5%（8113 / 15743。gcov、2026-10-30、
  #27 より前。台本だけの数）を golden が通る。テストは本体と同じ `-O2` で組む（#97）。本体と
  テストの組みたての警告 0、ヘッダの global 36 個、層の依存 918 本・112 単位で違反 0
  （→ `HANDOVER.md` 第 0 節）。
- 番号は着手時の検出に振ったもので、振りなおさない。新しい候補は #97 から振る。
- **P1〜P3 と棚上げの区分はやめた。** P1・P2 は空になり、P3 と棚上げに残っていた「着手しない
  理由」の多くは「テストが届かない」「保護のコストが高い」だった。golden（#94）、総当たりの
  ハッシュ（所見 74）、`scripts/inline_back.py`（所見 73）、turn の記録（所見 76）で、その多くは
  動かせるようになった。下の計画は、いまの守りの手で見なおした結果。

## いま使える守りの手

| 手 | 守るもの | 置き場 |
|---|---|---|
| golden の台本 | 本体を通しで（画面とキーごとの turn） | `tests/golden/*.keys`、`run.sh`（第 4 節） |
| 総当たりのハッシュ | 1 つの関数の入力の組すべて（文字列・状態） | `tests/objdes_all.c`・`recall_all.c`・`bolts_all.c`・`magic_all.c`・`melee_all.c`・`turns_all.c` |
| 字のまま戻す比較 | 塊を関数に出す移動 | `scripts/inline_back.py` |
| 全関数の機械語の比較 | 移動・統合・ヘッダの組みかえ | `scripts/cmp_all.sh` |
| 単体テストと写し | 窓口の中、表 | `tests/*_test.c`（`legacy_*` の写し） |

総当たりの道具は、試しの階（床と壁・プレイヤー・モンスター）を作れる（`bolts_all` が手本）。
起動時の表（`object_levels_init`・`monster_levels_init`）と画面の区画を先に整える（所見 79）。
プレイヤーを決まった状態に置く手本は `melee_all`（経験の係数も入れる）。ハッシュには、呼び手が
計算しなおさせる値も入れる（所見 81）。

## 計画

並びは勧める順。どれも着手の前にユーザーと決める（HANDOVER 第 3 節）。大きさと被覆は
2026-10-30 に測った。判断待ちの項目を決めるのに要る事実と選べる道は
[decisions-2026-10-30.md](docs/refactoring/decisions-2026-10-30.md)。

### A. 長い関数と switch（いまの守りの手で動かせる）

| # | 場所・大きさ | golden の被覆 | 守りの手と進めかた | 前提・判断 |
|---|---|---|---|---|
| 72 | command/inven_menu.c `inven_command`（485 行） | 55% | 字のまま出せる塊は出しきった（#72-6・#72-7）。選ぶ状態を構造体にまとめる案と、持ちなおす状態の一覧は [decisions-2026-10-30.md](docs/refactoring/decisions-2026-10-30.md) の「#72 の残り」。get_char は済んだ（#72-8）が、**`get_char(bool *generate)` の出力の引数はわかりにくいので、あとで直す**（ユーザーの指摘、2026-10-31。戻り値と合わせて「読めたか・階を作るか」を返す形に） | **設計の判断が先**（構造体を 2 つにするか、段の分けかた）。守りは golden |

### B. 重複

| # | 場所 | 中身 | 守りの手と進めかた | 前提・判断 |
|---|---|---|---|---|
| 28 | command/magic.c `cast`（58 行）と command/prayer.c `pray`（59 行） | 骨格が同型（本を選ぶ・呪文を選ぶ・失敗・効果・経験とマナ）。違いは言いまわし 8 つ、`pray` だけの「何も持っていない」、経験を足す 2 行の順 | 効果の switch は `spell_effect`・`prayer_effect` に出した（#28-1・#28-2）。残る骨格を、#73 の `use_charged_device` と同じく 1 本と種類ごとの表に | 畳む方向（ユーザーの判断）。まとめるかは切りだしたあとの差を見て |
| 51 | world/combat の光る手（2 か所） | 違い 6 つ（台帳の旧行。[done/plan-2026-10-30.md](docs/refactoring/done/plan-2026-10-30.md)）。6（B24）は直した | `glowing_hands` が両方の枝を守る | 畳む（ユーザーの判断）。**違い 3・5 のどちらを正とするかを訊いている** |
| 19 | world/item/spells.c の壁・扉を変える 5 つ（`light_line`・`disarm_all`・`wall_to_mud`・`td_destroy2`・`build_wall`）、world/combat/projectiles.c `fire_bolt`・`fire_ball` | 前の 5 つはマスごとに働く別の骨格、後の 2 つは 1 マスごとに弾を描く | `bolts_all` を壁・扉・罠の並びに広げてから。`fire_ball`（golden 0%）は範囲の当たりの総当たりが要る | 効き目は中くらい |
| 29 | `fire_bolt` と world/combat/throw.c `throw_object`（108 行） | 飛ぶ道のりの描画が 2 か所 | 描画だけ切りだせる | 効き目が小さい（記録のみ） |
| 30 | world/store/store_haggle.c `purchase_haggle`（154 行）と `sell_haggle`（202 行） | 骨格が似ていて意味が反転（値段の向き・侮辱の上限・成立条件） | 売る側は golden 0%。台本が要る | 畳まない（記録のみ） |

### C. 構造とデータ

| # | 場所 | 中身 | 前提・判断 |
|---|---|---|---|
| 31 | 19 ファイル・70 定義の `(int y, int x)` | 座標の型が無い | 入れる（ユーザーの判断）：名前は `coordinate`、欄は `row`・`column`（省略しない）、欄の型は `int`。向きとモンスターの位置も、順に同じ型へ。**着手の順を訊いている**（decisions-2026-10-30.md） |
| 32 | model/data/player.c（516 行・関数 0） | 職業・種族・呪文の表 | 構造の設計変更。いまの層の規則のままでも困っていない |
| 98 | world/ の段の中のループ | 行動の段の 8 単位のループは解いた（#98-1〜3：見え方の更新を monster_view.c、増殖を monster_multiply.c、どちらも床と生死の段）。残るのは player_move・run_path・traps の 3 単位（run_path ⇄ player_move、traps ⇄ player_move） | プレイヤーの移動の 3 つを、段を分けるか 1 単位にまとめるか |
| 99 | `constant.h` の定数 262 個・`types.h` の型 | 2 つ以上のライブラリーかテストが使うもの、表の一部（使われない 15 個を含む） | 小さく続けられる。急がない |

### D. 未接続の基盤（消すか残すか）

済んだ（#34〜#36 は消した。索引）。

| # | 場所 | 中身 | 前提・判断 |
|---|---|---|---|

### E. テストと守り

| # | 中身 | 前提・判断 |
|---|---|---|
| 50 | 窓口を呼ぶ順番と条件を見るテストが無い（`calc_bonuses` の呼び手など。旧行は [done/plan-2026-10-30.md](docs/refactoring/done/plan-2026-10-30.md)） | 2026-10-30 に数えなおした：呼び手 8 か所を消して 5 か所が赤（前は 12 度で 3 度）、`melee_all` を広げて 6 か所に（#50-1）。残るのは command/scrolls.c:112（To-Dam の巻物で呪いが解けたあと）と world/item/spells.c:366（装備を鑑定したあと）。どちらも画面の数を見る場面が要る |
| 100 | golden の穴の残り | 走っている途中で壁や見えているモンスターに当たる、100 歩走って息が切れる（町では作れない。地下の長い通路の台本が要る）、`fire_ball`・`sell_haggle`・祈りの方向の呪文 |

### F. 判断待ちのバグ候補

2026-10-31 に、ユーザーの判断で開いていた候補を直すか閉じた（`docs/refactoring/bugs.md`。案は
[bugfix-proposals-2026-10-31.md](docs/refactoring/bugfix-proposals-2026-10-31.md)）。開いているのは
B34（飛ぶ呪文の止まりかた。teleport は上流の注釈が「全部飛ばす」と言うので訊いている）と B36
（壊れたセーブファイルで止まらない読みこみと 0 で割る道。#72-8 の確かめで見つけた）。

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
| 72（一部） | `roff_recall` 492 → 49 行（記憶の項目ごとの関数 11 本。総当たり `recall_all` を golden に常設）、`eat` 173 → 32 行・`quaff` 295 → 38 行（効果の switch を `food_effect`・`potion_effect` に）、`inven_command`・`get_char` の一部。行は開いたまま | `worklog.md`（2026-10-30） |
| テストの組みたて | 警告 11 → 0（include の漏れ、旗を本体に合わせる、`snprintf`、代役のずれ）、`check.sh` がテストの組みたての警告も数える | `worklog.md`（2026-10-30） |
| golden の穴（一部） | 台本 `rare_paths`（呪われた鎧・鎧の巻物・罠を踏む・外す・箱） | 同上 |
| 19（一部） | 飛ぶ呪文 8 つの骨格を `bolt_to_next_creature()` に。総当たり `bolts_all` を golden に常設 | 同上 |
| 27（一部） | `read_scroll` 402 → 328 行（武器の巻物 4 つを static 関数に）。行は開いたまま | 同上 |
| 26 | `magic_treasure` 831 → 105 行（品物の種類ごとの static 関数 14 本）。総当たり `magic_all` を golden に常設 | 同上 |
| 25 | `make_attack` 600 → 100 行（当たり・言いかた・効果の 3 つの switch を static 関数に。光る手の塊は #51 のために残した）。総当たり `melee_all` を golden に常設 | 同上 |
| 21 | `mon_cast_spell` 209 → 73 行（呪文の番号に名前 `enum monster_spell`、効果の switch を `cast_monster_spell` に）。総当たり `turns_all`（モンスターの手番）を golden に常設 | 同上 |
| 97 | テストを本体と同じ `-O2` で組む（`shared_stubs_reset` が randint の代役の答えを戻すようにしてから） | 同上 |
| 50（一部） | 呼び手の穴を数えなおし、`melee_all` のハッシュに戦いの数と AC を足した。行は開いたまま | 同上 |
| バグ候補 | B2・B3・B5・B7〜B9・B15〜B20・B22〜B27・B29〜B33・B35 を直し、B4・B6・B10〜B14・B21 を閉じた（ユーザーの判断） | `worklog.md`（2026-10-31）、`bugs.md` |
| 27 | `read_scroll` 402 → 249 行（武器の 4 つ・鎧の 3 つを static 関数に。B35 を直して i_ptr を読んだ巻物だけに） | 同上 |
| 34・35・36 | 未接続の基盤を消した：ui/view_observer.c、app/game_state.c（init_seeds() は app/seeds.c に）、RenderBackend の使われない欄 5 つと render_present・render_get_size（ユーザーの判断） | 同上 |
| 72（一部） | `get_char` 463 → 159 行（読む段ごとに成否を返す static 関数 6 本） | 同上 |
| 98（一部） | 行動の段の 8 単位のループを解いた（update_mon・update_monsters を monster_view.c、multiply_monster を monster_multiply.c に） | 同上 |
| 28（一部） | 効果の switch を `spell_effect`・`prayer_effect` に | 同上 |

**ほかの作業で消えた項目：** #23（描画と計算の同居。#42 の分割で `ui/status_line.c` に分かれた）、
#33（misc3.c。#42 で無くなった）、#43（番号つきファイル。#54・#56 で無くなった）、
#46（`makefile.win` の `SRCS` の抜け。#52 で `sources.mk` を読むようになった。Windows では未確認）。
#3〜#5 は棚上げの #34〜#36 に移した番号。

## 記録の置き場

- バグ候補 → [docs/refactoring/bugs.md](docs/refactoring/bugs.md)
- 所見（わかったこと）→ [docs/refactoring/findings.md](docs/refactoring/findings.md)
- #18 の棚おろしと区分ごとの見立て → [docs/refactoring/globals_inventory.md](docs/refactoring/globals_inventory.md)
- 作業ログ → [docs/refactoring/worklog.md](docs/refactoring/worklog.md)
- 判断待ちの項目の材料 → [docs/refactoring/decisions-2026-10-30.md](docs/refactoring/decisions-2026-10-30.md)
- 済んだ列の経過（整理の前の台帳を含む）→ [docs/refactoring/done/](docs/refactoring/done/)
