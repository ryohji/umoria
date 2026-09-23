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

# Include path for headers in src directory
INCLUDES = -I$(SRCDIR)

# Combine all compiler flags
CFLAGS = $(STD) $(POSIX) $(WARNINGS) $(DEBUG_FLAGS) $(OPT_FLAGS) $(INCLUDES)

# Linker flags
LDFLAGS =
CURSES = -lncurses

# Source directory
SRCDIR = src
VPATH = $(SRCDIR)

SRCS = main.c misc1.c misc2.c misc3.c misc4.c store1.c files.c io.c \
	create.c desc.c generate.c sets.c dungeon.c creature.c death.c \
	eat.c help.c magic.c potions.c prayer.c save.c staffs.c wands.c device.c \
	item_ident.c abilities.c options.c messages.c \
	scrolls.c spells.c wizard.c store2.c signals.c signal_flags.c \
	render.c render_ncurses.c view_observer.c game_state.c \
	input.c input_ncurses.c platform.c panel.c stores.c stats.c str_insert.c \
	inventory.c progress.c score_death.c save_state.c player_pos.c \
	hp_table.c \
	moria1.c moria2.c moria3.c moria4.c monsters.c treasure.c variable.c \
	rnd.c recall.c player.c tables.c

OBJS = main.o misc1.o misc2.o misc3.o misc4.o store1.o files.o io.o \
	create.o desc.o generate.o sets.o dungeon.o creature.o death.o \
	eat.o help.o magic.o potions.o prayer.o save.o staffs.o wands.o device.o \
	item_ident.o abilities.o options.o messages.o \
	scrolls.o spells.o wizard.o store2.o signals.o signal_flags.o \
	render.o render_ncurses.o view_observer.o game_state.o \
	input.o input_ncurses.o platform.o panel.o stores.o stats.o str_insert.o \
	inventory.o progress.o score_death.o save_state.o player_pos.o \
	hp_table.o \
	moria1.o moria2.o moria3.o moria4.o monsters.o treasure.o variable.o \
	rnd.o recall.o player.o tables.o

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
# These ensure that object files are rebuilt when headers change
HEADERS_COMMON = $(SRCDIR)/constant.h $(SRCDIR)/types.h $(SRCDIR)/config.h
HEADERS_FULL = $(HEADERS_COMMON) $(SRCDIR)/externs.h

create.o: $(HEADERS_FULL)
creature.o: $(HEADERS_FULL)
death.o: $(HEADERS_FULL)
desc.o: $(HEADERS_FULL)
device.o: $(SRCDIR)/device.h $(HEADERS_FULL)
dungeon.o: $(HEADERS_FULL)
eat.o: $(HEADERS_FULL)
files.o: $(HEADERS_FULL)
game_state.o: $(SRCDIR)/game_state.h $(HEADERS_FULL)
generate.o: $(HEADERS_FULL)
help.o: $(HEADERS_FULL)
# hp_table.c does not include externs.h either, so HEADERS_COMMON is enough.
hp_table.o: $(SRCDIR)/hp_table.h $(HEADERS_COMMON)
# inventory.c does not include externs.h, so HEADERS_COMMON is enough here
# (the same as stats.o and str_insert.o below). It provides both windows on the
# one array, so equipment.h is a dependency too.
inventory.o: $(SRCDIR)/inventory.h $(SRCDIR)/equipment.h $(HEADERS_COMMON)
io.o: $(HEADERS_FULL)
item_ident.o: $(SRCDIR)/item_ident.h $(HEADERS_FULL)
magic.o: $(HEADERS_FULL)
main.o: $(HEADERS_FULL)
misc1.o: $(HEADERS_FULL)
misc2.o: $(HEADERS_FULL)
misc3.o: $(HEADERS_FULL)
misc4.o: $(HEADERS_FULL)
monsters.o: $(HEADERS_COMMON)
moria1.o: $(HEADERS_FULL)
moria2.o: $(HEADERS_FULL)
moria3.o: $(HEADERS_FULL)
moria4.o: $(HEADERS_FULL)
panel.o: $(SRCDIR)/panel.h $(HEADERS_FULL)
player.o: $(HEADERS_COMMON)
# player_pos.c does not include externs.h, so HEADERS_COMMON is enough here
# (the same as stats.o and str_insert.o below).
player_pos.o: $(SRCDIR)/player_pos.h $(HEADERS_COMMON)
potions.o: $(HEADERS_FULL)
prayer.o: $(HEADERS_FULL)
recall.o: $(HEADERS_FULL)
render.o: $(SRCDIR)/render.h
render_ncurses.o: $(SRCDIR)/render.h $(SRCDIR)/backend_ncurses.h
rnd.o: $(HEADERS_COMMON)
view_observer.o: $(SRCDIR)/view_observer.h
input.o: $(SRCDIR)/input.h
input_ncurses.o: $(SRCDIR)/input.h $(SRCDIR)/backend_ncurses.h
platform.o: $(SRCDIR)/platform.h $(SRCDIR)/backend_ncurses.h $(SRCDIR)/render.h $(SRCDIR)/input.h
save.o: $(HEADERS_FULL)
scrolls.o: $(HEADERS_FULL)
sets.o: $(SRCDIR)/constant.h $(SRCDIR)/config.h
signal_flags.o: $(SRCDIR)/signal_flags.h
signals.o: $(HEADERS_FULL) $(SRCDIR)/signal_flags.h
spells.o: $(HEADERS_FULL)
staffs.o: $(SRCDIR)/device.h $(HEADERS_FULL)
# progress.c does not include externs.h, so HEADERS_COMMON is enough here
# (the same as stats.o and str_insert.o below).
progress.o: $(SRCDIR)/progress.h $(HEADERS_COMMON)
# score_death.c does not include externs.h either, so HEADERS_COMMON is enough.
score_death.o: $(SRCDIR)/score_death.h $(HEADERS_COMMON)
# save_state.c does not include externs.h either, but it does read the other
# two windows of this batch, so its headers are here as well.
save_state.o: $(SRCDIR)/save_state.h $(SRCDIR)/progress.h \
              $(SRCDIR)/score_death.h $(HEADERS_COMMON)
store1.o: $(SRCDIR)/stores.h $(HEADERS_FULL)
store2.o: $(SRCDIR)/stores.h $(HEADERS_FULL)
stores.o: $(SRCDIR)/stores.h $(HEADERS_FULL)
stats.o: $(SRCDIR)/stats.h $(HEADERS_COMMON)
# str_insert.c does not include externs.h, so HEADERS_COMMON is enough here
# (the same as stats.o above).
str_insert.o: $(SRCDIR)/str_insert.h $(HEADERS_COMMON)
tables.o: $(HEADERS_COMMON)
treasure.o: $(HEADERS_COMMON)
variable.o: $(HEADERS_COMMON)
wands.o: $(SRCDIR)/device.h $(HEADERS_FULL)
wizard.o: $(HEADERS_FULL)
