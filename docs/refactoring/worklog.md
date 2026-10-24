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
