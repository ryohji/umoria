# バグ候補

もとは `REFACTORING_PLAN.md` の節（`994818a` の 7626〜7696 行）。2026-09-29 に字を変えずに移した。

## バグ候補（このリファクタリングでは直さない）

| # | 場所 | 内容 | 見つけた経緯 |
|---|---|---|---|
| ~~B1~~ | ~~misc3.c:1162-1181 と 1242-1281~~ | **解消済み（#10-B）。** `items_can_stack(existing, incoming)` に抽出したので、2 箇所が「同一である保証」を人手のコメントに頼る状態そのものが無くなった。`tests/inven_stack_test.c` の 40 件が判定を固定している | #10 の調査中 → #10-B で解消 |
| B2 | spells.c:555-593 `get_flags` | 失敗時に `msg_print("ERROR in get_flags()\n")` でデバッグ文字列を画面に出力する | #8 の調査中 |
| B3 | game_state.c:22-112 | `game_state_init` が既存グローバル約50個をコピーする設計で、「コピーした瞬間に元と乖離する」。呼ばれていないので現状は無害だが、将来 externs.h を変えると同期漏れが起きる | #4 の調査中 |
| B4 | creature.c:75-86 `movement_rate` | 速度>0 で移動回数、速度<=0 で bool(0/1) を返す。単位が2種類混在しており、呼び出し側の解釈が正しいか未確認 | #14 の調査中 |
| B5 | staffs.c:39, wands.c:47 | **混乱すると魔法道具の成功率が上がる場合がある。** `chance` が負のとき `chance/2` が 0 方向に丸められて絶対値が縮み、救済抽選 `randint(USE_DEVICE - chance + 1)` の幅が狭くなる。幅が狭いほど 1 を引きやすく `USE_DEVICE` に引きあげられやすい。実測：`chance=-10` で非混乱 `randint(14)`（1/14）に対し混乱 `randint(9)`（1/9）。混乱がペナルティになっていない | #7 のステップ A（テスト保護）中。`device_chance_test.c` の `confusion_narrows_lucky_roll_range_when_chance_is_negative` に TODO で記録済み |
| B6 | staffs.c:45, wands.c:53 | **`chance == 1` では成功が原理的にありえない。** `randint(1)` は常に 1 を返し、判定は `randint(chance) < USE_DEVICE`(=3) が失敗条件なので必ず失敗する。直前の `if (chance <= 0) chance = 1;` は「わずかな成功機会を与える」意図に見えるが、実際の成功率は 0 | 同上。`device_use_always_fails_when_chance_is_one` で固定済み |
| **B15** | signals.c:88-104 と render_ncurses.c:109 | **セーブ後にサスペンド（^Z）が効かなくなる。** `nosignals()`（`signals.c:88`）が `signal(SIGTSTP, SIG_IGN)` で無効化するが、対になる `signals()`（`:97`）は `sigsetmask` を戻すだけで **`SIGTSTP` を再登録しない**。コメント `// SIGTSTP is handled by the rendering system` が理由だが、`render_ncurses.c:109` の `signal(SIGTSTP, suspend)` は初期化時に1回だけ実行されるので、`SIG_IGN` で潰されたあとは誰も戻さない。`nosignals()` は `save.c:406`, `save.c:480`, `death.c:445` から呼ばれる → **セーブすると以後 ^Z が無反応になる**。ncurses 導入後に判明。ふるまいの変更になるので直さない | 全体の再評価中（2026-08-28）。独立に確認済み |
| **B13** | creature.c:83 | **コメント `// speed must be negative here` が誤り。** この枝の条件は `speed > 0` の否定なので `speed == 0` も入る。`cspeed = r_ptr->speed - 10 + py.flags.speed`（`misc1.c:600`）なので 0 は容易に発生する。0 のときも `2 - 0 = 2` で周期 2 として正しく動くので、**コードは正しくコメントだけが誤り** | #14 のステップ A 中。独立に `cspeed` の範囲を確認済み |
| **B14** | creature.c:84 | **`turn` が負のとき「動かない」と判定される。** `turn` は `variable.c:69` で `-1` に初期化される。C の `%` は被除数が負なら 0 か負を返すので `-1 % 2 == -1` で `== 0` が成りたたない。ゲーム開始直後にこの枝を通る可能性があるが、**意図的かどうか判断できない**（実害は「最初の1ターン、遅い敵が動かない」程度で軽微）。ふるまいの変更になるので直さない | 同上。テストで現状を固定済み |
| **B12** | makefile.win | **`makefile.win` が本体の構成に追随していない。** 新規追加した `device.c` / `item_ident.c` / `abilities.c` が入っていないが、**このリファクタリング以前から放置されていた**（UI 抽象化で追加された `render.c` / `input.c` / `platform.c` / `view_observer.c` も未登録）。最終更新は `70ab060 Moves all unix.c code to io.c`。Windows ビルドは現状すでに通らないと考えられる。**私たちが壊したものではない**が、Windows ビルドを復活させるなら一括で追随が必要 | #11 のステップ B 中に判明。独立に確認済み |
| **B10** | misc3.c:973, files.c:236 | **コメント `// this results in a range from 0 to 29` が誤り。** `xfos = 40 - fos`（負なら0クランプ）なので、`fos` が 11 未満だと 29 を超える。作成時の `fos` は 11..43 に収まるが、探索つき装備が `py.misc.fos -= amount`（`moria1.c:48`）で下げるため実プレイ中に 11 を下まわる。実測：`fos=10` → `xfos=30`、`fos=0` → `xfos=40`。**コードは正しく、コメントだけが誤り**（読み手を誤解させる） | #11 のステップ A 中。独立に検算済み。`xfos_exceeds_twenty_nine_when_fos_falls_below_eleven` で固定 |
| **B11** | misc3.c:977, files.c:243 | **コメント `// this results in a range from 0 to 9` が誤り。** `xstl = stl + 1` なので最小値は 1（`stl` が 0 のとき）。範囲は 1..10 が正しい。**コードは正しく、コメントだけが誤り** | 同上。`xstl_is_one_when_stl_is_zero` で固定 |
| **B8** | misc3.c:1251-1272 `inven_carry` | **配列外読み（global-buffer-overflow）。** ループ `for (locn = 0;; locn++)` に**終了条件がない**。スタックも割りこみもできないアイテムを渡すと `inventory[]`（34 枠）を走りぬけて配列外を読む。**AddressSanitizer で実証済み**（全 34 枠を `tval=90` で埋め、`tval=10` を渡すと `misc3.c:1254` で `READ of size 1` の global-buffer-overflow）。`inven_check_num()` を先に呼ぶ約束に依存しており、破られたときの防御がない。**このリファクタリングの範囲を超える（ふるまいの変更）ので直さない** | #10 のステップ A 中。独立に ASan で再現を確認済み |
| **B9** | misc3.c:1251 vs 1164 | **`inven_carry` と `inven_check_num` の満杯判断が食いちがう。** `inven_check_num` は `inven_ctr < INVEN_WIELD`(=22) で満杯を判定して false を返すが、`inven_carry` は満杯でも挿入し `inven_ctr` を 23 にする。配列（34 枠）の範囲内なので即座には壊れないが、持ち物枠が装備欄（`INVEN_WIELD` 以降）を侵食する | 同上 |
| **B16** | str_insert.c:56-63（移動前は misc3.c:1672-1681） | **差しこみ結果が 80 バイトを超えると溢れる。** `insert_str` の作業領域は `char out_val[80]` 固定で、`strncpy` も 2 度の `strcat` も長さを見ない。呼びだし元の `desc.c` は `bigvtype`（160 バイト）を渡しているので、**呼びだし側の器のほうが作業領域より大きい**——長い名前で到達しうる。`insert_lnum` 側も `vtype str1, str2`（各 80 バイト）で同じ形。テストでは溢れさせていない（未定義動作は固定できない）。ふるまいの変更になるので直さない | #41 のステップ A 中に発見。移動前からの状態で、#41 では 1 行も変えていない。`tests/str_insert_test.c` の冒頭コメントに記録 |
| **B17** | spells.c:2124, magic.c:103, prayer.c:99 | **同じ「呪いを解く」効果で走る枠の範囲が 3 通りある。** `remove_curse()`（`spells.c:2124`）は 22..`INVEN_OUTER`(31) で **`INVEN_LIGHT`(32) と `INVEN_AUX`(33) を含まない**。`magic.c:103` は 22..33（装備の全枠）。`prayer.c:99` は 0..33 を `tval`（`TV_MIN_WEAR`..`TV_MAX_WEAR`）で絞る形なので、**袋の中の装備品も呪いが解ける**。3 つが同じ効果を別の範囲で実装している。意図的な差（効果の強さの違い）か取りこぼしかは判別できない。ふるまいは変えていない | #18-5 のステップ B（群 5・群 6）で装備の全域ループを窓口に置きかえる際に、範囲が揃っていないことに気づいた。**窓口ができたので 3 通りの差が字面に出るようになった** —— `spells.c` は `equipment_first_slot()` から始めて上端だけ `INVEN_OUTER`、`magic.c` は上端が `equipment_end_slot()`、`prayer.c` は跨ぎの窓口。3 箇所を並べれば範囲の違いが読める（`magic.c` の下端 22 は字面のまま残っている）。関連するコメントの誤り 2 件は `c25dce4` で直した |
| **B18** | save.c:914（2026-10-02 には save/save.c:1196） | **`((!noscore) & 0x04)` は常に 0 なので、この `else if` の枝は 1 度も走らない。** `!noscore` は 0 か 1 にしかならないので `& 0x04` は必ず 0。意図は `!(noscore & 0x04)`（「まだ重複と印を付けていないなら」）で、括弧が 1 つ内側にずれている。**結果として `duplicate_character()` は呼ばれず、`noscore \|= 0x4` もここからは起きない。** そのため "This character is already on the scoreboard" のメッセージは出ず、`noscore & 0x4` を見る `prt_winner()`（`misc3.c:458`）の "Duplicate" の表示も**到達不能**になっている。`noscore` の他の 2 ビットは生きている（0x1 は `save.c:755` の蘇生、0x2 は `misc3.c:1661` の wizard 入り）。直すと「重複キャラはスコアに入れない」判定が動きだすので**ふるまいが変わる**——スコアボードの結果に影響するため、判断はユーザーの領分 | #19（セーブ／スコア／進行メタ）の着手前計測で `noscore` の全 15 参照を読んでいて気づいた。ビット演算としての `noscore` は 3 ビットあるが、`prt_winner()` が読む 3 通りのうち 1 通りが立たない |
| **B19** | files.c:28-35 `init_scorefile` | **開いた得点ファイルを誰も読まず、誰も閉じない。** `init_scorefile()` は `MORIA_TOP` を `"rb+"` で開いて `highscore_fp`（global）に入れる。setuid の権限があるうちに開いておく、という意図のコメントが付いており、`main.c:45` が `setuid(getuid())`（`main.c:323`）より前に呼ぶ。ところが**得点ファイルの読み書きをする 2 つの関数は自分で `fopen` する** —— `display_scores()` は `"rb"`、`highscores()` は `"rb+"`。以前はどちらもこの global に上書きしていたので「開いたものは捨てられていた」が、#19-5-1 で 2 関数を局所変数にしたので、**この handle は開かれたまま一度も読まれず、`exit(0)` まで閉じられない**（他の参照は `game_state.c:80` の写しとりだけで、`game_state_init()` は呼ばれていない → B3）。つまり setuid の意図は最初から果たされていない —— 権限を落としたあとに開きなおすので、権限の要る環境では `highscores()` 側が失敗する。**直すには「開いた handle を 2 関数に渡す」か「開かずに存在確認だけする」かの設計判断が要る**（前者は setuid の意図を活かす方向、後者は現状のふるまいをそのまま認める方向）。ふるまいが変わる — 得点ファイルの権限の扱いに影響するため、判断はユーザーの領分 | #19-5-1（`highscore_fp` の局所化）で 2 関数から global を外したときに、残った書きこみが 1 箇所だけになって見えた。局所化の前後で実物の `scores.dat` を両方向に読みかえて確認済み（`display_scores()` の表示も `highscores()` の追記も一致） |
| **B20** | variable.c:107（消す前）・externs.h:50（消す前） | **`closing_flag` は誰も読み書きしていない。** 唯一の参照は `game_state.c:114` の写しとり 1 行で、`game_state_init()` は呼ばれていない（→ B3）。**死んだのはこのリポジトリの履歴の中** —— `e42f8dc`（2016-11-08, "Removes all remaining HOURS code"）が `dungeon.c` から HOURS の判定を消したとき、この旗を読み書きしていた唯一の場所が一緒に消えた。消えたコードは「モリアの門が閉まる」時間制限で、`closing_flag` はその**警告の回数**を数えていた（`closing_flag > 4` で強制セーブして退出。`eof_flag > 100` と同じ形）。後の上流（C++ 化した Umoria）は `db2effd`（2017-06-25, "Remove unused globals: wait_for_more and closing_flag"）で global ごと消しているが、**その commit はこのリポジトリの祖先ではない**。消すだけならふるまいは変わらないが、`game_state.h:92` の項目をどうするかの判断が要る（B3 と同じ話）。**ユーザーの判断で #18-11-5 では触らず、記録だけにした** | #18-11-5（`eof_flag`）の下読みで、同じ単位に挙がっていた 2 個目を数えたら参照が写しとり 1 件だけだった。`git log -S` で死んだ時点まで辿って確認した |
| **B21** | create.c:274（`get_history()`）・save.c:754（`rd_string`） | **生い立ちの 60 字ちょうどの行が行の外に `'\0'` を書く。ただし到達しない。** フィールドは `char history[4][60]`（`types.h:310`）で、`strncpy(py.misc.history[line_ctr], …, cur_len)` のあとに `py.misc.history[line_ctr][cur_len] = '\0'` を書く。`cur_len` は 60 になりうるので（`cur_len = end_pos - start_pos + 1` が 60 以下なら切らずに `flag = true`）、添字 60 は行の外。**`background[]` の連なりを全部たどって 330,984 通りの生い立ちを作り、`create.c:241`〜`:277` の折りかえしを写して走らせて数えた** —— **行数は最大 4 で 4 を超えるものは 0**（`line_ctr` は 4 に届かない）、**60 字ちょうどの行があるのは 8,057 通り**（3 行 7,657 ＋ 2 行 400）、**4 行めが 60 字ちょうど（`py.stats.max_stat[0]` に当たる）のは 0**。60 字の行は必ず最後の行で、書きこむ先はいつも直前の消去で `'\0'` にしたばかりの `history[line + 1][0]`。**同じ値を同じ場所に書いているので見えるふるまいは変わらない** —— **「直さないバグ」ではなく「字面は範囲外だが到達しない」**（もし 4 行めに来れば `create.c:489` の `get_all_stats()` が `:490` の `get_history()` より先に走るので、振ったばかりの STR が 0 になる）。**#18-12-26A でフィールドを `char [4][61]` に広げる** —— 窓口の約束（60 字 ＋ 終端）を 60 バイトの行では書けないから。**広げても見えるふるまいは変わらない**（8,057 通りでも書きこむ先が次の行の頭から自分の行の 61 バイトめに移るだけで、`wr_string()` は行を文字列として書くのでセーブファイルも変わらない）。**59 字に切る道は選ばない** —— それは 8,057 通りで最後の 1 字を落とす、本当に見えるふるまいの変更になる | #18-12-26 の下調べ。番地を渡す書き手 4 か所を「局所の器に受けてから置く」形に直す段取りを書いていて、器の幅が 60 で足りるかを数えたら出た。**到達するかどうかは下調べの直しで測った** |
| **B22** | store2.c:921-925 `store_sell` | **B9 の状態（持ち物 23 個）で、`char mask[INVEN_WIELD]`（22 枠）の外に 1 つ書く。** `for (counter = 0; counter < inventory_count(); counter++) mask[counter] = flag;` は数を `INVEN_WIELD` で抑えていない。B9 のとおり `inven_carry` は満杯でも挿入して数を 23 にしうるので、そのあと店で売ろうとするとスタックの枠の外へ書く。B9 が起きなければ到達しない | #55 の下調べ（2026-09-30）。読みで見つけ、`INVEN_WIELD` = 22（`constant.h:199`）と B9 の記述を突きあわせた。起こしてはいない |
| **B23** | item/spellbook.c:349 `gain_spells`・item/spellbook.c:199 `calc_spells` | **戦士では `&magic_spell[player_class() - 1][0]` が `magic_spell[-1]` の番地になる（規格上は未定義）。ただし読まない。** 2 本とも魔法系か祈り系かを訊く前に番地を作る。戦士は `player_spells_to_learn()` が必ず 0 なので `gain_spells` は `if (!new_spells)` で帰り、"You can't learn any new prayers!" と祈りのほうを断られる。B21 と同じ「届かないから直さない」 | worklog #18-12-28（2026-09-28）で「バグ候補 B22」と名づけ、`gain_spells` のコメントにだけ書いてあった。bugs.md の B22（#55 の `store_sell`）と番号がぶつかるので、#42 で B23 に付けなおした（ユーザーの判断）。戦士で `G` を押して確かめてある |
| **B24** | combat/monster_melee.c:597 `make_attack` | **光る手で殴られたモンスターが混乱に抵抗すると、名前と述語のあいだの空白が抜ける**（"The orcis unaffected."）。`CONCAT(cdesc, verb)` の `verb` が `"is unaffected."` で、隣の `" appears confused."` と combat/player_melee.c:98 の `" is unaffected."` には頭の空白がある。`9a0185a2`（2021、`monster_name` が末尾の空白を返さなくなった変更）で `"%sis unaffected."` を書きかえたときに 1 か所だけ空白を足しそこねたと読める | 2026-10-01 の文書の整理で #51 の場所を調べたとき |
| B25 | dungeon.c:1859 `jamdoor` | **楔で扉を止めようとしたとき、見えていないモンスターの名前が漏れる。** `"The %s is in your way!"` に `monster_get_creature(...)->name` を生のまま渡している。同じ文を出すほかの 4 か所（dungeon/terrain_commands.c:50, 166, 265、dungeon/traps.c:48）は `monster_name_or_something()` で "Something" と出す。`24cd0925`（2016）の初版から同じ形で、上流由来 | 2026-10-02 の再検出 |
| B26 | store/store_stock.c:82-84 `store_carry`、:29-50 `store_check_num` | **店の重ねた品の数が `uint8_t` を桁あふれする疑い（未確認）。** `store_check_num` は店に空きがある（`store_ctr < STORE_INVEN_MAX`）と個数を見ずに true を返し、`store_carry` は `number += item_num` の前に 256 未満かを見ない。持ち物の側の `items_can_stack`（item/inven_ops.c）は上限を見る。実際に 255 を超える積みかたが起きうるかは確かめていない | 2026-10-02 の再検出（#10 の店の版として調べた） |
| B7 | potions.c:323, eat.c:195, scrolls.c:470 | **`py.misc.lev == 0` でゼロ除算。** 経験値加算 `(i_ptr->level + (m_ptr->lev >> 1)) / m_ptr->lev` に防御がない。通常プレイでは到達しない（`create.c:86` が 1 で初期化、`lose_exp` は下限 1 を保つ）が、`save.c:655` の `rd_short(&m_ptr->lev)` はセーブファイルの値をそのまま読み、範囲検証をしていない。壊れた／改変されたセーブファイルでクラッシュする | #8 のステップ A 中。テストではゼロを渡していない（クラッシュするため）。`tests/item_ident_test.c` に TODO を記録 |

### バグ候補の 3 分類（2026-08-27 に整理）

作業を進めるうち、「バグ候補」に性質の違うものが混ざることがわかった。扱いが変わるので分けて記録する。

| 種類 | 例 | このリファクタリングでの扱い |
|---|---|---|
| **コードが誤っている** | B8（配列外読み）、B9（判断の食いちがい）、B7（ゼロ除算） | **直さない。** ふるまいの変更になる。報告して別作業にする |
| **コードは正しく、コメントが誤っている** | B10（`xfos` の range）、B11（`xstl` の range） | **直せる。** コメントの修正はふるまいを変えない。ただし 1 コミット 1 変更を守り、コード変更とは分ける |
| **設計上の疑問（正誤ではない）** | `xdev` が `disarm` ではなく `save` を基にしている | **直さない。** 意図が不明なものは記録して報告する。判断はユーザーの領分 |

2 番目は見落としやすい。**コメントを直すのはリファクタリングの範囲内**（外から見たふるまいが変わらない）なので、見つけたら直してよい。ただしコード変更と同じコミットに混ぜないこと。

### #12 の追加調査（2026-08-27）：a 系で片方の引数が捨てられるのは設計

ステップ A の担当が「`comment2a` の 3 件すべてが `%A1` を持たず、`comment3a` の 3 件すべてが
`%A2` を持たないので、a 系では片方の引数がつねに捨てられる」と報告した。事実を確認したうえで、
**これはバグではなく意図的な設計**と判断した。

```
comment2a（購入・最終）: "%A2 is my final offer; take it or leave it."
comment3a（売却・最終）: "I'll pay no more than %A1; take it or leave it."
```

`a` 系は「最終提示」のメッセージで、**店が示す額だけを表示すればよい**。
購入時は店の要求額（`%A2`）、売却時は店の提示額（`%A1`）。
呼びだし側の引数順の逆転（購入 `(offer, asking)` / 売却 `(asking, offer)`）と整合している。

ただし担当の指摘のうち次は妥当で、記録に値する。

- **プレースホルダの綴りを間違えても気づけない。** `insert_lnum` は置換対象がないと黙って
  数値を捨てる（`misc3.c:1907` の `strchr` が 0 を返す経路）。`%A1` を `%1A` と書いても
  エラーにならず、単に数値が表示されないだけ
- **要素数がリテラルで 2 箇所に書かれている。** 配列宣言（`comment2b[16]`）と `randint` の
  引数（`randint(16)`）。片方だけ増減させると壊れる。**現状は 4 配列すべて一致を確認済み**
  （宣言と実要素数も照合した）が、統合時にここを崩さないこと
- **`vtype`（`char[80]`）に境界検査がない。** 両プレースホルダを持つ最長テンプレートは
  `comment2b[9]` の 48 文字で、`int32_t` 最悪 11 文字 ×2 でも 64 文字に収まる。
  ただしテンプレートを 1 つ長くしただけで溢れる余地がある

**検証して却下したバグ候補**（記録しておくと再調査の手間が省ける）

| 場所 | 報告内容 | 却下の理由 |
|---|---|---|
| potions.c:322 ほかのコメント | 「`// round half-way case up` は `lev` が偶数のときだけ成りたつ。奇数では `lev >> 1` の切り捨てで切りあげにならない」 | **成立しない。** 奇数の `lev` では真の商が x.5 になりえない（分母が奇数だから）。`lev` が 3, 5 の全ケースで四捨五入と実際の値が一致することを確認済み。偶数でも .5 のケース（`lev=2, il=1` → 1、`lev=4, il=6` → 2）で切りあげが効いている。コメントは正確 |

