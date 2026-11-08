# PumpkinOS for the Nintendo Switch (devkitPro libnx + SDL3)

This directory holds the entry point of the Nintendo Switch homebrew port.
Like the MS-DOS and PlayStation Vita ports it is a single, statically linked
executable: libpit, libpumpkin, libos, the SDL3 window provider (`liblsdl3`),
the OpenSSL TLS provider (`liblopenssl`) and every application (Launcher,
Preferences, Command, vi, LuaSyntax, Memo Pad, Address Book, To Do List, Date
Book, Space Trader, MineHunt, GNUBoy, Browser, Mail, Net Install). There are
no shared libraries and no Lua script engine: the boot parameters normally
read from `pumpkin.lua` are fixed in `script.c`.

The OS runs in single application mode: the Launcher and the applications run
in-process as cooperative threads (`libpit/threadsc.c`, the same coroutine
scheduler as on DOS and the Vita, with an AArch64 stack switch), each
application's `PilotMain` is linked under its own name (`PrefPilotMain`,
`MemoPilotMain`, ...) and bound by creator ID in `Launcher.c`. The `.prc`
files only carry the resources. The platform macro is `SWITCH` (next to the
toolchain's `__SWITCH__`), `SYS_OS` is 9.

## Building

Requirements: [devkitPro](https://devkitpro.org) in `/opt/devkitpro` (or
`DEVKITPRO=<dir>` on the make command line) with the `switch-dev` group and
the portlibs `switch-sdl3`, `switch-sdl3_mixer` (and its decoders:
`switch-fluidsynth`, `switch-libxmp`, `switch-wavpack`), `switch-mesa`,
`switch-libdrm_nouveau` and `switch-openssl` installed under
`$DEVKITPRO/portlibs/switch`. From `src/`:

    make OSNAME=switch -f GNUMakefile

Add `TYPE=release` for an optimized build. The result is `bin/PumpkinOS.nro`
(plus `bin/pumpkinos.elf` for `aarch64-none-elf-addr2line`, and
`bin/pumpkinos.map`). The NRO embeds a romfs with the application resources
(`vfs/app_install/*.prc`) and, when they exist in the PumpkinOS root,
`soundfont.sf2` (MIDI) and `cert.pem` (https CA bundle).

When switching from another `OSNAME` run `make OSNAME=switch -f GNUMakefile
clean` first: objects of another platform that are newer than their sources
would otherwise be archived as they are.

## Installing and running

Copy `PumpkinOS.nro` to `sdmc:/switch/pumpkinos/PumpkinOS.nro` (any
directory under `switch/` works) and start it from the Homebrew Menu. On the
first run the port creates its home directory `sdmc:/switch/pumpkinos/` and
copies the VFS embedded in the NRO (`romfs:/vfs/`) there:

* `sdmc:/switch/pumpkinos/vfs/app_install`: the application PRCs (drop other
  PumpkinOS applications built for the Switch here, they are deployed at the
  next boot),
* `sdmc:/switch/pumpkinos/vfs/app_storage`: the database storage,
* `sdmc:/switch/pumpkinos/vfs/app_card/PALM/Programs`: the card's Programs
  directory (data files of applications, e.g. Game Boy ROMs for GNUBoy),
* `sdmc:/switch/pumpkinos/tmp`: temporary files,
* `sdmc:/switch/pumpkinos/pumpkin.log`: the log, `pumpkin.err`: stderr,
* `sdmc:/switch/pumpkinos/soundfont.sf2` and `cert.pem` override the copies
  in the NRO.

The storage layout is the regular one (the same as Linux, macOS and
Windows), so an `app_storage` directory can be moved between those platforms
and the Switch.

Start the application through a title override (hold R while launching a
game) rather than from the Album applet: in applet mode the system grants
homebrew far less memory, and PumpkinOS allocates a 64MB global heap plus
8MB per running application.

For development `nxlink -s bin/PumpkinOS.nro -- -nxlink` uploads and starts
the NRO over the network and mirrors stderr to the host (`-nxlink` is the
only option that is Switch specific; the others are listed below).

### Screen and controls

The 1280x720 screen is used at double density in single application mode
with the dynamic input area: a 1280x496 application area over a 224 pixel
strip holding the graffiti area and the taskbar with the hard buttons (Date
Book, Address Book, To Do List, Memo Pad), centered on the screen (see the
Switch layout in `libpumpkin/dia.c` and `libos/libos.c`). Docked, the
system scales the 720p image to the TV.

* The touch screen is the pen (SDL's touch to mouse emulation); a USB mouse
  and keyboard work as well.
* D-pad: arrow keys. L / R: Page Up / Page Down.
* Plus (+): Home (closes the current application and returns to the
  Launcher). Minus (-): Tab.
* A: Enter. B: Escape.
* Text is entered with graffiti strokes or the on-screen keyboard of the
  dynamic input area (or a USB keyboard).

The mapping lives in `liblsdl3/liblsdl3.c` (`map_gamepad_button`). The HOME
button is handled by the system; SDL's event pump runs `appletMainLoop()`
and ends PumpkinOS when the application is closed from the HOME menu.

### Network

Sockets come from libnx's BSD socket service, initialized at startup
(`libpit/switchnet.c`) with the connection configured in the system
settings. The port only uses IPv4. https needs a CA bundle: `cert.pem` in
the PumpkinOS root when building (embedded in the romfs), or in
`sdmc:/switch/pumpkinos/`.

### Memory

libnx gives the process all the memory the system grants it (no fixed heap
size, unlike the Vita). PumpkinOS takes a 64MB global heap plus 8MB per
running application (`HEAP_SIZE` in `libpumpkin/pumpkin.c`), the
cooperative threads 1MB of stack each.

## Options (`nxlink` arguments)

The Homebrew Menu starts the application without arguments. `nxlink -s
PumpkinOS.nro -- <options>` accepts the same options as the DOS and Vita
ports plus `-nxlink`:

    [-width n] [-height n] [-density 72|144] [-depth n] [-hdepth 16|32]
    [-mode n] [-dia 0|1] [-app <name>] [-vfs <dir>] [-log <file>] [-d 0|1|2]
    [-nxlink]

## Differences from the Vita port

* The data shipped with the executable is a romfs inside the NRO
  (`romfs:/`), the writable home is on the SD card (`sdmc:/switch/pumpkinos/`).
* The devkitA64 newlib has `fmodl`, so `libpit/fmodl.c` is not used; it also
  has `getentropy()` (on the system's random source), so OpenSSL needs no
  custom seeding.
* No DXE modules / `dlib` resources: `sys_lib_load()` returns NULL, every
  application must be linked statically (add it to `APPS` in `GNUMakefile`,
  to `BASE_MODULES` in `src/GNUMakefile`, and its `PilotMain` to
  `LauncherStaticPilotMain()` in `Launcher.c`). The CrispyDoom games are
  not part of this build.
* Everything is compiled with `-fPIE` (the NRO is position independent) and
  the devkitPro `switch.specs` linker script.
