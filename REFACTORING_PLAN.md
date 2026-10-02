# リファクタリング計画：umoria

台帳。開いている項目は 1 項目 1 行、済んだ項目は末尾の索引に 1 行ずつ。
2026-10-01 に整理した。整理の前の全文（前提・検出結果の内訳・済んだ行・#50 の経過）は
[done/plan-2026-10-01.md](docs/refactoring/done/plan-2026-10-01.md)。

- 着手時：`488db72`（2026-08-26）、テスト 0 件、62 ファイル・約 30,000 行・C17。
- いま：テスト 1699 件・78 本、`externs.h` の global 42 個（→ `HANDOVER.md` 第 0 節）。
- 番号は着手時の検出に振ったもので、振りなおさない。場所は 2026-10-01 の名前と行。
  判断（着手しない理由）は整理の前と同じで、字を詰めただけ。
- #58〜#76 は 2026-10-02 の再検出（4 つの担当がディレクトリーごとに調べた）で足した。
  場所は 2026-10-02 の行。同じ日に、開いている行の場所と数を今のコードに合わせた。

## 開いている項目

### P1（次に着手する候補。効果が大きく、作業が軽い）

いまは無い。2026-10-02 に足した #58〜#61 は済んだ（経過は
[p1-2026-10.md](docs/refactoring/p1-2026-10.md)）。

### P2（時間・合意しだい）

| # | 状態 | 分類 | 場所 | 内容 | 手法 |
|---|---|---|---|---|---|
| 13 | 棚上げ | 不適切な責務配置 | platform/render_ncurses.c:60-63, 115-118, 279-302、platform/signals.c:94, 103 | 描画の backend が SIGTSTP を直に `signal()` で登録する（`:117`）。`signals.c:103` は `// SIGTSTP is handled by the rendering system` と書くだけで、シグナルの責務が 2 ファイルに割れている。#75 と一緒に扱う | Move（signals.c へ寄せる） |
| 62 | 未着手 | 重複コード | dungeon.c:840、:930、:1056 の `switch` | 「方向 1〜9 → コマンド文字」の switch が 3 本。差は変換先が大文字（走る）・CTRL（掘る）・小文字（歩く）であることと default の字だけ | 引数だけで答えが決まる関数と表にして新しい module に置く（単体テストが届く） |
| 63 | 未着手 | 重複コード | dungeon/generate.c:398 `build_type2`（:460, 479, 529, 586） | 内側の部屋の 4 辺のどれかに秘密の扉を置く 13 行が 4 か所に同じ字で書かれている | `static` な関数に抜く。generate.o を引くテストは 0 本なので機械語の比較か新しいテスト |
| 64 | 未着手 | 重複コード | monster/creature.c:849 ほか `mon_move` | `mm[0..4] = randint(9);` の並びが 5 か所。でたらめな移動の 3 分岐は旗と百分率だけが違う | 抽出。tests/creature_stubs.c に creature.c を `#include` する足場がある |
| 65 | 未着手 | 不適切な責務配置 | data/variable.c:150-190 の `recall_*` | モンスターの記憶の規則（上限・飽和）が「表と置き場」の data/ にある。依存は `static` な配列と定数だけで、テストは 0 本 | monster/ へ移し、テストを足す |
| 66 | 未着手 | 重複コード（テスト） | tests/fixture.c:61-104、tests/save_stubs.c:29-83 | #37 のあとも shared_stubs.c と同じ代役が残る。fixture.c:104 の `add_inscribe(inven_type *, int)` は本物（externs.h:414 は `uint8_t`）と型が違う | #37 の手順で外す。外したら壊してレッドを確かめなおす |
| 67 | 未着手 | データの散在 | `py.stats` の直参照 58 行・17 ファイル | `py` で窓口の無い最後の塊（#32 の残り）。player/player_body_weight.h:68 も「窓口がまだ無い」と書く | player/stats.c に読みの窓口を足す（#18 の手順） |
| 68 | 未着手 | 重複コード | item/scrolls.c:104 の case 3 と :335 の case 35、combat/player_damage.c `minus_ac` | 着ている防具を並べて 1 つ選ぶ（呪われた品を優先する）約 38 行が写し。差はメッセージと付呪の値だけ | 選ぶ部分を抜く。scrolls.o を引くテストは 0 本 |
| 69 | 未着手 | 重複コード | item/spells.c:1007 と :1420（`build_wall` と `earthquake`）、`detect_*` の 3 本、`cure_*` の 4 本 | 壁に埋まったモンスターの処理が写し（差は `build_wall` だけが壁づくりを止めること）。`detect_*` は述語とメッセージだけ、`cure_*` は効果の ID だけが違う | 抽出。spells.c は単体でリンクしにくい（#19 と同じ壁） |
| 70 | 未着手 | （道具の欠陥） | scripts/globals.py の参照の数えかた | 注釈と文字列の中も参照に数えている（今の出力は合計 560）。#45 と同じ族 | #45 と一緒に、過去の記録との整合を決めてから |

### P3（記録のみ。着手しない）

将来その場所を触るときの手掛かり。

| # | 分類 | 場所 | 内容 | 着手しない理由 |
|---|---|---|---|---|
| 19 | 重複コード | item/spells.c の `light_line` `disarm_all` `hp_monster` `drain_life` `speed_monster` `confuse_monster` `sleep_monster` `wall_to_mud` `td_destroy2` `poly_monster` `build_wall` `clone_monster` `teleport_monster`、combat/projectiles.c:69-141 `fire_bolt`・:144-268 `fire_ball` | 「方向に飛ばして最初の対象を探す」骨格の写し。うち 5 関数はほぼ行単位で一致 | 終了条件に微妙な差がある（`fire_ball` は `fval` の判定を else 側に置く）。テストの保護なしの抽出は危険 |
| 20 | 重複コード | item/spells.c:1547-1605 | `lose_str/int/wis/dex/con/chr` の 6 関数が同型。`lose_str` と `lose_con` はメッセージまで一致。**`lose_dex` の呼び手は item/eat.c:92 の `#if 0` の中だけ**なので、生きているのは 5 関数 | 表で 6 → 1 にできるが、人物の状態に依存するので足場が要る |
| 21 | 複雑な条件分岐 | monster/creature.c:471-679 `mon_cast_spell`（switch は :534） | switch 20 ケース。ケース番号がビット位置の生の数で、魔法 ID が前段の分岐に直書き | 変更頻度が低く、テストのコストが高い |
| 22 | 長すぎるメソッド | item/desc.c:251-559 `objdes` | 309 行。被参照 約 51 か所で、表示文字列を全域が頼る | 影響は大きいが、分割には広いテストが要る |
| 24 | データの散在 | item/desc.c:109-153 ほか | 鑑定の状態が生のビット操作とアクセサで不統一。`known1_p` などはアクセサになったが、生の `ident` 操作が desc.c の外に 31 か所残る（item_enchant.c 24、save.c 2、main.c・dungeon.c・inscription.c・store_price.c・ui/wizard.c 各 1）。staffs.c・scrolls.c の `ident` は局所の `bool` で別物 | 影響度 Low。#39 と一緒に扱う |
| 39 | 名前が意図を表さない | item/desc.c:113, 132, 149（`known1_p` `known2_p` `store_bought_p`） | 述語なのに `int` でフラグの値そのものを返す。真のとき 1 でなくビット値。**item/inven_ops.c:138 `items_can_stack()` は 2 つの戻り値を `==` で比べている**（両辺が同じビット位なので今は正しい） | 実害は無い。危ういのは片方だけ 0/1 に正規化する掃除で、そのとき `:138` が静かに壊れる。#24 を動かすときに一緒に扱う |
| 45 | （道具の欠陥） | scripts/globals.py:315 の `alias` 正規表現 | 別名の数えかたが `&&` とビット AND を誤検出する（`inventory` で 20 件）。`(?<![&|])` 相当の除外が無い | 着手は安全だが、この数は #18 の着手順の指標に使ってきた。直すと `globals_inventory.md` の別名の列が下がりうるので、過去の記録との整合を決めてから |
| 48 | 理解しづらいロジック | core/str_insert.c:29-61 `insert_str`（走査 :35-48、再評価 :50） | 走査の終端と置きかえの可否が 2 つの独立した条件で書かれている。「見つかったか」の変数が無く、抜けた理由を条件の再評価で言いあてる | ふるまいを変えずに直せる（`char *found = NULL;`）。#41 は純粋な移動に限ったので混ぜなかった。保護 22 件・リンク 2 単位で、**P3 で着手コストが最も低い部類** |
| 49 | （テストの穴） | save/save.c:124-415 `sv_write`（`wr_*` 130）、:524-1259 `get_char`（`rd_*` の呼び出し 139） | **セーブファイルの項目の並びを見るテストが無い。** 型が合えば並びが入れちがっても通る。#19B4 の壊し 4 件は全部生き残った | もとから空いていた穴。往復させるテストには状態を丸ごと組みたてる足場が要る。それまでは `wr_*` / `rd_*` の呼び順を `diff` する（`HANDOVER.md` 第 7 節） |
| 50 | （テストの穴） | player/player_bonuses.c:94-200 `calc_bonuses`、player/create.c:422-423、dungeon.c:466, 473, 485, 493、save/save.c:734-745 | **窓口を呼ぶ順番と条件（呼び手に残した規則）を見るテストが無い。** 窓口の中は `player_display_numbers_test` が守るが、呼び手は守られていない | もとから空いていた穴。呼び手を単体で呼べる足場が要る。経過は下の「#50 の族」 |
| 51 | 重複コード | combat/monster_melee.c:592-609（モンスターの攻撃）と combat/player_melee.c:93-111（プレイヤーの攻撃） | 光る手が当たったときの約 18 行がほぼ写し。差は 4 つ：(1) 前者だけ `adesc != 99` を見る、(2) 大文字化の手が違う、(3) 見えているかの条件が `visible && !player_is_dead()` か `m_ptr->ml` か、(4) `msg_print("Your hands stop glowing.")` と `player_glowing_hands_spend()` の順が逆（monster_melee.c:593-594 と player_melee.c:94-95） | (1) は意図がコメントにある。(2)〜(4) は意図か分からず、畳むと決めた時点でふるまいが変わる。まず「どちらが正しいか」の判断が要る |
| 71 | 長すぎるメソッド | dungeon.c:73-817 `dungeon()`（745 行）、dungeon.c:1042 `do_command`（536 行） | `dungeon()` の約 420 行は、時間で切れる効果 18 種の同じ形の並び（#50 の :466-493 もこの中）。局所の `i` を 3 つの用途に使いまわす。:229 の `else if (player_food() < PLAYER_FOOD_WEAK)` は外側の条件で必ず真。`do_command` はウィザード用の switch を入れ子に持つ | dungeon.c には単体テストが届かない。効果ごとの塊を窓口の側へ出す足場が先。ウィザードの塊を ui/wizard.c へ移すだけなら軽い |
| 72 | 長すぎるメソッド | ui/inven_menu.c:394-970 `inven_command`（577 行）、ui/recall.c:163-655 `roff_recall`（493 行）、save/save.c:524-1259 `get_char`（736 行）、item/potions.c `quaff`・item/eat.c `eat` | 台帳の #22・#25〜#27 に無い長い関数。`get_char` は版の分岐と goto が入りまじる | どれも externs.h・画面・乱数に全面的に依存する。`get_char` は #49 と同じ足場が要る |
| 73 | 重複コード | item/staffs.c:29-174 `use`、item/wands.c:29-173 `aim` | 杖と魔法棒の手順の骨格がまるごと写し。差は魔法棒だけが方向を訊くこと。`aim` は `find_range` の出力の `j,k` を別の用途に使いまわす | 両方とも spells.c と `fire_*` を呼ぶのでリンクが重い |
| 74 | データの散在 | `free_turn_flag`（externs.h の外で 80 か所、dungeon.c に 32） | 中身は「このコマンドで turn を使ったか」という戻り値で、それを global で伝えている | `do_command()` の戻り値をどうするかの設計判断が先（`globals_inventory.md` の見立てでは残置） |
| 75 | 不適切な責務配置 | platform/signals.c:145 `handle_pending_signals`、ui/io.c:86-94 `inkey` | platform 層が save・ui・player を呼ぶ。パニックセーブの手順が signals.c:204-212 と io.c:86-94 で写し | 関数ポインタで向きを逆にできるが、#13 と一緒に設計を決める。層の規則はいま core/ しか見ていない（ユーザーの判断） |
| 76 | 肥大化モジュール（分けすぎ） | player/player_disarm.c・player_saving_throw.c・player_stealth.c ほか技能の module | `static` な値 1 つと get/set/adjust の 3 本だけの module が名前のほかは同じ字。save.c は player の .h を 28 本 include する | 「1 問 1 module」の方針とぶつかる。方針を見なおすかの判断が先 |

### #50 の族（要約）

#18 の単位を閉じるたびに呼び手側を壊して走らせ、12 度のうち捕まったのは 3 度・4 つだけだった（5 度め 1・9 度め 1・11 度め 2）。
分かったことは 1 つ：**穴は「窓口の外だから届かない」のではなく、呼び手を単体で呼べる
足場があるかどうかで決まる。** 捕まったのは足場のある呼び手（`calc_hitpoints_test`・
`put_misc3_test`・`calc_spells_test`・`gain_spells_test`）だけで、画面に出ないもの・
ファイルに書くもの・`dungeon.c` の中のものは素通りした。**#44・#49 も同じ族。**
窓口を 2 本に分けると順番の約束が生まれ、その約束にはテストが届かない
（`calc_bonuses()`・`beginning` → `count_down`・休息の残りターン）。
回ごとの壊しかたと件数は [done/plan-2026-10-01.md](docs/refactoring/done/plan-2026-10-01.md)
の P3 の節と `findings.md`。

### 棚上げ（やらない）

| # | 分類 | 場所 | 内容 | 棚上げの理由 |
|---|---|---|---|---|
| 25 | 長すぎるメソッド | combat/monster_melee.c:30-629 `make_attack` | 600 行・89 ケース。switch が 4 段 | 保護するテストのコストが極端に高い。global と乱数への依存が全面的。#57 でも中を割らずに移した |
| 26 | 長すぎるメソッド | item/item_enchant.c:28-858 `magic_treasure` | 831 行・switch 10・119 ケース。品物の生成の中枢 | 同上。付呪の規則の網羅テストは現実的でない |
| 27 | 長すぎるメソッド | item/scrolls.c:28-486 `read_scroll` | 459 行・40 ケース | 同上。局所変数を用途を変えて使いまわしており、分割の前に変数の整理が要る |
| 28 | 重複コード | item/magic.c:26-212 と item/prayer.c:26-209 | `cast()` と `pray()` が構造ごと同型 | 分岐の本体・メッセージ・階級の判定の 3 点が違う。差 3 つ以上は抽出しない |
| 29 | 重複コード | combat/projectiles.c:69-141 `fire_bolt` と combat/throw.c:181-285 `throw_object` | 飛ぶ道のりのループを 2 か所で書いている | 命中・終了条件・外れの 3 点が違う。描画だけ切りだせるが効果が小さい |
| 30 | 重複コード | store/store_haggle.c:316-469 `purchase_haggle` と :472-674 `sell_haggle` | 値切りが買いと売りで並行して書かれている | 値段の向き・侮辱の上限・成立条件が逆。骨格が似ているだけで意味は反転している |
| 31 | データの散在 | 19 ファイル・70 定義の `(int y, int x)` | 座標が 4 つの形で表され、Coord 型が無い | 置きかえが広すぎる。入れるならテストを揃えてから |
| 32 | 強すぎる依存関係 | data/player.c | 524 行・関数 0。`py.` の直参照は、2026-10-02 には `py.stats` の 58 行・17 ファイルだけ（→ #67） | 構造の設計変更で、このリファクタリングの範囲を超える |
| 34 | 未接続の基盤 | ui/view_observer.c/.h（296 行） | Observer の基盤。参照は宣言と定義だけ | 残骸ではなく先に用意された基盤（導入コミットによる）。消さない。接続の予定がなくなったら再評価 |
| 35 | 未接続の基盤 | game_state.c/.h（328 行） | GameState 構造体。呼ばれていない（同じファイルの `init_seeds()` だけは main.c:117 から呼ばれる生きたコード） | 同上。ただし「写した瞬間に元と離れる」設計なので、つなぐときに見なおす（→ バグ候補 B3） |
| 36 | 未接続の基盤 | ui/render.h:62-63、platform/render_ncurses.c:55-56, 82-83, 228-262 | `get_char` / `check_input` が InputBackend と役割が重なる。参照 0。同じ基盤（`2cd0529c`）の `render_present`（ui/render.c:95）・`render_get_size`（:119）も参照 0 | 同上。3 つのうち唯一「重複」の性が強い。input 側への統合が確かめられれば削除の候補に戻せる |

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
| 58 | 本体のヘッダ依存を `-MMD -MP` に（`a6b8535`） | [p1-2026-10.md](docs/refactoring/p1-2026-10.md) |
| 59 | `distance()` を `core/` へ、geometry に本物のテスト（`8afcf63`） | 同上 |
| 60 | 生の数 4 族に名前（`b4a84c7`。(d) は作りなおした） | 同上 |
| 61 | 注釈の経緯・作業番号・日本語を外し、手書きの宣言を `externs.h` の include に（`f9d283b`・`babd322`・`01f0a98`） | 同上 |

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
