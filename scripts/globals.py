#!/usr/bin/env python3

# Copyright (c) 2026 Umoria Contributors
#
# Umoria is free software released under a GPL v2 license and comes with
# ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
# for further details.

"""externs.h のグローバル変数を数えあげる。

臭い #18（データの散在）の全体像をつかむための計測。手で数えあげると
見落とすし、直すたびに数えなおすことになるので機械化しておく。
scripts/warnings.sh と同じ位置づけ（現状を数字で見るための道具）。

  $ python3 scripts/globals.py            # 区分ごとの集計
  $ python3 scripts/globals.py --full     # 112 個すべての一覧
  $ python3 scripts/globals.py --check    # 分類の網羅性だけ確認（CI 向け）

数えるもの
  参照数     src/ の .c（サブディレクトリーを含む）に現れる回数（定義そのものを含む）
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
        py""",
    # 覚えている呪文 4 個（spell_learned・spell_worked・spell_forgotten・
    # spell_order）は #18-8-C で spells_known.c の static になり、externs.h から
    # 外れた。窓口は src/player/spells_known.h。導出する形にはしていない —— 何を覚えた
    # かは履歴で、覚えていた記録がなければ復元できない（progress・score_death と
    # 同じ理由）。ビット演算（1L << spell）は窓口の内側にある。
    # 重さに負けているか 2 個（weapon_heavy・pack_heavy）は #18-7-4C1 で
    # burden.c の static になり、externs.h から外れた。窓口は src/player/burden.h。
    # 装備と持ち物の重さから毎回導出する形にはしていない —— 重い／軽いの
    # **変わり目**でしか起きないこと（4 通りの message と change_speed への
    # 差分）が check_strength()（inven_ops.c）にあるので、覚えた答えを返す
    # 置き場のままにしてある（player_light と同じ理由）。
    # 明かりの有無（player_light）は #18-7-3C1 で player_light.c の static に
    # なり、externs.h から外れた。窓口は src/player/player_light.h（player_has_light /
    # set_player_has_light）。装備の欄から導出する形にはしていない —— 明るい／
    # 暗いの**変わり目**でしか起きないこと（message・モンスターの出しなおし）が
    # dungeon.c にあるので、覚えた答えを返す置き場のままにしてある。
    # レベルごとの HP 表（player_hp）は #18-7-2C1 で hp_table.c の static に
    # なり、externs.h から外れた。窓口は src/player/hp_table.h（hp_total_at_level /
    # set_hp_total_at_level / hp_table_slots）。表は 1 起点で読む。
    # 勝ちと最高得点の 2 個（total_winner max_score）は #18-7-1C1 で
    # score_death.c の static になり、externs.h から外れた。窓口は
    # src/save/score_death.h（player_has_won / best_score_so_far）。
    # 居場所の 2 個（char_row char_col）は #18-6C1 で player_pos.c の static に
    # なり、externs.h から外れた。窓口は src/player/player_pos.h（player_row /
    # player_col / player_place / player_pos_forget）。同じ区分に残る 11 個は
    # 居場所とは独立なので残す。片づいた名前は行から消える（消えた記録は
    # docs/refactoring/globals_inventory.md 側）。
    # 床の一枚一枚（cave）は #18-14-8C で dungeon_map.c の static になり、
    # externs.h から外れた。窓口は src/dungeon/dungeon_map.h（dungeon_map_reset /
    # square_at の 2 本だけ）。**この区分の 8 問め＝最後**で、**#18 に残っていた
    # global のどれよりも大きい**（15 ファイル 258 参照・書き 47・別名 145）。
    # **これで「ダンジョンとその中身」の区分は空になった**（行ごと消した。記録は
    # docs/refactoring/globals_inventory.md 側）。窓口が 2 本で足りたのは、**1 マスの 7 つの欄が
    # 7 つの別の問い**で、この module はどれにも答えないから —— 配るのは
    # 書きこめる別名で、欄の意味は呼び手の側にある（だから最後に置いた。
    # `.cptr` と `.tptr` は 4 問め・7 問めの表の行番号で、あの 2 つが窓口を
    # 持つまで「1 マスを配る」が何を意味するか決まらなかった）。
    # 測って分かった 3 つ —— **表は階より大きい**（表はいつでも 66x198、町は
    # その左上の 22x66。だから白紙に戻すのは表ぜんぶで、階の広さで止めると
    # 次に町へ上がったとき前の階の壁が残る。広さは 5 問めの問いで、窓口は
    # 訊かない）、**白紙のマスは「空いた床」ではない**（fval 0 は NULL_WALL ＝
    # まだ決めていない印、cptr 0 と tptr 0 は隣の 2 つの表の行 0 と対で、
    # あの 2 問が行 0 を白紙にするのと同じ約束の両端）、そして
    # **19 のループが表をポインタで歩いていた**（generate.c に 18、save.c に 1。
    # 3 つの形 —— 行に沿って 17・列を下って 1・表ぜんぶを 1 ――で、ぜんぶ
    # 添字のループに畳んだ。save.c の読みもどしは表を**1 本の列**として歩いて
    # いたので `square_at(n / MAX_WIDTH, n % MAX_WIDTH)` になった）。
    # 畳んだ包み 1 本（generate.c の static blank_cave()。この区分で 3 本め）と
    # 別名 145 件もここで消えた。B は 1 コミットに入らず 4 つに分けた
    # （7 ファイル 32 か所・generate.c 99 か所・5 ファイル 109 か所・save.c 7 か所）。
    # 税は 12 本で、**A の見こみとぴったり一致**した。
    # 床に落ちているもの（t_list ＋ tcptr）は #18-14-7C で floor_items.c の
    # static になり、externs.h から外れた。窓口は src/dungeon/floor_items.h
    # （floor_items_reset / floor_item_at / floor_items_used /
    # set_floor_items_used / floor_items_is_full / floor_items_claim_slot /
    # floor_items_drop_last）。**この区分の 7 問め**で、#18-14-4 のモンスターの
    # 表とまったく同じ形（表と印で 1 つの入れ物、マスの片われ）。定義表
    # （420 品の「その品目とは何か」）の隣に置かれていたが、こちらは
    # 「いまこの階のどこに何が載っているか」で、別の表。
    # 測って分かった 3 つ —— **これは「宝の表」ではない**（扉・階段・瓦礫・
    # 罠・店の入口も行で、マスに載っているモンスターでないものぜんぶが入る。
    # だから 17 ファイルに散っていた）、**行 0 は予約されているだけでなく
    # 読まれる**（拾った直後の `t_list[c_ptr->tptr]` が行 0 に落ちて
    # TV_NOTHING を得る。階の頭で白紙にするのは片づけではなく仕事）、そして
    # **行は自分がどのマスに載っているかを知らない**（pusht() が階を掃いて
    # 探す。上流も types.h:136 でそう書いている ―― 掃く側は 8 問めの cave の
    # 仕事なので残した）。別名 1 件（save.c の読みもどし）もここで消えた。
    # 税は 12 本で **A で測りなおした見こみと一致**した（下調べは 13 本と
    # 見ていたが、13 本めは呼び手ではなく置き場だった。所見 34）。
    # いま何階か（dun_level）は #18-14-6C で dungeon_level.c の static になり、
    # externs.h から外れた。窓口は src/dungeon/dungeon_level.h（dungeon_level /
    # player_is_in_town / set_dungeon_level）。**この区分の 6 問め**で、
    # **同じ問いが 3 通りに書かれていた** —— 「町にいるか」を `!= 0`・`> 0`・
    # `== 0` と 6 か所が別々に綴っていたので、窓口 1 本に寄せた。走りだしの
    # 0 は空の器ではなく場所（町）なので、その初期値をテストで押さえた。
    # 測って分かった 2 つ —— **同じ `深さ * 50` が 2 つの単位を持つ**
    # （death.c では点、status_line.c では feet。同じ算術だが畳めない。所見 54 の
    # 2 つめ）、そして**負の階は誰も比べていないから起きない**（町に上りの
    # 階段が無いから起きない。窓口も検めない ―― 上流のまま）。
    # 別名 1 件（save.c の復元が int16_t へポインタ型を偽って読んでいた
    # 1 か所）もここで消えた。税は 12 本で、**2 問続けて下調べの見こみと
    # 一致した**。
    # この階の広さ（cur_height ＋ cur_width）は #18-14-5C で dungeon_size.c の
    # static になり、externs.h から外れた。窓口は src/dungeon/dungeon_size.h
    # （dungeon_height / dungeon_width / set_dungeon_size）。**この区分の
    # 5 問め**で、**2 つの名前が 1 つの行い**だった —— 書き手はどちらも必ず
    # 両方を書き、読み手 48 のうち 40 が対で読む。だから**窓口は両方を取る
    # 1 本**にして、半分だけ変える道を無くした（#18-12-18 の pac/ptoac で
    # 見た穴と同じ形）。
    # 測って分かった 3 つ —— **この対は 2 つの値しか取らない**（町の 22x66 と
    # 階の 66x198。選ぶのは generate_cave() の 1 か所で、つまり「町にいるか」の
    # 言いかえ）、**無作為な 1 マスの取りかたが 2 通りある**（randint(高さ-2) は
    # 外周に当たらず randint(高さ)-1 は当たる。畳むと出現位置が変わるので
    # そのまま残した）、そして **dungeon.c の 'L' が行を幅と比べている**
    # （上流のバグ。そのまま写して header に書きとめた）。
    # 別名 2 件（save.c の `rd_short((uint16_t *)&cur_height)` の対）も
    # ここで消えた。税は 11 本で、**下調べの見こみと初めて一致した**。
    # この階にいるモンスター（m_list ＋ mfptr）は #18-14-4C で
    # monster_list.c の static になり、externs.h から外れた。窓口は
    # src/monster/monster_list.h（monster_list_reset / monster_list_at /
    # monster_list_used / set_monster_list_used / monster_list_is_full /
    # monster_list_free_slots / monster_list_claim_slot /
    # monster_list_drop_last）。**この区分の 4 問め**で、**2 つの名前が
    # 1 つの入れ物**だった —— 行を詰めた表と、どこまで使っているかの印。
    # 定義表（279 体の「その種とは何か」）の隣に置かれていたが、こちらは
    # 「いまこの階に誰が立っているか」で、別の表。
    # 測って分かった 2 つ —— **14 か所が同じ数えおろしループを手で書いて
    # いた**（うち 11 本は spells.c に 1 文字も違わず並ぶ）、そして
    # **B を 1 コミットで通せないほど大きい**（12 ファイル 101 参照）ので、
    # この区分で初めて**置き場を A で動かせなかった**（所見 52 の条件）。
    # 別名 1 件（save.c の `rd_short((uint16_t *)&mfptr)`）もここで消えた。
    # この階で増えたモンスターの数（mon_tot_mult）は #18-14-3C で
    # monster_breeding.c の static になり、externs.h から外れた。窓口は
    # src/monster/monster_breeding.h（monster_breeding_reset / monster_breeding_allowed /
    # monster_breeding_note_birth / monster_breeding_note_death ＋ セーブ用の
    # monster_breeding_count / set_monster_breeding_count）。**この区分の 3 問め**。
    # 定義表の隣（monsters.c）に置かれていたが表とは無関係な階ごとの数だった。
    # 測って分かった 2 つ —— 比較が `MAX_MON_MULT >= mon_tot_mult` なので
    # **許される出産は 76 回**（上限 75 と 1 ずれる）、そして**これは頭数ではなく
    # 予算**（減るのは fix1_delete_monster() を通った 1 体だけで、生まれた子か
    # どうかは訊かない）。別名 1 件（save.c の `rd_short((uint16_t *)&...)`）も
    # ここで消えた。
    # レベルごとのモンスター定義の索引（m_level）は #18-14-2C で
    # monster_levels.c の static になり、externs.h から外れた。窓口は
    # src/monster/monster_levels.h（monster_levels_init / monsters_up_to_level /
    # monsters_at_level / first_monster_at_level）。**この区分の 2 問め**。
    # 組みたてていた init_m_level() は main.c の static で、**テストから
    # 届かなかった**（#18-10 の init_t_level() と同じ形）。読み手 6 か所が
    # `m_level[level] - m_level[level - 1]` や `+ m_level[0]` と手で書いていた
    # 引き算に名前が付いた。品物のほうは並べなおした本体（sorted_objects）が
    # 別に要ったが、モンスター定義表はもとからレベルの昇順なので索引だけで足りる。
    # いま creatures() が誰の手番を処理しているか（hack_monptr）は #18-14-1C で
    # monster_turn.c の static になり、externs.h から外れた。窓口は
    # src/monster/monster_turn.h（monster_turn_begin / monster_turn_end /
    # monster_delete_may_shift / monster_turn_index）。読み手 2 つが
    # `hack_monptr < i` と手で書いていた比較に名前が付いた ——
    # 「その席を詰めなおしてよいか」。**この区分の 1 問め**で、残る 10 個は
    # 手番とは独立なので残す。
    # レベルごとに並べたダンジョンの品物表（sorted_objects と t_level）は
    # #18-10-C で object_levels.c の static になり、externs.h から外れた。窓口は
    # src/item/object_levels.h（object_levels_init / object_at_level_position /
    # objects_up_to_level / objects_at_level / first_position_at_level）。
    # **この 2 個は台帳では別の区分に分かれていた**（sorted_objects は
    # 「持ち物・アイテム」、t_level は「ダンジョンとその中身」）が、実測すると
    # 1 つの表の目次と本体で、片方だけでは意味をなさない。区分は参照の多さで
    # 分けたものなので、module の切れ目とは一致しない。
    # **これで「持ち物・アイテム」の区分は空になった**（行ごと消した。記録は
    # docs/refactoring/globals_inventory.md 側）。
    # 品目ごとの覚え（object_ident）は #18-9-C で item_ident.c の static になり、
    # externs.h から外れた。窓口は src/item/item_ident.h（item_kind_is_known /
    # item_kind_was_tried / item_kind_mark_known / item_kind_mark_tried /
    # item_kind_clear_tried / item_kind_has_record、セーブ用に
    # item_kind_record_bytes / item_kind_record_count）。持ち物の中身から
    # 導出する形にはしていない —— 覚えは**品目ごと**で、持っていない品目や
    # まだダンジョンにある品目についても覚えているから。
    # 持ち物の 4 個（inventory inven_ctr inven_weight equip_ctr）は #18-5 で
    # inventory.c の static になり、externs.h から外れた。窓口は
    # src/item/inventory.h（持ち物・跨ぎ）と src/item/equipment.h（装備）。
    # 片づいた名前は行から消える（消えた記録は docs/refactoring/globals_inventory.md 側）。
    # 店の区分（store last_store_inc の 2 個）は #18-4 で externs.h から
    # 全部外れた。6 軒の記録は stores.c、値切りの途中の入力は store2.c の
    # static になった。片づいた区分は行ごと消える（記録は
    # docs/refactoring/globals_inventory.md 側）。
    # 画面の見えている範囲（パネル）の 10 個は #18-3 で panel.c の static に
    # なり、externs.h から全部外れた。メッセージ表示と同じく、片づいた区分は
    # 行ごと消える（消えた記録は docs/refactoring/globals_inventory.md 側）。
    # メッセージ表示の区分（msg_flag old_msg last_msg wait_for_more の 4 個）は
    # #18-2 で messages.c の static になり、externs.h から全部外れた。この一覧は
    # externs.h にあるものを数える道具なので、片づいた区分は行ごと消える。
    # 消えた記録は docs/refactoring/globals_inventory.md 側に残す。
    # 打っているコマンドについて覚えていること 3 個（command_count default_dir
    # last_command）は #18-11-7C で src/ui/command_state.c の static になった。
    # どれも「コマンドに繰りかえしの回数を付けられる」ことから出ているので
    # 1 本の module にまとめた。
    "コマンド入力・実行中のフラグ": """
        free_turn_flag closing_flag""",
    # セーブ／スコア／進行メタの 14 個のうち 13 個は #19C1 で 3 つの module の
    # static になり、externs.h から外れた（progress.c に turn randes_seed
    # town_seed wizard to_be_wizard、score_death.c に death died_from
    # birth_date noscore、save_state.c に savefile character_generated
    # character_saved panic_save）。残るのは highscore_fp 1 個。得点ファイルを
    # 読み書きする 2 関数（death.c の display_scores() と highscores()）は
    # #19-5-1 で局所変数にしたので、書きこみは init_scorefile()（files.c）の
    # 1 箇所だけになった。その handle は誰も読まず誰も閉じない（バグ候補
    # B19）ので、この 1 個を片づけるにはふるまいの判断が要る。
    "セーブ／スコア／進行メタ": """
        highscore_fp""",
}

# 定義の置き場。ここでの代入は初期化なので「散らばった書きこみ」に数えない。
# rnd.c は #54 で normal_table を tables.c から受けとった。
HOME_FILES = {"variable.c", "player.c", "tables.c", "treasure.c", "monsters.c", "rnd.c"}
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
# 添字の中の添字（#18-10 より前の sorted_objects[t_level[l] - tmp[l]]）も
# 1 段だけ許す。
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

    # src/ 直下とサブディレクトリー（#53-D の core/・player/ など）の .c すべて。
    # 集計はファイル名（basename）で見るので、置き場が変わっても数は変わらない。
    stats = counts(names, sorted(glob.glob("src/**/*.c", recursive=True)))

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
