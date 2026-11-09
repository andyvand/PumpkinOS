ifeq ($(PUMPKINOS),)
ifneq ($(wildcard ../script/pumpkin.lua),)
PUMPKINOS=..
else ifneq ($(wildcard ../../script/pumpkin.lua),)
PUMPKINOS=../..
else
$(error Set variable PUMPKINOS to the root of your PumpkinOS installation)
endif
endif

BIN=$(PUMPKINOS)/bin
SRC=$(PUMPKINOS)/src
VFS=$(PUMPKINOS)/vfs
TOOLS=$(PUMPKINOS)/tools
LIBPIT=$(SRC)/libpit
LIBPUMPKIN=$(SRC)/libpumpkin
STUBS=syscalls_stubs

SYSNAME=pit
VERSION=1.0

ifeq ($(MACHINE),)
ifeq ($(OSNAME),vita)
# the PlayStation Vita is always a 32-bit ARM (Cortex-A9)
MACHINE := arm
else ifeq ($(OSNAME),switch)
# the Nintendo Switch is always a 64-bit ARM (Cortex-A57)
MACHINE := aarch64
else ifeq ($(OSNAME),psp)
# the PlayStation Portable is always a 32-bit MIPS (Allegrex)
MACHINE := mips
else ifeq ($(OSNAME),ps2)
# the PlayStation 2 Emotion Engine is a MIPS (R5900, n32 ABI: 32-bit pointers)
MACHINE := mips
else ifeq ($(OSNAME),gamecube)
# the Nintendo GameCube is always a 32-bit big endian PowerPC (Gekko)
MACHINE := powerpc
else ifeq ($(OSNAME),wii)
# the Nintendo Wii is always a 32-bit big endian PowerPC (Broadway)
MACHINE := powerpc
else
MACHINE := $(shell uname -m)
endif
endif

ifeq ($(MACHINE),arm64)
# 64-bit ARM on macOS (uname -m prints arm64, not aarch64)
SYS_CPU=1
SYS_ENDIAN=1
ifeq ($(BITS),)
BITS=64
endif
MBITS=
else ifeq ($(findstring arm,$(MACHINE)),arm)
SYS_CPU=1
SYS_ENDIAN=1
ifeq ($(BITS),)
BITS=32
endif
else ifeq ($(MACHINE),aarch64)
SYS_CPU=1
SYS_ENDIAN=1
ifeq ($(BITS),)
BITS=64
endif
MBITS=
else ifeq ($(MACHINE),x86_64)
SYS_ENDIAN=1
SYS_CPU=2
ifeq ($(BITS),)
BITS=64
endif
MBITS=
else ifeq ($(MACHINE),x86_32)
SYS_ENDIAN=1
SYS_CPU=2
ifeq ($(BITS),)
BITS=32
MBITS=-m$(BITS)
endif
else ifeq ($(MACHINE),i686)
SYS_ENDIAN=1
SYS_CPU=2
ifeq ($(BITS),)
BITS=32
endif
MBITS=-m$(BITS)
else ifeq ($(MACHINE),i386)
SYS_ENDIAN=1
SYS_CPU=2
ifeq ($(BITS),)
BITS=32
endif
MBITS=-m$(BITS)
else ifeq ($(MACHINE),riscv32)
SYS_ENDIAN=1
SYS_CPU=3
ifeq ($(BITS),)
BITS=32
endif
else ifeq ($(MACHINE),riscv64)
SYS_ENDIAN=1
SYS_CPU=3
ifeq ($(BITS),)
BITS=64
endif
else ifeq ($(MACHINE),ppc64le)
SYS_ENDIAN=1
SYS_CPU=3
# ppc64 would be BIG_ENDIAN 4?
else ifeq ($(MACHINE),mips)
# little endian 32-bit MIPS (PlayStation Portable, PlayStation 2)
SYS_ENDIAN=1
SYS_CPU=4
ifeq ($(BITS),)
BITS=32
endif
else ifeq ($(MACHINE),powerpc)
# big endian 32-bit PowerPC (Nintendo GameCube)
SYS_ENDIAN=2
SYS_CPU=5
ifeq ($(BITS),)
BITS=32
endif
else
$(error Unknown CPU $(MACHINE))
endif

ifeq ($(BITS),32)
SYS_SIZE=1
else ifeq ($(BITS),64)
SYS_SIZE=2
else
$(error Missing BITS parameter (must be either 32 or 64))
endif

ifeq ($(OSNAME),msdosdjgpp)
AR=i686-pc-msdosdjgpp-ar
NM=i686-pc-msdosdjgpp-nm -j -u
else ifeq ($(OSNAME),vita)
VITASDK?=/usr/local/vitasdk
AR=$(VITASDK)/bin/arm-vita-eabi-ar
NM=$(VITASDK)/bin/arm-vita-eabi-nm -j -u
else ifeq ($(OSNAME),switch)
DEVKITPRO?=/opt/devkitpro
AR=$(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-ar
NM=$(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-nm -j -u
else ifeq ($(OSNAME),psp)
PSPDEV?=/opt/pspdev
AR=$(PSPDEV)/bin/psp-ar
NM=$(PSPDEV)/bin/psp-nm -j -u
else ifeq ($(OSNAME),ps2)
PS2DEV?=/opt/ps2dev
AR=$(PS2DEV)/ee/bin/mips64r5900el-ps2-elf-ar
NM=$(PS2DEV)/ee/bin/mips64r5900el-ps2-elf-nm -j -u
else ifneq ($(filter gamecube wii,$(OSNAME)),)
DEVKITPRO?=/opt/devkitpro
AR=$(DEVKITPRO)/devkitPPC/bin/powerpc-eabi-ar
NM=$(DEVKITPRO)/devkitPPC/bin/powerpc-eabi-nm -j -u
else
AR=ar
NM=nm -j -u
endif

ifneq (,$(wildcard /boot/firmware/config.txt))
RPI=4
RPI_DEFS=-DRPI=$(RPI)
endif

ifeq ($(OSNAME),)
OSNAME := $(shell uname -o)
endif

REAL_OSNAME := $(shell uname -o)

ifeq ($(OSNAME),GNU/Linux)
SYS_OS=1
EXTLIBS=-lrt -ldl
SOEXT=.so
LUAPLAT=linux
OS=Linux
OSDEFS=$(MBITS) -DLINUX $(RPI_DEFS) -DSOEXT=\"$(SOEXT)\"
CC=gcc
FC=gfortran
else ifeq ($(OSNAME),filc)
SYS_OS=1
EXTLIBS=-lrt -ldl
SOEXT=.so
LUAPLAT=linux
OS=Linux
OSDEFS=$(MBITS) -DLINUX $(RPI_DEFS) -DSOEXT=\"$(SOEXT)\"
CC=filcc
WARN_FLAGS=-Wno-typedef-redefinition -DFILC -O2

else ifeq ($(OSNAME),msdosdjgpp)
# MS-DOS (DJGPP cross compiler). Everything is linked statically into a
# single pumpkinos.exe (see src/msdos), there are no shared objects.
SYS_OS=7
EXTLIBS=-lm
SOEXT=.a
LUAPLAT=linux
OS=DOS
DJGPP_PREFIX=/usr/local/i686-pc-msdosdjgpp
OSDEFS=$(MBITS) -march=i586 -mtune=i586 -DDOS -DSOEXT=\"$(SOEXT)\" -Wno-format -Wno-attributes
CC=i686-pc-msdosdjgpp-gcc
CPP=i686-pc-msdosdjgpp-g++
PICFLAG=
# the DJGPP headers hide the POSIX API under -std=c99 (__STRICT_ANSI__)
CSTD=-std=gnu99
# 8.3 VFS layout (must match VFS_*_DIR in libpumpkin/pumpkin.h)
VFS_INSTALL_DIR=app_inst
VFS_STORAGE_DIR=app_stor
VFS_PROGRAMS_DIR=PALM/Progs

else ifeq ($(OSNAME),vita)
# PlayStation Vita (vitasdk cross compiler, newlib). Like MS-DOS everything is
# linked statically into a single executable (see src/vita), there are no
# shared objects.
SYS_OS=8
EXTLIBS=-lm
SOEXT=.a
LUAPLAT=linux
OS=Vita
VITASDK?=/usr/local/vitasdk
OSDEFS=-march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -DVITA -DSOEXT=\"$(SOEXT)\" -Wno-format -Wno-attributes
CC=$(VITASDK)/bin/arm-vita-eabi-gcc
CPP=$(VITASDK)/bin/arm-vita-eabi-g++
PICFLAG=
# the newlib headers hide the POSIX API under -std=c99 (__STRICT_ANSI__)
CSTD=-std=gnu99

else ifeq ($(OSNAME),switch)
# Nintendo Switch (devkitPro devkitA64 cross compiler, newlib + libnx). Like
# MS-DOS and the Vita everything is linked statically into a single
# executable (see src/switch), there are no shared objects. The NRO is
# position independent: every object is compiled with -fPIE (the devkitPro
# switch_rules flags), the libraries from $(DEVKITPRO)/portlibs/switch were
# built the same way.
SYS_OS=9
EXTLIBS=-lm
SOEXT=.a
LUAPLAT=linux
OS=Switch
DEVKITPRO?=/opt/devkitpro
DEVKITA64=$(DEVKITPRO)/devkitA64
LIBNX=$(DEVKITPRO)/libnx
PORTLIBS=$(DEVKITPRO)/portlibs/switch
SWITCH_ARCH=-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE -ftls-model=local-exec
OSDEFS=$(SWITCH_ARCH) -D__SWITCH__ -DSWITCH -DSOEXT=\"$(SOEXT)\" -I$(PORTLIBS)/include -I$(LIBNX)/include -Wno-format -Wno-attributes
CC=$(DEVKITA64)/bin/aarch64-none-elf-gcc
CPP=$(DEVKITA64)/bin/aarch64-none-elf-g++
PICFLAG=
# the newlib headers hide the POSIX API under -std=c99 (__STRICT_ANSI__)
CSTD=-std=gnu99

else ifeq ($(OSNAME),psp)
# Sony PlayStation Portable (pspdev psp-gcc cross compiler, newlib + pspsdk).
# Like MS-DOS, the Vita and the Switch everything is linked statically into
# a single executable (see src/psp), there are no shared objects. The
# compiler predefines PSP and __PSP__; -G0 keeps the small data section
# empty (the program is far too big for 64KB of gp-relative data). Always
# optimized: the unoptimized program is 14MB of code, which together with
# its heaps does not fit the console (-g is still added in debug builds).
SYS_OS=10
EXTLIBS=-lm
SOEXT=.a
LUAPLAT=linux
OS=PSP
PSPDEV?=/opt/pspdev
PSPSDK=$(PSPDEV)/psp/sdk
PSP_FW_VERSION?=600
OSDEFS=-G0 -O2 -DPSP -D_PSP_FW_VERSION=$(PSP_FW_VERSION) -DSOEXT=\"$(SOEXT)\" -I$(PSPDEV)/psp/include -I$(PSPSDK)/include -Wno-format -Wno-attributes
CC=$(PSPDEV)/bin/psp-gcc
CPP=$(PSPDEV)/bin/psp-g++
PICFLAG=
# the newlib headers hide the POSIX API under -std=c99 (__STRICT_ANSI__)
CSTD=-std=gnu99

else ifeq ($(OSNAME),ps2)
# Sony PlayStation 2 (ps2dev mips64r5900el-ps2-elf cross compiler, newlib +
# ps2sdk). Like the PSP everything is linked statically into a single ELF
# (see src/ps2), there are no shared objects. _EE is what the ps2sdk headers
# expect, PS2 / __PS2__ are the platform macros (SDL3 tests __PS2__ as well);
# -G0 keeps the small data section empty (the program is far too big for 64KB
# of gp-relative data). Always optimized for size: the console has 32MB in
# all and the program takes most of it.
SYS_OS=11
EXTLIBS=-lm
SOEXT=.a
LUAPLAT=linux
OS=PS2
PS2DEV?=/opt/ps2dev
PS2SDK?=$(PS2DEV)/ps2sdk
GSKIT?=$(PS2DEV)/gsKit
OSDEFS=-G0 -Os -D_EE -DPS2 -D__PS2__ -DSOEXT=\"$(SOEXT)\" -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include -I$(PS2SDK)/ports/include -Wno-format -Wno-attributes
CC=$(PS2DEV)/ee/bin/mips64r5900el-ps2-elf-gcc
CPP=$(PS2DEV)/ee/bin/mips64r5900el-ps2-elf-g++
PICFLAG=
# the newlib headers hide the POSIX API under -std=c99 (__STRICT_ANSI__)
CSTD=-std=gnu99

else ifeq ($(OSNAME),gamecube)
# Nintendo GameCube (devkitPro devkitPPC cross compiler, newlib + libogc).
# Like the Vita, the Switch, the PSP and the PS2 everything is linked
# statically into a single executable (see src/gamecube), there are no
# shared objects. GAMECUBE is the PumpkinOS platform macro, __gamecube__ and
# GEKKO are what the libogc and SDL3 headers (and devkitPro's own build
# rules) expect. The compiler flags are devkitPro's gamecube_rules MACHDEP.
# Always optimized for size, with every function and data item in its own
# section so that the link can drop what is not used: the console has 24MB
# of main memory in all and the program takes most of it.
SYS_OS=12
EXTLIBS=-lm
SOEXT=.a
LUAPLAT=linux
OS=GameCube
DEVKITPRO?=/opt/devkitpro
DEVKITPPC=$(DEVKITPRO)/devkitPPC
LIBOGC=$(DEVKITPRO)/libogc
PORTLIBS=$(DEVKITPRO)/portlibs/gamecube
GAMECUBE_ARCH=-mogc -mcpu=750 -meabi -mhard-float
OSDEFS=$(GAMECUBE_ARCH) -Os -ffunction-sections -fdata-sections -DGAMECUBE -D__gamecube__ -DGEKKO -DSOEXT=\"$(SOEXT)\" -I$(PORTLIBS)/include -I$(LIBOGC)/include -Wno-format -Wno-attributes
CC=$(DEVKITPPC)/bin/powerpc-eabi-gcc
CPP=$(DEVKITPPC)/bin/powerpc-eabi-g++
PICFLAG=
# the newlib headers hide the POSIX API under -std=c99 (__STRICT_ANSI__)
CSTD=-std=gnu99

else ifeq ($(OSNAME),wii)
# Nintendo Wii (devkitPro devkitPPC cross compiler, newlib + libogc). The
# same static model as the GameCube (see src/wii), with the libogc Wii
# libraries: WII is the PumpkinOS platform macro, __wii__ and GEKKO are what
# the libogc and SDL3 headers expect (-mrvl defines HW_RVL). The code shared
# with the GameCube tests "defined(GAMECUBE) || defined(WII)". The compiler
# flags are devkitPro's wii_rules MACHDEP. Optimized for size as well: MEM1
# (24MB) holds the program, the heap spills into MEM2 (64MB).
SYS_OS=13
EXTLIBS=-lm
SOEXT=.a
LUAPLAT=linux
OS=Wii
DEVKITPRO?=/opt/devkitpro
DEVKITPPC=$(DEVKITPRO)/devkitPPC
LIBOGC=$(DEVKITPRO)/libogc
PORTLIBS=$(DEVKITPRO)/portlibs/wii
WII_ARCH=-mrvl -mcpu=750 -meabi -mhard-float
OSDEFS=$(WII_ARCH) -Os -ffunction-sections -fdata-sections -DWII -D__wii__ -DGEKKO -DSOEXT=\"$(SOEXT)\" -I$(PORTLIBS)/include -I$(LIBOGC)/include -Wno-format -Wno-attributes
CC=$(DEVKITPPC)/bin/powerpc-eabi-gcc
CPP=$(DEVKITPPC)/bin/powerpc-eabi-g++
PICFLAG=
# the newlib headers hide the POSIX API under -std=c99 (__STRICT_ANSI__)
CSTD=-std=gnu99

else ifeq ($(OSNAME),Darwin)
SYS_OS=1
SOEXT=.dylib
LUAPLAT=linux
OS=Darwin
OSDEFS=$(MBITS) -DDARWIN $(RPI_DEFS) -DSOEXT=\"$(SOEXT)\" -Wno-invalid-pp-token -Wno-string-plus-int -Wno-typedef-redefinition -Wno-nonportable-include-path
CC=clang -arch x86_64 -arch arm64 -mmacosx-version-min=10.14 -std=gnu11
CPP=clang++ -arch x86_64 -arch arm64 -mmacosx-version-min=10.14 -std=gnu11

else ifeq ($(OSNAME),Serenity)
ifeq ($(SERENITY),)
$(error Missing SERENITY parameter (must point to SerenityOS home directory))
endif
SYS_OS=3
EXTLIBS=
SOEXT=.so
LUAPLAT=linux
OS=Serenity
OSDEFS=$(MBITS) -DSERENITY -DSOEXT=\"$(SOEXT)\" -I$(SERENITY)/Build/x86_64/Root/usr/include -D_GNU_SOURCE
CC=$(SERENITY)/Toolchain/Local/x86_64/bin/x86_64-pc-serenity-gcc

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

else ifeq ($(OSNAME),Emscripten)
EMSC=$(SRC)/emscripten
BIN=$(EMSC)
VFS=$(EMSC)/vfs
SYS_OS=5
SOEXT=.wasm
LUAPLAT=linux
OS=Emscripten
OSDEFS=$(MBITS) -DEMSCRIPTEN -DSOEXT=\"$(SOEXT)\" -pthread

else ifeq ($(OSNAME),Kernel)
SYS_OS=6
SOEXT=.a
LUAPLAT=linux
OS=Linux
OSDEFS=$(MBITS) -DKERNEL $(RPI_DEFS) -DSOEXT=\"$(SOEXT)\"
PILRCDEFS=-D KERNEL
CC=gcc

else ifeq ($(OSNAME),Haiku)
SYS_OS=7
EXTLIBS=-lnetwork
SOEXT=.so
LUAPLAT=posix
OS=Haiku
OSDEFS=$(MBITS) -DHAIKU -DSOEXT=\"$(SOEXT)\"
CC=gcc
FC=gfortran

else ifeq ($(OSNAME),Msys)
SYS_OS=2
EXTLIBS=-lws2_32 -L$(BIN) -lwinpthread -lkernel32 -luser32 -lgdi32 -ladvapi32 -lshlwapi
SOEXT=.dll
LUAPLAT=mingw
OS=Windows
OSDEFS=$(MBITS) -DWINDOWS -DWINDOWS$(BITS) -DSOEXT=\"$(SOEXT)\" -I../winpthreads/include
ifeq ($(MACHINE),arm)
AR=/usr/local/armv7-pc-mingw32/bin/llvm-ar
CC=/usr/local/armv7-pc-mingw32/bin/clang
CPP=/usr/local/armv7-pc-mingw32/bin/clang++
WINDRES=/usr/local/armv7-pc-mingw32/bin/llvm-windres
else ifeq ($(REAL_OSNAME),GNU/Linux)
AR=$(MACHINE)-w64-mingw32-ar
CC=$(MACHINE)-w64-mingw32-gcc
CPP=$(MACHINE)-w64-mingw32-g++
WINDRES=$(MACHINE)-w64-mingw32-windres
else ifeq ($(REAL_OSNAME),Darwin)
AR=$(MACHINE)-pc-mingw32-ar
CC=$(MACHINE)-pc-mingw32-gcc
CPP=$(MACHINE)-pc-mingw32-g++
WINDRES=$(MACHINE)-pc-mingw32-windres
endif
else
$(error Unknown OS $(OSNAME))
endif

EM_CC=emcc
EM_AR=emar

# VFS layout (see VFS_*_DIR in libpumpkin/pumpkin.h)
VFS_INSTALL_DIR?=app_install
VFS_STORAGE_DIR?=app_storage
VFS_PROGRAMS_DIR?=PALM/Programs

ifeq ($(TYPE),release)
OPTIMIZATION=-O2
HEAP_DEBUG=-DHEAP_DEBUG
MUTE_DEBUG=-DMUTE_DEBUG
ifeq ($(OSNAME),Darwin)
STRIP=echo
else ifeq ($(OSNAME),msdosdjgpp)
STRIP=i686-pc-msdosdjgpp-strip
else ifeq ($(OSNAME),vita)
STRIP=$(VITASDK)/bin/arm-vita-eabi-strip
else ifeq ($(OSNAME),switch)
STRIP=$(DEVKITA64)/bin/aarch64-none-elf-strip
else ifeq ($(OSNAME),psp)
STRIP=$(PSPDEV)/bin/psp-strip
else ifeq ($(OSNAME),ps2)
STRIP=$(PS2DEV)/ee/bin/mips64r5900el-ps2-elf-strip
else ifneq ($(filter gamecube wii,$(OSNAME)),)
STRIP=$(DEVKITPPC)/bin/powerpc-eabi-strip
else
STRIP=strip
endif
else
OPTIMIZATION=-g
HEAP_DEBUG=-DHEAP_DEBUG
LOGTRAP=-DLOGTRAP
STRIP=echo
endif

ifeq ($(USE_C99),no)
C99=
else
C99=-std=c99
endif

ifneq ($(APPID),)
FAPPID=-DAPPID=\'$(APPID)\'
endif

SYSDEFS=-DSYS_CPU=$(SYS_CPU) -DSYS_SIZE=$(SYS_SIZE) -DSYS_OS=$(SYS_OS) -DSYS_ENDIAN=$(SYS_ENDIAN)
PICFLAG?=-fPIC
CPPFLAGS=-Wall -Wno-unknown-pragmas -fsigned-char -Wno-multichar $(WARN_FLAGS) $(OPTIMIZATION) $(PICFLAG) -fno-stack-protector -I$(LIBPIT) -DSYSTEM_NAME=\"$(SYSNAME)\" -DSYSTEM_VERSION=\"$(VERSION)\" -DSYSTEM_OS=\"$(OS)\" -DAPPNAME="\"$(APPNAME)\"" $(FAPPID) $(CUSTOMFLAGS) $(SYSDEFS) $(OSDEFS) $(MUTE_DEBUG) -DLOGTRAP_SYS
CSTD?=$(C99)
CFLAGS=$(CPPFLAGS) -ffreestanding $(CSTD)
HOSTCFLAGS=-Wall
FFLAGS=-fPIC $(OPTIMIZATION)

ifneq ($(REAL_OSNAME),Msys)
COLOR_GREEN=\033[0;1;32m
COLOR_CYAN=\033[0;1;36m
COLOR_RED=\033[0;1;31m
COLOR_END=\033[0m
endif

ifneq ($(HOSTCC),)
%.o: %.c
	@echo Compiling $<
	@$(HOSTCC) $(HOSTCFLAGS) -c -o $@ $<
else
%.o: %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $<
endif

%.wasm : %.c
	@echo Compiling $<
	@$(EM_CC) $(CFLAGS) -c -o $@ $<

%.fo: %.f90
	@echo Compiling $<
	@$(FC) $(FFLAGS) -c -o $@ $<
