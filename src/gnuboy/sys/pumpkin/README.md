# gnuboy PumpkinOS port

A native [PumpkinOS](https://github.com/migueletto/PumpkinOS) (PalmOS
re-implementation) frontend for gnuboy. The emulator core is the normal
gnuboy core; this directory replaces `main.c` and provides the `sys/`
backend (video, audio, input, timing, paths) plus the PalmOS forms.

## Building

```sh
cd sys/pumpkin
make PUMPKIN=/path/to/PumpkinOS
```

`PUMPKIN` must point at a PumpkinOS checkout that has already been built
(`bin/libpumpkin`, `bin/libpit`, `tools/pilrc` and `tools/prcbuild` must
exist). The build produces `GnuBoy.prc` and installs it into
`$(PUMPKIN)/vfs/app_install/`; PumpkinOS deploys it on next boot and it shows
up in the Launcher as "GNUBoy".

The `dlib` resource id inside the PRC encodes the host OS/CPU/word-size
triple and must match the values baked into `libpumpkin` when it was built
(`dlib` id = `SYS_OS*64 + SYS_CPU*8 + SYS_SIZE`; PumpkinOS's `common.mak`
uses `SYS_SIZE=2` for every 64-bit host, including Apple Silicon). Override
`SYS_CPU`/`SYS_SIZE` on the make command line if the detected values do not
match your PumpkinOS build, e.g. `make SYS_SIZE=2`.

On macOS the dlib's reference to libpumpkin is rewritten to
`@rpath/libpumpkin.dylib`, because `bin/libpumpkin.dylib` carries a
relative install name that dyld cannot resolve from the app storage
directory.

## Usage

- Put ROMs in the PumpkinOS VFS at `/PALM/Programs/GnuBoy/`
  (host path: `<PumpkinOS>/vfs/app_card/PALM/Programs/GnuBoy/`).
  Recognized extensions: `.gb .gbc .sgb .cgb .dmg` and `.zip .gz .xz`
  (gnuboy decompresses them itself).
- Launch GNUBoy, pick a ROM, hit **Run**. If exactly one ROM is present it
  starts directly.
- Battery saves (`.sav`), RTC (`.rtc`) and save states (`.000` ... `.009`)
  go to `/PALM/Programs/GnuBoy/saves/`.
- An optional `gnuboy.rc` in the ROM directory and per-ROM `<name>.rc`
  files are sourced like in the other gnuboy frontends.

### Controls

All eight Game Boy buttons are on screen and driven by the pen/mouse:
a 3x3 D-pad (corners give diagonals; sliding the pen across the pad
switches direction), **A**, **B**, **START** and **SELECT**. With a mouse
only one on-screen button can be held at a time, so the keyboard is also
mapped:

| Game Boy | Key                        |
|----------|----------------------------|
| D-pad    | Arrow keys                 |
| A        | X or D                     |
| B        | Z or S                     |
| Start    | Enter                      |
| Select   | Space, Tab or Backspace    |
| 0-9      | select save state slot     |
| Ins/Del  | save / load state          |
| Esc      | back to the ROM chooser    |

The **Game** menu has Open ROM, Reset, Save State, Load State, About and
Quit.

### Layout

The window requests 640x318 pixels through its `wind` resource: a 30 pixel
title bar, the Game Boy screen pixel-doubled (320x288) in the middle, the
D-pad in the 160 pixel panel on the left and A/B/START/SELECT on the right.
The layout is computed from the actual window size at run time: the largest
integer scale that leaves at least 64 pixel side panels is used (3x in a
640x480 single-app window), and on narrow windows the controls move below
the screen instead.

## Implementation notes

- **Video**: gnuboy renders 160x144 RGB565 at scale 1 into a private
  buffer (`fb.pelsize = 2`, `fb.cc` set up for 5/6/5). `vid_end()` scales
  it into `BmpGetBits(WinGetBitmap(WinGetDisplayWindow()))` and calls
  `pumpkin_screen_dirty()`, like the Doom and VBA-M ports. The byte order
  of the window bitmap (big-endian 16-bit on the desktop) is detected at
  the first frame by painting one pixel through the Window manager and
  reading it back, so 16 and 32 bpp windows of either byte order work.
- **Audio**: `pcm_*` is implemented on `SndStreamCreate` (44100 Hz, 16-bit
  stereo, the rate liblsdl3 opens the device with). gnuboy's unsigned 8-bit
  samples are converted and pushed into a mutex-protected ring; the stream
  callback drains it on the host audio thread, always returns a full
  buffer (silence on underrun), paces itself with a sleep and is never
  stopped (only muted), following the stream contract documented in the
  VBA-M port. `pcm_submit()` blocks while more than ~50 ms is queued and
  returns non-zero, so the audio ring is the frame pacing clock; without a
  stream gnuboy falls back to its wall-clock pacing (`framelen`).
- **Input**: pen down/move/up events on the main form are hit-tested
  against the on-screen controls and turned into gnuboy `K_JOY*` events,
  so the standard `bind` mechanism applies (`joyup`, `joy0` = B, `joy1` =
  A, `joy2` = SELECT, `joy3` = START). The keyboard is polled once per
  frame with `pumpkin_status()` (hardware key bits for the arrows, the
  128-bit ASCII bitmap for the rest), since PalmOS key events carry no
  key-up edges.
- **Files**: the gnuboy core uses stdio, so the frontend derives host
  paths from `VFSGetMount()` plus the PalmOS volume directory (found by
  creating a marker file through the VFS). Directories are created with
  the VFS API; the ROM list uses `VFSDirEntryEnumerate`.
- **Core changes**: `exit()` calls in `rccmds.c` and `menu.c` and the
  `atexit()` in `loader.c` are compiled out with `GNUBOY_PUMPKIN`; the
  emulator must never terminate the PumpkinOS process. `die()` shows an
  alert and `longjmp`s back to the ROM chooser.
- Once per second a `perf:` line with fps, scale and audio ring level is
  written to the PumpkinOS log (tag `GNUBOY`).

`esp32.h` carries the build parameters for an ESP32 PumpkinOS firmware
component (where `PilotMain` is named `GnuBoyPilotMain`); wiring the
component into the PumpkinOS ESP32 build is not done here.
