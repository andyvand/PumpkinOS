# PumpkinOS for the PlayStation Vita (vitasdk + SDL3)

This directory holds the entry point of the PlayStation Vita port (it also
runs on the PlayStation TV). Like the MS-DOS port it is a single, statically
linked executable: libpit, libpumpkin, libos, the SDL3 window provider
(`liblsdl3`), the OpenSSL TLS provider (`liblopenssl`) and every application
(Launcher, Preferences, Command, vi, LuaSyntax, Memo Pad, Address Book, To Do
List, Date Book, Space Trader, MineHunt, GNUBoy, Browser, Mail, Net Install).
There are no shared libraries and no Lua script engine: the boot parameters
normally read from `pumpkin.lua` are fixed in `script.c`.

The OS runs in single application mode: the Launcher and the applications run
in-process as cooperative threads (`libpit/threadsc.c`, the same coroutine
scheduler as on DOS, with an ARM stack switch), each application's `PilotMain`
is linked under its own name (`PrefPilotMain`, `MemoPilotMain`, ...) and bound
by creator ID in `Launcher.c`. The `.prc` files only carry the resources. The
platform macro is `VITA`, `SYS_OS` is 8.

## Building

Requirements: the [vitasdk](https://vitasdk.org) in `/usr/local/vitasdk`
(or `VITASDK=<dir>` on the make command line) with SDL3, SDL3_mixer (and its
decoders: FluidSynth, libxmp, mpg123, WavPack) and OpenSSL installed in its
sysroot (`$VITASDK/arm-vita-eabi/lib`). From `src/`:

    make OSNAME=vita -f GNUMakefile

Add `TYPE=release` for an optimized build. The result is `bin/PumpkinOS.vpk`
(plus `bin/eboot.bin`, `bin/pumpkinos.elf` for `arm-vita-eabi-addr2line`).
The VPK carries the application resources (`vfs/app_install/*.prc`) and, when
they exist in the PumpkinOS root, `soundfont.sf2` (MIDI) and `cacert.pem`
(https CA bundle).

When switching from another `OSNAME` run `make OSNAME=vita -f GNUMakefile
clean` first: objects of another platform that are newer than their sources
would otherwise be archived as they are.

### libfluidsynth.a and position independent code

`vita-elf-create` rejects the GOT relocations of position independent code
("Invalid relocation type 25"). A FluidSynth built with
`BUILD_SHARED_LIBS=ON` (which also makes the static library PIC) cannot be
linked into a Vita executable. Build it as

    cmake -S fluidsynth -B build -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake \
          -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=OFF \
          -Denable-network=OFF -Denable-ipv6=OFF
    cmake --build build

and either install it into the sysroot or point the link at it:

    make OSNAME=vita -f GNUMakefile EXTLIBDIRS=-L<build>/src

(`EXTLIBDIRS` is searched before the sysroot; it only affects the final link
in this directory, so `rm bin/pumpkinos.elf` when changing it.)

## Installing and running

Install `PumpkinOS.vpk` with VitaShell (or any other homebrew installer). On
the first run the port creates its home directory `ux0:data/pumpkinos/` and
copies the VFS shipped in the VPK (`app0:vfs/`) there:

* `ux0:data/pumpkinos/vfs/app_install`: the application PRCs (drop other
  PumpkinOS applications built for the Vita here, they are deployed at the
  next boot),
* `ux0:data/pumpkinos/vfs/app_storage`: the database storage,
* `ux0:data/pumpkinos/vfs/app_card/PALM/Programs`: the card's Programs
  directory (data files of applications, e.g. Game Boy ROMs for GNUBoy),
* `ux0:data/pumpkinos/tmp`: temporary files,
* `ux0:data/pumpkinos/pumpkin.log`: the log, `pumpkin.err`: stderr,
* `ux0:data/pumpkinos/soundfont.sf2` and `cacert.pem` override the copies in
  the VPK.

The storage layout is the regular one (the same as Linux, macOS and
Windows), so an `app_storage` directory can be moved between those platforms
and the Vita.

### Screen and controls

The 960x544 screen is used at double density in single application mode
with the dynamic input area: a 960x320 application area (a classic 160 pixel
tall Palm screen at double density) over a 224 pixel strip holding the
graffiti area and the taskbar with the hard buttons (Date Book, Address Book,
To Do List, Memo Pad), centered on the screen (see the Vita layout in
`libpumpkin/dia.c` and `libos/libos.c`).

* The front touch screen is the pen (SDL's touch to mouse emulation; the
  rear touch pad is disabled).
* D-pad: arrow keys. L / R: Page Up / Page Down.
* Start: Home (closes the current application and returns to the Launcher).
* Cross: Enter. Circle: Escape. Select: Tab.
* Text is entered with graffiti strokes or the on-screen keyboard of the
  dynamic input area.

The mapping lives in `liblsdl3/liblsdl3.c` (`map_gamepad_button`).

### Network

Sockets come from the vitasdk newlib on top of SceNet, initialized at
startup (`libpit/vitanet.c`) with the Wi-Fi connection configured in the
system settings. There is no IPv6. https needs a CA bundle: `cacert.pem` in
the PumpkinOS root when building (packed into the VPK), or in
`ux0:data/pumpkinos/`.

### Memory

The newlib heap is set to 192MB in `main.c` (`_newlib_heap_size_user`).
PumpkinOS takes a 64MB global heap plus 8MB per running application
(`HEAP_SIZE` in `libpumpkin/pumpkin.c`), the cooperative threads 1MB of stack
each.

## Options (emulator / `sceAppMgr` arguments)

Real hardware starts the application without arguments. Emulators that can
pass arguments (Vita3K `--app-args`) accept the same options as the DOS port:

    [-width n] [-height n] [-density 72|144] [-depth n] [-hdepth 16|32]
    [-mode n] [-dia 0|1] [-app <name>] [-vfs <dir>] [-log <file>] [-d 0|1|2]

## Differences from the MS-DOS port

* Regular (long) file names, the common storage layout; no 8.3 renaming.
* No DXE modules / `dlib` resources: `sys_lib_load()` returns NULL, every
  application must be linked statically (add it to `APPS` in `GNUMakefile`,
  to `BASE_MODULES` in `src/GNUMakefile`, and its `PilotMain` to
  `LauncherStaticPilotMain()` in `Launcher.c`). The CrispyDoom games are
  not part of this build.
* OpenSSL seeds its random number generator from the system
  (`getentropy()` on `sceKernelGetRandomNumber`); no custom seeding.
* SDL3 runs a real audio thread; the audio provider still uses the
  "pump" of the DOS build (the application callback is pulled from the event
  loop, because the PumpkinOS threads are cooperative).
