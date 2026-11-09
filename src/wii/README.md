# PumpkinOS for the Nintendo Wii (devkitPPC + libogc + SDL3)

This directory holds the entry point of the Wii homebrew port. It is the
GameCube port (`src/gamecube`) built for the Wii: a single, statically linked
DOL with libpit, libpumpkin, libos, the SDL3 window provider (`liblsdl3`) and
every application (Launcher, Preferences, Command, vi, LuaSyntax, Memo Pad,
Address Book, To Do List, Date Book, Space Trader, MineHunt, GNUBoy, Browser,
Mail, Net Install). There are no shared libraries and no Lua script engine:
the boot parameters normally read from `pumpkin.lua` are fixed in `script.c`.
There is no TLS provider: the devkitPro Wii portlibs have no OpenSSL, so
https is not available.

The OS runs in single application mode: the Launcher and the applications run
in-process as cooperative threads (`libpit/threadsc.c`, with the PowerPC
stack switch). The platform macros are `WII`, plus `__wii__` and `GEKKO` for
the libogc and SDL3 headers; the code shared with the GameCube tests
`defined(GAMECUBE) || defined(WII)`. `SYS_OS` is 13, `SYS_CPU` 5 (32-bit
PowerPC) and `SYS_ENDIAN` 2 (big endian).

## Building

Requirements: devkitPro in `/opt/devkitpro` (or `DEVKITPRO=<dir>`) with
devkitPPC, libogc and the Wii portlibs SDL3 and SDL3_mixer
(`dkp-pacman -S wii-dev wii-sdl3 wii-sdl3_mixer`). From `src/`:

    make OSNAME=wii -f GNUMakefile

When switching from another `OSNAME` run
`make OSNAME=wii -f GNUMakefile clean` first (the GameCube build shares the
object files and `bin/` libraries).

The results:

* `bin/wii/apps/pumpkinos/`: `boot.dol`, `meta.xml` and `icon.png` (the
  Homebrew Channel entry), the application resources
  (`vfs/app_install/*.prc`), `pumpkin.cfg` and this README. The same `apps/`
  tree is zipped as `bin/PumpkinOS_Wii.zip`.
* `bin/wii/pumpkinos_embed.dol`: the same program with the application PRCs
  linked in. It needs no storage, which suits an emulator.
* `bin/PumpkinOS.wbfs`: a Wii disc image (fake signed) with
  `pumpkinos_embed.dol` as its main program, for a USB loader or Dolphin.
  It is made only when Wiimms ISO Tools (`wit`) are on the `PATH` (or
  `WIT=<path to wit>`); `make OSNAME=wii -f GNUMakefile wbfs` builds it
  alone. `WII_DISC_ID` (default `RPKEPO`), `WII_DISC_REGION` (default
  `AUTO`: from the 4th ID character, `E` = USA; or `JAPAN`, `USA`, `EUROPE`,
  `KOREA`) and `WII_DISC_IOS` (default 58) change the disc ID, region and
  the IOS the console loads for it.
* `bin/pumpkinos.elf`, `bin/pumpkinos_embed.elf` (unstripped, for
  `powerpc-eabi-addr2line`) and `bin/pumpkinos.map`.

## Installing and running

Unzip `PumpkinOS_Wii.zip` (or copy `bin/wii/apps`) to the root of an SD card
or a USB storage device (FAT32) and start PumpkinOS from the Homebrew
Channel.

The home directory, holding `vfs/`, `tmp/`, `pumpkin.log` and `pumpkin.err`,
is chosen in this order:

1. the directory of the DOL (`sd:/apps/pumpkinos/` when started from the
   Homebrew Channel), when it is writable;
2. `apps/pumpkinos/` or `pumpkinos/` on the SD card or the USB device
   (`sd:`, `usb:`);
3. a RAM disk in MEM2, `ram:/pumpkinos/` (16MB by default, `-ramdisk <KB>`
   for another size). Everything stored there is lost at power off.

On the first boot `vfs/` is filled from the `vfs/` next to the DOL, else from
the archive built into `pumpkinos_embed.dol`. `-seed` does it again.

### Disc image (WBFS)

Copy `PumpkinOS.wbfs` to the `wbfs/` folder of a USB drive (or add it with
`wwt`) and start it from a USB loader (USB Loader GX, WiiFlow; the disc is
fake signed, so the loader needs a cIOS or a signature check patch). In
Dolphin, open the file like any game, or `Dolphin -b -e
bin/PumpkinOS.wbfs`. The disc carries no PRC files of its own: the program
runs from `apps/pumpkinos/` or `pumpkinos/` on the SD card or USB device
when one is present, else from the RAM disk seeded from the built-in
archive. The disc boots through a small apploader built from
`apploader/apploader.c`, which loads `sys/main.dol` (`mkdisc.py` lays out
the disc directory that `wit copy` turns into the image).

### Dolphin

    Dolphin -b -e bin/wii/pumpkinos_embed.dol

Started this way, the program gets no path, so it looks for
`apps/pumpkinos/` on Dolphin's virtual SD card and otherwise runs from the
RAM disk. Dolphin emulates the Wiimote pointer with the mouse. To see the
log, set memory card slot B to "USB Gecko"; Dolphin then listens on a TCP
port (see its log) where the PumpkinOS log appears.

## Controls

| Control                       | Action                                 |
|-------------------------------|----------------------------------------|
| Wiimote pointed at the screen | move the pointer                       |
| A                             | mouse button (the pen)                 |
| B                             | Escape                                 |
| +                             | Enter                                  |
| -                             | Backspace                              |
| 1 / 2                         | Page Up / Page Down                    |
| D-pad                         | arrow keys                             |
| Home                          | Home (back to the Launcher)            |
| Home held for 2 seconds       | quit PumpkinOS (back to the loader)    |
| Reset (console)               | quit PumpkinOS (back to the loader)    |
| Power (console or Wiimote)    | quit PumpkinOS and power off           |

A GameCube controller works as on the GameCube (control stick / D-pad move
the pointer, A is the pen, B Escape, X Enter, Y Backspace, Z Tab, L/R Page
Up/Down, Start Home). A USB keyboard types.

## Options

The command line (the `<arguments>` of `meta.xml`), or `pumpkin.cfg` next to
the DOL (read first; a loader may pass no command line):

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
    -ramdisk [KB]            use the RAM disk even with an SD card present
    -seed                    copy vfs/ again
    -nonet                   do not bring the network up
    -d <level>[:<system>]    log level

## Network

The connection configured in the Wii System Settings (the built-in Wi-Fi or a
USB ethernet adapter) is brought up with DHCP at startup (10 seconds at
most); a console without one boots offline. Sockets and name resolution come
from libogc's IOS network layer. libpit supplies what libogc lacks:
`select()`, `getaddrinfo()` and `gai_strerror()` (`libpit/gcnet.c`, shared
with the GameCube).

## Memory

The Wii has 24MB of MEM1, which holds the program, and 64MB of MEM2. The
libogc heap moves on to MEM2 when MEM1 is full. PumpkinOS sizes its global
heap from what the C heap has left at startup, between 4MB and 16MB, with
1/8 of that per task (`libpumpkin/pumpkin.c`). The RAM disk, when used, is a
block of MEM2 as well (`libpit/gcramdisk.c`; on the GameCube it lives in
ARAM, which the Wii lacks).

## Implementation notes

* The Wiimote: `main.c` initializes WPAD with the IR camera on (SDL's own
  startup code, which would do this, is not used). SDL turns the IR position
  into mouse motion; `liblsdl3` draws the same arrow pointer as on the
  GameCube (SDL's cursor is hidden). SDL also reports A and B as mouse
  buttons, and every Wiimote as a gamepad whose layout depends on the
  extension; `liblsdl3` ignores both and reads the held Wiimote buttons
  itself after each event pump (`libsdl_wii_buttons`), turning changes into
  GameCube gamepad buttons or arrow keys. Only GameCube controllers are
  opened as gamepads.
* SDL events are pumped at most once per frame, as on the GameCube (the pad
  scan problem is the same), and `SDL_UpdateTexture` replaces
  `SDL_LockTexture` (same renderer).
* The IOS the loader started the program with is kept (no IOS reload), so
  the Homebrew Channel's hardware access (`<ahb_access/>`) stays.
* Not tested on hardware yet.
