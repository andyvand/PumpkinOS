# Net Install for PumpkinOS (server)

`Net Install` is a PumpkinOS application that lets a desktop computer install
applications and databases on the device over the network. The desktop side is
the Qt program in [`src/netinstclient`](../netinstclient/README.md).

* Listens on a TCP port (default **5150**) and answers a UDP discovery
  broadcast on the same port, so the client can find the device with one
  click.
* Receives `.prc` / `.pdb` files and installs them: the file is staged in the
  hidden `/.netinst` directory of the card volume (`vfs/app_card/.netinst`)
  and, once complete, imported with `VFSImportDatabaseFromFile`, the same
  routine the Launcher uses to deploy `/app_install` at startup (it replaces
  an older version and registers new applications). The Launcher is then
  asked to rescan (`pumpkin_send_deploy`) so the new icon shows up at once.
  The staging file is deleted afterwards; an incomplete transfer is never
  imported. A running application cannot be replaced (Net Install refuses to
  replace itself and PumpkinOS refuses open databases).
* Lists the databases installed on the device (name, type, creator, version,
  size) and deletes databases on request. The Launcher and Net Install itself
  are protected against deletion.
* File management on the card: lists folders, creates folders (with their
  parents), uploads arbitrary files and deletes files or empty folders, all
  confined to `/PALM/Programs` of the card volume (`vfs/app_card/PALM/Programs`,
  the tree shown by the Launcher's file browser). Paths are canonicalized and
  checked by the engine (`..`, empty components and control characters are
  rejected; the root spelling is matched case-insensitively), so nothing
  outside that tree can be touched.
* Port and *Auto start* are stored in the application preferences. With
  *Auto start* checked the server starts listening as soon as the application
  is opened.
* An activity log shows the last connections, transfers and errors; the
  status line shows the client address and the transfer progress.

## Files

| File | Purpose |
| --- | --- |
| `NetInstall.c` | PalmOS UI, event loop and the storage callbacks (VFS, Data Manager, folder tree under `/PALM/Programs`) |
| `netinst.c/.h` | protocol engine: sockets, parsing, transfer state machine. libpit only, no PalmOS calls |
| `resource.rcp/.h` | form, menu, alert, icons |
| `esp32.h`, `make_esp32_prc.sh` | ESP32 firmware build (code is compiled into the firmware, this makes the resources-only prc) |

The engine is deliberately separated from the PalmOS code: every storage
operation goes through `netinst_ops_t` callbacks, all of which run in the
application's main thread because the engine is polled from the event loop
(non-blocking sockets, at most 20 ms per poll). No worker thread is involved,
so the PalmOS APIs are always called from the right task.

## Protocol

Text lines terminated by `\n`; replies start with `+` (success) or `-`
(failure).

```
S: +NETINST 1 <maxsize> <device name>          greeting
C: PING                     S: +PONG
C: LIST                     S: +LIST <n>  then n lines:
                               <type-hex8> <creator-hex8> <attr-hex4> <version> <size> <name>
C: PUT <size> <file.prc>    S: +GO          C: <size> raw bytes
                            S: +OK <database name>   or  -ERR <message>
C: DEL <database name>      S: +OK   or  -ERR <message>
C: QUIT                     S: +BYE
```

File commands (paths are absolute on the card volume, inside the file root):

```
C: LS [path]                S: +LS <n> <path>   then n lines:  <D|F> <size> <name>
                               (no path: the file root)
C: MKDIR <path>             S: +OK <path>       parents are created too
C: FPUT <size> <path>       S: +GO          C: <size> raw bytes (may be 0)
                            S: +OK <path>   or  -ERR <message>
C: FDEL <path>              S: +OK          file or empty folder
```

Discovery: a UDP datagram `NETINST?` sent to the port (broadcast) is answered
with `NETINST! <port> <device name>`.

Only one client is served at a time (a second connection gets `-ERR busy`).
A client that is silent for 60 seconds is dropped. The maximum accepted file
size is 64 MB on the desktop and 8 MB on the ESP32. The server checks the
file name (`name.prc` / `name.pdb`, no directories) and the PalmOS header
before anything is written.

Note for application writers: the PalmOS VFS volume 1 seen by applications
is `vfs/app_card`, so `/app_install` cannot be written from an application;
`VFSImportDatabaseFromFile` on a file of the card volume is the supported way
to install a database.

## Building

Desktop: the module is part of `BASE_MODULES` in `src/GNUMakefile`; on its
own:

```
make -C src/netinstserver
```

produces `NetInstall.prc` and copies it to `vfs/app_install`, from where it is
deployed at the next PumpkinOS start (to build against an installed runtime
instead of `bin/`, pass `BIN=<runtime dir>`). ESP32: `src/components/netinstserver`
is listed in the `REQUIRES` of `components/libpumpkin`, the Launcher maps
creator `NetI` to `NetInstPilotMain`, and `./make_esp32_prc.sh` puts the
resources-only prc into `src/data/vfs/app_install`. Android:
`Pumpkin.java` copies `R.raw.netinstall_a32` / `netinstall_a64` like the
other applications, so the prc built for each architecture must be placed in
`app/src/main/res/raw/` with those names.

## Testing without a device

`netinst.c` can be linked into a plain host program that implements the
callbacks with ordinary files (a `main()` with `debug_init`, `ptr_init`,
`thread_init`, then `netinst_create/start` and a `netinst_poll` loop, linked
against libpit); the client then talks to it on localhost. The server can also
be checked by hand with `nc`:

```
printf 'PING\nLIST\nQUIT\n' | nc <device> 5150
```
