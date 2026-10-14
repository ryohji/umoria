# sources.mk -- src/ の .c の一覧（ここ 1 か所だけに書く）
#
# makefile・makefile.win・makefile.test の 3 つがこれを include する。新しい
# .c はここに足す（以前は makefile と makefile.win の SRCS と OBJS の 4 か所に
# 足していて、makefile.win への足しわすれが続いた。台帳 #46）。
#
# 名前は src/ からの相対で書く（層のサブディレクトリーのものは
# model/player/burden.c のように）。読む側が src/ を補う。.o はどの
# makefile もディレクトリーを落とした名前で 1 か所に置く（makefile は根、
# makefile.win は src/、makefile.test は tests/build/core/）。.c を探すのは
# VPATH/vpath、ヘッダを探すのは -I で、どちらも下の SRC_SUBDIRS から作る。
#
# 並びは本体のリンクの順（makefile の OBJS がこの順になる）。並びを変えると
# 本体の実行形式のバイト列が変わるので、足すときは末尾か、近い仲間の隣に置く。

# src/ のサブディレクトリー（層ごとに分けた葉のディレクトリー）。ここに
# 書いたものが VPATH と -I に入る。ディレクトリーを足したらここにも足す。
# 名前が重なると -I の順で答えが変わるので、ファイルを足すときは
# ls src/*/ src/*/*/ | sort | uniq -d で同名が無いことを見る。
SRC_SUBDIRS = core model/data model/game model/player model/monster model/dungeon model/item model/store platform ui world/combat world/dungeon world/item world/monster world/player world/store world/view command app

SRCS = app/main.c command/files.c ui/io.c \
	command/create.c world/item/desc.c world/dungeon/generate.c model/data/sets.c app/dungeon.c world/monster/creature.c app/death.c app/input_end_handling.c \
	command/eat.c command/help.c command/magic.c command/potions.c command/prayer.c app/save.c command/staffs.c world/item/device_use.c command/wands.c model/item/device.c \
	model/item/item_ident.c world/item/item_learn.c model/item/item_flags.c model/player/abilities.c model/game/options.c ui/messages.c \
	command/scrolls.c world/item/spells.c command/wizard.c platform/signals.c platform/signal_flags.c app/signal_handling.c \
	ui/render.c platform/render_ncurses.c ui/view_observer.c app/game_state.c \
	ui/input.c platform/input_ncurses.c app/platform_init.c ui/panel.c world/view/map_view.c model/store/stores.c world/store/store_price.c world/store/store_stock.c command/store_ui.c world/store/store_haggle.c model/player/stats.c core/str_insert.c core/bits.c core/dice.c core/distance.c \
	model/item/inventory.c model/item/armor_selection.c model/game/progress.c model/game/score_death.c model/game/save_state.c model/player/player_pos.c \
	model/player/hp_table.c model/player/player_light.c model/player/burden.c model/player/spells_known.c model/item/object_levels.c model/item/object_copy.c world/item/inscription.c world/item/item_enchant.c \
	model/item/missile_serial.c ui/inven_command_state.c ui/screen_touched.c command/options_menu.c \
	world/dungeon/level_exit.c model/player/pending_teleport.c ui/input_ended.c model/player/running.c \
	ui/command_state.c model/player/player_gold.c model/player/player_food.c world/player/food_ops.c model/player/player_display_numbers.c \
	model/player/player_mana.c model/player/player_hp.c model/player/player_level.c model/player/player_status_flags.c \
	model/player/player_abilities.c model/player/player_timed_effects.c model/player/player_resting.c \
	model/player/player_speed.c model/player/player_infra_range.c model/player/player_glowing_hands.c \
	model/player/player_spells_to_learn.c model/player/player_max_depth.c model/player/player_hit_die.c model/player/player_armour_class.c model/player/player_base_to_hit.c \
	model/player/player_disarm.c model/player/player_saving_throw.c model/player/player_race.c model/player/player_body_weight.c \
	model/player/player_attack_bonuses.c model/player/player_search_skill.c model/player/player_bio.c \
	model/player/player_stealth.c model/player/player_class.c \
	world/monster/monster_turn.c model/monster/monster_levels.c world/monster/monster_breeding.c model/monster/monster_list.c world/monster/monster_place.c model/monster/monster_recall.c \
	model/dungeon/dungeon_size.c model/dungeon/dungeon_level.c model/dungeon/floor_items.c model/dungeon/dungeon_map.c model/dungeon/geometry.c world/dungeon/object_place.c \
	command/inven_menu.c world/player/run_path.c world/monster/monster_death.c command/look.c model/data/monsters.c model/data/treasure.c model/game/variable.c \
	core/rnd.c command/recall.c model/data/player.c model/data/tables.c \
	world/combat/hit_rolls.c world/combat/player_damage.c world/item/inven_ops.c world/item/item_picker.c world/player/player_move.c \
	world/dungeon/object_alloc.c ui/screen_fields.c world/view/status_line.c command/char_screen.c \
	world/player/stat_ops.c world/player/stat_loss.c world/item/spellbook.c world/player/level_ops.c \
	world/dungeon/search.c \
	world/player/player_bonuses.c \
	world/player/rest_command.c \
	ui/direction.c \
	ui/direction_keys.c \
	world/dungeon/lighting.c \
	world/combat/player_melee.c \
	world/combat/throw.c \
	world/dungeon/traps.c \
	world/dungeon/terrain_commands.c \
	world/combat/monster_damage.c \
	world/combat/monster_melee.c \
	world/combat/projectiles.c

# 本体の実行形式にだけ入り、テストのライブラリー（makefile.test の libcore.a）
# には入れないもの。main() を持つ app/main.c と、ncurses を直に呼ぶ 2 本。
# ディレクトリーを付けずに名前だけで書く（下の %/ の形で、どのディレクトリー
# にあっても当たる）。
APP_SRCS = app/main.c render_ncurses.c input_ncurses.c

# それ以外のすべて。テストはこの中から要るものだけをリンクする。
CORE_SRCS = $(filter-out $(APP_SRCS) $(addprefix %/,$(APP_SRCS)),$(SRCS))

# ---- 層ごとのライブラリー ----------------------------------------------
#
# 各層を静的ライブラリーにして、上の層から下の層の順でリンクする。GNU ld は
# archive を左から右に 1 度ずつ読むので、下の層から上の層への参照は、その
# 相手がまだ引かれていなければ undefined reference で止まる。すでに別の理由で
# 引かれている相手への参照は通ってしまうので、層の規則を確かめる正本は
# scripts/layer_deps.py --check のほう。
#
# model/ の副ライブラリーも分ける。data と game は互いを呼びあえるので 1 つの
# archive（libmodel_base.a）にまとめる。dungeon と store は item を使える。

LIB_LINK_ORDER = command world ui platform model_player model_monster \
                 model_dungeon model_store model_item model_base core

# Sources that go into libraries (exclude app/ - those are linked directly)
LIB_SRCS_ALL = $(filter-out app/%,$(CORE_SRCS))

# Per-library sources (derived from LIB_SRCS_ALL by path prefix)
LIB_SRCS_command       = $(filter command/%,$(LIB_SRCS_ALL))
LIB_SRCS_world         = $(filter world/%,$(LIB_SRCS_ALL))
LIB_SRCS_ui            = $(filter ui/%,$(LIB_SRCS_ALL))
LIB_SRCS_platform      = $(filter platform/%,$(LIB_SRCS_ALL))
LIB_SRCS_model_player  = $(filter model/player/%,$(LIB_SRCS_ALL))
LIB_SRCS_model_monster = $(filter model/monster/%,$(LIB_SRCS_ALL))
LIB_SRCS_model_dungeon = $(filter model/dungeon/%,$(LIB_SRCS_ALL))
LIB_SRCS_model_store   = $(filter model/store/%,$(LIB_SRCS_ALL))
LIB_SRCS_model_item    = $(filter model/item/%,$(LIB_SRCS_ALL))
# model_base: data and game may call each other, so one archive
LIB_SRCS_model_base    = $(filter model/data/% model/game/%,$(LIB_SRCS_ALL))
LIB_SRCS_core          = $(filter core/%,$(LIB_SRCS_ALL))
