# PumpkinOS for the Nintendo GameCube (devkitPPC + libogc + SDL3)

This directory holds the entry point of the GameCube homebrew port. Like the
PlayStation Vita, Switch, PSP and PS2 ports it is a single, statically linked
executable: libpit, libpumpkin, libos, the SDL3 window provider (`liblsdl3`)
and every application (Launcher, Preferences, Command, vi, LuaSyntax, Memo
Pad, Address Book, To Do List, Date Book, Space Trader, MineHunt, GNUBoy,
Browser, Mail, Net Install). There are no shared libraries and no Lua script
engine: the boot parameters normally read from `pumpkin.lua` are fixed in
`script.c`. There is no TLS provider: the devkitPro GameCube portlibs have no
OpenSSL, so https is not available.

The OS runs in single application mode: the Launcher and the applications run
in-process as cooperative threads (`libpit/threadsc.c`, with the PowerPC
stack switch). The platform macros are `GAMECUBE`, plus `__gamecube__` and
`GEKKO` for the libogc and SDL3 headers. `SYS_OS` is 12, `SYS_CPU` 5
(32-bit PowerPC) and `SYS_ENDIAN` 2: this is the first big endian host
PumpkinOS runs on.

## Building

Requirements: devkitPro in `/opt/devkitpro` (or `DEVKITPRO=<dir>`) with
devkitPPC, libogc and the GameCube portlibs SDL3 and SDL3_mixer
(`dkp-pacman -S gamecube-dev gamecube-sdl3 gamecube-sdl3_mixer`). From `src/`:

    make OSNAME=gamecube -f GNUMakefile

When switching from another `OSNAME` run
`make OSNAME=gamecube -f GNUMakefile clean` first.

The results:

* `bin/gamecube/pumpkinos/`: `pumpkinos.dol`, the application resources
  (`vfs/app_install/*.prc`), `pumpkin.cfg` and this README; the directory to
  copy to an SD card. The same is zipped as `bin/PumpkinOS_GameCube.zip`.
* `bin/gamecube/pumpkinos_embed.dol`: the same program with the application
  PRCs linked in. It needs nothing else, which suits a console without an SD
  card adapter or an emulator.
* `bin/pumpkinos.elf`, `bin/pumpkinos_embed.elf` (unstripped, for
  `powerpc-eabi-addr2line`) and `bin/pumpkinos.map`.

## Installing and running

Copy `bin/gamecube/pumpkinos` to the root of an SD card used in an SD Gecko
(memory card slot A or B) or an SD2SP2 (serial port 2), and start
`pumpkinos/pumpkinos.dol` from a loader such as Swiss.

The home directory, holding `vfs/`, `tmp/`, `pumpkin.log` and `pumpkin.err`,
is chosen in this order:

1. the directory of the DOL, when the loader says where that is and the
   directory is writable;
2. `pumpkinos/` on the first SD card found (`carda:`, `cardb:`, `sd2:`);
3. the ARAM RAM disk, `ram:/pumpkinos/`. This uses the 16MB of auxiliary
   memory, and everything stored there is lost at power off.

On the first boot `vfs/` is filled from the `vfs/` next to the DOL, else from
a `vfs/` directory on a disc in the drive, else from the archive built into
`pumpkinos_embed.dol`. `-seed` does it again.

### Dolphin

    Dolphin -b -e bin/gamecube/pumpkinos_embed.dol

Dolphin gives the program no storage, so it runs from the RAM disk. To see
the log, set memory card slot B to "USB Gecko". Dolphin then listens on a TCP
port (55020 here, see its log) where the PumpkinOS log appears, for example
with `nc localhost 55020`.

## Controls

The GameCube has no touch screen. PumpkinOS draws an arrow pointer instead:

| Control              | Action                                 |
|----------------------|----------------------------------------|
| D-pad / control stick| move the pointer                       |
| A                    | mouse button (the pen)                 |
| B                    | Escape                                 |
| X                    | Enter                                  |
| Y                    | Backspace                              |
| Z                    | Tab                                    |
| L / R                | Page Up / Page Down                    |
| Start                | Home (back to the Launcher)            |
| Reset (console)      | quit PumpkinOS and restart the console |

## Options

The command line, or `pumpkin.cfg` next to the DOL (read first; a loader
may pass no command line):

    -width <n> -height <n>   screen size (default 640x480)
    -density <n>             72 (default) or 144
    -depth <n> -hdepth <n>   application / host bit depth (default 16)
    -dia <0|1>               dynamic input area (default 1)
    -app <name>              start this application instead of the Launcher
    -boot <dir>              where the DOL and its vfs/ are
    -home <dir>              the home directory ("ram:/..." for the RAM disk)
    -vfs <dir>               VFS root (default <home>/vfs)
    -log <file>              log file ("gecko" for the USB Gecko)
    -gecko                   log to the USB Gecko in slot B
    -ramdisk                 use the RAM disk even with an SD card present
    -seed                    copy vfs/ again
    -nonet                   do not bring the Broadband Adapter up
    -dns <ip>                name server (default: the gateway)
    -nodvd                   do not look for a disc
    -d <level>[:<system>]    log level

## Network

The Broadband Adapter is brought up with DHCP at startup (10 seconds at most).
A console without one boots offline. Sockets come from libogc's lwIP stack.
libpit supplies what libogc lacks: `select()`, `getaddrinfo()` and a small
DNS client that queries the gateway or the `-dns` server.

## Memory

The console has 24MB of main memory. The program and its libraries take
about 10MB (`powerpc-eabi-size bin/pumpkinos.elf`). PumpkinOS sizes its
global heap from what is left at startup, between 2MB and 6MB, with 1/8 of
that per task. Everything is compiled with `-Os -ffunction-sections
-fdata-sections` and linked with `--gc-sections`.

## Implementation notes

* Big endian host: `pumpkin_s2id`/`pumpkin_id2s` used to view the 32-bit
  creator/type value as bytes in little endian order. They now use shifts.
  The GNUBoy core gets `IS_BIG_ENDIAN`, and its private copy of the xz
  decoder is renamed so it does not clash with the one in libxmp.
* SDL's GameCube joystick driver scans the controller ports on every event
  pump. A scan made before the serial interface has new data finds no
  controller, and after four such scans the driver reports the pad unplugged
  and then plugged in again. `liblsdl3` therefore pumps SDL events at most
  once per frame on the GameCube.
* SDL's GameCube renderer implements `SDL_LockTexture` with a malloc'ed
  staging buffer and returns success even when the allocation fails.
  `liblsdl3` uses `SDL_UpdateTexture` there instead.
* Mounting the DVD with no disc in the drive never returns, so the disc
  probe runs on its own thread with a 4 second timeout.
