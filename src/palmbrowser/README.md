# Browser for PumpkinOS

A small text-mode web browser that runs as a native PumpkinOS application.

* Fetches pages over HTTP using PumpkinOS's built-in HTTP client
  (HTTPS works too when PumpkinOS is started with a TLS provider such as `libls2n`).
* Follows redirects, decodes chunked transfer encoding, converts UTF-8 to the
  Latin-1 character set used by the PalmOS fonts.
* Strips HTML down to formatted text: headings, bold, paragraphs, lists,
  preformatted text, image alt text and tappable links. Scripts and styles are skipped.
* Word-wrapped page view with a scrollbar; page up/down and the 5-way rocker scroll.
* URL field with Go button, Back button and history, Home page (stored in the
  application preferences), Reload, and "Open via FrogFind" which loads the current
  address through the FrogFind HTTP text proxy (useful for https-only sites).
* A built-in start page (`about:start`) with links to sites that still serve plain HTTP.

## Building

The makefile expects a PumpkinOS source tree next to this directory
(`../PumpkinOS`) with its libraries and tools already built
(`bin/libpumpkin.dylib`, `bin/libpit.dylib`, `tools/pilrc`, `tools/prcbuild`).

    make -f GNUMakefile                 # uses ../PumpkinOS
    make -f GNUMakefile ROOT=/path/to/PumpkinOS

This produces `Browser.prc` and copies it into `$(ROOT)/vfs/app_install`, from
where PumpkinOS installs it on the next start. Launch it from the Launcher.

If the PumpkinOS tree was cleaned, rebuild the pieces the app needs first:

    cd /path/to/PumpkinOS/src
    make -f GNUMakefile pilrc prcbuild libpit lua libpumpkin

## Required PumpkinOS fix (macOS)

`src/libpit/sys.c` put the whole connect timeout into `tv_usec` when calling
`select()`. macOS rejects `tv_usec >= 1000000` with `EINVAL`, so every outgoing
TCP connection failed instantly ("connect ... failed: 22 (Invalid argument)" in
`pumpkin.log`). The fix splits the value into `tv_sec`/`tv_usec` in the two
places that build a `timeval` (`sys_select` and `sys_tcpip_connect`). Rebuild
`libpit` after applying it.

On macOS the SDL3 runtime must be findable when starting PumpkinOS, e.g.
`DYLD_LIBRARY_PATH=./bin:/usr/local/lib ./pumpkin ...`.

## Testing without PumpkinOS

`Makefile.test` compiles the parser, URL, chunked-encoding, UTF-8 and layout code
natively with stubbed PalmOS calls:

    make -f Makefile.test
    DYLD_LIBRARY_PATH=../PumpkinOS/bin ./test/test_host            # unit tests
    DYLD_LIBRARY_PATH=../PumpkinOS/bin ./test/test_host page.html  # dump the layout of a saved page

## Files

* `Browser.c` – the application
* `resource.rcp`, `resource.h` – form, menu, icon resources
* `globe32.bmp`, `globe64.bmp` – launcher icons
* `GNUMakefile` – PumpkinOS app build
* `Makefile.test`, `test/test_host.c` – host-side tests
