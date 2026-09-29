# sources.mk -- src/ の .c の一覧（ここ 1 か所だけに書く）
#
# makefile・makefile.win・makefile.test の 3 つがこれを include する。新しい
# .c はここに足す（以前は makefile と makefile.win の SRCS と OBJS の 4 か所に
# 足していて、makefile.win への足しわすれが続いた。台帳 #46）。
#
# 名前は src/ からの相対で書く（src/ 直下のものは名前だけ、サブディレクトリー
# のものは player/burden.c のように）。読む側が src/ を補う。.o はどの
# makefile もディレクトリーを落とした名前で 1 か所に置く（makefile は根、
# makefile.win は src/、makefile.test は tests/build/core/）。.c を探すのは
# VPATH/vpath、ヘッダを探すのは -I で、どちらも下の SRC_SUBDIRS から作る。
#
# 並びは本体のリンクの順（makefile の OBJS がこの順になる）。並びを変えると
# 本体の実行形式のバイト列が変わるので、足すときは末尾か、近い仲間の隣に置く。

# src/ のサブディレクトリー（docs/refactoring/layout.md の D0 の表）。ここに
# 書いたものが VPATH と -I に入る。ディレクトリーを足したらここにも足す。
# 名前が重なると -I の順で答えが変わるので、ファイルを足すときは
# ls src/*/ | sort | uniq -d で同名が無いことを見る。
SRC_SUBDIRS = core data player

SRCS = main.c misc1.c misc2.c misc3.c misc4.c store1.c files.c io.c \
	player/create.c desc.c generate.c data/sets.c dungeon.c creature.c death.c \
	eat.c help.c magic.c potions.c prayer.c save.c staffs.c wands.c device.c \
	item_ident.c player/abilities.c data/options.c messages.c \
	scrolls.c spells.c wizard.c store2.c signals.c signal_flags.c \
	render.c render_ncurses.c view_observer.c game_state.c \
	input.c input_ncurses.c platform.c panel.c stores.c player/stats.c core/str_insert.c \
	inventory.c data/progress.c score_death.c save_state.c player/player_pos.c \
	player/hp_table.c player/player_light.c player/burden.c player/spells_known.c object_levels.c \
	missile_serial.c inven_command_state.c screen_touched.c \
	level_exit.c player/pending_teleport.c input_ended.c player/running.c \
	command_state.c player/player_gold.c player/player_food.c player/player_display_numbers.c \
	player/player_mana.c player/player_hp.c player/player_level.c player/player_status_flags.c \
	player/player_abilities.c player/player_timed_effects.c player/player_resting.c \
	player/player_speed.c player/player_infra_range.c player/player_glowing_hands.c \
	player/player_spells_to_learn.c player/player_max_depth.c player/player_hit_die.c player/player_armour_class.c player/player_base_to_hit.c \
	player/player_disarm.c player/player_saving_throw.c player/player_race.c player/player_body_weight.c \
	player/player_attack_bonuses.c player/player_search_skill.c player/player_bio.c \
	player/player_stealth.c player/player_class.c \
	monster_turn.c monster_levels.c monster_breeding.c monster_list.c \
	dungeon_size.c dungeon_level.c floor_items.c dungeon_map.c \
	moria1.c moria2.c moria3.c moria4.c data/monsters.c data/treasure.c data/variable.c \
	core/rnd.c recall.c data/player.c data/tables.c

# 本体の実行形式にだけ入り、テストのライブラリー（makefile.test の libcore.a）
# には入れないもの。main() を持つ main.c と、ncurses を直に呼ぶ 2 本。
# ディレクトリーを付けずに名前だけで書く（下の %/ の形で、どのディレクトリー
# にあっても当たる）。
APP_SRCS = main.c render_ncurses.c input_ncurses.c

# それ以外のすべて。テストはこの中から要るものだけをリンクする。
CORE_SRCS = $(filter-out $(APP_SRCS) $(addprefix %/,$(APP_SRCS)),$(SRCS))
