# This Makefile is derived from my "template" Makefile : although it has been
# edited with love and care, some unnecessary rules may remain.
#
# When starting a new project, the following sections will most likely need to
# be edited (and should not need to be edited again) :
# - Program
# - Paths
# - Files
# - Flags
#
# Of course, some projects will need special care and others sections may need
# to be edited as well.
#
# The section 'Actual building' is the section that will constantly be edited
# as the project grows and new files are added. The most naïve way is to merely
# add a rule for each new `.o` object file and add it to the `PROGRAM_OBJECTS`
# variable.
#
# For instance:
#     PROGRAM_OBJECTS: main.o
#
#     main.o: main.c version.h

################################################################################
# Completion monitoring utility.
################################################################################

ifneq ($(words $(MAKECMDGOALS)),1)
.DEFAULT_GOAL = release
%:
	@$(MAKE) $@ --no-print-directory -rRf $(firstword $(MAKEFILE_LIST))
else
ifndef ECHO
	T := $(shell $(MAKE) $(MAKECMDGOALS) --no-print-directory \
		-nrRf $(firstword $(MAKEFILE_LIST)) \
		ECHO="COUNTTHIS" | grep -c "COUNTTHIS")

N := x
C = $(words $N)$(eval N := x $N)
ECHO = echo -e "\x1B[1m`expr " [\`expr $C '*' 100 / $T\`" : '.*\(....\)$$'`%]\x1B[0m"
endif

################################################################################
# Special targets
################################################################################

# Some variables are Target-specific (they have a value only when a specific
# target was used).
#
# Examples:
#
#     $ make release
#     $ make debug
#

release: all
debug: all

################################################################################
# Program
################################################################################

PROGRAM_NAME = nsa

################################################################################
# Paths
################################################################################

PATH_SRC = src
PATH_OBJ = obj
PATH_DOC = build/doc
PATH_LIB = lib
PATH_BIN = bin
PATH_MAN = man
PATH_INCLUDE = include

################################################################################
# Files
################################################################################

vpath %.h $(PATH_INCLUDE) $(PATH_INCLUDE)/$(PROGRAM_NAME)
vpath %.h build/autogen/$(PATH_INCLUDE) build/autogen/$(PATH_INCLUDE)/$(PROGRAM_NAME)

vpath %.c $(PATH_SRC) $(PATH_SRC)/$(PROGRAM_NAME)
vpath %.o $(PATH_OBJ)
vpath %.a $(PATH_LIB)
vpath %.so $(PATH_LIB)

vpath $(PROGRAM_NAME) $(PATH_BIN)
vpath factoint28inter $(PATH_BIN)

################################################################################
# Flags, first pass.
################################################################################

debug: FLAGS_CC_DEBUG = -g
debug: FLAGS_CC_WARNINGS = -W -Wall -Wextra -Wfloat-equal -Wswitch-default \
	-Winit-self -Wshadow -Wbad-function-cast -Wcast-qual -Wcast-align \
	-Wconversion -Wlogical-op -Wstrict-prototypes -Wnested-externs

release: FLAGS_CC_OPTIMIZATIONS = -O3 -march=native

FLAGS_CC_INCLUDE = -I$(PATH_INCLUDE) -Ibuild/autogen/$(PATH_INCLUDE)
FLAGS_CC_LIB = -L$(PATH_LIB)
FLAGS_CC_MINIMAL = -std=gnu99 $(FLAGS_CC_INCLUDE)

################################################################################
# Conventionnal (mostly) variables
################################################################################

# Users usually expect Makefiles to provide support for various variables in
# order to ease configuration.
# For instance:
#     $ make CC=clang
#     $ make CFLAGS="-O2"
#     $ make install PREFIX=/opt/nsa
#     $ make LDFLAGS="-L/opt/mylib/lib"

# These are the only variables the user is *allowed* (nothing prevents the user
# from overriding other variables...) to override.

## Shell
SHELL = /bin/sh

## Programs
# These variables usually are implicitly defined.
# Ensure they are defined.
CC = mpicc
AR = ar
RM = rm -rf

# Extras (not really conventionnal)
DOC = doxygen
MKDIR = mkdir -p
CP = cp
#ECHO = echo

# Install
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

## Flags
ARFLAGS = crvs
CFLAGS = $(FLAGS_CC_DEBUG) $(FLAGS_CC_WARNINGS) $(FLAGS_CC_OPTIMIZATIONS)
LDFLAGS =
LDLIBS =

## Directories
# In most situations, changing only PREFIX is enough.
#
#     $ make install PREFIX=/opt/nsa
PREFIX = /usr/local
EXEC_PREFIX = $(PREFIX)
BINDIR = $(EXEC_PREFIX)/bin
SBINDIR = $(EXEC_PREFIX)/sbin
LIBEXECDIR = $(EXEC_PREFIX)/libexec
DATAROOTDIR = $(PREFIX)/share
DATADIR = $(DATAROOTDIR)
SYSCONFDIR = $(PREFIX)/etc
SHAREDSTATEDIR = $(PREFIX)/com
LOCALSTATEDIR = $(PREFIX)/var
RUNSTATEDIR = $(LOCALSTATEDIR)/run
INCLUDEDIR = $(PREFIX)/include
DOCDIR = $(DATAROOTDIR)/doc/$(PROGRAM_NAME)
LIBDIR = $(EXEC_PREFIX)/lib
MANDIR = $(DATAROOTDIR)/man

################################################################################
# Flags, second pass
################################################################################

# The user is allowed to override some flags. But there are minimal requirements.
# Ensure these requirements are set even if the flags are empty.
override CFLAGS += $(FLAGS_CC_MINIMAL)
override LDLIBS += $(FLAGS_CC_LIB)
override LDFLAGS += -lmpi -lm -lgmp

################################################################################
# Actual building
################################################################################

PROGRAM_OBJECTS = main.o eratostene.o

all: $(PROGRAM_NAME) factoint28inter | bin_dir

## Executable
$(PROGRAM_NAME): $(PROGRAM_OBJECTS) | bin_dir
	@$(ECHO) "\x1B[38;5;106mLinking C executable \x1B[1m$@\x1B[0m"
	@$(CC) -o $(PATH_BIN)/$@ \
		$(patsubst %.o,$(PATH_OBJ)/%.o, $(patsubst $(PATH_OBJ)/%,%, $^)) \
		$(LDFLAGS) $(LDLIBS)

factoint28inter: factoint28inter.o | bin_dir
	@$(ECHO) "\x1B[38;5;106mLinking C executable \x1B[1m$@\x1B[0m"
	@$(CC) -o $(PATH_BIN)/$@ \
		$(patsubst %.o,$(PATH_OBJ)/%.o, $(patsubst $(PATH_OBJ)/%,%, $^)) \
		$(LDFLAGS) $(LDLIBS)

## Object files
# Generate .o object files.
%.o: %.c | obj_dir
	@$(ECHO) "\x1B[38;5;33mBuilding C object \x1B[1m$@\x1B[0m"
	@$(CC) $(CFLAGS) -o $(PATH_OBJ)/$@ -c $<

# Rules for object files
main.o: main.c version.h eratostene.h
eratostene.o: eratostene.c eratostene.h tools.h
factoint28inter.o: factoint28inter.c

################################################################################
# Documentation
################################################################################

doc:
	@$(DOC) build/autogen/Doxyfile

################################################################################
# Directories
################################################################################

obj_dir:
	@$(MKDIR) $(PATH_OBJ)

lib_dir:
	@$(MKDIR) $(PATH_LIB)

bin_dir:
	@$(MKDIR) $(PATH_BIN)

################################################################################
# (Un)Installing
################################################################################

# DESTDIR: Support staged (un)installs.
# DESTDIR should be defined as a command line argument (thus it is empty by
# default).
#
#     $ make install DESTDIR=/tmp/stage
#     $ make uninstall DESTDIR=/tmp/stage
#
# Note: DESTDIR is different from PREFIX!
#
# Warning: PREFIX should have a leading '/' (the default does) when overriding
#          DESTDIR.

install:
	@$(INSTALL_PROGRAM) -D $(PATH_BIN)/$(PROGRAM_NAME) \
		$(DESTDIR)$(BINDIR)/$(PROGRAM_NAME) \
		&& echo "install: $(DESTDIR)$(BINDIR)/$(PROGRAM_NAME)"
	@$(INSTALL_DATA) -D $(PATH_MAN)/man1/$(PROGRAM_NAME).1 \
		$(DESTDIR)$(MANDIR)/man1/$(PROGRAM_NAME).1 \
		&& echo "install: $(DESTDIR)$(MANDIR)/man1/$(PROGRAM_NAME).1"

uninstall:
	@$(RM) $(DESTDIR)$(BINDIR)/$(PROGRAM_NAME) \
		&& echo "uninstall: $(DESTDIR)$(BINDIR)/$(PROGRAM_NAME)"
	@$(RM) $(DESTDIR)$(MANDIR)/man1/$(PROGRAM_NAME).1 \
		&& echo "uninstall: $(DESTDIR)$(MANDIR)/man1/$(PROGRAM_NAME).1"

################################################################################
# Cleaning
################################################################################

clean:
	@$(RM) $(PATH_BIN) $(PATH_OBJ) $(PATH_LIB)
	@$(ECHO) "\x1B[1m\x1B[38;5;226mClean.\x1B[0m"

cleandoc: clean_doc
clean_doc:
	@$(RM) $(PATH_DOC)
	@$(ECHO) "\x1B[1m\x1B[38;5;220mDoc cleansed.\x1B[0m"

cleanall: clean_all
clean_all: clean clean_doc
	@$(ECHO) "\x1B[1m\x1B[38;5;214mVery clean.\x1B[0m"

distclean: dist_clean
dist_clean: clean_all
	@$(RM) build
	@$(ECHO) "\x1B[1m\x1B[38;5;208mSuper clean.\x1B[0m"

endif
