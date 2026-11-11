# PumpkinOS for the Nintendo Wii U (devkitPro + wut + SDL3)

This directory holds the entry point of the Wii U homebrew port. Like the
Wii, PS3 and other console ports it is a single, statically linked program:
libpit, libpumpkin, libos, the SDL3 window provider (`liblsdl3`), the
OpenSSL TLS provider (`liblopenssl`) and every application (Launcher,
Preferences, Command, vi, LuaSyntax, Memo Pad, Address Book, To Do List,
Date Book, Space Trader, MineHunt, GNUBoy, Browser, Mail, Net Install).
There are no shared libraries and no Lua: the boot parameters normally read
from `pumpkin.lua` are fixed in `script.c`.

The OS runs in single application mode: the Launcher and the applications
run in-process as cooperative threads (`libpit/threadsc.c`, the 32-bit
PowerPC stack switch). The platform macros are `WIIU`, `__WIIU__` and
`__WUT__` (the last two are what wut and SDL3 test). `SYS_OS` is 15,
`SYS_CPU` is 5 (PowerPC), 32-bit big endian.

## Building

You need devkitPro with devkitPPC and wut in `/opt/devkitpro` (or
`DEVKITPRO=<dir>`), and in the Wii U portlibs
(`/opt/devkitpro/portlibs/wiiu`):

* SDL3 for the Wii U (GX2 video and renderer, the GamePad touch screen,
  VPAD / KPAD controllers, AX audio)
* SDL3_mixer, with FluidSynth, libxmp, WavPack and vorbis
* OpenSSL

From `src/`:

    make OSNAME=wiiu -f GNUMakefile

Run `make OSNAME=wiiu -f GNUMakefile clean` first when the tree was built
for another platform.

**The libraries must not use PIC or thread local storage.** `elf2rpl`
cannot convert the relocations they produce (`ERROR: Unsupported relocation
type 18/23/70/72`): build SDL3_mixer with
`-DCMAKE_POSITION_INDEPENDENT_CODE=OFF -DSDLMIXER_DEPS_SHARED=OFF` and
FluidSynth with `-Dosal=embedded` (its `cpp11` OS layer uses
`thread_local`). Libraries built elsewhere can be linked first with
`EXTLIBDIRS=-L<dir>` on the make command line.

The result:

| File | What |
|------|------|
| `bin/PumpkinOS_WiiU.wua` | the game image: a Wii U Archive with the whole title, for Cemu |
| `bin/PumpkinOS_WiiU.wuhb` | Wii U Homebrew Bundle, for Aroma |
| `bin/PumpkinOS_WiiU.zip` | the bundle in its SD card layout (`wiiu/apps/pumpkinos.wuhb`) |
| `bin/PumpkinOS_WiiU_HBL.zip` | the bare RPX with its files, for the Homebrew Launcher (`wiiu/apps/pumpkinos/`) |
| `bin/PumpkinOS_WiiU_WUP.zip` | the installable title package in `install/PumpkinOS/` (only with the common key, see below) |
| `bin/wiiu/wup/` | the same package unpacked: `title.tmd`, `title.tik`, `0000000N.app`, `.h3` |
| `bin/wiiu/title/0005000010F5A500_v0/` | the title directory (`code/`, `content/`, `meta/`) the `.wua` is made of |
| `bin/wiiu/pumpkinos.elf` | unstripped, for `powerpc-eabi-addr2line` |

`WIIU_TITLE_ID` (16 hex digits, default `0005000010F5A500`),
`WIIU_TITLE_VERSION` and `WIIU_PRODUCT` change the title's identity.
`PYTHON` runs `mktga.py` (the icon and boot screens as TGA) and `mkwua.py`
(the archive; it needs zstd: Python 3.14, or an older Python with the
`zstandard` module).

### The title package (WUP)

`mkwup.py` turns the title directory into an installable package, the
format Nintendo's own installer and WUP Installer GX2 take: content 0 is
the file table (FST), content 1 `code/` (unhashed), contents 2 and 3
`meta/` and `content/` (hashed: 64KB blocks with their H0/H1/H2 SHA-1
hashes, plus the `.h3` files), every content AES-128-CBC encrypted with
the title key; `title.tmd` lists them with their sizes and hashes and
`title.tik` carries the title key, encrypted with the Wii U common key.

The common key is not part of PumpkinOS: give it to make, and the package
is built along with the rest (without it the build prints a note and
skips it; `make ... wup` builds only the package):

    make OSNAME=wiiu -f GNUMakefile WIIU_COMMON_KEY=<32 hex digits>
    make OSNAME=wiiu -f GNUMakefile WIIU_OTP=<path to otp.bin>

(`WIIU_OTP` reads the key from a dump of the console's OTP, at 0xE0.)
`WIIU_CERT=<title.cert>` adds a certificate chain to the package (some
installers want the file; any title's `title.cert` will do).

The TMD and the ticket carry a fake (zero) signature, so the console
installs the package only with signature patches (Aroma, Tiramisu). Copy
`install/` from the zip to the SD card and install `PumpkinOS` with WUP
Installer GX2 (or a similar installer); it then shows up on the Wii U
Menu like any other title (title ID `0005000010F5A500`). In Cemu,
File > Load `bin/wiiu/wup/title.tmd` runs it (Cemu lists it as [NUS]).

### The game image

A Wii U disc image (WUD/WUX) needs Nintendo's disc keys and signatures, so
the build makes the formats that do not:

* `PumpkinOS_WiiU.wua` is Cemu's single file game format (a ZArchive:
  zstd compressed blocks, one directory per title named
  `<title id>_v<version>`). Load it with File > Load or put it in a game
  path folder; Cemu lists it as "PumpkinOS [WUA]".
* `bin/wiiu/title/0005000010F5A500_v0/` is the same title unpacked. Cemu
  loads `code/pumpkinos.rpx` from it directly, and it is what `mkwup.py`
  packs into the title package above.

## Running

* **Aroma**: copy `wiiu/` from `PumpkinOS_WiiU.zip` to the root of the SD
  card; PumpkinOS shows up on the Wii U Menu.
* **Homebrew Launcher**: copy `wiiu/` from `PumpkinOS_WiiU_HBL.zip` to the
  SD card.
* **Cemu**: `Cemu -g bin/PumpkinOS_WiiU.wua` (or File > Load). The home
  goes to Cemu's emulated SD card (`sdcard/wiiu/pumpkinos/` in Cemu's data
  folder). `--forward-console-logging` shows the `OSReport` lines.

The program finds the application PRCs, `cert.pem` (the CA bundle for
https), `soundfont.sf2` (MIDI) and `pumpkin.cfg` in the bundle / title
content (`fs:/vol/content/`) or, for the bare RPX, next to it. The home is
`sd:/wiiu/pumpkinos/`: `vfs/` (copied there on the first boot and again
when a new build brings other PRCs), `tmp/`, `pumpkin.log` and
`pumpkin.err`.

## Controls

The picture is 1280x720 at double density (a 1280x496 application area
over the 224 pixel input area strip), on the TV (scaled to 1080p, or
letterboxed on a 4:3 TV) and on the GamePad.

* GamePad touch screen: the pen
* A: Enter, B: Escape
* D-pad: arrow keys
* L / R: Page Up / Page Down
* +: Home (back to the Launcher), -: Tab
* HOME: the HOME menu (Aroma); closing the software from there stops
  PumpkinOS. In the Homebrew Launcher HOME quits.

A Pro Controller works the same way (except the pen). On a 4:3 TV the
right 214 GamePad pixels are lost to SDL clamping touches to the 640 pixel
wide window.

## Options

The Wii U passes no arguments: options are read from `pumpkin.cfg` in the
content (or next to the RPX), then from `sd:/wiiu/pumpkinos/pumpkin.cfg`
(the content is read-only: put your own options there). One or more per
line, `#` starts a comment.

    -width <n>     screen width in pixels (default 1280)
    -height <n>    screen height in pixels (default 720)
    -density <n>   72 (low) or 144 (double, default)
    -depth <n>     application bit depth (default 16)
    -hdepth <n>    host bit depth, 16 or 32 (default 16)
    -mode <n>      1 = single application (default), 0 = desktop
    -dia <0|1>     dynamic input area (default 1)
    -app <name>    start this application instead of the Launcher
    -boot <dir>    directory holding vfs/ to seed from
    -home <dir>    PumpkinOS home (default sd:/wiiu/pumpkinos/)
    -vfs <dir>     VFS root directory (default <home>/vfs)
    -log <file>    log file (default <home>/pumpkin.log)
    -seed          copy vfs/ from the content to the home again
    -nonet         do not wait for the network connection
    -d <level>     log level: 0 = errors, 1 = info (default), 2 = trace

## Notes

* The network is wut's socket layer (IPv4), started by wut itself with the
  console's default connection; `libpit/wiiusys.c` waits up to 5 seconds
  for it at startup and logs the address.
* `malloc()` comes from the MEM2 default heap (about 800MB for an
  application): the global PumpkinOS heap is 64MB plus 8MB per task.
* SDL runs ProcUI (the HOME menu, foreground release / acquire) from its
  event pump.
* Not yet run on hardware: the title package was verified in Cemu (which
  reads it with its own TMD / ticket / FST code) and by decrypting it and
  checking every hash, but not installed on a console.
