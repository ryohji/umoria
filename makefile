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

# Each library's internal/ headers are visible only to its own objects.
$(foreach l,$(INTERNAL_LIBS),$(eval $(notdir $(patsubst %.c,%.o,$(filter $(l)/%,$(SRCS)))): INCLUDES += -I$(SRCDIR)/$(l)/internal))

# app/ objects linked directly (not in any archive). APP_SRCS also includes
# the two ncurses files (platform layer, but not in libplatform.a).
APP_OBJS = $(notdir $(patsubst %.c,%.o,$(filter app/%,$(SRCS)))) \
           $(notdir $(patsubst %.c,%.o,$(filter %ncurses.c,$(APP_SRCS))))

# Static libraries (one per layer/sub-layer)
LIBS = $(addprefix lib,$(addsuffix .a,$(LIB_LINK_ORDER)))

LIBFILES = splash.hlp origcmds.hlp owizcmds.hlp roglcmds.hlp rwizcmds.hlp \
	version.hlp welcome.hlp

DOCS = manual.md

# Phony targets (not actual files)
.PHONY: all clean install TAGS help

# Default target
all: $(TARGET)

# Link the final executable: app objects + libraries in layer order + ncurses
$(TARGET): $(APP_OBJS) $(LIBS)
	@echo "Linking $(TARGET)..."
	$(CC) -o $(TARGET) $(APP_OBJS) $(LIBS) $(LDFLAGS) $(CURSES)
	@echo "Build complete: $(TARGET)"

# Build each library from its layer's objects
$(foreach lib,$(LIB_LINK_ORDER),$(eval \
lib$(lib).a: $(notdir $(LIB_SRCS_$(lib):.c=.o)) ; \
	@echo "Building lib$(lib).a..." ; \
	rm -f $$@ ; \
	$(AR) rcs $$@ $$^ \
))


# The whole game with the display backend of tests/headless_backend.c instead
# of ncurses (makefile.test golden runs it). Same objects and flags as $(TARGET).
HEADLESS = umoria_headless
HEADLESS_OBJS = $(filter-out platform_init.o %ncurses.o,$(APP_OBJS)) headless_backend.o

$(HEADLESS): $(HEADLESS_OBJS) $(LIBS)
	$(CC) -o $(HEADLESS) $(HEADLESS_OBJS) $(LIBS) $(LDFLAGS)

headless_backend.o: tests/headless_backend.c
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

# Pattern rule for compiling C source files
# -MMD -MP writes a .d beside each .o listing the headers it included.
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

-include $(OBJS:.o=.d)

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
	@rm -f $(OBJS:.o=.d)
	@rm -f $(LIBS)
	@rm -f $(TARGET) $(HEADLESS) headless_backend.o headless_backend.d
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
