# PumpkinOS for the Sony PlayStation Portable (pspdev + SDL3)

This directory holds the entry point of the PlayStation Portable homebrew
port. Like the MS-DOS, PlayStation Vita and Nintendo Switch ports it is a
single, statically linked executable: libpit, libpumpkin, libos, the SDL3
window provider (`liblsdl3`), the OpenSSL TLS provider (`liblopenssl`) and
every application (Launcher, Preferences, Command, vi, LuaSyntax, Memo Pad,
Address Book, To Do List, Date Book, Space Trader, MineHunt, GNUBoy, Browser,
Mail, Net Install). There are no shared libraries and no Lua script engine:
the boot parameters normally read from `pumpkin.lua` are fixed in `script.c`.

The OS runs in single application mode: the Launcher and the applications run
in-process as cooperative threads (`libpit/threadsc.c`, the same coroutine
scheduler as on DOS, the Vita and the Switch, with a MIPS stack switch), each
application's `PilotMain` is linked under its own name (`PrefPilotMain`,
`MemoPilotMain`, ...) and bound by creator ID in `Launcher.c`. The `.prc`
files only carry the resources. The platform macro is `PSP` (which psp-gcc
predefines, next to `__PSP__`), `SYS_OS` is 10, `SYS_CPU` 4 (MIPS).

## Building

Requirements: the [pspdev](https://pspdev.github.io) toolchain in
`/opt/pspdev` (or `PSPDEV=<dir>` on the make command line) with the SDL3,
SDL3_mixer (and its decoders: FluidSynth, libxmp, mpg123, WavPack, opusfile,
FLAC, vorbis) and OpenSSL packages installed in its sysroot
(`$PSPDEV/psp/lib`, as `psp-pacman` puts them). From `src/`:

    make OSNAME=psp -f GNUMakefile

Add `TYPE=release` for an optimized build. The result is `bin/psp/PumpkinOS/`
(the directory to copy to the memory stick) and the same as
`bin/PumpkinOS_PSP.zip`, plus `bin/pumpkinos.elf` (unstripped, for
`psp-addr2line`) and `bin/pumpkinos.map`. The directory carries `EBOOT.PBP`,
the application resources (`vfs/app_install/*.prc`) and, when it exists in
the PumpkinOS root, `cert.pem` (https CA bundle). `soundfont.sf2` (MIDI) is
only added with `SOUNDFONT=1` on the make command line: FluidSynth loads the
whole 6MB file into memory, which does not fit a PSP-1000.

When switching from another `OSNAME` run `make OSNAME=psp -f GNUMakefile
clean` first: objects of another platform that are newer than their sources
would otherwise be archived as they are.

## Installing and running

Copy `bin/psp/PumpkinOS` to `ms0:/PSP/GAME/PumpkinOS` (`ef0:` on a PSP go)
and start it from the Game menu of the XMB. The port keeps everything next to
`EBOOT.PBP`:

* `vfs/app_install`: the application PRCs (drop other PumpkinOS applications
  built for the PSP here, they are deployed at the next boot),
* `vfs/app_storage`: the database storage,
* `vfs/app_card/PALM/Programs`: the card's Programs directory (data files of
  applications, e.g. Game Boy ROMs for GNUBoy),
* `tmp`: temporary files,
* `pumpkin.log`: the log, `pumpkin.err`: stderr,
* `cert.pem`: the CA bundle for https, `soundfont.sf2`: the MIDI soundfont
  (optional, see above).

The storage layout is the regular one (the same as Linux, macOS, Windows,
the Vita and the Switch), so an `app_storage` directory can be moved between
those platforms and the PSP.

A custom firmware (or a firmware that runs unsigned homebrew) is required,
as for any PSP homebrew. The PBP asks for the large memory layout
(`MEMSIZE=1`): on a PSP-2000 and later PumpkinOS gets about 52MB of user
memory, on a PSP-1000 24MB (see Memory below).

### Screen and controls

The 480x272 screen is used at low density in single application mode with
the dynamic input area: a 480x160 application area over a 112 pixel strip
holding the graffiti area and the taskbar with the hard buttons (Date Book,
Address Book, To Do List, Memo Pad), centered on the screen (see the PSP
layout in `libpumpkin/dia.c` and `libos/libos.c`). It is the Vita layout at
half the scale.

The PSP has no touch screen, so the window provider draws a pointer:

* Analog stick: moves the pointer (it appears when the stick is first moved).
* Cross: the pen (press and hold to drag, graffiti strokes work the same way).
* D-pad: arrow keys. L / R: Page Up / Page Down.
* Start: Home (closes the current application and returns to the Launcher).
  Select: Tab.
* Triangle: Enter. Circle: Escape. Square: Backspace.
* Text is entered with graffiti strokes or the on-screen keyboard of the
  dynamic input area.
* HOME button (the system menu): PumpkinOS stops and returns to the XMB.

The mapping lives in `liblsdl3/liblsdl3.c` (`map_gamepad_button` and the
`LIBSDL_STICK_POINTER` code).

### Network

Sockets come from the pspsdk newlib glue on top of sceNetInet. At startup
`libpit/pspnet.c` loads the network modules and connects to the first
infrastructure connection configured in the system settings (Settings >
Network Settings > Infrastructure Mode, connection 1), waiting up to 15
seconds for the access point; without a connection PumpkinOS boots anyway
(offline). `-nonet` skips this. The PSP network stack is IPv4 only; the glue
has no `getaddrinfo()`, `pspnet.c` supplies one on top of `gethostbyname()`
(sceNetResolver). https needs a CA bundle: `cert.pem` in the PumpkinOS root
when building, or next to `EBOOT.PBP`.

### Memory

The newlib heap takes all the user memory the PBP is granted minus 2.5MB
(`PSP_HEAP_THRESHOLD_SIZE_KB` in `main.c`: the network modules and their
pool, SDL's threads and the exit callback thread live there). PumpkinOS then
asks for a 16MB global heap plus 2MB per running application (`HEAP_SIZE` in
`libpumpkin/pumpkin.c`), halving the request until it fits: a PSP-2000 or
later gets 16MB + 2MB, a PSP-1000 8MB + 1MB. The cooperative threads use 1MB
of stack each.

## Options

The XMB starts the application without arguments; a launcher that passes
them (PSPLink's `pspsh`, for development: `psp-gdb`/`psplink` from the
pspdev tools) can use the same options as the Vita and Switch ports plus
`-nonet`:

    [-width n] [-height n] [-density 72|144] [-depth n] [-hdepth 16|32]
    [-mode n] [-dia 0|1] [-app <name>] [-vfs <dir>] [-log <file>] [-d 0|1|2]
    [-nonet]

## Differences from the Vita and Switch ports

* No read-only package image: the VFS is shipped next to `EBOOT.PBP` as
  plain files (so there is no first-run copy).
* The pointer is emulated (analog stick + Cross) and drawn by `liblsdl3`
  over its persistent render target; it needs the accelerated PSP renderer.
* psp-gcc types `uint32_t` as `long unsigned int`: `-Wno-format` hides the
  resulting printf warnings, `libpumpkin/ssfn.h` no longer redefines the
  stdint types when gcc's `stdint.h` was included.
* The pspdev newlib has `fmodl`, `getentropy()` and `statvfs()`, but no
  `dlfcn.h`, `sys/syscall.h`, `getaddrinfo()`, `inet_pton()` or IPv6
  address structures (`libpit/pspnetdb.h` declares the latter so that the
  generic socket code compiles); `select()` is a polling loop that knows
  files, pipes and sockets. `clock_gettime()` only returns the wall clock,
  so `sys_get_clock()` uses `sceKernelGetSystemTimeWide()`.
* No DXE modules / `dlib` resources: `sys_lib_load()` returns NULL, every
  application must be linked statically (add it to `APPS` in `GNUMakefile`,
  to `BASE_MODULES` in `src/GNUMakefile`, and its `PilotMain` to
  `LauncherStaticPilotMain()` in `Launcher.c`). The CrispyDoom games are
  not part of this build.
* Everything is compiled with `-G0` (no gp-relative small data: the program
  is far too large for the 64KB window).
