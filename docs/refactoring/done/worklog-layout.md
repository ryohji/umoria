# 作業ログ（実装の再配置と #37・#38・#44）

> この文書は `worklog.md` の #52（L1）から #44 の記録までの行を、`3413d256` の時点から 2026-10-01 に字を変えずに移したもの。

| 日時 | 項目# | 何をしたか | テスト件数 | 結果 |
|---|---|---|---|---|
| 2026-09-29 | #52-L1① | **`sources.mk` を新設し、本体の `makefile` がそれを読む**（`e7c3a9c`）。`SRCS` 109 本（旧 `makefile` と同じ並び）・`APP_SRCS`（`main.c`・`render_ncurses.c`・`input_ncurses.c`）・`CORE_SRCS`（残る 106 本）。`makefile` の手書き `SRCS`・`OBJS` 46 行が消え、`OBJS = $(SRCS:.c=.o)`。**109 個の `.o` の md5 と `umoria` の sha256 が変更前と一致**（ビルドが決定的なことは先に 2 回組んで確かめた）。壊し: `tables.c` を抜くと undefined reference 26 件（13 シンボル）。 |
| 2026-09-29 | #52-L1② | **`makefile.win` も `sources.mk` を読む —— #46 がこれで解消**（`a72ca8c`。**Windows で組めるかは確かめていない**）。未登録だった 16 本と、その依存行 16 行が入った。`src/` の中での `make -n` のコンパイル行 93 → 109。参考に Linux で `PTHREAD=` を上書きして組むと、変更前は undefined reference 176 件、変更後は実行形式ができる。壊し: `device.c` を抜くとその Linux でのリンクが 4 件で落ちる。 |
| 2026-09-29 | #52-L1③ | **`makefile.test` も `sources.mk` を読み、`libcore.a` を組む規則を持つ**（`ad67a70`。まだ誰もリンクしない）。テストと同じ `CFLAGS` で 103 本。**`dungeon.c`・`moria1.c`・`signals.c` の 3 本は外した** —— どのテストもリンクしておらず、テストのフラグ（`-O2` も `-D_DEFAULT_SOURCE` も無い）だと警告 7 件（`-Wformat-overflow` 1＋4、暗黙宣言 2）。壊し: `signals.c` を戻すと警告 2 件。 |
| 2026-09-29 | #52-L2① | **手書きの recipe 74 本を消し、型の規則 1 つと足場の表 19 行に**（`2d8458d`）。`makefile.test` 2368 → 186 行（枝の前は 2335）。**74 本の Map の突きあわせ: 72 本は引かれた `.o`・`nm`・出力の 3 つとも一致。2 本（`save_bool_test`・`store_save_test`）は旧 recipe の `src/tables.c`・`src/treasure.c` が引かれない** —— 2 本の中の誰も参照しておらず、旧 recipe から外してもリンクでき結果も同じ（`nm` の差はデータ 15 個だけ）。テスト側は変えていない。代役は archive より先の入力で弱いシンボルも無いので、同名のメンバーが引かれれば multiple definition で止まる。壊し: `ar d` で `inventory.o` を抜くと 14 本がリンクに失敗、`tests/fixture.c` の `insert_str` の代役を消すと**黙ってグリーン**（`str_insert.o` が引かれ 7 → 8 本）。**コミットメッセージの「`src/*.c` の名前 1415 回 → 0 回」は誤りで、正しくは 2 回**（解説のコメントの `src/tables.c`・`src/treasure.c`）。 |
| 2026-09-29 | #52-L2② | **`libcore.a` の `.o` に `-MMD -MP`**（`bf10f9c`）。`makefile.test` 186 → 194 行。`.o` 103 本の md5 は `-MMD` 無しと一致。**共有ヘッダの地雷（HANDOVER 第 7 節）が塞がった** —— `MAX_MON_MULT` 75 → 74 を `tests/build` を消さずに打つと、前は偽のグリーン、後は `monster_breeding_test` 2 件で RED。 |
| 2026-09-29 | #52-L3① | **`scripts/link_units.py`**（`c05c944`）。Map を読んで実行形式ごとに引かれた `.o` を出す。`--count`（合計 608。旧 recipe の 612 との差 4 は L2 の 2 本 × 2）・`--who`（`misc3.o` は 9 本。税の数えかたの代わり）・`--why`・`--shadows`（代役が本物を覆っている名前。74 本中 22 本に出る）。壊し: `insert_str` の代役を消すと 7 → 8 本・`--shadows` から `str_insert.o` が消える。 |
| 2026-09-29 | #52-L3② | **`scripts/layer_deps.py`**（`c188e58`）。`nm` で `.o` どうしの依存を出す層の検査の枠。いまは 109 本・辺 757 本・層は `src` の 1 つで、`RULES` は空（**D の後に入れる**と冒頭に英語で書いた）。壊し: `RULES = {"src": {"src"}}` で違反 757 本・終了コード 1。7 コミットとも本体の警告 0、1662 件 74 本 GREEN、globals OK 42、起動 1。 |
| 2026-09-29 | #53 D0 | **`src/` の `.c` 109 本・`.h` 76 本の行き先の表を `layout.md` に書いた**（`2dfa7b8`）。迷いどころ 10 点をユーザーに問い合わせ、**すべて案のとおり**と決まった（`89395b0`）。`src/` に残すのは `main.c`・`dungeon.c`・`game_state.c` と全域ヘッダ 5 本、ディレクトリーは 11 個のまま。ファイルはまだ動かしていない。 |
| 2026-09-29 | 3 本のマージ | **ユーザーの許可を得て `docs/reorganize-records`（`986e85d`）・`refactor/52-test-library`（`5e0cd6e`）・`docs/53-d0-destinations`（`111d2e7`）を `develop` へ `--no-ff` マージした。** マージ後に全ビルド・全テスト・起動を確認 —— 警告 0 / 1662 件 74 本 failed=0 `RESULT: GREEN` / globals OK 42 / 起動 1。`origin` に push はしていない。ブランチは消していない。 |
| 2026-09-29 | #53-D① | **3 つの makefile に `src/` のサブディレクトリーを読む仕組み**（`68dbae9`）。`sources.mk` に `SRC_SUBDIRS`（1 か所）を足し、`SRCS` は `src/` からの相対の名前にした。`makefile` は `VPATH`・`-I` をそこから作り、`OBJS` は `notdir` で根に並べ、依存行 65 か所の `$(SRCDIR)/` を外して名前だけに（VPATH で探す）。`makefile.win` は `VPATH` と `-iquote`（前から `-I` に `src/` が無いので `<curses.h>` の探し先を変えない）。`makefile.test` は `-I` と `vpath %.c`。`APP_SRCS`・`LIB_EXCLUDE` は名前だけで書き `%/` の形でも外す。`globals.py` は `src/**/*.c` を数える。壊し: 依存行の `burden.h` を `burdn.h` にすると "No rule to make target" 1 件。 |
| 2026-09-29 | #53-D② | **`layer_deps.py --matrix`**（`cf0d20c`）。層（`sources.mk` のパスの最初のディレクトリー、直下は `src`）どうしの辺の本数を行列で出す。`RULES` はまだ空（行列を見て決める）。壊し: `RULES = {"src": {"src"}}` で違反 757 行・終了コード 1。 |
| 2026-09-29 | #53-D③ | **`core/` に 2 本**（`dd9ae67`。`rnd.c`・`str_insert.c/.h`）。以下 10 コミットとも `git mv` と `sources.mk` の 2 か所だけで、rename はすべて 100%、`#include` は 0 行、makefile そのものは変わらない。壊し: `-Isrc/core` を外すとコンパイル 2 本、`str_insert.c` を抜くと undefined reference 本体 5・テスト 31 件。 |
| 2026-09-29 | #53-D④ | **`data/` に 8 本**（`5bc051e`。`.h` 2 本）。壊し: `-I` を外すと 14 本（テストの `libcore.a` は 11 本）、`progress.c` を抜くと 50・151 件。 |
| 2026-09-29 | #53-D⑤ | **`player/` に 37 本**（`7842729`。`.h` 36 本）。壊し: `-I` を外すと 29 本（26 本）、`burden.c` を抜くと 17・118 件。 |
| 2026-09-29 | #53-D⑥ | **`monster/` に 5 本**（`61cd919`。`.h` 4 本）。`#include "creature.c"` の `movement_rate_test` は `-Isrc/monster` で見つける。壊し: `-I` を外すと 11 本（8 本）、`libcore.a` を組んでからテストだけ外すと 7 本（`movement_rate_test` ほか）、`monster_list.c` を抜くと 82・88 件。 |
| 2026-09-29 | #53-D⑦ | **`dungeon/` に 6 本**（`c2eb659`。`.h` 5 本）。`src/dungeon.c` は `src/` に残る。壊し: `-I` を外すと 18 本（16 本）、`dungeon_map.c` を抜くと 13・94 件。 |
| 2026-09-29 | #53-D⑧ | **`item/` に 14 本**（`affdf9c`。`.h` 6 本）。壊し: `-I` を外すと 15 本（12 本）、`desc.c` を抜くと 125・185 件。 |
| 2026-09-29 | #53-D⑨ | **`store/` に 1 本**（`d18375e`。`stores.c/.h`）。壊し: `-I` を外すと 4 本（4 本）、`stores.c` を抜くと 29・47 件。 |
| 2026-09-29 | #53-D⑩ | **`ui/` に 14 本**（`595a702`。`.h` 9 本）。壊し: `-I` を外すと 15 本（9 本）が「ヘッダが無い」で落ち、**`panel.h` だけは `/usr/include/panel.h`（ncurses）に黙って落ちて** `misc1.c`・`spells.c` が別のエラーになる（正しい組みでは `-I` がシステムより先なので当たらない）。`panel.c` を抜くと 60・63 件。 |
| 2026-09-29 | #53-D⑪ | **`save/` に 4 本**（`9d153ae`。`.h` 2 本）。`#include "save.c"` の 2 本は `-Isrc/save` で見つける。壊し: `-I` を外すと 10 本（6 本）、テストだけ外すと 5 本、`save_state.c` を抜くと 44・142 件。 |
| 2026-09-29 | #53-D⑫ | **`platform/` に 5 本**（`0002b5e`。`.h` 4 本。`curses.h`・`backend_ncurses.h` を含む）。`libcore.a` は 103 本のまま。壊し: `-I` を外すと 2 本（1 本）、`platform.c` を抜くと本体 3 件・テスト 0 件（どのテストも引かない）。**③〜⑫ の 10 コミットで動かしたのは `.c` 96 本・`.h` 70 本の 166 本**（`src/` に残るのは `.c` 13 本・`.h` 6 本）。どのコミットでも本体の `objdump -d` と `.text`・`.rodata`・`.data` が `aefc979` と一致、`link_units.py` の 74 本の集合と `layer_deps.py` の辺 757 本も一致、`makefile.win` は Linux で `PTHREAD=` にして組めて警告 7（前と同じ）・`objdump -d` 一致。 |
| 2026-09-29 | #53-D⑬ | **コメント: 動かしたファイルを指す `src/<名前>` を直した**（`cfaadc4`）。247 件（`src/` 49・`tests/` 179・`scripts/globals.py` 19）。行番号つき 26 件は行番号をそのまま。残したのは 5 件（`gcc` のコマンド行 2・消えた recipe の話 3）と `makefile.test` の旧 recipe の 2 件。13 コミットとも本体の警告 0、1662 件 74 本 GREEN、globals OK 42、起動 1。 |
| 2026-09-29 | #53-D マージ | **ユーザーの許可を得て `refactor/53-directories`（`99fe489`）と `docs/54-r-survey`（`cb2c7cc`。misc4・misc1 の下調べ、`get_panel` と `add_food` の行き先の改め）を `develop` へ `--no-ff` マージした。** マージ後に全ビルド・全テスト・起動を確認 —— 警告 0 / 1662 件 74 本 failed=0 `RESULT: GREEN` / globals OK 42 / 起動 1。層の規則はユーザーの判断で「まず `core/` だけ守る」。`origin` に push はしていない。ブランチは消していない。 |
| 2026-09-29 | #52 層の規則 | **層の規則を「まず `core/` だけ守る」にした**（ユーザーの判断。`a65e32f`・`b8610dc`、マージ `d6b95e3`）。`layer_deps.py` の `RULES` は core から他の全層への依存を禁じる 1 行。毎ステップの確認の 3 に `--check` を足した。壊し: `core/str_insert.c` に `randint` の呼びだしを足すと違反 1・exit 1。 |
| 2026-09-29 | #54-misc2-1 | **`magic_treasure`（833 行）をそのまま `item/item_enchant.c` へ**（`0baa1af`）。`misc2.c` は 914 → 78 行。機械語・`nm` の集合・`gcc -S` が一致。壊し: 本体 3 件・テスト 0 件（本物に届くテストが無い）。`misc3_stubs.c` の代役 9 個が覆う相手が `item_enchant.o` に替わった。1662 件。 |
| 2026-09-29 | #54-misc2-2 | **`set_options` を `ui/options_menu.c` へ移し、`misc2.c` を消す**（`124b099`）。`libcore.a` 103 → 104 本。壊し: 本体 1 件・テスト 0 件。1662 件。 |
| 2026-09-29 | #54-misc2-C | **コメント: `misc2.c` を指す注釈 6 か所を新しい置き場に直した**（`7479a8d`）。1662 件。 |
| 2026-09-29 | #54-misc4-1 | **`scribe_object`・`add_inscribe`・`inscribe` を `item/inscription.c` へ**（`427a390`）。宣言は `externs.h` のまま、自前のヘッダは作らない。機械語が一致、`nm` の集合も同じ。壊し 0 件（本物に届くテストが無く、代役 11 本が覆う）。1662 件。 |
| 2026-09-29 | #54-misc4-2 | **`check_view` を `ui/map_view.c` へ**（`7abbbb2`）。機械語が一致。壊し 0 件（代役 9 本。コミットの本文の「10 本」は誤りで、正しくは 9 本）。1662 件。 |
| 2026-09-29 | #54-misc4-3A | **`tests/concat_test.c` に 9 件**（`dcb4f9f`。#47）。引くのは `misc4.o` 1 本で代役 0。壊し 6 つで落ちた件数は 1・5・5・5・7、残る 1 つは segfault。1671 件・75 本。 |
| 2026-09-29 | #54-misc4-3B | **`concat` を `core/str_insert.c` へ移し、`misc4.c` を消す**（`cdd5bf8`）。`str_insert.h` にも宣言を置いた。core から外への依存 0。壊しは 3A と同じ件数。`desc.o` 越しに `str_insert.o` を引く 10 本に、本物の `concat` も入るようになった（どれも代役を持たないので衝突しない）。1671 件。 |
| 2026-09-29 | #54-misc4-C | **コメント: `externs.h` の見出し「// misc4.c」を 3 つの置き場に分け、`concat_test.c` の冒頭を済んだ形にした**（`4d4b515`）。1671 件。 |
| 2026-09-29 | #54 マージ | **ユーザーの許可を得て 3 本をマージ**: `refactor/52-layer-rules`（`d6b95e3`）、`refactor/54-misc2`（`1647652`）、`refactor/54-misc4`（`d437df0`）。`sources.mk` の衝突は 2 か所で、両方の変更を残して解いた（SRCS の先頭行から `misc2.c`・`misc4.c` の両方を消し、`item/` の行に `inscription.c` と `item_enchant.c` の両方を残す）。マージごとに確認を回し、最後は警告 0・75 本 1671 件 GREEN・globals OK・core の違反 0・起動 1。 |
| 2026-09-29 | #54 道具 | **`scripts/dis_compare.py` を足した**（`8f1ae16`）。misc4 の担当の台本は switch の跳び先の表を文字列として読み、`magic_treasure`・`set_options` を誤って「違う」とした。跳び先を関数の先頭からの位置に直して比べる形にすると、#54 で動かした 9 本がすべて一致。壊し 3 通り（case の札の入れかえ・`|=` → `=`・文字列の 1 語）は、どれも壊した関数だけを捕まえた。 |
| 2026-09-29 | HANDOVER の整理 | **HANDOVER 第 7 節の #18 の頃の 25 項目を `done/handover-7-18.md` へ字を変えずに移した**（`a32cb6a`。57,751 → 約 42,900 字）。/tmp のメッセージと控えを担当ごとの名前の下に置く手順にした（`6aec91b`。misc4 の担当が別の担当の `/tmp/msg.txt` で一度誤ってコミットしたため）。 |
| 2026-09-29 | #54-misc1-1 | **`bit_pos` を `core/bits.c` へ**（`f107639`）。`externs.h` を読まないので `core/bits.h` にも宣言（`concat` と同じ形）。34 本の機械語が一致。core から外への辺 0（層 762 → 770 本、111 → 112 単位）。1671 件。 |
| 2026-09-29 | #54-misc1-2 | **`m_bonus` を `item/item_enchant.c` へ**（`6af6304`）。同じ翻訳単位に来た `m_bonus` を `-O2` が `magic_treasure` の 45 か所に展開したので、**比べ方を `-O2 -fno-inline` の組みなおしに広げた**（35 本が一致）。壊し 0 件（本物に届くテストが無い）。 |
| 2026-09-29 | #54-misc1-3 | **`next_to_walls`・`next_to_corr` を新しい `dungeon/geometry.c` へ**（`83f4acf`）。`-fno-inline` で 35 本一致。壊し 2 つとも 0 件。 |
| 2026-09-29 | #54-misc1-4 | **`compact_objects`・`popt`・`pusht` を新しい `dungeon/object_place.c` へ**（`66963f5`）。`static` の `compact_objects` は呼び手の `popt` と一緒に。`-fno-inline` で 35 本一致。 |
| 2026-09-30 | #54-misc1-5 | **`get_panel`・`loc_symbol`・`test_light`・`prt_map` を `ui/map_view.c` へ**（`7a28508`）。`misc1.c` から要らなくなった include 4 行を消した（1 つずつ外して組めることを確かめた）。`-fno-inline` で 35 本一致。 |
| 2026-09-30 | #54-misc1-6 | **モンスターを置く 9 本を新しい `monster/monster_place.c` へ**（`d660b8b`）。`static` の `get_mons_num`・`summon` は前方宣言ごと。include 7 行を消した。`-fno-inline` で 34 本一致、`place_monster` だけ `max_hp ? … : pdamroll` の選び方が cmov か分岐かで違う（2 本が別の翻訳単位になったため）。 |
| 2026-09-30 | #54-misc1-7 | **`add_food` を新しい `player/food_ops.c` へ**（`327d5b6`。ユーザーの判断。`player_food.c` には入れない）。壊し 0 件（本物に届くテストが無い）。 |
| 2026-09-30 | #54-misc1-8 | **`in_bounds`・`distance`・`los` を `dungeon/geometry.c` へ**（`13de86e`）。代役（`creature_stubs.c`・`misc3_stubs.c`）が覆う相手が `misc1.o` から `geometry.o` に替わっただけで、引かれる `.o` は 75 本とも同じ。 |
| 2026-09-30 | #54-misc1-9 | **`init_seeds` を `src/game_state.c` へ**（`d18fbf6`。ユーザーの判断。core には入れない）。`game_state.o` の依存行に `progress.h` が無いのは前からの漏れで、直していない。 |
| 2026-09-30 | #54-misc1-10 | **コメント: 1〜9 で動かした関数を指す「misc1.c」を新しい置き場に直した**（`a4df61c`）。**コミットの 1 行目に `#54-misc1-10:` が付いていない**が、履歴は書きかえずにここで番号を振る。 |
| 2026-09-30 | #54-misc1-11 | **`externs.h` の見出し「// misc1.c」を移った先ごとに分け、ディレクトリーを付けた**（`a74e12c`。ユーザーの判断）。`// options_menu.c` なども同じ形に直した。本体は `.note.gnu.build-id` のほかどの section も同じ。 |
| 2026-09-30 | #54 道具 | **`dis_compare.py` の跳び先の表の判定を文字列より先にした**（`b8bc5bd`）。NUL で始まる表が空の文字列 `STR''` と読まれ、`magic_treasure` の違いを見落としていた（misc1 の担当が見つけた）。前の版は 5 と 6 の間の `magic_treasure` を DIFFERENT と言い、直した版は same。壊し（case の入れかえ）は捕まえる。 |
| 2026-09-30 | #54 コメント | **misc2 の頃の注釈 3 か所を移動先の話として書き直した**（`01c4971`。`tests/options_test.c`・`src/data/options.h`。ユーザーの判断）。 |
| 2026-09-30 | #55 下調べ | **store1・store2 の下調べを layout.md に書き、B22 を足した**（`b62de9d`）。B22 は B9 の状態で `store_sell` が `mask` の外に 1 つ書く。直さない。store2 の確かめ方は、ユーザーの判断で**テストと差分の読みだけ**（機械語の一致は見込めない）。 |
| 2026-09-30 | #54 マージ | **ユーザーの許可を得て 4 本を `develop` へ `--no-ff` マージした**：`refactor/54-misc1`（`76ced6f`）、`refactor/54-options-comments`（`8c7db69`）、`refactor/54-dis-compare-jt`（`337ca99`）、`docs/55-store-survey`（`9456204`）。衝突なし。警告 0・75 本 1671 件 GREEN・globals OK 42・層 814 本 116 単位で違反 0・起動 1。**ユーザーの指示で、マージ済みのブランチ 19 本とそのワークツリーを消した。** |
| 2026-09-30 | #54-misc1-12 | **`set_seed`・`reset_seed`・`randint`・`randnor`・`magik` と `normal_table` を `core/rnd.c` へ**（`c2fb744`）。表は `tables.c` から表ごと（ユーザーの判断）、512 バイトは同じ。`globals.py` の `HOME_FILES` に `rnd.c`、`rnd.o` の依存を `HEADERS_FULL` に。`-fno-inline` では 4 本が違う —— `rnd()` と同じ翻訳単位になって退避するレジスターが減った（IPA-RA）。`-fno-ipa-ra` を足すと `randnor` の `sub/add $0x8,%rsp` の 1 組（とそれに連れた跳び先）のほかは一致。層 822 本。 |
| 2026-09-30 | #54-misc1-12 追 | **`makefile.win` の `rnd.o` の依存を揃えた**（`ebbcdb6`）。12 で直し忘れた。 |
| 2026-09-30 | #54-misc1-13 | **`damroll`・`pdamroll`・`max_hp` を `core/dice.c` へ移し、`misc1.c` を消した**（`ac880fd`。`git mv`）。`-fno-inline -fno-ipa-ra` で 3 本と `randint` が一致。**このコミットは一度 `git add` の失敗で名前変えだけになり（組めない）、作った直後・マージ前に `--amend` で正しい中身にした。** `libcore.a` は 110 本のまま（`misc1.o` が `dice.o` に）。 |
| 2026-09-30 | #54-misc1-14 | **コメント: いまの置き場として「misc1.c」を指す 6 か所を直した**（`196a837`）。`test_hit` を `misc1.c` と書いていた 2 本（`player_armour_class.h`・`player_base_to_hit.h`）は、少なくとも 2016 年から `moria1.c` で、書いたときからの誤りだった。昔の経緯として書いたものは残した。 |
| 2026-09-30 | #54 マージ | **ユーザーの許可を得て 2 本をマージした**：`refactor/54-misc1-rnd`（`76d223f`）、`docs/54-misc1-records`（`12ce7e0`）。衝突なし。 |
| 2026-09-30 | #55-store-1 | **`item_value`・`sell_price`・`noneedtobargain`・`updatebargain` を新しい `store/store_price.c` へ**（`855c998`）。`static` は 0 本で、宣言は `externs.h` のまま。動かした 4 本は `-fno-inline` で機械語が一致。 |
| 2026-09-30 | #55-store-2 | **`externs.h` の見出し「// store1.c」から値段の 4 本を `// store/store_price.c` に分けた**（`36ae8e9`）。 |
| 2026-09-30 | #55 マージ | **ユーザーの許可を得て `refactor/55-store-price` をマージした**（`59e0c92`）。`sources.mk` の衝突は 1 か所で、両方を残して解いた（`store/store_price.c` と、misc1 の `core/dice.c` など）。警告 0・75 本 1671 件 GREEN・globals OK 42・層 827 本 117 単位で違反 0・起動 1。 |
| 2026-09-30 | #55-store-3 | **`store1.c` の残り 7 本（品ぞろえ）を `git mv` で `store/store_stock.c` にし、`store1.c` を消した**（`8351ce2`）。`-fno-inline` で機械語が一致。 |
| 2026-09-30 | #55-store-4 | **`externs.h` の見出し「// store1.c」を `// store/store_stock.c` に替えた**（`0ea41fd`）。 |
| 2026-09-30 | #55-store-5 | **`store2.c` の画面と買う・売るの命令 10 本と表 `comment1`（`prt_comment1`・`display_*`・`store_prt_gold`・`get_store_item`・`store_purchase`・`store_sell`・`enter_store`）を新しい `store/store_ui.c` へ**（`a341211`）。`prt_comment1` は画面、`decrease_insults` は `insult_cur` を書くので値切り、はユーザーの判断。値切りの 4 本（`increase_insults`・`decrease_insults`・`purchase_haggle`・`sell_haggle`）は `static` を外し、新しい `store/store_haggle.h` に宣言。確かめ方はテストと差分の読み（ユーザーの判断）。参考に `-fno-inline` で比べると、16 本が一致、3 本は比べられず、5 本は `endbr64`（`static` を外したため）とレジスターの割り当てだけが違う。層 834 本 118 単位。 |
| 2026-09-30 | #55-store-6 | **値切りだけになった `store2.c` を `git mv` で `store/store_haggle.c` にした**（`1f714ba`）。`haggle_comment_test` の `#include` と、それを説明するコメントも付けかえた。`externs.h` の `enter_store` の見出しは `// store/store_ui.c`。 |
| 2026-09-30 | #55-store-7 | **コメント：いまの呼び手や置き場として `store1.c`・`store2.c` を指す 16 ファイルを直した**（`3eaa18b`）。行番号も移った先に直した。来歴として書いたものは残した。`check_strength_test` の `inven_check_weight` の呼び手（「moria1.c と store1.c」）はもとから誤りで、呼ぶのは `moria3.c:315` だけ。`haggle_comment_test` の冒頭の「中身が完全に同一の 13 行」は #12 の統合の前の話で古いが、ここでは直していない。代役の一部（画面の側だけが呼んでいた名前）も要らなくなった見込みで、これも残した。 |
| 2026-09-30 | #55 マージ | **ユーザーの許可を得て `refactor/55-store2`（`refactor/55-store-stock` を含む）をマージした**（`ced26a4`）。衝突なし。警告 0・75 本 1671 件 GREEN・globals OK 42・層 834 本 118 単位で違反 0・起動 1。**マージ済みのブランチ 3 本（`refactor/55-store2`・`refactor/55-store-stock`・`worktree-agent-a94e68b8307dfd112`）とワークツリーを消した。** |
| 2026-09-30 | #42 下調べ | **misc3 の下調べを layout.md に書いた**（読みとりと /tmp の写しだけ。コミットは 0）。10 塊・77 関数。本物に届くテストがあるのは 15 本だけ。A を `object_place.c` に合流させると `object_levels_test` が `popt` の二重定義でリンクに落ちる。`gain_spells` のコメントの「B22」は bugs.md の B22 と番号がぶつかる。迷いどころ 13 点はユーザーに見せる。 |
| 2026-09-30 | #42-1 | **`enter_wiz_mode` を `ui/wizard.c` へ**（`b3c0ad6`）。合流先の既存ファイル。`-O2 -fno-inline` で同じ。 |
| 2026-09-30 | #42-2 | **`combat/` を作り、`attack_blows`・`tot_dam`・`critical_blow` を `combat/hit_rolls.c`、`player_saves` を `combat/player_damage.c` へ**（`f076308`）。`SRC_SUBDIRS` に `combat` を足した。`-O2 -fno-inline` と素の `-O2` で同じ。 |
| 2026-09-30 | #42-3 | **コメント：I・J の移った先**（`2c9f6d4`）。`player_saves` の中の日本語のコメントを英語に。 |
| 2026-09-30 | #42-4 | **K の 3 本を近い仲間へ**（`4b5244c`）。`mmove` → `dungeon/geometry.c`、`find_range` → 新しい `item/inven_ops.c`、`teleport` → 新しい `player/player_move.c`。同じ。 |
| 2026-09-30 | #42-5 | **コメント：`teleport`**（`aed3507`）。 |
| 2026-09-30 | #42-6 | **A（配置の 7 本）を新しい `dungeon/object_alloc.c` へ**（`a9dbc5c`）。object_place.c には合流させない（`object_levels_test` が `popt` の二重定義で落ちるため）。同じ。 |
| 2026-09-30 | #42-7 | **コメント：A と `mmove`・`teleport`**（`32fc0d5`）。 |
| 2026-09-30 | #42-8 | **E（持ち物の 10 本）を `item/inven_ops.c` へ**（`a24105d`）。`find_range` の前に misc3.c の並びのまま。include 6 本が misc3.c から落ちた。同じ。 |
| 2026-09-30 | #42-9 | **コメント：E**（`78a1c5b`）。11 ファイル。 |
| 2026-09-30 | #42-10 | **表の下ごしらえ**（`999363e`）。misc3.c の中に `static` の `erase_field`・`prt_stat_name` を足し、`blank_string` の 10 か所と `stat_names` の 2 か所をその呼びだしにした。**ふるまいを変えない書きかえ**で、素の `-O2` で呼び手 13 本が同じ（展開される）。 |
| 2026-09-30 | #42-11 | **`ui/screen_fields.c` / `.h` を作った**（`6b34fb8`）。表 2 つ（`static` のまま）と、`static` を外した `erase_field`・`prt_stat_name`・`prt_num`・`prt_long`。ユーザーの「UI 層の操作として呼べるように。ヘッダーに実装は書かない」のとおり。`-fno-ipa-cp -fno-ipa-icf` で呼び手 27 本が同じ（4 本は `endbr64` だけ違う）。`-fno-inline` では `prt_long.constprop` が無くなり、呼び手が列 6 を自分で渡す。 |
| 2026-09-30 | #42-12 | **B のステータス行（25 本）を新しい `ui/status_line.c` へ**（`fff38d8`）。人物画面だけが使う `prt_7lnum` は残した。`-fno-ipa-cp -fno-ipa-icf` で 27 本同じ。 |
| 2026-09-30 | #42-13 | **B の人物画面と D を新しい `ui/char_screen.c` へ**（`e46483c`）。`prt_7lnum` も一緒に。同じ。 |
| 2026-09-30 | #42-14 | **コメント：状態行・人物画面・画面の欄**（`019442d`）。12 ファイル。worktree のサブエージェントが書き、C′ の作業と並べて進めた。 |
| 2026-09-30 | #42-15 | **C′（能力値の 6 本）を新しい `player/stat_ops.c` へ**（`6ae30df`）。同じ。 |
| 2026-09-30 | #42-16 | **F（呪文の 6 本）を新しい `item/spellbook.c` へ**（`d227b86`）。同じ。 |
| 2026-09-30 | #42-17 | **G だけが残った misc3.c を `git mv` で `player/level_ops.c` に**（`82eba34`）。**misc3.c が無くなった。** 同じ。 |
| 2026-09-30 | #42-18 | **コメント：F・G と、前の段で残った misc3.c**（`bea98fe`）。「バグ候補 B22」を B23 に。移った関数の中の日本語のコメントを英語に。サブエージェントが worktree で。 |
| 2026-09-30 | #42-19 | **`misc3_stubs.c` の冒頭と REFACTORING_PLAN:262 を今に合わせた**（`38325a1`）。 |
| 2026-09-30 | #42 マージ | **ユーザーの許可を得て `refactor/42-misc3` をマージした**（`cb5279a`）。衝突なし。警告 0・75 本 1671 件 GREEN・globals OK 42・層 963 本 128 単位で違反 0・起動 1。**マージ済みのブランチ `refactor/42-misc3` と、作業用の worktree（`/tmp/mgd/wt-old`・サブエージェント用の 2 本）を消した。** 最上位に残る古い `.o`（`misc1.o` など）はユーザーがあとで掃除する。 |
| 2026-10-01 | #56 進めかた | **moria1〜4 の迷いどころ 8 点はすべて案のとおり**（ユーザーの判断）。2 列で並べた：列 X（moria2 → moria1、自分）と列 Y（moria4 → moria3、worktree のサブエージェント）。どの移動も、足した行と消した行の差がファイルの頭と `#include` だけであることと、`-O2 -fno-inline` と `-O2 -fno-inline -fno-ipa-cp -fno-ipa-icf` の両方で関数の機械語が直前のコミットと一致することを確かめた。テストは足していない。 |
| 2026-10-01 | #56-2-1 | **`change_trap`・`search` を新しい `dungeon/search.c` へ**（`b9bc379`）。同じ。 |
| 2026-10-01 | #56-2-2 | **`minus_ac`〜`acid_dam` の 7 本を `combat/player_damage.c` の末尾へ**（`77bee3a`）。同じ。 |
| 2026-10-01 | #56-2-3 | **`git mv` で moria2.c を `player/run_path.c` に**（`75ad428`）。**このコミットは壊れている**：`git add src/moria2.c` が消えたファイルで失敗し、名前の変更だけが入った（sources.mk が moria2.c のまま）。amend はせず、次の #56-2-3b で残りを入れた。マージの前にまとめるかはユーザーに尋ね、そのままマージと決まった。 |
| 2026-10-01 | #56-2-3b | **改名の残り**（`69135e7`）。冒頭の説明・sources.mk・`run_path.o` の依存・externs.h の見出し `// player/run_path.c`。同じ。 |
| 2026-10-01 | #56-2-4 | **コメント：moria2.c を指すところを移った先に**（`8c06bfa`）。 |
| 2026-10-01 | #56-1-1 | **`change_speed`・`py_bonuses`・`calc_bonuses` を新しい `player/player_bonuses.c` へ**（`012cb46`）。同じ。コミットのメッセージは #56-2-4 と比べたと書くが、比べたのは #56-2-3b（#56-2-4 はコメントだけなので機械語は同じ）。 |
| 2026-10-01 | #56-1-2 | **`test_hit` を `combat/hit_rolls.c` の `attack_blows` の前へ**（`fa14f99`）。同じ。 |
| 2026-10-01 | #56-1-3 | **`take_hit` を `combat/player_damage.c` の `minus_ac` の前へ**（`feec8ff`）。同じ。 |
| 2026-10-01 | #56-1-4 | **`disturb`・`search_on`・`search_off`・`rest`・`rest_off` を新しい `player/rest_command.c` へ**（`cfba8c0`）。同じ。 |
| 2026-10-01 | #56-1-5 | **`map_roguedir`・`get_dir`・`get_alldir` を新しい `ui/direction.c` へ**（`d107063`）。同じ。 |
| 2026-10-01 | #56-1-6 | **明かりの 7 本を新しい `dungeon/lighting.c` へ**（`03a8f75`）。同じ。 |
| 2026-10-01 | #56-1-7 | **`git mv` で moria1.c を `ui/inven_menu.c` に**（`225e610`）。makefile.test の `LIB_EXCLUDE` も inven_menu.c に（中身は変えていない）。同じ。 |
| 2026-10-01 | #56-1-8 | **コメント：moria1.c を指すところを移った先に**（`ae2f40a`）。50 ファイル。サブエージェントが worktree で。 |
| 2026-10-01 | #56-4-1 | **`py_bash` の `static` を外した**（`0a4ce56`）。違いは頭の `endbr64` とその後ろの飛び先のずれだけ。ほかは同じ。 |
| 2026-10-01 | #56-4-2〜5 | **`py_bash` → 新しい `combat/player_melee.c`、投げる 4 本 → 新しい `combat/throw.c`、`disarm_trap` → 新しい `dungeon/traps.c`、`tunnel`・`bash` → 新しい `dungeon/terrain_commands.c`**（`07926c0`・`339e62d`・`fb9d147`・`2a75bf0`）。同じ。 |
| 2026-10-01 | #56-4-6 | **`git mv` で moria4.c を `ui/look.c` に**（`7068e41`）。同じ。 |
| 2026-10-01 | #56-4-7 | **コメント：moria4.c を指すところを移った先に**（`0105184`）。メッセージの「src の 11 本」は誤りで、12 本。 |
| 2026-10-01 | #56-3-1 | **`hit_trap` の `static` を外した**（`0083e5e`）。違いは頭の `endbr64` だけ。 |
| 2026-10-01 | #56-3-2〜7 | **`hit_trap`・`chest_trap` → `dungeon/traps.c`、`cast_spell` → `item/spellbook.c` の末尾、`carry`・`move_char` → `player/player_move.c` の末尾、`mon_take_hit` → 新しい `combat/monster_damage.c`、`py_attack` → `player_melee.c` の `py_bash` の前、`openobject`・`closeobject`・`twall` → `terrain_commands.c` の `tunnel` の前**（`ee1b6bb`・`90377f5`・`09a44f5`・`f5f1d8c`・`3922c44`・`56b0e3a`）。同じ。 |
| 2026-10-01 | #56-3-8 | **`git mv` で moria3.c を `monster/monster_death.c` に**（`cf0d9a0`）。**moria1〜4.c が無くなった。** 同じ。 |
| 2026-10-01 | #56-3-9 | **コメント：moria3.c を指すところを移った先に**（`27e871c`）。メッセージの「src の 21 本」は誤りで、22 本。来歴として moria3.c を書くコメント 4 か所は残した。 |
| 2026-10-01 | #56 マージ | **ユーザーの許可を得て 4 本をマージした**（`refactor/56-moria2` `c0fe8d3`・`56-moria1` `7a2bc35`・`56-moria4` `ce9251e`・`56-moria3` `3787dc4`）。後の 2 本は sources.mk・makefile・makefile.win と、コメント 5 ファイルで衝突し、両方の移り先を残して解いた。警告 0・75 本 1671 件 GREEN・globals OK 42・層 1104 本 138 単位で違反 0・起動 1。**マージ済みのブランチ 4 本と worktree 2 本を消した。** |
| 2026-10-01 | #57 進めかた | **combat の残りの迷いどころ 5 点はすべて案のとおり**（ユーザーの判断）。movement_rate_test は変えない。 |
| 2026-10-01 | #57-1 | **`make_attack` の `static` を外した**（`6b23689`）。違いは頭の `endbr64` と、それに続く nop と飛び先のずれだけ。creature.c のほかの 9 本は同じ。 |
| 2026-10-01 | #57-2 | **`make_attack` を新しい `combat/monster_melee.c` へ**（`d7a0bd6`）。同じ。movement_rate_test は libcore から `monster_melee.o` を引くようになった（24 件とも通る）。 |
| 2026-10-01 | #57-3 | **`get_flags`・`fire_bolt`・`fire_ball`・`breath` を新しい `combat/projectiles.c` へ**（`02c5f92`）。同じ。例外は spells.c の `hp_monster` の jmp 1 つ（`eb 82` → `e9 …`、飛び先は同じ）。 |
| 2026-10-01 | #57-4 | **コメント：移った先とずれた行番号**（`0a04327`）。src 6・tests 11。サブエージェントが worktree で。 |
| 2026-10-01 | #57 マージ | **ユーザーの許可を得て `refactor/57-combat` をマージした**（`0416bb0`）。衝突なし。警告 0・75 本 1671 件 GREEN・globals OK 42・層 1139 本 140 単位で違反 0・起動 1。**ブランチと worktree を消した。** |
| 2026-10-01 | 後始末 進めかた | **案のとおり**（ユーザーの判断）。12 本の `static` 化、使われていない代役 13 本の削除、古いコメントの修正、経緯だけのコメントの削除。#37・#44 は #38 まで棚上げ。 |
| 2026-10-01 | cleanup-1〜10 | **12 本を `static` に**（`6a6e343`〜`996f256`、1 ファイル 1 コミット）。noipa では違うのは 12 本だけで、`endbr64` と nop を除けば同じ。noinline では `minus_ac`・`prt_field` に `.constprop.0` ができ、呼ぶ側 4 本が変わった。 |
| 2026-10-01 | cleanup-11・12 | **使われていない代役を消した**（`688e0a5` misc3_stubs 7 本、`e63445c` save_stubs 6 本）。リンクされるメンバーは変わらない。 |
| 2026-10-01 | cleanup-13 | **コメント：いまのコードと合わない記述**（`b698337`）。src 5・tests 3。 |
| 2026-10-01 | cleanup-14〜16 | **コメント：経緯だけの記述を消した**（`1fe25c6` src 27 ファイル、`11ddb63` makefile、`40811f7` misc3_stubs.c の頭）。機械語は全関数で同じ。 |
| 2026-10-01 | 後始末 マージ | **ユーザーの許可を得て 3 本をマージした**（`acdcd51`・`77c8182`・`082b0e4`）。衝突なし。警告 0・75 本 1671 件 GREEN・globals OK 42・層 1139 本 140 単位で違反 0・起動 1。**ブランチ 3 本と worktree を消した。** |
| 2026-10-01 | #38 進めかた | **案のとおり**（ユーザーの判断）。「1 実行形式 1 対象 module」は、実行形式が引くのは対象の module と、対象（とテスト本体）が本当に呼ぶものだけで、共有の足場がつれてくる単位は 0、という意味にする。今回は段階 A（テストだけ）。段階 B（`item_ident` → `prt_experience` の中心を切る src の変更）は A の後で別に決める。足場は対象 module ごと、コメントは要点だけ、定数を返す代役（#44）は中身を変えずに運ぶ。 |
| 2026-10-01 | #38-1〜7 | **`misc3_stubs.c` の 9 本を対象 module ごとの足場 7 つへ**（`61b98ee`〜`5d978be`、1 足場 1 コミット）。`fixture_reset()` はリンクされる窓口だけを 0 に戻し、代役は呼ばれる名前だけ。リンク単位は haggle_comment 40→7、calc_spells・gain_spells 39→12、calc_hitpoints 39→27、put_misc3 43→32、objdes 39→32、check_strength・inven_stack 44→38、object_levels 45→38。テストごとの出力は着手前と同じ。 |
| 2026-10-01 | #38-8・9 | **コメントを新しい足場に向けて、`misc3_stubs.c` を消した**（`e431daf`・`791f808`）。`fixture.h` の「misc3_stubs.c だけが提供する」は `fixture_randint_last_maxval` の分がもともと誤り（`fixture.c` も持つ）。 |
| 2026-10-01 | #38-10〜17 | **写しをまとめた**（ユーザーの判断）。足場を分けたことで、2 つ以上のファイルに定義がある名前が 32 本・76 定義から 44 本・226 定義に増えたため。画面・入力・乱数などの代役を `tests/shared_stubs.c` に置き、各足場は `fixture_reset()` だけにした（`7fce6dd`〜`08591da`、コメントは `cfbb67b`）。代役は libcore の中を呼ばないので、リンク単位は変わらない。重なりは develop と同じ 32 本・76 定義に戻った。 |
| 2026-10-01 | #38 マージ | **ユーザーの許可を得て `refactor/38-fixtures` をマージした**（`48af494`）。17 コミットそれぞれを空の状態から組んで、警告 0・75 本 1671 件 GREEN。globals OK 42・層 1139 本 140 単位で違反 0。src の変更は `object_alloc.c` のコメント 1 か所だけ。**ブランチと worktree を消した。** |
| 2026-10-01 | #37-1 | **`creature_stubs.c` の重なり 12 本を `shared_stubs.c` に寄せた**（`3fb28ad`、ユーザーの判断）。movement_rate は両方をリンクし、引くメンバー 24 個は同じ。観測窓口 3 つが記録する版になった。マージ `df6de6d`。 |
| 2026-10-01 | #44 調査 | **定数を返す代役 57 本のうち、テストで呼ばれるのは `no_light`・`set_large` の 2 本だけ**（全部を abort にして確かめた）。コードは変えていない。穴と手の入れかたは layout.md の「#38 で決まったこと・やったこと」。 |
| 2026-10-01 | #38-18・19 | **段階 B：`learn_item_effect()` を `src/item/item_learn.c` へ移した**（`fd89496`、ユーザーの判断。ふるまいは同じ）。続けて 3 つの足場から、もう誰もリンクしない窓口の片づけを外した（`32f8706`）。check_strength・inven_stack 38→15、object_levels 38→14、objdes 32→7。 |
| 2026-10-01 | #38 段階 B マージ | **ユーザーの許可を得てマージした**（`65773e1`）。2 コミットそれぞれを空の状態から組んで、警告 0（本体・テスト）・75 本 1671 件 GREEN・globals OK 42・層 1139 本 141 単位で違反 0。**ブランチと worktree を消した。** |
| 2026-10-01 | #44 進めかた | **代役のせいの穴 2 か所にテストを足し、届かない代役は呼ばれたら止める**（ユーザーの判断）。全部は覆わず、それぞれ 2〜3 通り。 |
| 2026-10-01 | #44-1・2 | **`set_large`・`no_light` の代役を記録・操作できるようにし、テストを 3 本ずつ足した**（`4ea0a61`・`518b7c7`）。get_obj_num の位置と番号の取りちがえ・条件の削除・反転、gain_spells の明かりと盲の判定の削除・入れかえの 6 つの変異が赤になる。 |
| 2026-10-01 | #44-3〜6 | **届かない代役 51 本を `stub_unreached()` で止めた**（`133e714`〜`815f8cc`、1 ファイル 1 コミット。shared 13・creature 25・save 7・fixture.c 6）。`fixture.c` の「store_bought_p が参照する」は誤りだったので直した（参照するのは `tables.c` の表）。 |
| 2026-10-01 | #44 マージ | **ユーザーの許可を得てマージした**（`264d258`）。6 コミットそれぞれを空の状態から組んで、警告 0（本体・テスト）・75 本 1677 件 GREEN・globals OK 42・層 1139 本 141 単位で違反 0。**ブランチと worktree を消した。** |
| 2026-10-01 | #44-7〜14 | **代役を本物（`sets.o`・`geometry.o`・`dice.o`・`bits.o`・`panel.o`・`str_insert.o`）に替えた**（`9db83b2`〜`11c4c17`、ユーザーの判断）。recall の 6 本は `variable.o` が `shared_stubs.c` の 2 つの global と重なるので代役のまま。**`9db83b2` の説明の「ほかの 8 本は本物を引く」は誤り**で、`sets.o` を引くのは 4 本（haggle_comment・check_strength・inven_stack・objdes）。 |
| 2026-10-01 | #44 本物へ マージ | **ユーザーの許可を得てマージした**（`230ddcc`）。8 コミットそれぞれを空の状態から組んで、警告 0（本体・テスト）・75 本 1677 件 GREEN・globals OK 42・層 1139 本 141 単位で違反 0。**ブランチと worktree を消した。** 誤って本体側に作られた `tests/build/`（無視対象）も消した。 |
| 2026-10-01 | #44-15・16 | **recall の 6 本も本物（`variable.o`）に替えた**（`36278c9`・`cec6310`、ユーザーの判断）。先に `shared_stubs.c` の `free_turn_flag`・`display_counts` を外した。`variable.o` を引くのは 5 本（calc_hitpoints・calc_spells・gain_spells・put_misc3 +1、movement_rate 30→31）。 |
| 2026-10-01 | #44 recall マージ | **ユーザーの許可を得てマージした**（`c2a97d5`）。2 コミットそれぞれを空の状態から組んで、警告 0（本体・テスト）・75 本 1677 件 GREEN・globals OK 42・層 1139 本 141 単位で違反 0。**ブランチと worktree を消した。** |
