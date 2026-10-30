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
| 2026-10-30 | テストの組みたての警告 | 11 → 0。テスト 2 本に dungeon_state.h（`2c9a06c`）、`makefile.test` の旗を本体に合わせて効いていない LIB_EXCLUDE を外す（`daa2442`）、品物の名前の sprintf 4 か所を snprintf に（`adae044`）、haggle_comment_test の代役を本体に合わせる（`1dad4c2`）、check.sh がテストの組みたての警告も数える（`5bfe971`） | 1850（89 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | golden の穴 | 台本 `rare_paths`（`12203a1`。呪われた鎧・鎧の巻物・罠を踏む・外す・箱）。被覆 50.3% → 51.4% | 1850（89 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #19 | 総当たり `bolts_all`（`a29f552`。10,920 回、8 つが 100%）、飛ぶ呪文 8 つの骨格を bolt_to_next_creature() に（`6e7b95e`。ハッシュが 9 つすべてで一致、spells.c 1681 → 1624 行） | 1850（89 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | 計画の組みなおし | ユーザーの依頼で、台帳の P1〜P3 と棚上げの区分をやめ、主題ごとの計画（A〜F）に組みなおした（`4d32c2a`。前の全文は `done/plan-2026-10-30.md`。本文の「13069 字」は誤りで、REFACTORING_PLAN.md 12929 → 13068 字）。B34・所見 79・80（`a17d531`）、HANDOVER を合わせる（`715d9c4`。前の全文は `done/handover-2026-10-30-replan.md`。20969 → 21263 字） | 1850（89 本） | 文書だけ |
| 2026-10-30 | マージ | `fix/test-warnings`・`test/golden-gaps`・`refactor/19-bolts`・`docs/replan-2026-10-30` を順に `--no-ff` で（`e4d208d`・`c3ee80f`・`df3fb03`・`4b4c79e`）。13 コミットを 1 つずつ空から組んで確かめた（警告を減らしている途中の 2 コミットは、今の check.sh が数えるテストの組みたての警告が 5・4 件残る形。ほかは ALL OK） | 1850（89 本） | `scripts/check.sh` ALL OK（マージ後の develop） |
| 2026-10-30 | #27（一部） | read_scroll の武器の巻物 4 つを static 関数に（`0e106ab`。402 → 328 行）。鎧の 3 つ（case 3・35・36）は i_ptr を塊の奥で条件つきで指しなおすので残した。鑑定の case が i_ptr を当てにする形を B35 に | 1850（89 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #26 | 総当たり `magic_all`（`95ba3f6`。18 万 4,800 回、magic_treasure の 97.9%。本文の「AC のプラスの上限 30 → 31（26 品目）」は誤りで 25 品目。違った 26 行は合計の行を含む）、magic_treasure を品物の種類ごとの static 関数 14 本に（`8e39139`。831 → 105 行、ハッシュが 420 品目すべてで一致） | 1850（89 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #25 | 総当たり `melee_all`（`96551d9`。6,696 回、make_attack の 91.0%）、make_attack の 3 つの switch を `attack_hits`・`describe_attack`・`attack_effect` に（`59ae8ef`。600 → 100 行、ハッシュが 279 種すべてで一致）。光る手の塊（#51）は残した。`attack_effect` は 286 行の switch 1 本 | 1850（89 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #21 | 総当たり `turns_all`（`b754f47`。モンスターの手番 6,696 回、mon_cast_spell の 92.3%・mon_move の 48.7%）、呪文の番号に名前 `enum monster_spell`（`bae0995`。`cmp_all.sh` で機械語が一致）、効果の switch を `cast_monster_spell` に（`7c0ce9d`。209 → 73 行、ハッシュが一致） | 1850（89 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #97 | `shared_stubs_reset` が randint の代役の答えを 1 に戻す（`0696cd9`。haggle_comment_test が前の件の 16・15 で 3 要素の表の外を読んでいた）、テストを `-O2` で組む（`892c804`）。`-O2` に `-fsanitize=address,undefined` を付けて全テストを走らせても何も報告されない | 1850（89 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #50（数えなおし） | `calc_bonuses()` の呼び手 8 か所を 1 つずつ消して単体テストと golden を走らせた。5 か所で赤：stat_ops.c:59（`turns_all`）、inven_ops.c:147・154（単体 2 件）、inven_menu.c:708（台本 `commands`・`free_turn`・`items`・`rare_paths`）、player_damage.c:91（`melee_all`）。素通りの 3 か所のうち monster_melee.c:505（Disenchant のあと）は、`melee_all` のハッシュに戦いの数と AC を足して捕まえた（`1bc34d6`。61 種のハッシュが変わる）。残るのは scrolls.c:112（To-Dam の巻物で呪いが解けたあと）と spells.c:366（装備を鑑定したあと） | 1850（89 本） | `scripts/check.sh` ALL OK |
| 2026-10-30 | #98（調べ） | 行動の段の 11 単位の呼び出しを数えた。8 単位（creature・monster_melee・projectiles・spells・traps・terrain_commands・player_move・run_path）が 1 つの強連結成分。creature に戻る辺は projectiles → `update_mon`、spells → `creatures`・`multiply_monster`・`update_mon`、player_move → `creatures`（どれも `creatures(false)`、つまり見え方の更新か、増殖）。ほかに run_path ⇄ player_move、traps ⇄ player_move。直すのは段の置き場の判断なので、台帳に提案だけ書いた | 1850（89 本） | 文書だけ |
