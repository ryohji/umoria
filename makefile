# BINDIR is the directory where the moria binary while be put
# LIBDIR is where the other files (score, news, hours) will be put
# LIBDIR must be the same directory defined in config.h
# OWNER is who you want the game to be chown to.
# GROUP is who you wnat the game to be chgrp to.
BINDIR = ~/umoria
LIBDIR = ~/umoria/data

# Game binary name. e.g `moria` or `umoria`.
TARGET = umoria

# Compiler and standard
CC = gcc
# Use C17 standard (latest widely-supported C standard)
STD = -std=c17

# POSIX 拡張を有効にする。-std=c17 だけでは kill() や getpid() のような
# POSIX 関数が宣言されず、暗黙宣言（引数と戻り値の型検査が効かない状態）
# になっていた。C17 は ISO C の範囲しか公開しないため、明示的に要求する。
POSIX = -D_DEFAULT_SOURCE

# Warning flags for better code quality
WARNINGS = -Wall -Wextra -Wpedantic -Wformat=2 -Wno-unused-parameter \
           -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
           -Wnested-externs -Wmissing-prototypes -Wno-deprecated-declarations

# Debug flags
DEBUG_FLAGS = -g -DDEBUG

# Optimization flags
# Note: -O2 enables most optimizations without aggressive inlining
# If issues arise, fall back to -O1 or -Og (optimize for debugging)
OPT_FLAGS = -O2

# Include path for headers in src directory and its subdirectories
# (SRC_SUBDIRS, from sources.mk). #include "..." lines name the header alone,
# so every directory that holds headers is on the path.
INCLUDES = -I$(SRCDIR) $(addprefix -I$(SRCDIR)/,$(SRC_SUBDIRS))

# Combine all compiler flags
CFLAGS = $(STD) $(POSIX) $(WARNINGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(INCLUDES)

# Linker flags
LDFLAGS =
CURSES = -lncurses

# Source directory
SRCDIR = src
VPATH = $(SRCDIR) $(addprefix $(SRCDIR)/,$(SRC_SUBDIRS))

# src/ の .c の一覧は sources.mk に 1 つだけ書く。OBJS はその名前からディレクトリー
# を落として .o に置きかえただけなので、並びも sources.mk と同じ（＝リンクの順）。
# .o はすべて根に並ぶ（名前は src/ の中で重ならない）。
include sources.mk

OBJS = $(notdir $(SRCS:.c=.o))

LIBFILES = splash.hlp origcmds.hlp owizcmds.hlp roglcmds.hlp rwizcmds.hlp \
	version.hlp welcome.hlp

DOCS = manual.md

# Phony targets (not actual files)
.PHONY: all clean install TAGS help

# Default target
all: $(TARGET)

# Link the final executable
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS) $(CURSES)
	@echo "Build complete: $(TARGET)"

# Pattern rule for compiling C source files
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

# Legacy lint targets (kept for compatibility)
lintout : $(SRCS)
	lint $(addprefix $(SRCDIR)/,$(SRCS)) $(CURSES) > lintout

lintout2 : $(SRCS)
	lint -bach $(addprefix $(SRCDIR)/,$(SRCS)) $(CURSES) > lintout

# Generate tags file
TAGS : $(SRCS)
	ctags -x $(addprefix $(SRCDIR)/,$(SRCS)) > TAGS

# you must define BINDIR and LIBDIR before installing
# assumes that BINDIR and LIBDIR exist
install:
	mkdir -p $(BINDIR)
	chmod 755 $(BINDIR)
	cp -f $(TARGET) $(BINDIR)/$(TARGET)
	chmod 4711 $(BINDIR)/$(TARGET)
	mkdir -p $(LIBDIR)
	chmod 711 $(LIBDIR)
	cp -f ../LICENSE $(BINDIR)
	(cd ../data; cp -f $(LIBFILES) $(LIBDIR))
	(cd ../docs; cp -f $(DOCS) $(BINDIR))
	(cd $(LIBDIR); chmod 444 $(LIBFILES))
	(cd $(BINDIR); touch scores.dat; chmod 644 scores.dat)
# If you are short on disk space, or aren't interested in debugging moria.
#	strip $(BINDIR)/moria

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(OBJS)
	@rm -f $(TARGET)
	@rm -f lintout
	@echo "Clean complete!"

# Help target
help:
	@echo "Umoria Build System"
	@echo "==================="
	@echo ""
	@echo "Available targets:"
	@echo "  all (default) - Build the game executable"
	@echo "  clean         - Remove all build artifacts"
	@echo "  install       - Install the game to BINDIR and LIBDIR"
	@echo "  TAGS          - Generate ctags file"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Build configuration:"
	@echo "  CC       = $(CC)"
	@echo "  CFLAGS   = $(CFLAGS)"
	@echo "  TARGET   = $(TARGET)"
	@echo "  BINDIR   = $(BINDIR)"
	@echo "  LIBDIR   = $(LIBDIR)"

# Header dependencies
# These ensure that object files are rebuilt when headers change.
# ヘッダはディレクトリーを付けずに名前だけで書く。make は VPATH（src/ と
# SRC_SUBDIRS）から探すので、ヘッダが src/ のどのサブディレクトリーに
# あっても書きかえずに済む。見つからない名前は "No rule to make target" で止まる。
HEADERS_COMMON = constant.h types.h config.h
HEADERS_FULL = $(HEADERS_COMMON) externs.h

# bits.c came out of misc1.c (#54). It needs nothing but <stdint.h> and its own
# header -- not even constant.h (the same as dungeon_size.o below).
bits.o: bits.h
# burden.c does not include externs.h either, so HEADERS_COMMON is enough.
burden.o: burden.h $(HEADERS_COMMON)
create.o: $(HEADERS_FULL)
creature.o: $(HEADERS_FULL)
death.o: $(HEADERS_FULL)
# dice.c is what was left of misc1.c (#54). It includes externs.h for
# randint().
dice.o: $(HEADERS_FULL)
desc.o: $(HEADERS_FULL)
device.o: device.h $(HEADERS_FULL)
dungeon.o: $(HEADERS_FULL)
eat.o: $(HEADERS_FULL)
files.o: $(HEADERS_FULL)
game_state.o: game_state.h burden.h $(HEADERS_FULL)
# food_ops.c came out of misc1.c (#54). It includes externs.h, and reads the
# stomach and the timed effects (the slowness that overeating costs).
food_ops.o: player_food.h player_timed_effects.h $(HEADERS_FULL)
generate.o: $(HEADERS_FULL)
# geometry.c came out of misc1.c (#54). It includes externs.h, and reads the
# map, its size and what lies on the floor.
geometry.o: dungeon_map.h dungeon_size.h floor_items.h $(HEADERS_FULL)
help.o: $(HEADERS_FULL)
# hp_table.c does not include externs.h either, so HEADERS_COMMON is enough.
hp_table.o: hp_table.h $(HEADERS_COMMON)
# inscription.c came out of misc4.c (#54). It includes externs.h, and reads
# the pack through both windows of inventory.c.
inscription.o: inventory.h equipment.h $(HEADERS_FULL)
# inventory.c does not include externs.h, so HEADERS_COMMON is enough here
# (the same as stats.o and str_insert.o below). It provides both windows on the
# one array, so equipment.h is a dependency too.
inventory.o: inventory.h equipment.h $(HEADERS_COMMON)
io.o: $(HEADERS_FULL)
item_enchant.o: floor_items.h missile_serial.h $(HEADERS_FULL)
item_ident.o: item_ident.h $(HEADERS_FULL)
magic.o: $(HEADERS_FULL)
main.o: $(HEADERS_FULL)
# map_view.c came out of misc4.c and misc1.c (#54). It includes externs.h, and
# reads the map, what lies on it (items and monsters), the panel, where the
# player stands, whether the player is blind, hallucinating or running.
map_view.o: dungeon_map.h floor_items.h monster_list.h panel.h player_pos.h \
            player_status_flags.h player_timed_effects.h running.h $(HEADERS_FULL)
# missile_serial.c does not include externs.h either (MAX_SHORT comes from
# constant.h), so HEADERS_COMMON is enough.
missile_serial.o: missile_serial.h $(HEADERS_COMMON)
# The two halves of the interrupted inventory command (#18-11-3). Neither
# includes externs.h; inven_command_state.c also needs screen_touched.h, because
# suspending is one act that touches both.
inven_command_state.o: inven_command_state.h screen_touched.h $(HEADERS_COMMON)
screen_touched.o: screen_touched.h $(HEADERS_COMMON)
# The two ways a turn can be cut short (#18-11-4): this level is finished, and a
# teleport is waiting. Neither includes externs.h; level_exit.c declares the two
# symbols it touches itself (its own flag and dun_level, which belongs to another
# group and stays a global for now).
level_exit.o: level_exit.h $(HEADERS_COMMON)
pending_teleport.o: pending_teleport.h $(HEADERS_COMMON)
# The third way a turn can end: the input has run out (#18-11-5). No externs.h
# either; it declares the one symbol it touches itself while that symbol is still
# a global.
input_ended.o: input_ended.h $(HEADERS_COMMON)
# The fourth: whether the player is running, and how far (#18-11-6). No
# externs.h either; the direction of the run stays a static of moria2.c.
running.o: running.h $(HEADERS_COMMON)
# What the game remembers about the command being typed (#18-11-7): the repeat
# count, whether the direction comes from memory, and the command before this
# one. No externs.h either; the remembered direction stays a static of get_dir().
command_state.o: command_state.h $(HEADERS_COMMON)
# The dungeon and what is in it. Neither includes externs.h, so HEADERS_COMMON
# is enough for both (the same as object_levels.o below).
# Whose turn it is (#18-14-1): an int and one comparison, nothing else.
monster_turn.o: monster_turn.h $(HEADERS_COMMON)
# Where each level's monsters sit in the definition table (#18-14-2). It
# declares the walk over the definitions itself rather than including
# externs.h, which would drag in ncurses for the sake of one loop.
monster_levels.o: monster_levels.h $(HEADERS_COMMON)
# How many monsters have been bred on this level (#18-14-3). One int16_t, one
# comparison against MAX_MON_MULT (constant.h) and two steps; no externs.h.
monster_breeding.o: monster_breeding.h $(HEADERS_COMMON)
# The monsters standing on this level, and how much of the table is in use
# (#18-14-4). Does not include externs.h -- during A/B it reaches the storage in
# monsters.c through three hand-written externs, and in C the storage moves here.
monster_list.o: monster_list.h $(HEADERS_COMMON)
# monster_place.c came out of misc1.c (#54). It includes externs.h, and reads
# the map and its size, the depth, the monster list and levels, whose turn it
# is, where the player stands, the player's speed and whether the game is won.
monster_place.o: dungeon_level.h dungeon_map.h dungeon_size.h monster_levels.h \
                 monster_list.h monster_turn.h player_pos.h player_speed.h \
                 score_death.h $(HEADERS_FULL)
# How tall and how wide this level is (#18-14-5). Two int16_t and one setter
# that takes both halves, so no caller can change half of the size. It needs
# nothing but <stdint.h> and its own header -- not even constant.h.
dungeon_size.o: dungeon_size.h
# Which level the game is on now (#18-14-6). One int16_t, one predicate for
# the question six callers wrote three different ways, and one setter. Like
# dungeon_size.o it needs nothing but its own header and the standard ones.
dungeon_level.o: dungeon_level.h
# The things lying on the floor of this level (#18-14-7). Unlike the two above
# it is not self-contained: an empty row is a copy of object_list[OBJ_NOTHING],
# so it calls invcopy() (desc.c). The prototype is declared by hand, so
# HEADERS_COMMON is still enough.
floor_items.o: floor_items.h $(HEADERS_COMMON)
# dungeon_map.c reaches the one cave table by a hand-written extern until
# #18-14-8C moves the storage in, and calls nothing outside itself.
dungeon_map.o: dungeon_map.h $(HEADERS_COMMON)
monsters.o: $(HEADERS_COMMON)
# object_levels.c does not include externs.h (it declares the three things it
# needs itself), so HEADERS_COMMON is enough -- the same as inventory.o above.
object_levels.o: object_levels.h $(HEADERS_COMMON)
moria1.o: burden.h $(HEADERS_FULL)
moria2.o: $(HEADERS_FULL)
moria3.o: $(HEADERS_FULL)
look.o: dungeon_map.h floor_items.h monster_list.h panel.h player_pos.h \
        player_timed_effects.h $(HEADERS_FULL)
# object_place.c came out of misc1.c (#54). It includes externs.h, and reads
# the map, its size, what lies on the floor and where the player stands.
object_place.o: dungeon_map.h dungeon_size.h floor_items.h player_pos.h $(HEADERS_FULL)
options_menu.o: options.h $(HEADERS_FULL)
panel.o: panel.h $(HEADERS_FULL)
player.o: $(HEADERS_COMMON)
# player_pos.c does not include externs.h, so HEADERS_COMMON is enough here
# (the same as stats.o and str_insert.o below).
# player_light.c does not include externs.h either, so HEADERS_COMMON is enough.
player_light.o: player_light.h $(HEADERS_COMMON)
player_pos.o: player_pos.h $(HEADERS_COMMON)
potions.o: $(HEADERS_FULL)
prayer.o: $(HEADERS_FULL)
recall.o: $(HEADERS_FULL)
render.o: render.h
render_ncurses.o: render.h backend_ncurses.h
# rnd.c includes externs.h, and since #54 also holds what was misc1.c's
# randint() and the rest.
rnd.o: $(HEADERS_FULL)
view_observer.o: view_observer.h
input.o: input.h
input_ncurses.o: input.h backend_ncurses.h
platform.o: platform.h backend_ncurses.h render.h input.h
save.o: burden.h $(HEADERS_FULL)
scrolls.o: $(HEADERS_FULL)
sets.o: constant.h config.h
signal_flags.o: signal_flags.h
signals.o: $(HEADERS_FULL) signal_flags.h
spells.o: $(HEADERS_FULL)
# spells_known.c does not include externs.h either, so HEADERS_COMMON is enough.
spells_known.o: spells_known.h $(HEADERS_COMMON)
staffs.o: device.h $(HEADERS_FULL)
# progress.c does not include externs.h, so HEADERS_COMMON is enough here
# (the same as stats.o and str_insert.o below).
progress.o: progress.h $(HEADERS_COMMON)
# score_death.c does not include externs.h either, so HEADERS_COMMON is enough.
score_death.o: score_death.h $(HEADERS_COMMON)
# save_state.c does not include externs.h either, but it does read the other
# two windows of this batch, so its headers are here as well.
save_state.o: save_state.h progress.h \
              score_death.h $(HEADERS_COMMON)
stores.o: stores.h $(HEADERS_FULL)
# store_price.c came out of store1.c (#55). It includes externs.h, and reads
# the stores and the race of the customer (a column of the price table).
store_price.o: stores.h player_race.h $(HEADERS_FULL)
# store_stock.c is the rest of store1.c (#55). It includes externs.h, and
# makes candidate items on the floor list to restock the stores.
store_stock.o: stores.h floor_items.h $(HEADERS_FULL)
# store_haggle.c is what was left of store2.c (#55). It includes externs.h,
# and reads the price of the item and the race of the player.
store_haggle.o: stores.h store_haggle.h player_race.h progress.h stats.h \
                str_insert.h $(HEADERS_FULL)
# store_ui.c came out of store2.c (#55). It includes externs.h, and calls the
# haggling through store_haggle.h.
store_ui.o: inven_command_state.h inventory.h stores.h store_haggle.h messages.h \
            player_gold.h progress.h stats.h $(HEADERS_FULL)
stats.o: stats.h $(HEADERS_COMMON)
# str_insert.c does not include externs.h, so HEADERS_COMMON is enough here
# (the same as stats.o above).
str_insert.o: str_insert.h $(HEADERS_COMMON)
tables.o: $(HEADERS_COMMON)
treasure.o: $(HEADERS_COMMON)
variable.o: $(HEADERS_COMMON)
wands.o: device.h $(HEADERS_FULL)
# enter_wiz_mode() came from misc3.c (#42).
wizard.o: progress.h score_death.h $(HEADERS_FULL)
# hit_rolls.c and player_damage.c came from misc3.c (#42). They include externs.h.
hit_rolls.o: player_class.h player_level.h $(HEADERS_FULL)
player_damage.o: player_class.h player_level.h player_saving_throw.h stats.h \
                 $(HEADERS_FULL)
# object_alloc.c came from misc3.c (#42). It includes externs.h.
object_alloc.o: dungeon_level.h dungeon_map.h dungeon_size.h floor_items.h \
                object_levels.h player_pos.h $(HEADERS_FULL)
# inven_ops.c and player_move.c came from misc3.c (#42). They include externs.h.
inven_ops.o: burden.h dungeon_map.h equipment.h floor_items.h inventory.h \
             item_ident.h player_body_weight.h player_pos.h \
             player_status_flags.h $(HEADERS_FULL)
player_move.o: dungeon_map.h dungeon_size.h pending_teleport.h player_pos.h \
               $(HEADERS_FULL)
# screen_fields.c came from misc3.c (#42). It includes externs.h.
screen_fields.o: screen_fields.h $(HEADERS_FULL)
# char_screen.c came from misc3.c (#42). It includes externs.h.
char_screen.o: abilities.h player_bio.h player_body_weight.h player_class.h \
              player_display_numbers.h player_gold.h player_hp.h \
              player_level.h player_mana.h player_race.h save_state.h \
              screen_fields.h $(HEADERS_FULL)
# status_line.c came from misc3.c (#42). It includes externs.h.
status_line.o: command_state.h dungeon_level.h player_bio.h player_class.h \
              player_display_numbers.h player_gold.h player_hp.h \
              player_level.h player_mana.h player_race.h player_resting.h \
              player_speed.h player_spells_to_learn.h player_status_flags.h \
              player_timed_effects.h progress.h score_death.h screen_fields.h \
              $(HEADERS_FULL)
# stat_ops.c came from misc3.c (#42). It includes externs.h.
stat_ops.o: player_class.h player_status_flags.h $(HEADERS_FULL)
# spellbook.c came from misc3.c (#42). It includes externs.h.
spellbook.o: inventory.h player_class.h player_level.h player_mana.h \
            player_spells_to_learn.h player_status_flags.h \
            player_timed_effects.h spells_known.h stats.h $(HEADERS_FULL)
# level_ops.c is what was left of misc3.c (#42). It includes externs.h.
level_ops.o: hp_table.h player_class.h player_hp.h player_level.h \
             player_status_flags.h screen_fields.h stats.h $(HEADERS_FULL)
player_melee.o: dungeon_map.h equipment.h monster_list.h player_body_weight.h \
                player_class.h player_level.h player_timed_effects.h \
                $(HEADERS_FULL)
throw.o: dungeon_map.h equipment.h floor_items.h inventory.h monster_list.h \
         panel.h player_attack_bonuses.h player_base_to_hit.h player_class.h \
         player_level.h player_pos.h player_status_flags.h \
         player_timed_effects.h $(HEADERS_FULL)
traps.o: dungeon_map.h floor_items.h monster_list.h player_class.h \
         player_disarm.h player_level.h player_pos.h player_timed_effects.h \
         stats.h $(HEADERS_FULL)
terrain_commands.o: burden.h command_state.h dungeon_map.h equipment.h \
                    floor_items.h monster_list.h player_body_weight.h \
                    player_pos.h player_search_skill.h player_timed_effects.h \
                    $(HEADERS_FULL)
