// PumpkinOS entry point for the Nintendo Wii (devkitPPC + libogc + SDL3).
//
// The GameCube port (src/gamecube/main.c) with the Wii hardware: everything
// is linked statically, there is no script engine and the OS runs in single
// application mode (the Launcher and the applications run in-process, as
// cooperative threads).
//
// Layout: the DOL is started by the Homebrew Channel (or another loader)
// from the front SD card ("sd:/") or a USB mass storage device ("usb:/"),
// normally as apps/pumpkinos/boot.dol. The PumpkinOS home is the directory
// holding the DOL: the VFS (vfs/, shipped with the DOL), tmp/, the log
// (pumpkin.log) and stderr (pumpkin.err). Without a writable FAT device the
// home is a RAM disk in MEM2 ("ram:/pumpkinos/", lost at power off, see
// libpit/gcramdisk.c) and the VFS is seeded from the archive linked into the
// program (pumpkinos_embed.dol, see GNUMakefile). A loader may pass no
// command line, so the options can also come from a pumpkin.cfg file next to
// the DOL (the Homebrew Channel also passes the <arguments> of meta.xml).
//
// Input: the Wiimote pointed at the screen moves the pointer (SDL turns the
// IR position into mouse motion), its buttons are read by liblsdl3; a
// GameCube controller works as on the GameCube; a USB keyboard types. The
// Reset button, the console or Wiimote power button, and Home held for two
// seconds stop PumpkinOS (back to the loader, or power off).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <gccore.h>
#include <fat.h>
#include <ogc/console.h>
#include <ogc/usbgecko.h>
#include <wiiuse/wpad.h>
#include <wiikeyboard/keyboard.h>

#include <SDL3/SDL.h>

#include "sys.h"
#include "thread.h"
#include "ptr.h"
#include "vfs.h"
#include "vfslocal.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "pwindow.h"
#include "script.h"
#include "debug.h"
#include "wii.h"

#define MAX_PATH_LEN 256

// the PumpkinOS directory on a storage device when the DOL was not started
// from one (the Homebrew Channel layout is apps/pumpkinos/boot.dol)
#define WII_HOME_DIR "pumpkinos"
#define WII_APP_DIR  "apps/pumpkinos"

// the RAM disk device (libpit/gcramdisk.c)
#define WII_RAMDISK "ram"

// how long to wait for the network at startup
#define WII_NET_TIMEOUT_MS 10000

// the options file next to the DOL (a loader may pass no command line)
#define WII_CONFIG_FILE "pumpkin.cfg"
#define WII_CONFIG_MAX_ARGS 64
#define WII_CONFIG_MAX_SIZE 4096

// the FAT devices libfat may have mounted (the front SD card slot, a USB
// mass storage device), in the order they are tried for a home
static const char *fat_devices[] = { "sd", "usb", NULL };

extern int liblsdl3_load(void);
extern int liblsdl3_init(int pe, script_ref_t obj);
extern int liblsdl3_unload(void);

extern int libos_app_init(int pe);
extern int libos_start(int pe);

// the SDL OGC video driver turns these into a quit event (SDL_ogcevents.c);
// OGC_PowerOffRequested would also power the console off at once, so a
// power button only sets OGC_ResetRequested here and main() powers off
// after PumpkinOS has stopped
extern bool OGC_ResetRequested;

// the VFS archive linked into pumpkinos_embed.dol (bin2s, see GNUMakefile
// and ../gamecube/mkvfsblob.py); absent from the plain DOL (weak: NULL then)
extern const uint8_t vfsblob_bin[] __attribute__((weak));
extern const uint32_t vfsblob_bin_size __attribute__((weak));

static volatile int reset_requested = 0;
static volatile int power_requested = 0;

// the Reset button: ask PumpkinOS to stop (the quit event reaches the
// window provider, which ends the Launcher loop and lets main() clean up),
// then go back to the loader
static void reset_callback(u32 irq, void *ctx) {
  reset_requested = 1;
  OGC_ResetRequested = true;
}

// the console power button, or a Wiimote's: stop, then power off
static void power_callback(void) {
  power_requested = 1;
  OGC_ResetRequested = true;
}

static void wpad_power_callback(s32 chan) {
  power_callback();
}

static void usage(void) {
  fprintf(stderr, "usage: boot.dol [options]\n");
  fprintf(stderr, "  -width <n>     screen width in pixels (default 640)\n");
  fprintf(stderr, "  -height <n>    screen height in pixels (default 480)\n");
  fprintf(stderr, "  -density <n>   72 (low, default) or 144 (double)\n");
  fprintf(stderr, "  -depth <n>     application bit depth (default 16)\n");
  fprintf(stderr, "  -hdepth <n>    host bit depth, 16 or 32 (default 16)\n");
  fprintf(stderr, "  -mode <n>      1 = single application (default), 0 = desktop\n");
  fprintf(stderr, "  -dia <0|1>     dynamic input area (default 1)\n");
  fprintf(stderr, "  -app <name>    start this application instead of the Launcher\n");
  fprintf(stderr, "  -boot <dir>    directory holding the DOL and the vfs/ to seed from (default: where the DOL was loaded from)\n");
  fprintf(stderr, "  -home <dir>    PumpkinOS home (default: the boot directory, else pumpkinos/ on the SD card or USB device, else the RAM disk)\n");
  fprintf(stderr, "  -vfs <dir>     VFS root directory (default <home>/vfs)\n");
  fprintf(stderr, "  -log <file>    log file (default pumpkin.log in <home>; \"gecko\" = the USB Gecko in slot B)\n");
  fprintf(stderr, "  -gecko         log to the USB Gecko in slot B\n");
  fprintf(stderr, "  -ramdisk [KB]  use the RAM disk (default 16384 KB) as home even when an SD card is present\n");
  fprintf(stderr, "  -seed          copy vfs/ from the boot directory again (overwrites the application PRCs)\n");
  fprintf(stderr, "  -nonet         do not bring the network up\n");
  fprintf(stderr, "  -d <level>     log level: 0 = errors, 1 = info (default), 2 = trace; <level>:<system> for one system\n");
  fprintf(stderr, "the same options are read from %s next to the DOL first\n", WII_CONFIG_FILE);
}

static int dir_exists(const char *path) {
  sys_stat_t st;

  return sys_stat(path, &st) == 0 && (st.mode & SYS_IFDIR);
}

static void mkdir_if_missing(const char *path) {
  if (!dir_exists(path)) {
    if (sys_mkdir(path) != 0) {
      debug(DEBUG_ERROR, "MAIN", "could not create directory \"%s\"", path);
    }
  }
}

// "dev:/dir/" from "dev:/dir/" or "dev:/dir": a device prefix is kept as is
static void add_slash(char *path, int len) {
  int n = sys_strlen(path);

  if (n > 0 && n < len-1 && path[n-1] != '/' && path[n-1] != ':') {
    path[n] = '/';
    path[n+1] = 0;
  }
}

// "dev:/dir" from "dev:/dir/"
static void strip_slash(char *path) {
  int n = sys_strlen(path);

  if (n > 1 && path[n-1] == '/' && path[n-2] != ':') path[n-1] = 0;
}

// is this a path on the RAM disk ?
static int is_ramdisk(const char *path) {
  return !strncmp(path, WII_RAMDISK ":", sizeof(WII_RAMDISK));
}

// the directory the DOL was loaded from, from argv[0] ("sd:/apps/pumpkinos/
// boot.dol", "usb:/apps/pumpkinos/boot.dol"); empty when the loader gave
// none (an emulator, a network loader)
static void find_boot(const char *arg0, char *boot, int len) {
  char buf[MAX_PATH_LEN];
  char *p, *last;
  int i;

  boot[0] = 0;
  if (arg0 == NULL || arg0[0] == 0) return;

  sys_strncpy(buf, arg0, MAX_PATH_LEN-1);
  buf[MAX_PATH_LEN-1] = 0;
  for (i = 0; buf[i]; i++) {
    if (buf[i] == '\\') buf[i] = '/';
  }
  for (last = NULL, p = buf; *p; p++) {
    if (*p == '/' || *p == ':') last = p;
  }
  if (last != NULL && strchr(buf, ':') != NULL) {
    last[1] = 0;
    sys_strncpy(boot, buf, len-1);
    boot[len-1] = 0;
    add_slash(boot, len);
  }
}

// the options file next to the DOL: the same options as the command line,
// separated by blanks or line breaks, '#' starts a comment. They are placed
// before the real arguments, which therefore win.
static int read_config(const char *boot, char **argv, int max, char *buf, int size) {
  char path[MAX_PATH_LEN];
  int fd, n, i, argc = 0, comment = 0;

  if (boot[0] == 0) return 0;
  sys_snprintf(path, MAX_PATH_LEN-1, "%s%s", boot, WII_CONFIG_FILE);
  if ((fd = sys_open(path, SYS_READ)) == -1) {
    return 0;
  }
  n = sys_read(fd, (uint8_t *)buf, size-1);
  sys_close(fd);
  if (n <= 0) return 0;
  buf[n] = 0;

  for (i = 0; i < n && argc < max; i++) {
    if (buf[i] == '#') comment = 1;
    if (buf[i] == '\n' || buf[i] == '\r') comment = 0;
    if (comment || buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n' || buf[i] == '\r') {
      buf[i] = 0;
    } else if (i == 0 || buf[i-1] == 0) {
      argv[argc++] = &buf[i];
    }
  }

  return argc;
}

static int copy_file(const char *src, const char *dst) {
  uint8_t buf[16384];
  int fin, fout, n, r = 0;

  if ((fin = sys_open(src, SYS_READ)) == -1) {
    return -1;
  }
  if ((fout = sys_create(dst, SYS_WRITE | SYS_TRUNC, 0644)) == -1) {
    sys_close(fin);
    return -1;
  }
  while ((n = sys_read(fin, buf, sizeof(buf))) > 0) {
    if (sys_write(fout, buf, n) != n) {
      r = -1;
      break;
    }
  }
  if (n < 0) r = -1;
  sys_close(fout);
  sys_close(fin);

  return r;
}

// recursive copy of a directory tree (the initial VFS to the home). The
// directory is listed first and closed before its entries are copied.
static int copy_tree(const char *src, const char *dst) {
  char s[MAX_PATH_LEN], d[MAX_PATH_LEN];
  char *names, *name;
  sys_dir_t *dir;
  sys_stat_t st;
  int n, count, len, r = 0;

  if (!dir_exists(dst) && sys_mkdir(dst) != 0) {
    debug(DEBUG_ERROR, "MAIN", "could not create directory \"%s\"", dst);
    return -1;
  }

  if ((dir = sys_opendir(src)) == NULL) {
    debug(DEBUG_ERROR, "MAIN", "could not open directory \"%s\"", src);
    return -1;
  }

  len = 16 * FILE_PATH;
  if ((names = sys_calloc(1, len)) == NULL) {
    sys_closedir(dir);
    return -1;
  }
  for (n = 0, count = 0;;) {
    name = &names[n];
    if (sys_readdir(dir, name, FILE_PATH) != 0) break;
    if (!strcmp(name, ".") || !strcmp(name, "..")) continue;
    n += sys_strlen(name) + 1;
    count++;
    if (n + FILE_PATH > len) {
      len *= 2;
      if ((names = sys_realloc(names, len)) == NULL) {
        sys_closedir(dir);
        return -1;
      }
    }
  }
  sys_closedir(dir);

  for (name = names; count > 0; count--, name += sys_strlen(name) + 1) {
    sys_snprintf(s, MAX_PATH_LEN-1, "%s/%s", src, name);
    sys_snprintf(d, MAX_PATH_LEN-1, "%s/%s", dst, name);
    if (sys_stat(s, &st) != 0) continue;
    if (st.mode & SYS_IFDIR) {
      if (copy_tree(s, d) != 0) r = -1;
    } else {
      debug(DEBUG_INFO, "MAIN", "copying \"%s\" to \"%s\"", s, d);
      if (copy_file(s, d) != 0) {
        debug(DEBUG_ERROR, "MAIN", "could not copy \"%s\"", s);
        r = -1;
      }
    }
  }
  sys_free(names);

  return r;
}

// the VFS archive linked into the program (mkvfsblob.py): a sequence of
// records, each a one byte name length, the name (relative to vfs/, with
// '/' separators), a big endian 32 bit size and the data; a zero name
// length ends it. Directories are created as needed.
static int extract_blob(const char *vfsdir) {
  char path[MAX_PATH_LEN], *p;
  const uint8_t *blob, *end;
  uint32_t size, nlen;
  int fd, n, count = 0;

  if (vfsblob_bin == NULL || &vfsblob_bin_size == NULL) return -1;
  blob = vfsblob_bin;
  end = blob + vfsblob_bin_size;

  while (blob < end && (nlen = *blob++) != 0) {
    if (blob + nlen + 4 > end || nlen >= MAX_PATH_LEN - sys_strlen(vfsdir) - 1) return -1;
    sys_snprintf(path, MAX_PATH_LEN-1, "%s%.*s", vfsdir, (int)nlen, (const char *)blob);
    blob += nlen;
    size = ((uint32_t)blob[0] << 24) | ((uint32_t)blob[1] << 16) | ((uint32_t)blob[2] << 8) | blob[3];
    blob += 4;
    if (blob + size > end) return -1;

    // the directories on the way
    for (p = path + sys_strlen(vfsdir); *p; p++) {
      if (*p == '/') {
        *p = 0;
        mkdir_if_missing(path);
        *p = '/';
      }
    }
    if ((fd = sys_create(path, SYS_WRITE | SYS_TRUNC, 0644)) == -1) {
      debug(DEBUG_ERROR, "MAIN", "could not create \"%s\"", path);
      return -1;
    }
    n = sys_write(fd, (uint8_t *)blob, size);
    sys_close(fd);
    if (n != (int)size) {
      debug(DEBUG_ERROR, "MAIN", "could not write \"%s\" (%d of %u bytes)", path, n, size);
      return -1;
    }
    blob += size;
    count++;
  }
  debug(DEBUG_INFO, "MAIN", "%d files extracted from the built-in archive", count);

  return 0;
}

// can files be created in this directory ?
static int writable_dir(const char *dir) {
  char path[MAX_PATH_LEN];
  int fd;

  sys_snprintf(path, MAX_PATH_LEN-1, "%spumpkin.err", dir);
  if ((fd = sys_create(path, SYS_WRITE | SYS_TRUNC, 0644)) == -1) {
    return 0;
  }
  sys_close(fd);

  return 1;
}

// the first mounted FAT device holding apps/pumpkinos/ or pumpkinos/, else
// pumpkinos/ on the first mounted FAT device; "" when there is none
static void find_fat_home(char *home, int len) {
  char path[MAX_PATH_LEN];
  int i, first = -1;

  home[0] = 0;
  for (i = 0; fat_devices[i]; i++) {
    sys_snprintf(path, MAX_PATH_LEN-1, "%s:/", fat_devices[i]);
    if (!dir_exists(path)) continue;
    if (first < 0) first = i;
    sys_snprintf(path, MAX_PATH_LEN-1, "%s:/%s/", fat_devices[i], WII_APP_DIR);
    if (dir_exists(path)) {
      sys_strncpy(home, path, len-1);
      return;
    }
    sys_snprintf(path, MAX_PATH_LEN-1, "%s:/%s/", fat_devices[i], WII_HOME_DIR);
    if (dir_exists(path)) {
      sys_strncpy(home, path, len-1);
      return;
    }
  }
  if (first >= 0) {
    sys_snprintf(home, len-1, "%s:/%s/", fat_devices[first], WII_HOME_DIR);
  }
}

int main(int argc, char *argv[]) {
  char boot[MAX_PATH_LEN], home[MAX_PATH_LEN];
  char vfsdir[MAX_PATH_LEN], tmpdir[MAX_PATH_LEN], logfile[MAX_PATH_LEN], errfile[MAX_PATH_LEN], path[MAX_PATH_LEN], seed[MAX_PATH_LEN];
  char *args[WII_CONFIG_MAX_ARGS + 1], config[WII_CONFIG_MAX_SIZE];
  const char *vfs_arg = NULL, *log_arg = NULL, *boot_arg = NULL, *home_arg = NULL;
  char *d, *p;
  int i, nargs = 0, level = DEBUG_INFO, net = 1, reseed = 0, gecko = 0, ramdisk = 0, have_fat;

  L2Enhance();

  // the buttons that stop PumpkinOS
  SYS_SetResetCallback(reset_callback);
  SYS_SetPowerCallback(power_callback);

  // the Wiimotes, with the IR camera on (SDL turns the pointer position into
  // mouse motion, scaled from this resolution), and USB keyboards (SDL reads
  // them); SDL's joystick driver expects WPAD to be initialized already
  WPAD_Init();
  WPAD_SetPowerButtonCallback(wpad_power_callback);
  WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
  WPAD_SetVRes(WPAD_CHAN_ALL, 640, 480);
  KEYBOARD_Init(NULL);

  // the front SD card and USB mass storage (libfat mounts what it finds and
  // makes the directory of the DOL the current one when the loader said
  // where it is)
  have_fat = fatInitDefault();

  // the USB Gecko in memory card slot B (early consoles), when there is
  // one, gets stdout
  if (usb_isgeckoalive(CARD_SLOTB)) {
    CON_EnableGecko(CARD_SLOTB, 0);
    gecko = 1;
  }

  // "-boot" is the only option that must be known before the options file
  // next to the DOL can be read
  for (i = 1; i < argc-1; i++) {
    if (!strcmp(argv[i], "-boot")) boot_arg = argv[i+1];
  }
  if (boot_arg) {
    sys_strncpy(boot, boot_arg, MAX_PATH_LEN-1);
    boot[MAX_PATH_LEN-1] = 0;
    add_slash(boot, MAX_PATH_LEN);
  } else {
    find_boot(argc > 0 ? argv[0] : NULL, boot, MAX_PATH_LEN);
  }
  if (boot[0] == 0) {
    // no path from the loader: apps/pumpkinos/ or pumpkinos/ on the SD card
    // or the USB device
    find_fat_home(boot, MAX_PATH_LEN);
  }

  // the options: pumpkin.cfg next to the DOL first, then the command line
  args[nargs++] = argc > 0 && argv[0] ? argv[0] : "boot.dol";
  i = WII_CONFIG_MAX_ARGS - argc;
  if (i > 0) nargs += read_config(boot, &args[nargs], i, config, WII_CONFIG_MAX_SIZE);
  for (i = 1; i < argc && nargs < WII_CONFIG_MAX_ARGS; i++) {
    args[nargs++] = argv[i];
  }
  args[nargs] = NULL;
  argv = args;
  argc = nargs;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-nodia")) {
      wii_script_set("dia", 0);
    } else if (!strcmp(argv[i], "-nonet")) {
      net = 0;
    } else if (!strcmp(argv[i], "-nodvd")) {
      // (accepted for a pumpkin.cfg shared with the GameCube: no disc here)
    } else if (i < argc-1 && !strcmp(argv[i], "-dns")) {
      i++;  // (GameCube only: the Wii uses the IOS resolver)
    } else if (!strcmp(argv[i], "-seed")) {
      reseed = 1;
    } else if (!strcmp(argv[i], "-gecko")) {
      gecko = 2;
    } else if (!strcmp(argv[i], "-ramdisk")) {
      ramdisk = 1;
      if (i < argc-1 && argv[i+1][0] >= '0' && argv[i+1][0] <= '9') {
        sys_gc_ramdisk_set_size(sys_atoi(argv[++i]));
      }
    } else if (i < argc-1 && !strcmp(argv[i], "-app")) {
      wii_script_set_launcher(argv[++i]);
    } else if (i < argc-1 && !strcmp(argv[i], "-boot")) {
      i++;  // already handled
    } else if (i < argc-1 && !strcmp(argv[i], "-home")) {
      home_arg = argv[++i];
    } else if (i < argc-1 && !strcmp(argv[i], "-vfs")) {
      vfs_arg = argv[++i];
    } else if (i < argc-1 && !strcmp(argv[i], "-log")) {
      log_arg = argv[++i];
    } else if (i < argc-1 && !strcmp(argv[i], "-d")) {
      // "-d <level>" or "-d <level>:<system>" (like pit_main)
      d = argv[++i];
      if ((p = strchr(d, ':')) != NULL) {
        debug_setsyslevel(p+1, sys_atoi(d));
      } else {
        level = sys_atoi(d);
      }
    } else if (i < argc-1 && argv[i][0] == '-' && wii_script_set(argv[i]+1, sys_atoi(argv[i+1])) == 0) {
      i++;
    } else if (argv[i][0] == '-') {
      usage();
      return 1;
    }
    // anything else is ignored
  }

  sys_init();
  debug_setsyslevel(NULL, level);

  // the home: the boot directory when it is writable, else pumpkinos/ on
  // the SD card or the USB device, else the RAM disk
  home[0] = 0;
  if (home_arg) {
    sys_strncpy(home, home_arg, MAX_PATH_LEN-1);
    home[MAX_PATH_LEN-1] = 0;
    add_slash(home, MAX_PATH_LEN);
    if (is_ramdisk(home)) ramdisk = 1;
  } else if (!ramdisk) {
    if (boot[0] && !is_ramdisk(boot)) {
      sys_strncpy(path, boot, MAX_PATH_LEN-1);
      strip_slash(path);
      if (!dir_exists(path)) sys_mkdir(path);
      if (writable_dir(boot)) sys_strncpy(home, boot, MAX_PATH_LEN-1);
    }
    if (home[0] == 0 && have_fat) {
      find_fat_home(home, MAX_PATH_LEN);
      if (home[0]) {
        sys_strncpy(path, home, MAX_PATH_LEN-1);
        strip_slash(path);
        if (!dir_exists(path)) sys_mkdir(path);
        if (!writable_dir(home)) home[0] = 0;
      }
    }
    if (home[0] == 0) ramdisk = 1;
  }
  if (ramdisk && (home[0] == 0 || is_ramdisk(home))) {
    if (sys_gc_ramdisk_mount(WII_RAMDISK) != 0) {
      // nothing at all to run from
      if (gecko) printf("PumpkinOS: no storage device and no RAM disk, giving up\n");
      return 1;
    }
    if (home[0] == 0) sys_snprintf(home, MAX_PATH_LEN-1, "%s:/%s/", WII_RAMDISK, WII_HOME_DIR);
  }
  sys_strncpy(path, home, MAX_PATH_LEN-1);
  strip_slash(path);
  mkdir_if_missing(path);

  // the log: a file in the home (an SD card or USB device), or the USB Gecko
  if (log_arg) {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%s", log_arg);
  } else if (gecko == 2 || is_ramdisk(home)) {
    sys_strncpy(logfile, gecko ? "stdout" : "stderr", MAX_PATH_LEN-1);
  } else {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%spumpkin.log", home);
  }
  if (!sys_strcmp(logfile, "gecko")) sys_strcpy(logfile, "stdout");
  if (!is_ramdisk(home)) {
    // there is no console: keep stderr in a file next to the log
    sys_snprintf(errfile, MAX_PATH_LEN-1, "%spumpkin.err", home);
    freopen(errfile, "w", stderr);
  }
  debug_init(logfile);
  debug(DEBUG_INFO, "MAIN", "%s starting on %s (boot \"%s\", home \"%s\"%s, IOS %d)", SYSTEM_NAME, SYSTEM_OS, boot, home, is_ramdisk(home) ? ", a RAM disk" : "", IOS_GetVersion());
  if (argc > 0) debug(DEBUG_INFO, "MAIN", "argv[0] \"%s\"", argv[0]);
  for (i = 1; i < argc; i++) {
    debug(DEBUG_INFO, "MAIN", "option \"%s\"", argv[i]);
  }
  debug(DEBUG_INFO, "MAIN", "FAT devices %s, USB Gecko %s", have_fat ? "mounted" : "none", gecko ? "present" : "absent");
  sys_gc_memlog("startup");

  if (vfs_arg) {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%s", vfs_arg);
  } else {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%svfs", home);
  }
  add_slash(vfsdir, MAX_PATH_LEN);
  sys_snprintf(tmpdir, MAX_PATH_LEN-1, "%stmp", home);

  if (!vfs_arg) {
    // first boot: seed the VFS (the application PRCs in vfs/app_install)
    // from the boot directory or from the built-in archive; -seed does it
    // again
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_install", vfsdir);
    if (reseed || !dir_exists(path)) {
      sys_snprintf(seed, MAX_PATH_LEN-1, "%svfs", boot);
      if (boot[0] == 0 || !sys_strcmp(seed, vfsdir) || !sys_strncmp(seed, vfsdir, sys_strlen(seed)) || !dir_exists(seed)) {
        seed[0] = 0;
      }
      sys_snprintf(path, MAX_PATH_LEN-1, "%svfs", home);
      if (seed[0]) {
        debug(DEBUG_INFO, "MAIN", "first boot: copying \"%s\" to \"%s\"", seed, path);
        if (copy_tree(seed, path) != 0) {
          debug(DEBUG_ERROR, "MAIN", "the VFS copy failed (device full?)");
        }
      } else if (vfsblob_bin != NULL) {
        debug(DEBUG_INFO, "MAIN", "first boot: extracting the built-in archive to \"%s\"", path);
        mkdir_if_missing(path);
        if (extract_blob(vfsdir) != 0) {
          debug(DEBUG_ERROR, "MAIN", "the extraction failed");
        }
      } else {
        debug(DEBUG_ERROR, "MAIN", "nothing to install the applications from (no vfs/ next to the DOL, no built-in archive)");
      }
    }
    // make sure the writable parts exist
    sys_snprintf(path, MAX_PATH_LEN-1, "%svfs", home);
    mkdir_if_missing(path);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_install", vfsdir);
    mkdir_if_missing(path);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_storage", vfsdir);
    mkdir_if_missing(path);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_card", vfsdir);
    mkdir_if_missing(path);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_card/PALM", vfsdir);
    mkdir_if_missing(path);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_card/PALM/Programs", vfsdir);
    mkdir_if_missing(path);
  }
  debug(DEBUG_INFO, "MAIN", "vfs \"%s\"", vfsdir);

  // the network configured in the Wii settings (Wi-Fi or USB ethernet)
  if (net) {
    sys_gc_net_init(WII_NET_TIMEOUT_MS);
  } else {
    debug(DEBUG_INFO, "MAIN", "network disabled");
  }

  sys_gc_memlog("before SDL");
  ptr_init();
  thread_init();
  vfs_init();

  if (liblsdl3_load() != 0) {
    debug(DEBUG_ERROR, "MAIN", "SDL3 initialization failed");
    vfs_finish();
    thread_close();
    debug_close();
    sys_gc_net_finish();
    return 1;
  }
  liblsdl3_init(0, 0);
  sys_gc_memlog("SDL loaded");

  vfs_local_mount(vfsdir, "/");
  mkdir_if_missing(tmpdir);
  sys_set_tmpdir(tmpdir);

  libos_app_init(0);
  libos_start(0);

  vfs_finish();
  thread_close();
  liblsdl3_unload();
  debug(DEBUG_INFO, "MAIN", "%s stopping%s", SYSTEM_NAME, power_requested ? " (power off)" : "");
  debug_close();

  sys_gc_net_finish();
  WPAD_Shutdown();
  fatUnmount("sd:/");
  fatUnmount("usb:/");

  if (power_requested) {
    SYS_ResetSystem(SYS_POWEROFF, 0, 0);
  }

  // back to the loader (the Homebrew Channel stub), else the System Menu
  return 0;
}
