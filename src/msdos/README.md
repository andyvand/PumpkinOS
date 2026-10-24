# PumpkinOS for MS-DOS (DJGPP + SDL3)

This directory holds the entry point of the MS-DOS port. The whole system is
linked statically into one executable, `bin/pumpkin.exe`: libpit,
libpumpkin, libos, the SDL3 window provider (`liblsdl3`) and every application
and plugin (Launcher, Preferences, Command, vi, LuaSyntax, Memo Pad, Address
Book, To Do List, Date Book, Space Trader, MineHunt, GNUBoy, Browser, Mail,
Net Install). There are no shared libraries and no Lua script engine: the boot
parameters normally read from `pumpkin.lua` are fixed in `script.c` and can be
changed from the command line.

Like the ESP32 build, the OS runs in single application mode: the Launcher and
the applications run in-process (`threadnull.c`), each application's
`PilotMain` is linked under its own name (`PrefPilotMain`, `MemoPilotMain`,
...) and bound by creator ID in `Launcher.c`. The `.prc` files only carry the
resources.

## Building

Requirements: the `i686-pc-msdosdjgpp` cross compiler (GCC 16, DJGPP 2.05)
with SDL3 built for DJGPP installed in its sysroot
(`/usr/local/i686-pc-msdosdjgpp/lib/libSDL3.a`). From `src/`:

    make OSNAME=msdosdjgpp MACHINE=i686 -f GNUMakefile

Add `TYPE=release` for an optimized, stripped executable. The result is
`bin/pumpkin.exe` plus the application resources in `vfs/app_inst/*.prc`.

## 8.3 file names

Every name the DOS build uses fits the 8.3 convention, so it runs on a DOS
without long file name support:

* the executable is `pumpkin.exe`,
* the VFS directories are `app_inst` (application install), `app_stor`
  (database storage) and `app_card/PALM/Progs` (the card's Programs
  directory); the installed PRCs get 8 character names (`Preferen.prc`),
* in `app_stor` a database directory is named by the 8 hex digit hash of
  the database name (the name itself is kept in the `header` file), a
  resource file is the type and id encoded in 11 base 36 digits as `8.3`,
  a record file is `uniqueid.attr` in hex (see `libpumpkin/storage.c`),
* temporary files are `tXXXXXX` in `tmp/`, the log is `pumpkin.log` and
  stderr goes to `pumpkin.err`.

A storage directory written by another platform is not compatible with the
DOS layout (and vice versa).

## Running

Copy `pumpkin.exe` and the `vfs/` directory (with `app_inst/`, `app_stor/`
and `app_card/`) to the DOS machine, keeping them side by side or with
`vfs/` next to the executable. The program needs:

* a 386 DPMI host: on plain DOS put `CWSDPMI.EXE` next to the executable or
  on the `PATH` (Windows 9x DOS boxes and DOSBox-X with a DPMI host work as
  well),
* a Pentium Pro class (i686) CPU, which is the default code generation of
  the DJGPP compiler and of the SDL3 library (`cmov` instructions are used),
* a VESA 2.0 BIOS with a 640x480 16 bit color mode (the default; other sizes
  can be requested with `-width`/`-height`),
* about 64MB of RAM: the global heap is 32MB plus 4MB per running
  application,

Options:

    pumpkin.exe [-width n] [-height n] [-density 72|144] [-depth n]
                  [-hdepth 16|32] [-mode n] [-dia 0|1] [-window]
                  [-vfs <dir>] [-log <file>] [-d 0|1|2]

The defaults are a 640x480 screen, density 144, 16 bit color, single
application mode with the dynamic input area: a 640x320 application area
over a 160 pixel DIA strip (Graffiti writing area plus the icon bar with
Home, Menu, Find, keyboard and the DIA trigger; the hard-button row of the
desktop DIA is left out, the PC keyboard has those keys). The mouse is the
pen and the keyboard `Home` key also returns to the Launcher. `-dia 0` gives
the applications the whole 640x480 screen. The log goes to `pumpkin.log` and
stderr (including a DJGPP crash traceback) to `pumpkin.err`, both next to
the executable.

DOSBox-X settings known to work: `machine=svga_s3`, `cputype=ppro_slow`,
`memsize=64`, `CWSDPMI.EXE` on the path (long file names are not needed).

## Network

TCP/IP comes from Watt-32 (linked statically, with its DJGPP file system
extension the sockets are ordinary file descriptors). It needs a packet
driver loaded before `pumpkin.exe` starts, for example `NE2000.COM 0x60`
for an NE2000 card (DOSBox-X: `ne2000=true` and `backend=slirp` in the
`[ne2000]` section, then `NE2000.COM 0x60 3 0x300`). The IP configuration
is read from `WATTCP.CFG` in the current directory (or in the directory the
`WATTCP.CFG` environment variable points to); without one Watt-32 asks a
DHCP server. Initialization happens at startup while the screen is still in
text mode, so the Watt-32 messages can be read; without a packet driver the
network is simply unavailable (Browser, Mail and Net Install cannot
connect) and the error is logged.

`https://` connections use OpenSSL (the DJGPP build in the cross compiler
sysroot, see `liblopenssl/`). Certificate verification needs a CA bundle:
put `cacert.pem` (for example the one from https://curl.se/docs/caextract.html)
next to `pumpkin.exe`; it is passed to OpenSSL through `SSL_CERT_FILE`.

## Not available on DOS

No audio provider (it needs a thread), no serial ports, no Lua scripting
and no desktop (multi window) mode.
