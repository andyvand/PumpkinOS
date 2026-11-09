# PumpkinOS for the Sony PlayStation 2 (ps2dev + gsKit + SDL3)

This directory holds the entry point of the PlayStation 2 homebrew port. Like
the MS-DOS, PlayStation Vita, Nintendo Switch and PSP ports it is a single,
statically linked executable: libpit, libpumpkin, libos, the SDL3 window
provider (`liblsdl3`), the OpenSSL TLS provider (`liblopenssl`) and every
application (Launcher, Preferences, Command, vi, LuaSyntax, Memo Pad, Address
Book, To Do List, Date Book, Space Trader, MineHunt, GNUBoy, Browser, Mail,
Net Install). There are no shared libraries and no Lua script engine: the
boot parameters normally read from `pumpkin.lua` are fixed in `script.c`.

The OS runs in single application mode: the Launcher and the applications run
in-process as cooperative threads (`libpit/threadsc.c`, the same coroutine
scheduler as on DOS, the Vita, the Switch and the PSP, with the MIPS stack
switch), each application's `PilotMain` is linked under its own name
(`PrefPilotMain`, `MemoPilotMain`, ...) and bound by creator ID in
`Launcher.c`. The `.prc` files only carry the resources. The platform macros
are `PS2` and `__PS2__` (the latter is what SDL3 tests), `_EE` is what the
ps2sdk headers expect; `SYS_OS` is 11, `SYS_CPU` 4 (MIPS, n32 ABI: 32-bit
pointers).

## Building

Requirements: the [ps2dev](https://github.com/ps2dev/ps2dev) toolchain in
`/opt/ps2dev` (or `PS2DEV=<dir>` on the make command line) with ps2sdk,
gsKit and the ps2sdk-ports (`ps2_drivers`, libgme, FLAC, opusfile, mpg123,
zlib), plus SDL3, SDL3_mixer (and its other decoders: FluidSynth, libxmp,
WavPack, opus, vorbis) and OpenSSL installed in the EE compiler sysroot
(`$PS2DEV/ee/mips64r5900el-ps2-elf/lib`). From `src/`:

    make OSNAME=ps2 -f GNUMakefile

The result is `bin/ps2/PumpkinOS/` (the directory to copy to a USB stick, a
memory card or wherever the loader finds it) and the same as
`bin/PumpkinOS_PS2.zip`, plus `bin/pumpkinos.elf` (unstripped, for
`mips64r5900el-ps2-elf-addr2line`) and `bin/pumpkinos.map`. The directory
carries `PUMPKINOS.ELF`, the application resources (`vfs/app_install/*.prc`)
and, when it exists in the PumpkinOS root, `cert.pem` (https CA bundle). No
soundfont: FluidSynth would load the whole 6MB file into the console's 32MB.

The build is always optimized for size (`-Os`, `-G0`): the program takes most
of the console's memory (see Memory below). When switching from another
`OSNAME` run `make OSNAME=ps2 -f GNUMakefile clean` first: objects of another
platform that are newer than their sources would otherwise be archived as
they are. The staged directory is not wiped by a rebuild (only the PRCs are
replaced), so it can be used as a working directory under PCSX2 or ps2link.

## Installing and running

Copy `bin/ps2/PumpkinOS` to a USB stick (`mass:/PumpkinOS/`) or anywhere else
a homebrew loader (uLaunchELF / wLaunchELF, OPL, FreeMcBoot...) can start an
ELF from, and start `PUMPKINOS.ELF`. The layout:

* **the memory card** holds the PumpkinOS home, `mc0:/PumpkinOS/` (the first
  slot with a formatted PS2 memory card; `mc1:` when the first one is empty
  or holds a PS1 card):
  * `vfs/app_install`: the application PRCs (copied from the ELF's directory
    on the first boot, see below; drop other PumpkinOS applications built for
    the PS2 here, they are deployed at the next boot),
  * `vfs/app_storage`: the database storage,
  * `vfs/app_card/PALM/Programs`: the card's Programs directory (data files of
    applications, e.g. Game Boy ROMs for GNUBoy),
  * `tmp`: temporary files,
* **next to the ELF** (when that place is writable: a USB stick, the host under
  PCSX2 / ps2link; else on the memory card as well): `pumpkin.log` (the log)
  and `pumpkin.err` (stderr); `cert.pem` (the CA bundle for https) is looked
  for next to the ELF, then in the home.

**First boot:** when `mc?:/PumpkinOS/vfs/app_install` does not exist yet the
`vfs/` directory shipped next to the ELF is copied to the memory card, then
PumpkinOS deploys the PRCs into `app_storage` as usual. A memory card is slow
(every small file costs a few hundred milliseconds), so the first boot takes
several minutes with a black screen: watch the log, or wait for the Launcher.
The deployed storage takes about 6MB of the 8MB card (1273 files); the PRCs
are removed from `app_install` once deployed. `-seed` copies `vfs/` again
(it overwrites the PRCs, not the storage). `-home <dir>` puts the home
somewhere else (`-home mass:/PumpkinOS/` keeps everything on the USB stick,
which is much faster).

The storage layout uses the hashed database directory names of the MS-DOS
port (`libpumpkin/storage.c`, `STO_HASHED_DIRS`): a memory card file name is
limited to 31 characters, the regular `<name>_<hex>` directory names are up
to 97. A PS2 `app_storage` can therefore be moved to a DOS installation and
back, but not to the other platforms (and the resource file names are the
regular ones, not the DOS 8.3 ones).

### Screen and controls

The 640x448 frame (NTSC; also used on PAL consoles, which get it centered in
the 512 line frame; `-pal` / `-ntsc` force the mode, `-progressive` gives
480p over a component cable, `-width` / `-height` change the frame) is used
at low density in single application mode with the dynamic input area: a
640x336 application area over a 112 pixel strip holding the graffiti area and
the taskbar with the hard buttons (Date Book, Address Book, To Do List, Memo
Pad), centered on the screen (see the PS2 layout in `libpumpkin/dia.c` and
`libos/libos.c`).

The PS2 has no pointing device, so the window provider draws a pointer:

* D-pad / left analog stick: move the pointer (shown at the screen center
  from the start; the D-pad moves at full speed, the stick is proportional).
* Cross: the pen / mouse click at the pointer (press and hold to drag,
  graffiti strokes work the same way).
* L1 / R1: Page Up / Page Down.
* Start: Home (closes the current application and returns to the Launcher).
  Select: Tab.
* Triangle: Enter. Circle: Escape. Square: Backspace.
* Text is entered with graffiti strokes or the on-screen keyboard of the
  dynamic input area.

The mapping lives in `liblsdl3/liblsdl3.c` (`map_gamepad_button` and the
`LIBSDL_STICK_POINTER` code). The SDL3 PS2 renderer ("PS2 gsKit") has no
render targets and flips between two back buffers, so the provider composes
the scene in memory (`LIBSDL_SOFT_SCENE`): the textures PumpkinOS draws are
blitted into a buffer in the window pixel format and the changed part is
uploaded to one streaming texture that is drawn whole at every present, with
the pointer on top.

### Network

Sockets come from the ps2sdk newlib glue on top of lwIP running on the EE
(`ps2_drivers`'s "eeip" driver, which plugs its socket operations into the
glue). At startup `libpit/ps2net.c` loads the network adapter modules (dev9,
netman, smap: the expansion bay adapter of the fat models, the built-in
ethernet of the slim ones) and asks for a DHCP lease, waiting up to 10
seconds for the link and 10 for the lease; without an adapter, a cable or a
DHCP server PumpkinOS boots anyway (offline). `-nonet` skips this. Under
PCSX2 the emulated adapter hands out 192.0.2.100 and routes to the host's
network. The stack is IPv4 only; `ps2net.c` also supplies the
`gai_strerror()` the glue's `netdb.h` declares but nothing defines. https
needs a CA bundle: `cert.pem` in the PumpkinOS root when building, next to
the ELF or in the home at run time.

### Memory

The console has 32MB; the kernel keeps the first one and the program
(code, data and bss) takes about 18MB more, so the C heap has 13-14MB at
startup. `pumpkin_global_init()` (`libpumpkin/pumpkin.c`) sizes the global
heap from what is left at that point, keeping 7MB for what follows (the
gsKit render queues SDL allocates, the textures, the task heaps and stacks):
between 4MB and 8MB, with 1/8 of it per running application. The cooperative
threads use 512KB of stack each (`libpit/threadsc.c`). The log line "KB of C
heap left" shows the figures, `sys_ps2_memlog()` lines show the C heap at a
few points of the startup.

## Options

Loaders usually start the ELF without arguments; one that passes them
(`ps2client`'s `execee`, PCSX2's `-gameargs`) can use the same options as
the PSP port plus the PS2 ones:

    [-width n] [-height n] [-density 72|144] [-depth n] [-hdepth 16|32]
    [-mode n] [-dia 0|1] [-app <name>] [-boot <dir>] [-home <dir>] [-vfs <dir>]
    [-log <file>] [-seed] [-nonet] [-pal | -ntsc] [-progressive] [-d 0|1|2]

## Testing with PCSX2

PCSX2 boots the ELF directly (`PCSX2 -batch -elf <path>/PUMPKINOS.ELF`);
enable "HostFs" in its settings (`HostFs = true` in `PCSX2.ini`, under
`[EmuCore]`) so that the ELF sees its own directory as `host:` (that is where
`vfs/` is seeded from and where the log lands; the ELF gets `argv[0]` as
`host:PUMPKINOS.ELF`). A formatted PS2 memory card must be in one of the two
slots. The keyboard mapping of PCSX2's default pad profile has the D-pad on
the arrow keys, Cross on K, Circle on L, Triangle on I, Square on J, Start on
Return and Select on Backspace.

## Differences from the PSP port

* The VFS lives on the memory card, seeded from the ELF's directory on the
  first boot (the PSP keeps it next to the EBOOT).
* The database directories are hashed (31 character file names on the card).
* The memory card driver (mcman) answers `stat()` for a path that does not
  exist with success and a directory mode without the owner bits (`0260`):
  `sys_stat()` double checks such answers with `opendir()`. `mkdir()` of an
  existing directory fails with `ENOENT`.
* The ps2sdk pthread library (pthread-embedded) only has 29 thread-specific
  data keys and OpenSSL 4 creates one for every RCU lock and thread-local it
  sets up, so its default provider failed to load (`internal error`) or a
  NULL lock hung it: `libpit/ps2sys.c` replaces the four key functions with a
  256 entry table (per EE kernel thread). It also implements the
  `ftruncate()` the glue stubs out (`ENOSYS`) by rewriting the file through a
  side file, like the PSP port.
* The ps2sdk newlib has `fmodl`, `getentropy()`, `getaddrinfo()` and
  `clock_gettime()` (wall clock only: `sys_get_clock()` uses the EE bus clock
  counter, `GetTimerSystemTime()`), but no `dlfcn.h`, `sys/syscall.h`,
  `sys/statvfs.h`, `poll()` or IPv6 address structures (`sys.c` declares a
  `sockaddr_in6` so that the generic socket code compiles). `select()` only
  knows sockets. Only 64 file descriptors exist in all.
* libpit's `sys_init()` is renamed `pit_sys_init` on the PS2 (`sys.h`): lwIP
  has a `sys_init()` of its own in `libps2_drivers`. GNUBoy's `inflate()` is
  renamed `gnuboy_inflate` (`gnuboy/sys/pumpkin/GNUmakefile`): the link
  carries zlib for libgme. The ps2sdk include directories are not on the
  GNUBoy glue's include path: their `font.h`, `debug.h` and `loadfile.h`
  would shadow the PalmOS and libpit headers on a case-insensitive file
  system.
* The SDL main wrapper (`SDL_RunApp`, which resets the IOP and loads the
  file system drivers) is not used: `main.c` does the same itself
  (`init_iop`) before anything touches the memory card.
* No DXE modules / `dlib` resources: `sys_lib_load()` returns NULL, every
  application must be linked statically (add it to `APPS` in `GNUMakefile`,
  to `BASE_MODULES` in `src/GNUMakefile`, and its `PilotMain` to
  `LauncherStaticPilotMain()` in `Launcher.c`). The CrispyDoom games are
  not part of this build.
