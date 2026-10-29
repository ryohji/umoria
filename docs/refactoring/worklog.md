# 作業ログ

1 作業 1 行。済んだ列の行は `done/` にある：

- 着手から #18 のマージまで → [done/worklog-to-18.md](done/worklog-to-18.md)
- 実装の再配置（#52〜#57）と #37・#38・#44 → [done/worklog-layout.md](done/worklog-layout.md)
- 2026-10-02 の再検出と P1（#58〜#61） → [done/worklog-p1-2026-10.md](done/worklog-p1-2026-10.md)
- P2（#62〜#70・#45） → [done/worklog-p2-2026-10.md](done/worklog-p2-2026-10.md)
- P3（#49・#48・#39・#24）・2026-10-10 の列・モジュール化（#80〜#88）・#89〜#93 → [done/worklog-2026-10-05-15.md](done/worklog-2026-10-05-15.md)

| 日時 | 項目# | 何をしたか | テスト件数 | 結果 |
|---|---|---|---|---|
| 2026-10-23 | 文書の整理 | **#80〜#93 の 2 つの列を閉じたので第 9 節の手順で詰めた**（`ca74f958 1462d2b1 3b5b48e9 4c0fb5fe`）。作業ログの 30 行を `done/worklog-2026-10-05-15.md` へ、#89〜#93 の経過を `done/section6-2026-10-15.md` に起こし、HANDOVER.md と REFACTORING_PLAN.md の全文を `done/handover-2026-10-23.md`・`done/plan-2026-10-23.md` に写してから直した。入口は HANDOVER.md 16617 → 17717 字・REFACTORING_PLAN.md 11116 → 11292 字（`len()`）。第 6 節の「どのテストも引かない .o」は 110 本のうち 60 本と数えなおした | 1826（87 本） | 文書だけ |
| 2026-10-24 | 文書の整理 マージ | `docs/tidy-2026-10-23` を `--no-ff` で（`64c0fb4`） | 1826（87 本） | 文書だけ |
| 2026-10-24 | #95 | 正順の iterator とテスト 5 件（`05e36f0`。`/tmp` の確かめの道具が消えていて、コミットのあとで第 2 節の 1〜4 を走らせた。本文の数と一致）、savefile_layout_test に表の両端の記憶（`8cd4c15`）、セーブの走査を iterator に（`2bce4df`。壊し 3 通りすべて赤。1 つは abort で実行形式が 86 本になる形） | 1831（87 本） | 警告 0・GREEN・globals OK 40・層 919 本・headers OK |
| 2026-10-24 | #95（続き） | advance・distance とテスト 6 件（`4de8fb3`。壊し 2 通りとも赤）、summon_undead_test（`6d14d80`。壊し 3 通りのうち 2 赤、m >= l は添字 277 の Evil Iggy がアンデッドでないので原理的に区別できない）、summon_undead を iterator に（`7c29413`。壊し 3 通りのうち 2 赤、> 1 は添字 278 の Balrog で同じ理由） | 1839（88 本） | 警告 0・GREEN・globals OK 40・層 919 本・headers OK |
| 2026-10-25 | #95 マージ | `refactor/95-creature-iterator` を `--no-ff` で（`b9b90c8`） | 1839（88 本） | 警告 0・GREEN・globals OK 40・層 919 本・headers OK（マージ後の develop で測った） |
| 2026-10-25 | #94 | golden（`40eff78`〜`421628f`。台本 8 本、本体の行の 44.5%）、`--shadows`（`6229ee9`）、`scripts/check.sh`（`07f9519`）。#84 の前の 89ca18a と入力の終わりの写しが一致。壊しは台本ごとに 1〜2 通りずつ赤（素通り 2 つは画面に出ない所） | 1839（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-25 | 確かめ | `develop..test/94-headless` の 13 コミットを 1 つずつ空から組んだ。どれも警告 0・1839 件 88 本 GREEN・層 OK・headers OK。golden は `d5d2947` 以降 GREEN で、その前の 3 つ（`40eff78`・`eae1914`・`585c84e`）は新しく取りだすと `run.sh` に実行の印が無く Permission denied で走らない（`d5d2947` で直した） | 1839（88 本） | 第 2 節の 1〜3 と golden |
| 2026-10-25 | #94 マージ | `test/94-headless` を `--no-ff` で（`7fa04a0`） | 1839（88 本） | `scripts/check.sh` ALL OK（マージ後の develop） |
| 2026-10-25 | #96 | iterator を添字の構造体に（`13483c9`）、handle をまとめる（`10f6d90`）、要素から戻す関数を消す（`2a024e6`）、添字は monsters.c だけ（`416b092`）。各段の全関数の比較で、違いは手を入れた関数と呼び先の名前だけ | 1844（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-25 | #96 マージ | `refactor/96-creature-iterator` を `--no-ff` で（`e87f89e`） | 1844（88 本） | `scripts/check.sh` ALL OK（マージ後の develop） |
| 2026-10-25 | #22 | `objdes` の switch を `base_name` に（`c6e6758`）、後半を 6 本に（`a6036e1`）、総当たり `objdes_all` を golden に（`2e32378`）。4,100 万回の出力のハッシュが develop と一致、壊し 5 通りすべて違いが出る | 1844（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-25 | #71（一部） | 効果 17 種（`7996394`）と毒（`ffc697e`）を `tick_<効果>()` に。本体を字のまま戻すと dungeon() が元と一致 | 1844（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-29 | 確かめ | `develop..refactor/71-timed-effects` の 6 コミット（#22 の 3 つ・#71 の 2 つ・記録）を一時的な worktree で 1 つずつ空から組んだ。どれも `scripts/check.sh` ALL OK | 1844（88 本） | 第 2 節の 1〜4 と golden |
| 2026-10-29 | #22 マージ | `refactor/22-objdes` を `--no-ff` で（`562ca62`） | 1844（88 本） | `scripts/check.sh` ALL OK（2 つのマージの後の develop で測った） |
| 2026-10-29 | #71（一部）マージ | `refactor/71-timed-effects` を `--no-ff` で（`4e941ed`）。#22 の 3 コミットを含むので、#22 を先にマージした | 1844（88 本） | `scripts/check.sh` ALL OK（マージ後の develop） |
| 2026-10-29 | #74（保護） | 台本で選べる turn の記録 `# record: turns`（`b3b9856`。今の 8 本の記録は変わらない）、台本 `free_turn`（`9bc64bd`。2 つの run。壊し 9 通りのうち 7 赤、素通り 2 つは原理的に見分けられない）。B28 の記述が誤りと分かった（魔法棒の方向の取りけしは turn を使わない） | 1844（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-29 | #51（保護） | 台本 `glowing_hands`（`f071a52`。3 つの run、眠らない相手を含む）。壊し 9 通りのうち 5 赤。B24 が実際の画面に出る。golden の被覆は本体の行の 48.2%（7627 / 15810。gcov） | 1844（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-29 | 確かめ・マージ | `docs/51-74-ledger`（#51・#74 の台帳の行、B28 の訂正、B30。5 コミット）と `test/74-free-turn`（4 コミット）を一時的な worktree で 1 つずつ空から組み、どれも `scripts/check.sh` ALL OK。`--no-ff` で（`322e3d1`・`b838568`） | 1844（88 本） | `scripts/check.sh` ALL OK（マージ後の develop） |
| 2026-10-29 | #74（本体 1） | 主ループと do_command（`1b90d7c`）、eat・quaff（`78e61f0`）、魔法棒と杖（`534c51e`）、扉・トンネル（`93b3f02`）を戻り値に。壊しで素通りした所に台本の場面を先に足した（`8ae2b4e` 一覧の再開、`b2f3b0f` 扉と掘る）。走りの続きの枝だけ届かなかった | 1844（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-29 | #74（本体 2） | 取りけせる効果 3 つの enum `effect_outcome`（`c83e732`。ユーザーの判断）、read_scroll（`5c3a981`。場面は `8a0880e`）、cast（`f1f6ba9`）、pray（`1cd4f42`）、throw・rest・gain_spells（`acce926`。gain_spells_test を戻り値に）。B30 を見つけた | 1844（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #74（本体 3） | 店と移動（`4970b42`。場面は `0c78dc0`。B31 を見つけた）、disarm・bash（`a8f62b5`）、走り（`2c62353`）、inven_command（`f7b2576`。場面は `7930cdd`）、dungeon.c の 4 つ（`58b862e`。場面は `50b16eb`） | 1844（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #74（本体 4） | do_command を局所の took_turn に（`4abe07a`）、旗を消す（`d8c23ba`。global 40 → 39、層の依存 919 → 905 本）、旗のためだけの include を 10 ファイルから外す（`21b8588`。`cmp_all.sh` で共通の 1096 関数の違い 0） | 1844（88 本） | `scripts/check.sh` ALL OK・globals OK 39 |
| 2026-10-30 | #74 マージ | `refactor/74-free-turn`（26 コミット。1 つずつ空から組んで、どれも `scripts/check.sh` ALL OK）を `--no-ff` で（`f3b3281`） | 1844（88 本） | `scripts/check.sh` ALL OK・globals OK 39（マージ後の develop） |
| 2026-10-30 | #71 | 道具 `scripts/inline_back.py`（`2cfa72c`）。dungeon() の 5 つの塊を関数に（`60c7780`。本文の「454 → 341 行」は誤りで、454 → 288 行）、command の宣言を関数の始めへ（`86dbeba`。塊に入りなおした変数の前の値に頼っていた）、コマンドの読みとりを read_command() に（`6f60642`。本文の「288 → 210 行」は誤りで、292 → 207 行）。場面は `a54d276`（^ と回数） | 1844（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #71（続き） | do_command の長い 5 つの case を関数に（`7c35733`。本文の「5 通りすべて赤」は誤りで、5 通りのうち 4 通りが赤）、逆引き command_key_direction() とテスト 2 件（`c2aae4d`）、24 の case を畳む（`a7d61df`。本文の「5 通りすべて赤」は誤りで、5 通りのうち 4 通りが赤）。場面は `a6bfa33`（地図の区画・wizard の切りかえ）・`d44d6d0`（- で拾わずに動く）・`389fd9a`（rogue-like の組の run）。dungeon() 207 行・do_command 227 行。golden の被覆は 50.3%（7960 / 15833） | 1846（88 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #71 マージ | `refactor/71-dungeon`（13 コミット。1 つずつ空から組んで、どれも `scripts/check.sh` ALL OK）を `--no-ff` で（`cff2cd9`） | 1846（88 本） | `scripts/check.sh` ALL OK（マージ後の develop） |
| 2026-10-30 | キーマップ | 原文の鍵の組の表 `ui/keymap.c` と `keymap_test`（`360dc0c`。本文の「同じキー 46」は誤りで、表は写すキー 24・同じキー 35・wizard 14）、`original_commands` が表を引く（`eab4bf4`）、コマンドの名前と rogue-like の組の表（`c58180c`。本文の「遊びの 50」は誤りで、名前は遊び 48・wizard 14・不正と何もしない 2）、場面（`fb29e3c` ^P の 2 回目・wizard の明かりと地図）、`do_command`・`wizard_command` を名前で分ける（`f782916`）、< > のコメント（`3b16b37`）。original_commands 15 行・do_command 224 行。B32・B33 を見つけた | 1850（89 本） | `scripts/check.sh` ALL OK |
