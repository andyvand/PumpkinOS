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
memory card or wherever the loader finds it), the same as
`bin/PumpkinOS_PS2.zip`, a bootable disc image `bin/PumpkinOS_PS2.iso` (see
Booting from a disc below), plus `bin/pumpkinos.elf` (unstripped, for
`mips64r5900el-ps2-elf-addr2line`) and `bin/pumpkinos.map`. The directory
carries `PUMPKINOS.ELF`, the application resources (`vfs/app_install/*.prc`),
the boot options file `pumpkin.cfg` (comments only, see Options) and, when it
exists in the PumpkinOS root, `cert.pem` (https CA bundle). No soundfont:
FluidSynth would load the whole 6MB file into the console's 32MB.

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

* **a RAM disk** holds the PumpkinOS home, `ram:/PumpkinOS/`, like on the
  GameCube (`libpit/ps2ramdisk.c`). It lives in the console's main memory
  and is lost at power off:
  * `vfs/app_install`: the application PRCs (copied from the ELF's
    directory at every boot, see below),
  * `vfs/app_storage`: the database storage,
  * `vfs/app_card/PALM/Programs`: the card's Programs directory (data files of
    applications, e.g. Game Boy ROMs for GNUBoy; put them in
    `vfs/app_card/PALM/Programs` next to the ELF and they are copied too),
  * `tmp`: temporary files,
* **next to the ELF** (when that place is writable: a USB stick, the host under
  PCSX2 / ps2link): `pumpkin.log` (the log) and `pumpkin.err` (stderr);
  `cert.pem` (the CA bundle for https) is looked for next to the ELF, then
  in the home. When the ELF's place is not writable (a disc) the log goes to
  stderr, the EE console (PCSX2's EE console log, ps2link), since in the RAM
  disk it would only eat memory; `-log <file>` names a file instead
  (`-log mass:/pumpkin.log`, or `-log mc0:/pumpkin.log`: on a memory card the
  log is buffered in memory and appended to the file every 5 seconds or
  16KB, with stderr in the same file, because the card driver has only 3
  file handles in all).

**Every boot:** the RAM disk starts empty, so the `vfs/` directory shipped
next to the ELF is copied to it and PumpkinOS deploys the PRCs into
`app_storage` as usual. This takes a second or two (the deployed storage is
about 1250 small files, 800KB of data). Nothing written by the applications
(memos, addresses, preferences...) survives a reboot.

The RAM disk takes its memory from the C heap: about 940KB once the PRCs are
deployed (contents plus about 80 bytes per file), 1.25MB at the peak of the
deploy. It may hold 3MB at most (`-ramdisk <KB>` changes that, 512 at
least); past that, writes fail with "no space". Every file is one block of
memory holding its whole contents, grown as it is written and trimmed when
the last descriptor is closed. The PS2 has no spare memory the CPU cannot
use directly like the GameCube's ARAM (the IOP's, the sound processor's and
the GS's are taken by the drivers and the frame buffers).

**Keeping the home on a memory card:** `-memcard` puts it on the first slot
with a formatted PS2 memory card, `mc0:/PumpkinOS/` (`mc1:` when the first
one is empty or holds a PS1 card), as before; `-home <dir>` puts it anywhere
else (`-home mass:/PumpkinOS/` keeps everything on a USB stick). Such a home
persists: `vfs/` is copied only when `vfs/app_install` does not exist yet
(or when `app_install` and `app_storage` are both empty, an install that
failed: the PRCs are removed as they are deployed), and `-seed` copies it
again (it overwrites the PRCs, not the storage). A memory card is very slow
(every small file costs a few hundred milliseconds): the first boot takes
15 minutes or more with a black screen, and the deployed storage takes
about 6MB of the 8MB card. The log goes to the home in that case (buffered,
see above).

The storage layout uses the hashed database directory names of the MS-DOS
port (`libpumpkin/storage.c`, `STO_HASHED_DIRS`): a memory card file name is
limited to 31 characters, the regular `<name>_<hex>` directory names are up
to 97. A PS2 `app_storage` can therefore be moved to a DOS installation and
back, but not to the other platforms (and the resource file names are the
regular ones, not the DOS 8.3 ones).

### Booting from a disc

`bin/PumpkinOS_PS2.iso` is a plain data disc image to burn to a CD-R (or a
DVD-R): ISO9660 with a Joliet volume, `SYSTEM.CNF` in the root
(`BOOT2 = cdrom0:\PUMPKINOS.ELF;1`, `VER = 1.00` and `VMODE = PAL`, or
`NTSC` with `make PS2_VMODE=NTSC ...`), `PUMPKINOS.ELF`, `pumpkin.cfg`,
`cert.pem`, this README and the `vfs/` directory with the application PRCs.
It is built by `mkiso.py` (a small ISO9660 + Joliet writer, the equivalent of
`mkisofs -iso-level 2 -J`; `mkiso.py -t <image>` lists an image) from a clean
copy of the staged directory in `bin/ps2/cd/`, so that the files a PCSX2 or
ps2link session leaves in `bin/ps2/PumpkinOS/` stay out.

The console boots the disc like a game (a homebrew disc needs a modchip or a
softmod such as FreeMcBoot / ESR / OPL; uLaunchELF can also start
`cdfs:/PUMPKINOS.ELF` from any data disc), PCSX2 boots the image directly
(`PCSX2 -batch bin/PumpkinOS_PS2.iso`). The BIOS passes `argv[0]` as
`cdrom0:\PUMPKINOS.ELF;1`; `main.c` maps the `cdrom0:` device (the BIOS
device, ISO9660 paths, no directory listing) to `cdfs:/`, the device of
ps2sdk's `cdfs.irx` that `ps2_drivers` loads at startup (it prefers the
Joliet names, so the PRCs keep their real names, and matches names
case-insensitively). The disc is read-only, so:

* the home is the RAM disk, seeded from the disc's `vfs/` at every boot
  (`-memcard` or `-home mass:/PumpkinOS/` in `pumpkin.cfg` on the disc keep it
  on a memory card or a USB stick instead),
* the log goes to the EE console, or to the home when it is not the RAM
  disk, or where `-log` says,
* the options come from `pumpkin.cfg` on the disc (there is no command line).

`cdfs.irx` has quirks `libpit/sys.c` and `main.c` work around: `getstat()`
of a path that does not exist succeeds (the driver returns the boolean
result of its lookup, 0, and leaves the fields as they were), so
`sys_stat()` double checks every `cdfs:` answer by opening the directory or
the file; only 4 directories and 4 files can be open at a time, so
`copy_tree()` lists a directory and closes it before descending (the seed
tree is already 4 levels deep). Disc reads are slow (a few hundred KB/s in
PCSX2), the 12MB ELF takes a while to load.

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
    [-mode n] [-dia 0|1] [-app <name>] [-boot <dir>] [-home <dir>] [-memcard]
    [-ramdisk <KB>] [-vfs <dir>] [-log <file>] [-seed] [-nonet] [-pal | -ntsc]
    [-progressive] [-d 0|1|2]

The same options are read first from `pumpkin.cfg` in the directory the ELF
was started from (`-boot <dir>` changes it): one or more options per line,
`#` starts a comment; the command line comes after it and wins. The shipped
file only holds comments. This is the only way to pass options on a disc, a
USB stick or a memory card started from a loader.

## Testing with PCSX2

PCSX2 boots the ELF directly (`PCSX2 -batch -elf <path>/PUMPKINOS.ELF`);
enable "HostFs" in its settings (`HostFs = true` in `PCSX2.ini`, under
`[EmuCore]`) so that the ELF sees its own directory as `host:` (that is where
`vfs/` is seeded from and where the log lands; the ELF gets `argv[0]` as
`host:PUMPKINOS.ELF`). No memory card is needed (only for `-memcard`). The
keyboard mapping of PCSX2's default pad profile has the D-pad on
the arrow keys, Cross on K, Circle on L, Triangle on I, Square on J, Start on
Return and Select on Backspace.

The disc image boots with `PCSX2 -batch bin/PumpkinOS_PS2.iso`; the log then
goes to the EE console, which PCSX2 writes to its `emulog.txt` when
`EnableEEConsole = true` (`[Logging]` in `PCSX2.ini`). To watch the disc code with the log on the
host, boot the ELF from the host with the image in the drive and point it at
the disc: `PCSX2 -batch -elf <path>/PUMPKINOS.ELF -gameargs "-boot cdfs:/
-log host:cd.log" bin/PumpkinOS_PS2.iso` seeds the RAM disk from the disc
and logs to `cd.log` next to the ELF (PCSX2 resolves `host:` against the
ELF's directory).

## Differences from the PSP port

* The VFS lives on a RAM disk, seeded from the ELF's directory at every boot
  (the PSP keeps it next to the EBOOT); `-memcard` keeps it on a memory
  card. The RAM disk plugs into the C library glue: its path operation
  table (`_libcglue_fdman_path_ops`) is replaced by one that handles `ram:`
  paths and passes the others to fileXio, and its descriptors carry their
  own read/write/seek/readdir operations (`ftruncate()` resizes in place).
  The ELF can be started
  from a disc (`bin/PumpkinOS_PS2.iso`), the options can come from a
  `pumpkin.cfg` file.
* The database directories are hashed (31 character file names on a memory
  card; the RAM disk does not need it, but keeps the same layout).
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
* The EE kernel has 256 semaphores in all and pthread-embedded spends one
  per mutex, two per condition variable and five per read-write lock; with
  the dozens of read-write locks OpenSSL 4 creates at initialization and the
  mutex libatomic creates per touched address bucket (the R5900 has no
  atomic instructions) the table overflowed, depending on the heap layout.
  pthread-embedded's `pthread_mutex_init()` ignores the failed
  `CreateSema()` and the destroy that follows deletes semaphore 0, the one
  libkernel's helper thread (`topThread`, priority 0) waits on: that thread
  then spins forever and the console freezes (typically right after
  `SECURE: initializing OpenSSL` in the log). `libpit/ps2sys.c` replaces the
  pthread mutex, read-write lock and condition variable functions with
  no-cost ones (everything that uses them runs on the single kernel thread
  the cooperative tasks share; PumpkinOS's own mutexes are `mutexnull` for
  the same reason) and the link wraps `DeleteSema()` to refuse deleting
  semaphore 0 (`-Wl,--wrap=DeleteSema`).
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
