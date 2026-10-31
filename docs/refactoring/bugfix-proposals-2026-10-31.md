# バグ候補の直しかたの案（2026-10-31）

ユーザーの判断（2026-10-31）：バグ候補は直す。`bugs.md` の開いている候補ごとに、いまのコードで
確かめなおし、直しかたの案を出す。どれも、直すと決めてから 1 つずつ別のコミットで直す。
golden の記録が変わるものは、直すコミットのあとに記録を作りなおすコミットを置く。

区分：

- **A. 直す。見えるふるまいが変わる**（ユーザーに決めてもらう）
- **B. 直す。いまの遊びかたでは見えるふるまいは変わらない**（防御・字面の誤りの直し）
- **C. 閉じる**（もう直っている、届かない、意図どおり）

## A. 見えるふるまいが変わる

| # | いまの形（確かめた場所） | 案 | 変わること | 記録 |
|---|---|---|---|---|
| B24 | world/combat/monster_melee.c の光る手：抵抗したとき `"is unaffected."` の頭に空白が無い | 空白を足す（`" is unaffected."`）。#51 で 2 か所を畳むならそこで | 「The orcis unaffected.」→「The orc is unaffected.」 | `glowing_hands` が変わる見込み |
| B34 | world/item/spells.c `teleport_monster`・`poly_monster` は最初のモンスターで止まらない | ほかの 6 つと同じく、最初のモンスターで止める（teleport は 1 体を飛ばして終わり、poly は抵抗されたら終わり） | 線の上の 2 体目以降に効かない | `bolts_all` の 2 つ |
| B30 | command/scrolls.c case 29：足もとに物があると Create Food は何もせず、turn は使わないが巻物は消える | `EFFECT_CANCELLED` のとき `used_up = false` も（呪文・祈りの扱いと同じ：何も起きなければ何も使わない） | 巻物が残る | 台本は通していない |
| B31 | command/store_ui.c `store_sell`：売る問いを取りけすと、店に入った 1 歩が turn を使わない | 取りけしで `took_turn` を下ろす行を消す（店に入る 1 歩は、店の中で何をしても turn を使う。買う側の取りけしはもとからそう） | 歩いて入った 1 歩が turn を使う | `free_turn` が変わる |
| B32 | app/dungeon.c `valid_countcommand`：^J は回数を受け、^M（Enter）は受けない | ^M を ^J と同じ側に足す（rogue-like の組では、どちらも下へ掘る） | Enter に回数を付けられる | 変わらない見込み |
| B33 | ui/keymap.c：原文の鍵の組で Enter・^J が `+`（wizard では経験値を倍に、ふだんは「Type '?' for help.」） | Enter・^J を空白と同じ「何もしない」に | wizard モードで Enter を押しても経験値が倍にならない。ふだんの「Type '?' for help.」も出なくなる | `free_turn` が変わる |
| B25 | app/dungeon.c `jamdoor`：楔で扉を止めるとき、見えないモンスターでも名前を出す | ほかの 4 か所と同じく `monster_name_or_something()` に（見えなければ「Something」） | 見えないモンスターの名前が漏れない | 変わらない見込み |
| B17 | 呪いを解く効果が 3 通りの範囲：巻物（`remove_curse()`）は装備のうち明かりと予備を除く、呪文（magic.c case 14）は装備のすべて、祈り（prayer.c case 15）は持ち物と装備のうち身につける品のすべて | 範囲を「装備のすべて」に揃え、呪文と祈りも `remove_curse()` を呼ぶ | 巻物：明かりと予備の枠も解ける。祈り：袋の中の品は解けなくなる（呪いは身につけたときだけ効くので、身につけた品だけ解くのが筋）。案 2：祈りだけは袋の中も解く強い版として残す | 変わらない見込み |
| B5 | model/item/device.c `device_use_chance`：混乱すると `chance / 2`。`chance` が負のときは 0 に近づくので、成功しやすくなる | 半分にするのは `chance > 0` のときだけ | 負の技量で混乱しても得をしない（損もしない） | `device_chance_test` の 1 件（TODO 付き）を書きかえる |
| B15 | platform/signals.c：セーブなどで `nosignals()` が SIGTSTP を無視にし、`signals()` が戻さない。セーブのあと ^Z が効かない | `signals_set_suspend_handler()` で受けた手を static に覚え、`signals()` で付けなおす | セーブのあとも ^Z で止められる | 変わらない（golden は端末を持たない） |
| B26 | world/store/store_stock.c `store_carry`：重ねるとき 256 未満かを見ない（`store_check_num` は店が満杯のときだけ見る） | 重ねる条件に `number + item_num < 256` を足す（超えるなら別の行に入れる） | 店の品の数が桁あふれしない | 変わらない見込み |

## B. いまの遊びかたでは見えるふるまいは変わらない

| # | いまの形 | 案 | 備考 |
|---|---|---|---|
| B35 | command/scrolls.c case 4：鑑定の巻物は、i_ptr が読んだ巻物を指し、旗がちょうど `0x00000008` であることを当てにして探す | switch の前に読んだ巻物の `tval`・`subval`・`flags` を写しておき、鑑定のあとはそれと同じ品を `item_val` から下へ探す（0 を下まわらない）。i_ptr は case の局所に。そのうえで #27 の鎧の 3 つを出す（ユーザーの判断で (b)） | bit を 2 つ持つ巻物（wizard の `@`）でだけ変わる |
| B7 | world/item/item_learn.c：経験を `/ player_level()` で割る。セーブファイルのレベルを確かめないので、0 なら 0 で割る | セーブを読むとき、レベルが 1〜`MAX_PLAYER_LEVEL` の外なら「壊れたファイル」として読みこみを断る | #72 の `get_char` を段ごとの関数にしてから（`rd_player` が成否を返せるようになる） |
| B8・B9・B22 | world/item/inven_ops.c `inven_carry`：ループに終わりの条件が無く、満杯でも入れる。B22 はその先で店の `mask[22]` の外に書く | 積めず、持ち物が `INVEN_WIELD` 個で満杯なら、入れずに -1 を返す。呼び手はどれも先に `inven_check_num` で確かめているので届かない。`store_sell` の mask のループも `INVEN_WIELD` で抑える | 約束を破ったときに配列の外を読まなくなる |
| B16・B27 | core/str_insert.c `insert_str`：作業領域が 80 字固定で長さを見ない。型が文字列より長いと範囲外のポインターを作る | 呼び手の器の大きさを引数で受け（`insert_str(char *s, size_t size, …)`）、収まらなければ切る。型のほうが長ければ何もせずに帰る | 呼び手は 5 か所（desc.c 3・store_haggle.c 2）。いまの呼び手は収まる |
| B2 | world/combat/projectiles.c `get_flags`：知らない種類で「ERROR in get_flags()」を画面に出し、出力を入れない | 出力を「何にも効かない」（0・0・`set_null`）にし、文字列は出さない | 呼び手はどれも知っている種類を渡す |
| B18 | app/save.c：`((!score_disqualifications()) & SCORE_DISQUALIFY_DUPLICATE)` は必ず 0 | 括弧を `!(score_disqualifications() & SCORE_DISQUALIFY_DUPLICATE)` に。`duplicate_character()` はつねに false を返すので、直してもメッセージは出ない | 案 2：枝ごと `duplicate_character()` を消す |
| B19 | command/files.c `init_scorefile`：開いた得点ファイルを誰も読まず閉じない（読み書きする 2 つの関数は自分で開く） | 開けるか確かめたら閉じる（開けなければいまと同じく終わる）。global `highscore_fp` を消す（39 → 38） | setuid の意図はもとから果たされていない（権限を落としてから開きなおす）。setuid で動かす気がなければこれでよい |
| B23 | world/item/spellbook.c `calc_spells`・`gain_spells`：戦士では `magic_spell[-1]` の番地を作る（読まない） | 番地を作るのを、魔法か祈りかを確かめたあとへ | 規格の上の未定義をなくすだけ |
| B29 | core/rnd.c `randnor(mean, 0)`：表の外の枝で 0 で割る | `stand <= 0` なら `mean` を返す | 呼び手はどれも 0 を渡さない |

## C. 閉じる

| # | 理由 |
|---|---|
| B4・B13 | #14 で `moves_this_turn` に直り、1 か 0 を明示して返す。誤ったコメントも無い |
| B10・B11 | 誤ったコメントは、もう無い |
| B14 | ゲームの turn は `dungeon()` のループの頭で進めてからモンスターを動かすので、`creatures(true)` が負の turn を見ることは無い |
| B21 | 生い立ちの行は 61 字（`PLAYER_HISTORY_LINE_SIZE`）に広げてあり、範囲の外に書かない |
| B12 | `makefile.win` は #46 で `sources.mk` を読むようになった（Windows で組めるかは確かめていない） |
| B3・B20 | #35 で app/game_state.c を消すと無くなる。`closing_flag` は写しとりのほかに参照が無いので、一緒に消す（global 39 → 38。B19 も直せば 37） |
| B6 | `chance <= 0` を 1 にするのは `randint(0)` を避けるためで、「わずかな機会」はその前の救済の抽選（`USE_DEVICE` に引きあげる）が与えている。意図どおりと読める |
