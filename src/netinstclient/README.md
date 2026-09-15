# netinstclient - PumpkinOS Net Install client (Qt)

Desktop program that installs PalmOS applications (`.prc`) and databases
(`.pdb`) on a PumpkinOS device over the network. The device runs the
`Net Install` application from [`src/netinstserver`](../netinstserver/README.md).

## GUI

```
netinstclient [files...]
```

* **Device**: host name / IP address and port of the device. *Discover*
  broadcasts on the local network and fills in every PumpkinOS device that
  answers. Recent hosts are remembered.
* **Files to install**: add `.prc` / `.pdb` files with the button, on the
  command line, or by dropping them on the window. Each entry shows the
  database name, type, creator and size read from the PalmOS header; invalid
  files are shown in red and refused. *Install* sends every file in order
  and shows per-file and overall progress; installed files turn green.
* **Installed on the device**: the databases present on the device (name,
  type, creator, version, size), by default filtered to applications.
  *Refresh* reloads the list, *Delete* removes the selected databases after a
  confirmation.
* **Files** tab: browses the `/PALM/Programs` tree of the device's card
  (double-click a folder to open it, *Up*, or type a path and *Go*).
  *New Folder...* creates a folder, *Upload Files...* and *Upload Folder...*
  copy files or a whole folder tree into the current folder (dropping files
  or folders on the table does the same), *Delete* removes the selected files
  and empty folders. Progress is shown with the same bars as installs.
* The log at the bottom records every action and error.

## Command line

Giving `--host` (or `--discover`) runs the tool without a window:

```
netinstclient --discover [--port 5150]             # list devices on the LAN
netinstclient --host 192.168.1.20 --list           # list installed databases
netinstclient --host 192.168.1.20 App.prc Data.pdb # install files
netinstclient --host 192.168.1.20 --delete "Old App"
netinstclient --host 192.168.1.20 -q --timeout 10 App.prc
netinstclient --host 192.168.1.20 --ls /                    # list /PALM/Programs
netinstclient --host 192.168.1.20 --mkdir /PALM/Programs/Doom/wads
netinstclient --host 192.168.1.20 --upload doom1.wad --dest /PALM/Programs/Doom/wads
netinstclient --host 192.168.1.20 --upload ./MyGame          # folder, recursively
netinstclient --host 192.168.1.20 --rm /PALM/Programs/Doom/wads/doom1.wad
```

`--ls`, `--mkdir`, `--rm` and `--upload` may be repeated; `--upload` writes
into `--dest` (default `/PALM/Programs`) and recreates a folder's tree below
it. Operations run in the order: installs, deletes, list, mkdir, uploads, rm,
ls.

Progress goes to stderr, results to stdout (one line per file / database),
the exit status is 0 when everything succeeded and 1 otherwise.

## Building

Requires CMake 3.16+ and Qt 6 (Qt 5.15 also works) with the Widgets and
Network modules.

```
cmake -S src/netinstclient -B build-netinstclient
cmake --build build-netinstclient
```

With a vcpkg Qt (as used on the development Mac):

```
cmake -S src/netinstclient -B build-netinstclient -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=arm64-osx
cmake --build build-netinstclient
```

## Sources

| File | Purpose |
| --- | --- |
| `src/NetInstClient.*` | protocol client: command queue, streaming PUT/FPUT with back-pressure, timeouts, LS/MKDIR/FDEL |
| `src/Discovery.*` | UDP broadcast discovery |
| `src/PrcInfo.*` | PalmOS database header parsing |
| `src/MainWindow.*` | the window |
| `src/main.cpp` | GUI / command line entry point |

`NetInstClient` is independent of the GUI and can be reused by other tools.
