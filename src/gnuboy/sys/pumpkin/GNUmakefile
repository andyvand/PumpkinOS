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

ifeq ($(OSNAME),)
OSNAME ?= $(shell uname -s)
endif

ifeq ($(MACHINE),)
ifeq ($(OSNAME),vita)
# the PlayStation Vita is always a 32-bit ARM (see $(PUMPKIN)/src/common.mak)
MACHINE := arm
else ifeq ($(OSNAME),switch)
# the Nintendo Switch is always a 64-bit ARM
MACHINE := aarch64
else ifeq ($(OSNAME),psp)
# the PlayStation Portable is always a 32-bit MIPS
MACHINE := mips
else ifeq ($(OSNAME),ps2)
# the PlayStation 2 is always a MIPS (n32: 32-bit pointers)
MACHINE := mips
else ifeq ($(OSNAME),gamecube)
# the Nintendo GameCube is always a 32-bit big endian PowerPC
MACHINE := powerpc
else ifeq ($(OSNAME),wii)
# the Nintendo Wii as well
MACHINE := powerpc
else
MACHINE ?= $(shell uname -m)
endif
endif

# The SYS_* triple must match the one baked into $(PUMPKIN)/bin/libpumpkin:
# at runtime the app's native code is loaded from the PRC resource
# 'dlib' id = SYS_OS*64 + SYS_CPU*8 + SYS_SIZE.
ifeq ($(OSNAME),Android)
SYS_OS ?= 4
else ifeq ($(OSNAME),Msys)
SYS_OS ?= 2
else ifeq ($(OSNAME),vita)
SYS_OS ?= 8
else ifeq ($(OSNAME),switch)
SYS_OS ?= 9
else ifeq ($(OSNAME),psp)
SYS_OS ?= 10
else ifeq ($(OSNAME),ps2)
SYS_OS ?= 11
else ifeq ($(OSNAME),gamecube)
SYS_OS ?= 12
else ifeq ($(OSNAME),wii)
SYS_OS ?= 13
else
SYS_OS ?= 1
endif

# mirror $(PUMPKIN)/src/common.mak: macOS reports 64-bit ARM as "arm64",
# Linux as "aarch64"; only other arm* machines are 32-bit
ifeq ($(MACHINE),arm64)
SYS_CPU ?= 1
SYS_SIZE ?= 2
else ifeq ($(MACHINE),aarch64)
SYS_CPU ?= 1
SYS_SIZE ?= 2
else ifeq ($(findstring arm,$(MACHINE)),arm)
SYS_CPU ?= 1
SYS_SIZE ?= 1
else ifeq ($(MACHINE),x86_64)
SYS_CPU ?= 2
SYS_SIZE ?= 2
else ifeq ($(MACHINE),i386)
SYS_CPU ?= 2
SYS_SIZE ?= 1
else ifeq ($(MACHINE),i686)
SYS_CPU ?= 2
SYS_SIZE ?= 1
else ifeq ($(MACHINE),riscv64)
SYS_CPU ?= 3
SYS_SIZE ?= 2
else ifeq ($(MACHINE),mips)
SYS_CPU ?= 4
SYS_SIZE ?= 1
else ifeq ($(MACHINE),powerpc)
SYS_CPU ?= 5
SYS_SIZE ?= 1
SYS_ENDIAN ?= 2
else
SYS_CPU ?= 3
SYS_SIZE ?= 1
endif

SYS_ENDIAN ?= 1
DLIBID := $(shell expr $(SYS_OS) \* 64 + $(SYS_CPU) \* 8 + $(SYS_SIZE))
DLIB := $(shell printf "dlib%04x.bin" $(DLIBID))

PILRC = $(PUMPKIN)/tools/pilrc -q -allowLargeRes
PRCBUILD = $(PUMPKIN)/tools/prcbuild

ifeq ($(OSNAME),Darwin)
CC = clang -arch x86_64 -arch arm64 -mmacosx-version-min=10.14
LINKFLAGS = -dynamiclib
OSDEFS = -DDARWIN -D_DARWIN_C_SOURCE -Wno-invalid-pp-token -Wno-string-plus-int -Wno-typedef-redefinition
else ifeq ($(OSNAME),Android)
ifeq ($(NDK),)
$(error You must define the NDK environment variable)
endif
SYS_OS=4
ifeq ($(MACHINE),i686)
ANDROID_ARCH=--target=i686-none-linux-androideabi26
else ifeq ($(MACHINE),x86_64)
ANDROID_ARCH=--target=x86_64-none-linux-android26
else ifeq ($(MACHINE),riscv64)
ANDROID_ARCH=--target=riscv64-none-linux-android35
else
ifeq ($(BITS),32)
ANDROID_ARCH=--target=armv7-none-linux-androideabi26 -march=armv7-a -mthumb
else
ANDROID_ARCH=--target=aarch64-none-linux-android26
endif
endif
OSLDEFS=$(ANDROID_ARCH) --gcc-toolchain=$(NDK) --sysroot=$(NDK)/sysroot -fPIC -Wl,--exclude-libs,libgcc.a -Wl,--exclude-libs,libgcc_real.a -Wl,--exclude-libs,libatomic.a -static-libstdc++ -Wl,--build-id -Wl,--fatal-warnings -Wl,--exclude-libs,libunwind.a -Wl,--no-undefined -Qunused-arguments
EXTLIBS=-llog -latomic
SOEXT=.so
LUAPLAT=linux
OS=Android
OSDEFS=$(ANDROID_ARCH) --gcc-toolchain=$(NDK) --sysroot=$(NDK)/sysroot -fdata-sections -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -D_FORTIFY_SOURCE=2 -DANDROID -DSOEXT=\"$(SOEXT)\"
CC=$(NDK)/bin/clang
LINKFLAGS = -shared
else ifeq ($(OSNAME),Msys)
# MS-DOS: static library linked into pumpkinos.exe by src/msdos, the PRC
# only carries the resources (see src/app.mak)
SYS_OS = 2
ifeq ($(MACHINE),arm)
CC = /usr/local/armv7-pc-mingw32/bin/clang
AR = /usr/local/armv7-pc-mingw32/bin/llvm-ar
else
CC = $(MACHINE)-pc-mingw32-gcc
AR = $(MACHINE)-pc-mingw32-ar
endif
# same CPU target as $(PUMPKIN)/src/common.mak: the DJGPP default (pentiumpro)
# emits cmov instructions, which a Pentium/i586 does not have
OSDEFS = -DWINDOWS -Wno-format -Wno-attributes
# 8.3 VFS layout (see VFS_*_DIR in libpumpkin/pumpkin.h)
PRCFILE = $(shell printf '%.8s' $(PROGRAM)).prc
LINKFLAGS = -shared
else ifeq ($(OSNAME),msdosdjgpp)
# MS-DOS: static library linked into pumpkinos.exe by src/msdos, the PRC
# only carries the resources (see src/app.mak)
SYS_OS = 7
CC = i686-pc-msdosdjgpp-gcc
AR = i686-pc-msdosdjgpp-ar
STATIC = 1
# same CPU target as $(PUMPKIN)/src/common.mak: the DJGPP default (pentiumpro)
# emits cmov instructions, which a Pentium/i586 does not have
OSDEFS = -DDOS -march=i586 -mtune=i586 -Wno-format -Wno-attributes
# 8.3 VFS layout (see VFS_*_DIR in libpumpkin/pumpkin.h)
INSTALL_DIR = app_inst
PRCFILE = $(shell printf '%.8s' $(PROGRAM)).prc
else ifeq ($(OSNAME),vita)
# PlayStation Vita: static library linked into eboot.bin by src/vita, the
# PRC only carries the resources (see src/app.mak)
VITASDK ?= /usr/local/vitasdk
CC = $(VITASDK)/bin/arm-vita-eabi-gcc
AR = $(VITASDK)/bin/arm-vita-eabi-ar
STATIC = 1
OSDEFS = -DVITA -march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -Wno-format -Wno-attributes
else ifeq ($(OSNAME),switch)
# Nintendo Switch: static library linked into PumpkinOS.nro by src/switch, the
# PRC only carries the resources (see src/app.mak)
DEVKITPRO ?= /opt/devkitpro
CC = $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-gcc
AR = $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-ar
STATIC = 1
OSDEFS = -D__SWITCH__ -DSWITCH -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE -ftls-model=local-exec -I$(DEVKITPRO)/libnx/include -Wno-format -Wno-attributes
else ifeq ($(OSNAME),psp)
# PlayStation Portable: static library linked into EBOOT.PBP by src/psp, the
# PRC only carries the resources (see src/app.mak)
PSPDEV ?= /opt/pspdev
CC = $(PSPDEV)/bin/psp-gcc
AR = $(PSPDEV)/bin/psp-ar
STATIC = 1
OSDEFS = -G0 -DPSP -D_PSP_FW_VERSION=600 -I$(PSPDEV)/psp/include -I$(PSPDEV)/psp/sdk/include -Wno-format -Wno-attributes
else ifeq ($(OSNAME),ps2)
# PlayStation 2: static library linked into PUMPKINOS.ELF by src/ps2, the
# PRC only carries the resources (see src/app.mak)
PS2DEV ?= /opt/ps2dev
CC = $(PS2DEV)/ee/bin/mips64r5900el-ps2-elf-gcc
AR = $(PS2DEV)/ee/bin/mips64r5900el-ps2-elf-ar
STATIC = 1
# (no ps2sdk include directories: their font.h / debug.h / loadfile.h would
# shadow the PalmOS and libpit headers on a case-insensitive file system, and
# the emulator glue does not need them)
# -Dinflate=...: the emulator ships its own inflate(), and the PS2 link also
# carries zlib (libgme, an SDL3_mixer decoder, needs it)
OSDEFS = -G0 -Os -DPS2 -D__PS2__ -Dinflate=gnuboy_inflate -Wno-format -Wno-attributes
else ifeq ($(OSNAME),gamecube)
# Nintendo GameCube: static library linked into pumpkinos.dol by src/gamecube,
# the PRC only carries the resources (see src/app.mak)
DEVKITPRO ?= /opt/devkitpro
CC = $(DEVKITPRO)/devkitPPC/bin/powerpc-eabi-gcc
AR = $(DEVKITPRO)/devkitPPC/bin/powerpc-eabi-ar
STATIC = 1
# (no libogc include directory: the emulator glue does not need it)
# -Dxz_dec_*=...: the emulator ships its own copy of the xz decoder, and so
# does the libxmp of the GameCube portlibs (an SDL3_mixer decoder)
GC_XZ_RENAMES = -Dxz_dec_init=gnuboy_xz_dec_init -Dxz_dec_reset=gnuboy_xz_dec_reset -Dxz_dec_run=gnuboy_xz_dec_run -Dxz_dec_end=gnuboy_xz_dec_end \
                -Dxz_dec_lzma2_create=gnuboy_xz_dec_lzma2_create -Dxz_dec_lzma2_reset=gnuboy_xz_dec_lzma2_reset -Dxz_dec_lzma2_run=gnuboy_xz_dec_lzma2_run -Dxz_dec_lzma2_end=gnuboy_xz_dec_lzma2_end \
                -Dxz_dec_bcj_create=gnuboy_xz_dec_bcj_create -Dxz_dec_bcj_reset=gnuboy_xz_dec_bcj_reset -Dxz_dec_bcj_run=gnuboy_xz_dec_bcj_run \
                -Dxz_crc32=gnuboy_xz_crc32 -Dxz_crc32_init=gnuboy_xz_crc32_init -Dxz_crc64=gnuboy_xz_crc64 -Dxz_crc64_init=gnuboy_xz_crc64_init
OSDEFS = -mogc -mcpu=750 -meabi -mhard-float -Os -ffunction-sections -fdata-sections -DGAMECUBE -D__gamecube__ -DGEKKO $(GC_XZ_RENAMES) -Wno-format -Wno-attributes
else ifeq ($(OSNAME),wii)
# Nintendo Wii: the GameCube setup with the Wii flags (src/wii links it)
DEVKITPRO ?= /opt/devkitpro
CC = $(DEVKITPRO)/devkitPPC/bin/powerpc-eabi-gcc
AR = $(DEVKITPRO)/devkitPPC/bin/powerpc-eabi-ar
STATIC = 1
GC_XZ_RENAMES = -Dxz_dec_init=gnuboy_xz_dec_init -Dxz_dec_reset=gnuboy_xz_dec_reset -Dxz_dec_run=gnuboy_xz_dec_run -Dxz_dec_end=gnuboy_xz_dec_end \
                -Dxz_dec_lzma2_create=gnuboy_xz_dec_lzma2_create -Dxz_dec_lzma2_reset=gnuboy_xz_dec_lzma2_reset -Dxz_dec_lzma2_run=gnuboy_xz_dec_lzma2_run -Dxz_dec_lzma2_end=gnuboy_xz_dec_lzma2_end \
                -Dxz_dec_bcj_create=gnuboy_xz_dec_bcj_create -Dxz_dec_bcj_reset=gnuboy_xz_dec_bcj_reset -Dxz_dec_bcj_run=gnuboy_xz_dec_bcj_run \
                -Dxz_crc32=gnuboy_xz_crc32 -Dxz_crc32_init=gnuboy_xz_crc32_init -Dxz_crc64=gnuboy_xz_crc64 -Dxz_crc64_init=gnuboy_xz_crc64_init
OSDEFS = -mrvl -mcpu=750 -meabi -mhard-float -Os -ffunction-sections -fdata-sections -DWII -D__wii__ -DGEKKO $(GC_XZ_RENAMES) -Wno-format -Wno-attributes
else
CC = gcc
LINKFLAGS = -shared
OSDEFS = -DLINUX
endif

INSTALL_DIR ?= app_install
PRCFILE ?= $(PROGRAM).prc

PUMPDEFS = -DSYS_CPU=$(SYS_CPU) -DSYS_SIZE=$(SYS_SIZE) -DSYS_OS=$(SYS_OS) \
           -DSYS_ENDIAN=$(SYS_ENDIAN) -DUNIX

PALMOS = $(PUMPKIN)/src/PalmOS
PUMPKIN_INCLUDES = -I. -I$(PUMPKIN)/src/libpit -I$(PUMPKIN)/src/libpumpkin -I$(PUMPKIN)/src/font \
           -I$(PALMOS) -I$(PALMOS)/Core -I$(PALMOS)/Core/System -I$(PALMOS)/Dynamic \
           -I$(PALMOS)/Core/UI -I$(PALMOS)/Libraries -I$(PALMOS)/Libraries/PalmOSGlue \
           -I$(PALMOS)/Core/Hardware -I$(PALMOS)/Extensions/ExpansionMgr

OPT ?= -O3
ifeq ($(STATIC),1)
PICFLAG =
else
PICFLAG = -fPIC
endif
# the emulator core needs to know the host byte order (register pair layout)
ifeq ($(SYS_ENDIAN),2)
ENDIANFLAG = -DIS_BIG_ENDIAN
else
ENDIANFLAG = -DIS_LITTLE_ENDIAN
endif
COMMONFLAGS = -std=gnu11 -Wall -Wno-unknown-pragmas -Wno-unused-function -Wno-multichar \
              -fsigned-char $(PICFLAG) $(OPT) -DGNUBOY_PUMPKIN $(ENDIANFLAG) -D_GNU_SOURCE $(OSDEFS)

# the emulator core only sees the gnuboy tree (its sys.h/debug.h clash
# with the libpit headers of the same name)
CORECFLAGS = $(COMMONFLAGS) -I$(GNUBOY)
# the glue only sees the PumpkinOS tree and reaches gnuboy headers via ../../
GLUECFLAGS = $(COMMONFLAGS) $(PUMPDEFS) $(PUMPKIN_INCLUDES)

GLUE_SRCS = pumpgboy.c pumpkin-audio.c DllMain.c

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
	@$(CC) $(LINKFLAGS) $(OSLDEFS) -o $@ $(OBJS) -L$(PUMPKIN)/bin -lpumpkin -lpit
ifeq ($(OSNAME),Darwin)
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

ifeq ($(STATIC),1)
APPLIB = $(PUMPKIN)/bin/lib$(PROGRAM).a

$(APPLIB): $(OBJS)
	@echo Creating lib$(PROGRAM).a
	@rm -f $(APPLIB)
	@$(AR) -cr $(APPLIB) $(OBJS)

$(PROGRAM).prc: $(APPLIB) res.flag
	@echo Building $(PROGRAM).prc
	@$(PRCBUILD) -f $(PROGRAM).prc -t appl -c $(APPID) -n "$(APPNAME)" resources/*
	@echo Installing $(PRCFILE) into $(PUMPKIN)/vfs/$(INSTALL_DIR)
	@mkdir -p $(PUMPKIN)/vfs/$(INSTALL_DIR)
	@cp $(PROGRAM).prc $(PUMPKIN)/vfs/$(INSTALL_DIR)/$(PRCFILE)
else
$(PROGRAM).prc: $(PROGRAM).dlib res.flag
	@echo Building $(PROGRAM).prc
	@cp $(PROGRAM).dlib $(DLIB)
	@$(PRCBUILD) -f $(PROGRAM).prc -t appl -c $(APPID) -n "$(APPNAME)" $(DLIB) resources/*
	@rm -f $(DLIB)
	@echo Installing $(PROGRAM).prc into $(PUMPKIN)/vfs/app_install
	@mkdir -p $(PUMPKIN)/vfs/app_install
	@cp $(PROGRAM).prc $(PUMPKIN)/vfs/app_install/
endif

clean:
	rm -rf $(OBJDIR) resources res.flag $(PROGRAM).dlib $(PROGRAM).prc $(DLIB) $(APPLIB)

.PHONY: all clean
