# 作業ログ

1 作業 1 行。済んだ列の行は `done/` にある：

- 着手から #18 のマージまで → [done/worklog-to-18.md](done/worklog-to-18.md)
- 実装の再配置（#52〜#57）と #37・#38・#44 → [done/worklog-layout.md](done/worklog-layout.md)
- 2026-10-02 の再検出と P1（#58〜#61） → [done/worklog-p1-2026-10.md](done/worklog-p1-2026-10.md)
- P2（#62〜#70・#45） → [done/worklog-p2-2026-10.md](done/worklog-p2-2026-10.md)
- P3（#49・#48・#39・#24）・2026-10-10 の列・モジュール化（#80〜#88）・#89〜#93 → [done/worklog-2026-10-05-15.md](done/worklog-2026-10-05-15.md)
- 文書の整理（2026-10-23）・golden と iterator（#94〜#96）・#22・#71・#74・#51 の保護・キーマップ → [done/worklog-2026-10-23-30.md](done/worklog-2026-10-23-30.md)

| 日時 | 項目# | 何をしたか | テスト件数 | 結果 |
|---|---|---|---|---|
| 2026-10-30 | 文書の整理 | **#74・#71・キーマップの 3 つの列を閉じたので第 9 節の手順で詰めた**（`45a1321 47945be`）。作業ログの 28 行を `done/worklog-2026-10-23-30.md` へ（日付を指していた台帳 4 か所と HANDOVER 4 か所を向けなおした）、HANDOVER.md の全文を `done/handover-2026-10-30.md` に写してから第 0・7 節を詰めた。列の文書（手順 2）と台帳の書きなおし（手順 4）は要らなかった。入口は HANDOVER.md 20685 → 20844 字・REFACTORING_PLAN.md 12661 → 12929 字（`len()`。指し先が長くなったぶん増えた） | 1850（89 本） | 文書だけ |
| 2026-10-30 | #72（一部） | 総当たり `recall_all`（`a958a0a`。158 万回、roff_recall の 96.9%）、roff_recall を 11 本に（`cece7bb`。ハッシュが 279 種すべてで一致）、`inline_back.py` を引数つきの呼び出しに（`dd23b75`）、eat（`2773cf5`）・quaff（`c0fcf77`）の効果の switch、inven_command の着る後半と見出し（`1171f12`）、get_char の rd_player（`08e8626`。本文の「462 行」は誤りで、463 行） | 1850（89 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #72（一部）マージ | `refactor/72-recall`（8 コミット。1 つずつ空から組んで、どれも `scripts/check.sh` ALL OK）を `--no-ff` で（`4e5149d`） | 1850（89 本） | `scripts/check.sh` ALL OK（マージ後の develop） |
