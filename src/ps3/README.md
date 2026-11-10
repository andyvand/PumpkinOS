# PumpkinOS for the Sony PlayStation 3 (ps3dev + PSL1GHT + SDL3)

This directory holds the entry point of the PlayStation 3 homebrew port. Like
the PS2, Wii and other console ports it is a single, statically linked
executable: libpit, libpumpkin, libos, the SDL3 window provider
(`liblsdl3`), the OpenSSL TLS provider (`liblopenssl`) and every
application (Launcher, Preferences, Command, vi, LuaSyntax, Memo Pad,
Address Book, To Do List, Date Book, Space Trader, MineHunt, GNUBoy,
Browser, Mail, Net Install). There are no shared libraries and no Lua: the
boot parameters normally read from `pumpkin.lua` are fixed in `script.c`.

The OS runs in single application mode: the Launcher and the applications
run in-process as cooperative threads (`libpit/threadsc.c`, with the 64-bit
PowerPC ELFv1 stack switch). The platform macros are `PS3` and `__PS3__` (the
one SDL3 tests). `SYS_OS` is 14, `SYS_CPU` is 5 (PowerPC) and `SYS_SIZE` is 2:
the Cell PPU is a 64-bit big endian CPU, LP64 in PSL1GHT.

## Building

You need the [ps3dev](https://github.com/ps3dev/ps3dev) toolchain in
`/opt/ps3dev` (or `PS3DEV=<dir>`) with PSL1GHT. These must be in the
portlibs (`/opt/ps3dev/portlibs/ppu`):

* SDL3, with the PS3 video, render, joystick and audio drivers
* SDL3_mixer, with FluidSynth, libxmp, WavPack and vorbis
* OpenSSL (the `ps3` Configure target)

From `src/`:

    make OSNAME=ps3 -f GNUMakefile

The build produces:

* `bin/PumpkinOS_PS3.pkg` and `bin/PumpkinOS_PS3.gnpdrm.pkg`: the
  installable package, in debug and finalized ("retail") format. Its title
  is `PUMP00001` and it installs to `/dev_hdd0/game/PUMP00001/`. Its
  `EBOOT.BIN` is an NPDRM SELF for content id
  `UP0001-PUMP00001_00-0000000000000000`.
* `bin/PumpkinOS_PS3.iso`: the disc image, title `PUMP00002`, category DG.
  It has a plain SELF as `EBOOT.BIN` and is built by `mkiso.py` (see Disc
  image).
* `bin/ps3/pumpkinos.self`: a signed SELF of the program, for
  `ps3load` / RPCS3 "Boot SELF/ELF".
* `bin/ps3/pumpkinos.elf` (unstripped, for `powerpc64-ps3-elf-addr2line`) and
  `bin/ps3/pumpkinos.map`.

Every `USRDIR` holds:

* `EBOOT.BIN`
* the application resources (`vfs/app_install/*.prc`)
* `pumpkin.cfg` (comments only)
* `README.md`
* `cert.pem` (https CA bundle) and `soundfont.sf2` (FluidSynth MIDI), when
  they exist in the PumpkinOS root

`make_self`, `make_self_npdrm` and `package_finalize` are linked against
Homebrew's `openssl@3` (`/opt/homebrew/opt/openssl@3/lib/libcrypto.3.dylib`).
When it is missing, the makefile looks for a `libcrypto.3.dylib` in
`/usr/local/opt/openssl@3/lib` or a python.org framework. It runs the tools
with `DYLD_FALLBACK_LIBRARY_PATH` set to that directory. You can also set
`PS3_TOOLS_LIBPATH=<dir>`, or run `brew install openssl@3`.

When you switch from another `OSNAME`, run
`make OSNAME=ps3 -f GNUMakefile clean` first.

## Installing and running

* **Package**: install `PumpkinOS_PS3.pkg` with Package Manager on a console
  running CFW or HEN, or with RPCS3 (File > Install Packages). Then start
  PumpkinOS from the Game column.
* **Disc image**: mount `PumpkinOS_PS3.iso` with webMAN MOD / IRISMAN /
  multiMAN (put it in `/dev_hdd0/PS3ISO/`), or boot it in RPCS3:
  `rpcs3 --no-gui PumpkinOS_PS3.iso`, or drop it on the window.

The program starts from `USRDIR` (`argv[0]`). The PumpkinOS home is
`/dev_hdd0/PumpkinOS/`, shared by the package and the disc. It holds:

* `vfs/`: the storage, with the PRCs copied from `USRDIR/vfs` on the first
  boot. When `USRDIR` holds PRCs of another build (an update), they are
  copied again; the build stamp is in `vfs/seed.txt`.
* `tmp/`
* `pumpkin.log` and `pumpkin.err`

## Controls

The pad is the mouse. The left stick and the D-pad move a pointer drawn by
the window provider, and **Cross** is the pen. The other buttons:

| Button   | Key       |
|----------|-----------|
| Triangle | Enter     |
| Circle   | Escape    |
| Square   | Backspace |
| Start    | Home (back to the Launcher) |
| Select   | Tab       |
| L1       | Page Up   |
| R1       | Page Down |

The PS button opens the XMB. "Quit Game" there stops PumpkinOS cleanly
(`SYSUTIL_EXIT_GAME` becomes an `SDL_EVENT_QUIT`).

SDL's PS3 joystick driver only sees the pads that are connected when it
starts. `liblsdl3` gives "PS3 Controller" a gamepad mapping when SDL
reports it only as a joystick: Triangle, Circle, Cross, Square, L1, R1,
Down, Left, Up, Right, Select, Start, L2, R2, L3, R3, and the two sticks.

## Screen

The screen is 1280x720 at double density, the Switch layout from
`libpumpkin/dia.c`: a 1280x496 application area over a 224 pixel DIA strip
(Graffiti area, taskbar and hard buttons).

SDL3's RSX renderer is young. Its render targets have no synchronization,
every texture is A8R8G8B8 whatever its SDL format, and textures are at most
1024 pixels on a side. So `liblsdl3` composes the scene in main memory, as
on the PS2 (`LIBSDL_SOFT_SCENE` + `LIBSDL_SCENE_TILES`). It uploads only the
changed part, into 640x720 texture tiles, at each present. The tiles are
drawn scaled to the TV mode chosen in the XMB: 1:1 at 720p, 1.5x at 1080p,
letterboxed when the aspect ratio differs. The pointer is drawn on top.

## Options

The XMB passes no arguments. Put options in `USRDIR/pumpkin.cfg`, with
blank or line separated words and `#` for comments:

* `-home <dir>`: the home, `/dev_hdd0/PumpkinOS/` by default, for example
  `/dev_usb000/PumpkinOS/`
* `-seed`: copy `USRDIR/vfs` to the home again
* `-nonet`: do not wait for the network
* `-app <name>`: start this application instead of the Launcher
* `-width`, `-height`, `-density`, `-depth`, `-hdepth`, `-mode`, `-dia`:
  the boot parameters
* `-boot <dir>`, `-vfs <dir>`, `-log <file>`
* `-d <level>[:<system>]`: the log level

## Platform notes

* **Network**: `libpit/ps3net.c` loads the NET and NETCTL modules, runs
  `netInitialize()` and waits up to 10 s for the XMB connection to get an
  address. It gives up after 3 s while the state stays "disconnected".
  PSL1GHT's libnet is IPv4 only and has no `getaddrinfo()`, so
  `ps3net.c` supplies IPv4 `getaddrinfo`, `freeaddrinfo` and
  `gai_strerror` on top of `gethostbyname()`.
* **Sockets**: an LV2 socket is tagged with `SOCKET_FD_MASK` (0x40000000).
  `read()`, `write()` and `close()` pass it on to libnet, but it cannot go
  into an `fd_set`. So `libpit/sys.c` waits with `poll()`
  (`socket_poll`), and sets non-blocking mode with the `SO_NBIO` socket
  option instead of `fcntl()`. `liblopenssl` waits with `poll()` too.
* **Time**: there is no `clock_gettime()`. `sys_get_clock` uses
  `sysGetSystemTime()` (microseconds since boot) and the wall clock comes
  from `sysGetCurrentTime()`.
* **Missing headers**: there is no `statvfs`, so `sys_statfs` fails.
  `TCP_NODELAY` is defined to the BSD value (there is no `netinet/tcp.h`).
  `sockaddr_in6` and `sockaddr_storage` are declared in `sys.c`.
* **Memory**: 256MB in all. The global heap is 64MB with 8MB per task, as on
  the Vita and the Switch. `sys_ps3_memlog()` (`ps3sys.c`, LV2 syscall 352)
  logs the user memory left: about 155MB once SDL is up.
* **Main thread**: `SYS_PROCESS_PARAM(1001, 0x100000)` gives it a 1MB stack.
* **setjmp / longjmp**: PSL1GHT's newlib ships the 32-bit PowerPC versions.
  They save r1, r2 and r13-r31 with `stw` and restore them with `lwz`, so
  each callee-saved register comes back with its upper half cleared.
  libpit's cooperative threads switch tasks with them, so 64-bit values
  were corrupted at every switch. The visible symptom was the SDL event
  wait never timing out, because the 64-bit divide-by-1000 constant in r25
  was truncated: the pointer was polled only when some other event arrived
  and jumped up to 36 px at a time. `libpit/ps3jmp.S` replaces both for the
  whole program with complete 64-bit versions: r1, r2, r14-r31, LR, CR,
  f14-f31 and v20-v31. They fit exactly in the 512-byte AltiVec `jmp_buf`;
  the 8-byte alignment slack holds CR.
* **Library clash**: both the portlibs and PSL1GHT have a `libssl.a` (PSL1GHT's
  is the cellSsl stub). The portlibs directory comes first on the link line.

## Disc image

`mkiso.py` writes ISO9660 (level 2, upper case, with `;1`) plus a Joliet
volume with the real names. It is based on `src/ps2/mkiso.py`; there is no
makeps3iso on the build host. The system area carries the PS3 disc header:

* sector 0: the region table, one plain region covering the whole image.
  RPCS3 reads it, and the count must be 1 to 127.
* sector 1: "PlayStation3" and the title id, `PUMP-00002`.

`-s` also writes `PS3_DISC.SFB` (HYBRID_FLAG "g", TITLE_ID), and `-t` lists
an image.

## Testing in RPCS3

Tested with RPCS3 v0.0.43 (macOS arm64) and firmware 4.93, installed from
Sony's `PS3UPDAT.PUP` with `rpcs3 --installfw`. The first boot spends a
minute precompiling shaders. Booting the ISO:

* the PRCs were deployed to `dev_hdd0/PumpkinOS`
* the network came up and SDL found the PS3 renderer and the pad
* the Launcher listed the 12 apps, rendered at 1280x720
* the pad pointer and the Cross pen opened the Browser
* the Browser loaded http://68k.news over an LV2 socket

The log is at `~/Library/Application Support/rpcs3/dev_hdd0/PumpkinOS/pumpkin.log`
and RPCS3's own log at `~/Library/Caches/rpcs3/RPCS3.log`.

The port has not been run on real hardware.
