# PumpkinOS for classic Mac OS (Retro68: 68K and PowerPC)

This directory holds the classic Mac OS host of PumpkinOS, for 68K Macs
(68020 or better, Color QuickDraw) and PowerPC Macs, System 7.1 to
Mac OS 9.2. Like `src/windows` it is a native host: `main.c` has its own
window provider (QuickDraw) and audio provider (Sound Manager), and the
mouse and the keyboard come from the Event Manager. Like the console ports
it is one statically linked application: libpit, libpumpkin, libos, the
OpenSSL TLS provider (`liblopenssl`) and every application (Launcher,
Preferences, Command, vi, LuaSyntax, Memo Pad, Address Book, To Do List,
Date Book, Space Trader, MineHunt, GNUBoy, Browser, Mail, Net Install).
There is no Lua and there are no shared libraries: the boot parameters
normally read from `pumpkin.lua` are fixed in `script.c` (the same script
engine stub as the consoles) and can be changed in `pumpkin.cfg`.

The OS runs in single application mode, the Launcher and the applications
being cooperative threads (`libpit/threadsc.c`, with a 68K and a classic
PowerPC (AIX ABI) stack switch). The platform macro is `MACCLASSIC`;
`SYS_OS` is 16, `SYS_CPU` is 6 on 68K (new) and 5 on PowerPC, 32-bit big
endian.

## Building

You need Retro68 (m68k and PowerPC toolchains, Multiversal interfaces) in
`/opt/retro68` (or `RETRO68=<dir>`), with OpenSSL built for both
architectures in its sysroots (`libssl.a` / `libcrypto.a`, see OpenSSL's
`NOTES-MACOS-CLASSIC.md`). From `src/`:

    make OSNAME=retro68 -f GNUMakefile                    # 68K
    make OSNAME=retro68 MACHINE=powerpc -f GNUMakefile    # PowerPC

Both share `bin/lib*.a`: run `make ... clean` with the same `OSNAME` and
`MACHINE` when switching architecture (or from another platform).

| File | What |
|------|------|
| `bin/PumpkinOS_Mac68K.dsk`, `bin/PumpkinOS_MacPPC.dsk` | HFS disk image (48MB, `DSK_SIZE=`) with the `PumpkinOS` folder: the application, `vfs/` with the PRCs, `cert.pem`, `pumpkin.cfg`, `README` |
| `bin/PumpkinOS_Mac68K.bin`, `bin/PumpkinOS_MacPPC.bin` | the application alone, MacBinary |
| `bin/retro68/68K/PumpkinOS.code.bin.gdb` | the unstripped 68K ELF (`m68k-apple-macos-addr2line`) |
| `bin/retro68/PPC/PumpkinOS.xcoff` | the unstripped PowerPC XCOFF |

`MEM_PREFERRED=` / `MEM_MINIMUM=` (KB) set the memory partition
(68K: 28MB / 18MB, PowerPC: 48MB / 24MB); it can also be changed in the
Finder's Get Info window.

## Installing and running

Copy the `PumpkinOS` folder from the disk image to a hard disk (the
application writes its storage in `vfs/` and its log next to itself, so
it does not run from a locked disk) and open `PumpkinOS`.

* The window holds the 320x320 double density Palm screen with the dynamic
  input area below it (320x544). On a screen too small for that (640x480)
  it is 320x320 without the input area.
* The mouse is the pen. The keyboard types; the arrows, Home, End, Page
  Up / Down, Help (Insert), forward Delete and F1-F12 are the PumpkinOS
  keys (F1-F4: the application buttons, F5: Menu).
* The Palm menu presses the Palm buttons: Home (Cmd-H), Menu (Cmd-M),
  Date Book, Address, To Do List, Memo Pad (Cmd-1 to Cmd-4). File > Quit
  (Cmd-Q) or the close box quits.
* Sound goes through the Sound Manager (8 or 16 bit, mono or stereo, at
  the rate PumpkinOS asks for; the Sound Manager converts). There is no
  MIDI synthesizer.
* Networking (Browser, Mail, Net Install, https) needs TCP/IP: MacTCP or
  Open Transport (whose MacTCP compatibility is used). The name resolver is
  the one of the MacTCP / TCP/IP control panel. `cert.pem` next to the
  application is the CA bundle for https.
* `pumpkin.cfg` holds the options (screen size, `-nonet`, `-app <name>`,
  log level), see the file.
* `pumpkin.log` / `pumpkin.err` are written next to the application.

## How it works

* **Files.** Retro68's C library opens files by HFS path and stubs out
  `stat()`, `mkdir()`, `unlink()`, `rename()` and has no directories:
  `libpit/macsys.c` implements the POSIX calls `sys.c` uses on top of the
  File Manager. Paths are POSIX paths (`/Volume/Folder/file`, relative to
  the application's folder otherwise); every component is looked up with
  `PBGetCatInfo` (no 255 character limit), `:` and `/` swap between POSIX
  and HFS names. Several descriptors can share one File Manager access
  path (HFS allows one writer per file), and a file deleted while open is
  deleted when it is closed. `macsys.h`, included only by `sys.c`, maps the
  newlib names to these functions.
* **Sockets.** `libpit/macnet.c` implements BSD sockets (TCP and UDP,
  IPv4) and `getaddrinfo()` on top of the MacTCP driver (`.IPP`) and its
  domain name resolver (the `'dnrp'` code resource, called like Apple's
  `dnr.c`, through the Mixed Mode Manager on PowerPC). Every driver call is
  asynchronous and polled, yielding to the other threads. The compatibility
  headers (`sys/socket.h`, `netinet/in.h`, `netdb.h`, `poll.h`,
  `dirent.h`, ...) are in `include/`.
* **TLS.** OpenSSL is built without sockets (no `SSL_set_fd()`):
  `liblopenssl` connects it to the MacTCP sockets with its own BIO.
* **Threads and the stack.** The Toolbox measures the free stack as the
  distance down to the heap, and the stack sniffer stops a program whose
  stack pointer is in the heap: the thread stacks are therefore slots of
  the real stack area (8 x 256KB, reserved with `SetApplLimit()` before
  `MaxApplZone()`), not heap blocks.
* **Memory.** The global PumpkinOS heap is sized from what the partition
  still has once the program is loaded (4MB kept for the rest, between 3MB
  and 16MB; `libpumpkin/pumpkin.c`).
* **The window.** A 16-bit GWorld holds the picture (RGB565 converted to
  QuickDraw's xRGB1555); the changed rectangle is copied to the window
  with `CopyBits()`, which also converts to the depth of the screen.
* **The case-insensitive file system trap.** On macOS, newlib's
  `<strings.h>` and the Toolbox `<Strings.h>` are the same file name: the
  sysroot keeps newlib's as `<bsdstrings.h>` and `include/strings.h`
  forwards to it, else `<string.h>` drags the whole Toolbox interface (and
  its `Boolean`, `ProcPtr`, ... which clash with PalmOS) into every file.

## Status

Built for both architectures; not yet run on a real Mac or in an emulator
(Basilisk II, SheepShaver, QEMU `q800` / `mac99`).
