# gnuboy PumpkinOS port.
#
# Builds the gnuboy core plus the PalmOS glue in this directory into
# GnuBoy.dlib, wraps it into GnuBoy.prc with pilrc/prcbuild from the
# PumpkinOS tree, and installs the PRC into the PumpkinOS VFS
# (vfs/app_install), where it is deployed at next boot.
#
# Usage:
#   make PUMPKIN=/path/to/PumpkinOS        (default below)

PUMPKIN ?= ${HOME}/Downloads/PumpkinOS
GNUBOY := $(abspath ../..)

PROGRAM = GnuBoy
APPNAME = GNUBoy
APPID   = GBoy

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# The SYS_* triple must match the one baked into $(PUMPKIN)/bin/libpumpkin:
# at runtime the app's native code is loaded from the PRC resource
# 'dlib' id = SYS_OS*64 + SYS_CPU*8 + SYS_SIZE.
SYS_OS ?= 1
# mirror $(PUMPKIN)/src/common.mak: macOS reports 64-bit ARM as "arm64",
# Linux as "aarch64"; only other arm* machines are 32-bit
ifeq ($(UNAME_M),arm64)
SYS_CPU ?= 1
SYS_SIZE ?= 2
else ifeq ($(UNAME_M),aarch64)
SYS_CPU ?= 1
SYS_SIZE ?= 2
else ifeq ($(findstring arm,$(UNAME_M)),arm)
SYS_CPU ?= 1
SYS_SIZE ?= 1
else ifeq ($(UNAME_M),x86_64)
SYS_CPU ?= 2
SYS_SIZE ?= 2
else ifeq ($(UNAME_M),i386)
SYS_CPU ?= 2
SYS_SIZE ?= 1
else ifeq ($(UNAME_M),i686)
SYS_CPU ?= 2
SYS_SIZE ?= 1
else ifeq ($(UNAME_M),riscv64)
SYS_CPU ?= 3
SYS_SIZE ?= 2
else
SYS_CPU ?= 3
SYS_SIZE ?= 1
endif

SYS_ENDIAN ?= 1
DLIBID := $(shell expr $(SYS_OS) \* 64 + $(SYS_CPU) \* 8 + $(SYS_SIZE))
DLIB := $(shell printf "dlib%04x.bin" $(DLIBID))

PILRC = $(PUMPKIN)/tools/pilrc -q -allowLargeRes
PRCBUILD = $(PUMPKIN)/tools/prcbuild

ifeq ($(UNAME_S),Darwin)
CC = clang -arch x86_64 -arch arm64 -mmacosx-version-min=10.14
LINKFLAGS = -dynamiclib
OSDEFS = -DDARWIN -D_DARWIN_C_SOURCE -Wno-invalid-pp-token -Wno-string-plus-int -Wno-typedef-redefinition
else
CC = gcc
LINKFLAGS = -shared
OSDEFS = -DLINUX
endif

PUMPDEFS = -DSYS_CPU=$(SYS_CPU) -DSYS_SIZE=$(SYS_SIZE) -DSYS_OS=$(SYS_OS) \
           -DSYS_ENDIAN=$(SYS_ENDIAN) -DUNIX

PALMOS = $(PUMPKIN)/src/PalmOS
PUMPKIN_INCLUDES = -I. -I$(PUMPKIN)/src/libpit -I$(PUMPKIN)/src/libpumpkin -I$(PUMPKIN)/src/font \
           -I$(PALMOS) -I$(PALMOS)/Core -I$(PALMOS)/Core/System -I$(PALMOS)/Dynamic \
           -I$(PALMOS)/Core/UI -I$(PALMOS)/Libraries -I$(PALMOS)/Libraries/PalmOSGlue \
           -I$(PALMOS)/Core/Hardware -I$(PALMOS)/Extensions/ExpansionMgr

OPT ?= -O3
COMMONFLAGS = -std=gnu11 -Wall -Wno-unknown-pragmas -Wno-unused-function -Wno-multichar \
              -fsigned-char -fPIC $(OPT) -DGNUBOY_PUMPKIN -DIS_LITTLE_ENDIAN -D_GNU_SOURCE $(OSDEFS)

# the emulator core only sees the gnuboy tree (its sys.h/debug.h clash
# with the libpit headers of the same name)
CORECFLAGS = $(COMMONFLAGS) -I$(GNUBOY)
# the glue only sees the PumpkinOS tree and reaches gnuboy headers via ../../
GLUECFLAGS = $(COMMONFLAGS) $(PUMPDEFS) $(PUMPKIN_INCLUDES)

GLUE_SRCS = pumpkin.c pumpkin-audio.c

CORE_SRCS = lcd.c refresh.c lcdc.c palette.c cpu.c mem.c rtc.c hw.c sound.c \
            events.c keytable.c menu.c loader.c save.c debug.c emu.c \
            rccmds.c rckeys.c rcvars.c rcfile.c exports.c \
            split.c path.c inflate.c miniz_tinfl.c \
            xz/xz_crc32.c xz/xz_crc64.c xz/xz_dec_lzma2.c xz/xz_dec_stream.c xz/xz_dec_bcj.c

OBJDIR = obj
GLUE_OBJS = $(patsubst %.c,$(OBJDIR)/glue/%.o,$(GLUE_SRCS))
CORE_OBJS = $(patsubst %.c,$(OBJDIR)/core/%.o,$(CORE_SRCS))
OBJS = $(GLUE_OBJS) $(CORE_OBJS)

all: $(PROGRAM).prc

$(OBJDIR)/glue/%.o: %.c pumpkin-glue.h resource.h
	@mkdir -p $(dir $@)
	@echo Compiling $<
	@$(CC) $(GLUECFLAGS) -c -o $@ $<

$(OBJDIR)/core/%.o: $(GNUBOY)/%.c
	@mkdir -p $(dir $@)
	@echo Compiling $<
	@$(CC) $(CORECFLAGS) -c -o $@ $<

$(PROGRAM).dlib: $(OBJS)
	@echo Linking $(PROGRAM).dlib
	@$(CC) $(LINKFLAGS) -o $@ $(OBJS) -L$(PUMPKIN)/bin -lpumpkin -lpit
ifeq ($(UNAME_S),Darwin)
	@# libpumpkin.dylib carries a relative install name; refer to it through
	@# the rpath of the pumpkin executable instead, like libos does
	@install_name_tool -change ../../bin/libpumpkin.dylib @rpath/libpumpkin.dylib $@
	@install_name_tool -change ../../bin/libpit.dylib @rpath/libpit.dylib $@ 2>/dev/null || true
	@codesign -f -s "Developer ID Application" $@ 2>/dev/null || codesign -f -s - $@
endif

res.flag: resource.rcp resource.h gnuboy.bmp
	@echo Compiling resources
	@mkdir -p resources
	@$(PILRC) resource.rcp resources
	@touch res.flag

$(PROGRAM).prc: $(PROGRAM).dlib res.flag
	@echo Building $(PROGRAM).prc
	@cp $(PROGRAM).dlib $(DLIB)
	@$(PRCBUILD) -f $(PROGRAM).prc -t appl -c $(APPID) -n "$(APPNAME)" $(DLIB) resources/*
	@rm -f $(DLIB)
	@echo Installing $(PROGRAM).prc into $(PUMPKIN)/vfs/app_install
	@mkdir -p $(PUMPKIN)/vfs/app_install
	@cp $(PROGRAM).prc $(PUMPKIN)/vfs/app_install/

clean:
	rm -rf $(OBJDIR) resources res.flag $(PROGRAM).dlib $(PROGRAM).prc $(DLIB)

.PHONY: all clean
