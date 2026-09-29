# sources.mk -- src/ の .c の一覧（ここ 1 か所だけに書く）
#
# makefile・makefile.win・makefile.test の 3 つがこれを include する。新しい
# .c はここに足す（以前は makefile と makefile.win の SRCS と OBJS の 4 か所に
# 足していて、makefile.win への足しわすれが続いた。台帳 #46）。
#
# 名前はディレクトリーを付けずに書く。読む側が src/ を補う（makefile は VPATH、
# makefile.win は src/ の中で動かす前提、makefile.test は規則の側で src/ を
# 付ける）。
#
# 並びは本体のリンクの順（makefile の OBJS がこの順になる）。並びを変えると
# 本体の実行形式のバイト列が変わるので、足すときは末尾か、近い仲間の隣に置く。

SRCS = main.c misc1.c misc2.c misc3.c misc4.c store1.c files.c io.c \
	create.c desc.c generate.c sets.c dungeon.c creature.c death.c \
	eat.c help.c magic.c potions.c prayer.c save.c staffs.c wands.c device.c \
	item_ident.c abilities.c options.c messages.c \
	scrolls.c spells.c wizard.c store2.c signals.c signal_flags.c \
	render.c render_ncurses.c view_observer.c game_state.c \
	input.c input_ncurses.c platform.c panel.c stores.c stats.c str_insert.c \
	inventory.c progress.c score_death.c save_state.c player_pos.c \
	hp_table.c player_light.c burden.c spells_known.c object_levels.c \
	missile_serial.c inven_command_state.c screen_touched.c \
	level_exit.c pending_teleport.c input_ended.c running.c \
	command_state.c player_gold.c player_food.c player_display_numbers.c \
	player_mana.c player_hp.c player_level.c player_status_flags.c \
	player_abilities.c player_timed_effects.c player_resting.c \
	player_speed.c player_infra_range.c player_glowing_hands.c \
	player_spells_to_learn.c player_max_depth.c player_hit_die.c player_armour_class.c player_base_to_hit.c \
	player_disarm.c player_saving_throw.c player_race.c player_body_weight.c \
	player_attack_bonuses.c player_search_skill.c player_bio.c \
	player_stealth.c player_class.c \
	monster_turn.c monster_levels.c monster_breeding.c monster_list.c \
	dungeon_size.c dungeon_level.c floor_items.c dungeon_map.c \
	moria1.c moria2.c moria3.c moria4.c monsters.c treasure.c variable.c \
	rnd.c recall.c player.c tables.c

# 本体の実行形式にだけ入り、テストのライブラリー（makefile.test の libcore.a）
# には入れないもの。main() を持つ main.c と、ncurses を直に呼ぶ 2 本。
APP_SRCS = main.c render_ncurses.c input_ncurses.c

# それ以外のすべて。テストはこの中から要るものだけをリンクする。
CORE_SRCS = $(filter-out $(APP_SRCS),$(SRCS))
