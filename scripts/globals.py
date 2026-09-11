#!/usr/bin/env python3
"""externs.h のグローバル変数を数えあげる。

臭い #18（データの散在）の全体像をつかむための計測。手で数えあげると
見落とすし、直すたびに数えなおすことになるので機械化しておく。
scripts/warnings.sh と同じ位置づけ（現状を数字で見るための道具）。

  $ python3 scripts/globals.py            # 区分ごとの集計
  $ python3 scripts/globals.py --full     # 112 個すべての一覧
  $ python3 scripts/globals.py --check    # 分類の網羅性だけ確認（CI 向け）

数えるもの
  参照数     src/*.c に現れる回数（定義そのものを含む）
  参照ファイル数
  書きこみ数 代入・++・-- のほか、strcpy 系の第 1 引数に渡る形
  書きこみファイル数
  別名       &x や、配列名をそのまま関数に渡す形の回数

書きこみ数は「下まわる見積り」であって正確な数ではない。数えられないのは
局所ポインタ経由の書きこみで、たとえば store は

    store_type *s_ptr = &store[store_num];   // ← ここは別名として数える
    s_ptr->store_ctr++;                      // ← この書きこみは数えられない

の形をとるため、代入の字面を追うだけでは見えない。だから「別名」の列を
併記している。**別名がとられている変数は、書きこみ数を信用してはいけない**
という合図として読む。カプセル化のときに厄介なのもここで、参照を機械的に
置きかえるだけでは足りない箇所を先に知っておきたい。

「書きこみ」からは 2 種類を除いてある。
  - 定義の置き場（variable.c, player.c, tables.c, treasure.c, monsters.c）
    にある初期値つき定義。これは散らばった書きこみではない
  - game_state.c の `state->x = x`。状態の写しとりで、x 自身は書かない
    （どちらも数えると「どこから状態が変えられるか」が見えなくなる）

区分は手で与えている。機械には「これはユーザー設定」「これは定数表」の
区別がつかないため。GROUPS に無い名前があれば --check が失敗するので、
externs.h に足したのに分類し忘れることはない。
"""

import glob
import os
import re
import sys

# externs.h の宣言を意味で分けたもの。名前順ではなく「誰のものか」で分ける。
GROUPS = {
    "定数表（読みとり専用データ）": """
        copyright days player_title race background player_exp class class_level_adj
        magic_spell spell_names player_init rgold_adj owners store_choice store_buy
        special_names syllables blows_table normal_table blank_monster""",
    # 見た目は定数表だが起動時に一度書きかえる。「定数」として扱うと壊す。
    #   colors 系 6 個  desc.c:34 magic_init() が randes_seed を種にシャッフル。
    #                   未鑑定アイテムの見た目をゲームごとに変える仕掛けで、
    #                   randes_seed がセーブされるのは再現のため
    #   object_list     main.c:308 price_adjust() が cost を一括で割りもどす。
    #                   COST_ADJ が 100（constant.h:72）なので現状は
    #                   #if で消えているが、変えると生きる
    "起動時に一度書きかえる表": """
        colors mushrooms woods metals rocks amulets object_list""",
    "オプション（ユーザー設定）": """
        rogue_like_commands find_cut find_examine find_prself find_bound
        prompt_carry_flag show_weight_flag highlight_seams find_ignore_doors
        sound_beep_flag display_counts""",
    "プレイヤー状態": """
        py char_row char_col player_hp spell_learned spell_worked spell_forgotten
        spell_order total_winner max_score player_light weapon_heavy pack_heavy""",
    "ダンジョンとその中身": """
        cave dun_level cur_height cur_width m_list m_level mfptr mon_tot_mult
        t_list tcptr t_level hack_monptr""",
    "持ち物・アイテム": """
        inventory inven_ctr inven_weight equip_ctr object_ident sorted_objects""",
    "店": "store last_store_inc",
    "画面の見えている範囲（パネル）": """
        panel_row panel_col panel_row_min panel_row_max panel_col_min panel_col_max
        panel_row_prt panel_col_prt max_panel_rows max_panel_cols""",
    # old_msg / last_msg（履歴の輪）は #18-2 で messages.c の static になり、
    # externs.h から外れた。この一覧は externs.h にあるものを数える道具なので、
    # 片づいたものは消えていく。消えた記録は GLOBALS_INVENTORY.md 側に残す。
    "メッセージ表示": "msg_flag wait_for_more",
    "コマンド入力・実行中のフラグ": """
        command_count default_dir last_command doing_inven screen_change find_flag
        free_turn_flag new_level_flag teleport_flag eof_flag light_flag closing_flag
        missile_ctr""",
    "セーブ／スコア／進行メタ": """
        savefile died_from birth_date highscore_fp noscore panic_save death wizard
        to_be_wizard character_generated character_saved turn randes_seed town_seed""",
}

# 定義の置き場。ここでの代入は初期化なので「散らばった書きこみ」に数えない。
HOME_FILES = {"variable.c", "player.c", "tables.c", "treasure.c", "monsters.c"}
# 状態の写しとり。`state->x = x` は x を書かない。
SNAPSHOT_FILES = {"game_state.c"}

TYPE_KEYWORDS = {
    "extern", "const", "char", "int", "bool", "FILE", "void", "unsigned", "signed",
    "int16_t", "int32_t", "uint8_t", "uint16_t", "uint32_t", "vtype", "bigvtype",
    "cave_type", "player_type", "race_type", "background_type", "class_type",
    "spell_type", "owner_type", "store_type", "treasure_type", "inven_type",
    "monster_type", "MAX_SAVE_MSG",
}


def declared_globals(header="src/externs.h"):
    """externs.h の extern 宣言から変数名を順序を保って抜きだす。"""
    names = []
    for line in open(header):
        if not line.startswith("extern "):
            continue
        decl = line.split("//")[0].rstrip().rstrip(";")
        # 名前のあとに来るのは , か 行末 か ) （関数ポインタ配列）
        for name in re.findall(r"\b([a-zA-Z_]\w*)\b(?=\s*(?:\[[^;]*\])?\s*(?:,|$|\)))", decl):
            if name not in TYPE_KEYWORDS and not name.isupper():
                names.append(name)
    return list(dict.fromkeys(names))


# 名前のあとに続く添字・メンバ参照。py.misc.exp や cave[y][x].fval を拾う。
# 添字の中の添字（sorted_objects[t_level[l] - tmp[l]]）も 1 段だけ許す。
INDEX = r"\[(?:[^\[\]]|\[[^\[\]]*\])*\]"
SUFFIX = r"(?:\s*(?:%s|\.\w+|->\w+))*" % INDEX
# 代入・複合代入・増減。== != <= >= と紛れないようにする。
ASSIGN = r"(?:(?<![=!<>+\-*/%&|^])=(?!=)|\+\+|--|[+\-*/|&^]=|>>=|<<=)"
# 第 1 引数を書きかえる標準関数。vtype（char 配列）はこの形でしか書かれない。
STR_WRITERS = r"(?:strcpy|strncpy|strcat|strncat|sprintf|snprintf|memcpy|memmove|memset)"


def counts(names, sources):
    """名前ごとに (参照数, 参照ファイル, 書きこみ数, 書きこみファイル, 別名数) を返す。"""
    text = {f: open(f, errors="replace").read() for f in sources}
    result = {}
    for name in names:
        n_ = re.escape(name)
        ref = re.compile(r"\b%s\b" % n_)
        # 前が . や -> のものは別の構造体のメンバなので数えない
        assign = re.compile(r"(?<![.\w>])\b%s\b%s\s*%s" % (n_, SUFFIX, ASSIGN))
        # strcpy(died_from, ...) のように第 1 引数として渡される形
        strwrite = re.compile(r"\b%s\s*\(\s*(?<![.\w>])\b%s\b%s\s*," % (STR_WRITERS, n_, SUFFIX))
        # &x / &x[i] は書きこみ可能な別名。配列名を裸で渡すのも同じ
        alias = re.compile(r"&\s*(?<![.\w>])\b%s\b%s" % (n_, SUFFIX))
        refs, ref_files, writes, write_files, aliases = 0, [], 0, [], 0
        for path in sources:
            base = os.path.basename(path)
            n = len(ref.findall(text[path]))
            if n:
                refs += n
                ref_files.append(base)
            if base in HOME_FILES or base in SNAPSHOT_FILES:
                continue
            n = len(assign.findall(text[path])) + len(strwrite.findall(text[path]))
            if n:
                writes += n
                write_files.append(base)
            aliases += len(alias.findall(text[path]))
        result[name] = (refs, ref_files, writes, write_files, aliases)
    return result


def group_of(names):
    table = {}
    for group, listing in GROUPS.items():
        for name in listing.split():
            table[name] = group
    return {n: table.get(n) for n in names}


def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else ""
    names = declared_globals()
    belongs = group_of(names)

    unclassified = [n for n in names if belongs[n] is None]
    phantom = [n for g in GROUPS.values() for n in g.split() if n not in names]
    if unclassified or phantom:
        for n in unclassified:
            print(f"未分類: {n}（GROUPS に足すこと）")
        for n in phantom:
            print(f"externs.h に無い: {n}（GROUPS から外すこと）")
        return 1
    if mode == "--check":
        print(f"OK: {len(names)} 個すべて分類されている")
        return 0

    stats = counts(names, sorted(glob.glob("src/*.c")))

    if mode == "--full":
        print(f"{'名前':<22}{'参照':>5}{'参照f':>6}{'書き':>5}{'書きf':>6}{'別名':>5}  区分")
        for group in GROUPS:
            for name in [n for n in names if belongs[n] == group]:
                refs, ref_files, writes, write_files, aliases = stats[name]
                print(f"{name:<22}{refs:>5}{len(ref_files):>6}"
                      f"{writes:>5}{len(write_files):>6}{aliases:>5}  {group}")
        return 0

    print(f"{'区分':<32}{'個数':>5}{'参照':>7}{'書き':>6}{'別名':>6}{'最多参照':>10}")
    total = [0, 0, 0, 0]
    for group in GROUPS:
        members = [n for n in names if belongs[n] == group]
        refs = sum(stats[n][0] for n in members)
        writes = sum(stats[n][2] for n in members)
        aliases = sum(stats[n][4] for n in members)
        top = max(members, key=lambda n: stats[n][0])
        print(f"{group:<32}{len(members):>5}{refs:>7}{writes:>6}{aliases:>6}"
              f"   {top}({stats[top][0]})")
        total[0] += len(members)
        total[1] += refs
        total[2] += writes
        total[3] += aliases
    print(f"{'合計':<32}{total[0]:>5}{total[1]:>7}{total[2]:>6}{total[3]:>6}")

    never = [n for n in names if stats[n][2] == 0 and stats[n][4] == 0]
    wide = [n for n in names if len(stats[n][3]) >= 3]
    aliased = [n for n in names if stats[n][4] > 0]
    print()
    print(f"書きこみも別名もない（実質定数）: {len(never)} 個")
    print(f"3 ファイル以上から書かれる:       {len(wide)} 個  "
          f"{' '.join(sorted(wide, key=lambda n: -len(stats[n][3]))[:8])} …")
    print(f"別名がとられる（数えもれあり）:   {len(aliased)} 個  "
          f"{' '.join(sorted(aliased, key=lambda n: -stats[n][4])[:8])} …")
    return 0


if __name__ == "__main__":
    sys.exit(main())
