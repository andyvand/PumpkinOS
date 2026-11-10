// PumpkinOS entry point for the Sony PlayStation 3 (PSL1GHT + SDL3).
//
// Mirrors src/ps2/main.c: everything is linked statically, there is no
// script engine and the OS runs in single application mode (the Launcher and
// the applications run in-process, as cooperative threads).
//
// Layout: EBOOT.BIN is started from the USRDIR of the installed package
// (/dev_hdd0/game/PUMP00001/USRDIR/) or of the disc image
// (/dev_bdvd/PS3_GAME/USRDIR/, read-only); next to it are vfs/ (the
// application PRCs in vfs/app_install), cert.pem, soundfont.sf2 and the
// options file pumpkin.cfg. The home, where PumpkinOS keeps its storage
// (vfs/), its temporary files (tmp/) and the log (pumpkin.log, pumpkin.err),
// is /dev_hdd0/PumpkinOS/ by default, shared by the package and the disc.
// On the first boot vfs/ is copied from the boot directory to the home and
// PumpkinOS deploys the PRCs from there; when the boot directory carries
// PRCs of another build (an updated package or disc) they are copied to
// the home's vfs/app_install again (the stamp file vfs/seed.txt tells).
//
// The pad is the mouse: the left stick and the D-pad move a pointer, Cross
// is the pen (see the PS3 notes in liblsdl3). "Quit Game" in the XMB (PS
// button) stops PumpkinOS and returns to the XMB.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/process.h>
#include <sysutil/sysutil.h>

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
#include "ps3.h"

// main thread priority and stack size (the default stack is 64KB)
SYS_PROCESS_PARAM(1001, 0x100000);

#define MAX_PATH_LEN 256

// the default home (the hard disk, outside the game directories)
#define PS3_HOME "/dev_hdd0/PumpkinOS/"

// where to look for vfs/ when argv[0] gives no path: the installed package,
// then the disc
#define PS3_PKG_BOOT  "/dev_hdd0/game/PUMP00001/USRDIR/"
#define PS3_DISC_BOOT "/dev_bdvd/PS3_GAME/USRDIR/"

// how long to wait for the network connection at startup
#define PS3_NET_TIMEOUT_MS 10000

// the options file next to EBOOT.BIN (the XMB passes no arguments)
#define PS3_CONFIG_FILE "pumpkin.cfg"
#define PS3_CONFIG_MAX_ARGS 64
#define PS3_CONFIG_MAX_SIZE 4096

// what build the PRCs in the home were copied from
#define PS3_SEED_STAMP "seed.txt"
#define PS3_BUILD_STAMP __DATE__ " " __TIME__

extern int liblsdl3_load(void);
extern int liblsdl3_init(int pe, script_ref_t obj);
extern int liblsdl3_unload(void);

extern int libos_app_init(int pe);
extern int libos_start(int pe);

#ifndef NO_HTTPS
extern int liblopenssl_load(void);
extern int liblopenssl_init(int pe, script_ref_t obj);
#endif

static volatile int quit_requested = 0;

// XMB events (slot 0; SDL's video driver uses slot 1): "Quit Game" asks
// PumpkinOS to stop (the console waits a few seconds for the program to end)
static void sysutil_callback(u64 status, u64 param, void *data) {
  SDL_Event ev;

  if (status == SYSUTIL_EXIT_GAME && !quit_requested) {
    quit_requested = 1;
    debug(DEBUG_INFO, "MAIN", "XMB: quit requested");
    sys_memset(&ev, 0, sizeof(ev));
    ev.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&ev);
  }
}

static void usage(void) {
  fprintf(stderr, "usage: EBOOT.BIN [options]\n");
  fprintf(stderr, "  -width <n>     screen width in pixels (default 1280)\n");
  fprintf(stderr, "  -height <n>    screen height in pixels (default 720)\n");
  fprintf(stderr, "  -density <n>   72 (low) or 144 (double, default)\n");
  fprintf(stderr, "  -depth <n>     application bit depth (default 16)\n");
  fprintf(stderr, "  -hdepth <n>    host bit depth, 16 or 32 (default 16)\n");
  fprintf(stderr, "  -mode <n>      1 = single application (default), 0 = desktop\n");
  fprintf(stderr, "  -dia <0|1>     dynamic input area (default 1)\n");
  fprintf(stderr, "  -app <name>    start this application instead of the Launcher\n");
  fprintf(stderr, "  -boot <dir>    directory holding vfs/ to seed from (default: where EBOOT.BIN is)\n");
  fprintf(stderr, "  -home <dir>    PumpkinOS home (default %s)\n", PS3_HOME);
  fprintf(stderr, "  -vfs <dir>     VFS root directory (default <home>/vfs)\n");
  fprintf(stderr, "  -log <file>    log file (default <home>/pumpkin.log)\n");
  fprintf(stderr, "  -seed          copy vfs/ from the boot directory again\n");
  fprintf(stderr, "  -nonet         do not bring the network up\n");
  fprintf(stderr, "  -d <level>     log level: 0 = errors, 1 = info (default), 2 = trace; <level>:<system> for one system\n");
  fprintf(stderr, "the same options are read from %s next to EBOOT.BIN first\n", PS3_CONFIG_FILE);
}

static int dir_exists(const char *path) {
  sys_stat_t st;

  return sys_stat(path, &st) == 0 && (st.mode & SYS_IFDIR);
}

static int file_exists(const char *path) {
  sys_stat_t st;

  return sys_stat(path, &st) == 0 && !(st.mode & SYS_IFDIR);
}

static void mkdir_if_missing(const char *path) {
  if (!dir_exists(path)) {
    if (sys_mkdir(path) != 0) {
      debug(DEBUG_ERROR, "MAIN", "could not create directory \"%s\"", path);
    }
  }
}

// "dir/" from "dir/" or "dir"
static void add_slash(char *path, int len) {
  int n = sys_strlen(path);

  if (n > 0 && n < len-1 && path[n-1] != '/') {
    path[n] = '/';
    path[n+1] = 0;
  }
}

// the directory EBOOT.BIN was started from, from argv[0]
// ("/dev_hdd0/game/PUMP00001/USRDIR/EBOOT.BIN"); else the installed package
// or the disc, whichever has a vfs/
static void find_boot(const char *arg0, char *boot, int len) {
  char path[MAX_PATH_LEN];
  char *p;

  boot[0] = 0;
  if (arg0 && arg0[0] == '/') {
    sys_strncpy(boot, arg0, len-1);
    boot[len-1] = 0;
    if ((p = strrchr(boot, '/')) != NULL) p[1] = 0;
    else boot[0] = 0;
  }

  if (boot[0] == 0) {
    sys_snprintf(path, MAX_PATH_LEN-1, "%svfs", PS3_PKG_BOOT);
    sys_strncpy(boot, dir_exists(path) ? PS3_PKG_BOOT : PS3_DISC_BOOT, len-1);
  }
  add_slash(boot, len);
}

// the options file next to EBOOT.BIN: the same options as the command line,
// separated by blanks or line breaks, '#' starts a comment. They are placed
// before the real arguments, which therefore win.
static int read_config(const char *boot, char **argv, int max, char *buf, int size) {
  char path[MAX_PATH_LEN];
  int fd, n, i, argc = 0, comment = 0;

  sys_snprintf(path, MAX_PATH_LEN-1, "%s%s", boot, PS3_CONFIG_FILE);
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
  static uint8_t buf[65536];
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

// recursive copy of a directory tree (the initial VFS to the home); the
// directory is listed and closed before its entries are copied
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

  // the names, one after the other, each terminated by a zero
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

// is this a directory with no entries ? (a missing directory counts as empty)
static int dir_empty(const char *path) {
  char name[FILE_PATH];
  sys_dir_t *dir;
  int empty = 1;

  if ((dir = sys_opendir(path)) == NULL) return 1;
  while (sys_readdir(dir, name, sizeof(name)) == 0) {
    if (strcmp(name, ".") && strcmp(name, "..")) {
      empty = 0;
      break;
    }
  }
  sys_closedir(dir);

  return empty;
}

// the build the home's PRCs come from (vfs/seed.txt), or "" when unknown
static void read_stamp(const char *vfsdir, char *stamp, int len) {
  char path[MAX_PATH_LEN];
  int fd, n;

  stamp[0] = 0;
  sys_snprintf(path, MAX_PATH_LEN-1, "%s%s", vfsdir, PS3_SEED_STAMP);
  if ((fd = sys_open(path, SYS_READ)) == -1) return;
  n = sys_read(fd, (uint8_t *)stamp, len-1);
  sys_close(fd);
  stamp[n > 0 ? n : 0] = 0;
}

static void write_stamp(const char *vfsdir) {
  char path[MAX_PATH_LEN];
  int fd;

  sys_snprintf(path, MAX_PATH_LEN-1, "%s%s", vfsdir, PS3_SEED_STAMP);
  if ((fd = sys_create(path, SYS_WRITE | SYS_TRUNC, 0644)) == -1) return;
  sys_write(fd, (uint8_t *)PS3_BUILD_STAMP, sys_strlen(PS3_BUILD_STAMP));
  sys_close(fd);
}

int main(int argc, char *argv[]) {
  char boot[MAX_PATH_LEN], home[MAX_PATH_LEN], stamp[64];
  char vfsdir[MAX_PATH_LEN], tmpdir[MAX_PATH_LEN], logfile[MAX_PATH_LEN], errfile[MAX_PATH_LEN], path[MAX_PATH_LEN], seed[MAX_PATH_LEN];
  char *args[PS3_CONFIG_MAX_ARGS + 1], config[PS3_CONFIG_MAX_SIZE];
  const char *vfs_arg = NULL, *log_arg = NULL, *boot_arg = NULL, *home_arg = NULL;
  char *d, *p;
  int i, nargs = 0, level = DEBUG_INFO, net = 1, reseed = 0;

  // "-boot" is the only option that must be known before the options file
  // next to EBOOT.BIN can be read
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

  // the options: pumpkin.cfg next to EBOOT.BIN first, then the command line
  args[nargs++] = argc > 0 ? argv[0] : "EBOOT.BIN";
  i = PS3_CONFIG_MAX_ARGS - argc;
  if (i > 0) nargs += read_config(boot, &args[nargs], i, config, PS3_CONFIG_MAX_SIZE);
  for (i = 1; i < argc && nargs < PS3_CONFIG_MAX_ARGS; i++) {
    args[nargs++] = argv[i];
  }
  args[nargs] = NULL;
  argv = args;
  argc = nargs;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-nodia")) {
      ps3_script_set("dia", 0);
    } else if (!strcmp(argv[i], "-nonet")) {
      net = 0;
    } else if (!strcmp(argv[i], "-seed")) {
      reseed = 1;
    } else if (i < argc-1 && !strcmp(argv[i], "-app")) {
      ps3_script_set_launcher(argv[++i]);
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
    } else if (i < argc-1 && argv[i][0] == '-' && ps3_script_set(argv[i]+1, sys_atoi(argv[i+1])) == 0) {
      i++;
    } else if (argv[i][0] == '-') {
      usage();
      return 1;
    }
    // anything else is ignored
  }

  sys_init();
  debug_setsyslevel(NULL, level);

  sys_strncpy(home, home_arg ? home_arg : PS3_HOME, MAX_PATH_LEN-1);
  home[MAX_PATH_LEN-1] = 0;
  add_slash(home, MAX_PATH_LEN);

  // the home directory must exist before anything is written there
  sys_strncpy(path, home, MAX_PATH_LEN-1);
  i = sys_strlen(path);
  if (i > 1 && path[i-1] == '/') path[i-1] = 0;
  mkdir_if_missing(path);

  if (log_arg) {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%s", log_arg);
  } else {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%spumpkin.log", home);
  }
  sys_snprintf(errfile, MAX_PATH_LEN-1, "%spumpkin.err", home);
  // there is no console (printf goes to the debug TTY): stderr to a file
  if (sys_strcmp(logfile, "stderr")) freopen(errfile, "w", stderr);
  debug_init(logfile);
  debug(DEBUG_INFO, "MAIN", "%s starting on %s (boot \"%s\", home \"%s\")", SYSTEM_NAME, SYSTEM_OS, boot, home);
  if (argc > 0) debug(DEBUG_INFO, "MAIN", "argv[0] \"%s\"", argv[0]);
  for (i = 1; i < argc; i++) {
    debug(DEBUG_INFO, "MAIN", "option \"%s\"", argv[i]);
  }

  // relative names (soundfont.sf2) are looked up next to EBOOT.BIN
  if (chdir(boot) != 0) {
    debug(DEBUG_ERROR, "MAIN", "could not change to \"%s\"", boot);
  }

  if (vfs_arg) {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%s", vfs_arg);
  } else {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%svfs", home);
  }
  add_slash(vfsdir, MAX_PATH_LEN);
  // the temporary files live in the home, next to vfs/
  sys_snprintf(tmpdir, MAX_PATH_LEN-1, "%stmp", home);

  if (!vfs_arg) {
    // first boot: copy the VFS shipped next to EBOOT.BIN to the home; -seed
    // does it again, and so does a home whose app_install and app_storage
    // are both empty (an install that failed: the PRCs are removed as they
    // are deployed)
    sys_snprintf(seed, MAX_PATH_LEN-1, "%svfs", boot);
    sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_install", vfsdir);
    if (!reseed && dir_exists(path) && dir_empty(path)) {
      sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_storage", vfsdir);
      if (dir_empty(path)) {
        debug(DEBUG_INFO, "MAIN", "no applications installed: seeding again");
        reseed = 1;
      }
      sys_snprintf(path, MAX_PATH_LEN-1, "%sapp_install", vfsdir);
    }
    if (sys_strcmp(seed, vfsdir) && sys_strncmp(seed, vfsdir, sys_strlen(seed))) {
      if (reseed || !dir_exists(path)) {
        sys_snprintf(path, MAX_PATH_LEN-1, "%svfs", home);
        debug(DEBUG_INFO, "MAIN", "first boot: copying \"%s\" to \"%s\"", seed, path);
        if (dir_exists(seed)) {
          if (copy_tree(seed, path) != 0) {
            debug(DEBUG_ERROR, "MAIN", "the VFS copy failed (disk full?)");
          }
          write_stamp(vfsdir);
        } else {
          debug(DEBUG_ERROR, "MAIN", "\"%s\" not found: nothing to install", seed);
        }
      } else {
        // the PRCs of another build (an update): installed again
        read_stamp(vfsdir, stamp, sizeof(stamp));
        if (sys_strcmp(stamp, PS3_BUILD_STAMP)) {
          sys_snprintf(seed, MAX_PATH_LEN-1, "%svfs/app_install", boot);
          debug(DEBUG_INFO, "MAIN", "PRCs from build \"%s\", this is \"%s\": copying \"%s\" to \"%s\"", stamp, PS3_BUILD_STAMP, seed, path);
          if (dir_exists(seed) && copy_tree(seed, path) == 0) {
            write_stamp(vfsdir);
          }
        }
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

  sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sysutil_callback, NULL);

  // the connection configured in the XMB network settings
  if (net) {
    sys_ps3_net_init(PS3_NET_TIMEOUT_MS);
  } else {
    debug(DEBUG_INFO, "MAIN", "network disabled");
  }

  sys_ps3_memlog("startup");
  ptr_init();
  thread_init();
  vfs_init();

  if (liblsdl3_load() != 0) {
    debug(DEBUG_ERROR, "MAIN", "SDL3 initialization failed");
    vfs_finish();
    thread_close();
    debug_close();
    sys_ps3_net_finish();
    sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);
    return 1;
  }
  liblsdl3_init(0, 0);
  sys_ps3_memlog("SDL loaded");

  vfs_local_mount(vfsdir, "/");
  mkdir_if_missing(tmpdir);
  sys_set_tmpdir(tmpdir);

#ifndef NO_HTTPS
  // OpenSSL looks for the CA bundle in SSL_CERT_FILE: cert.pem next to
  // EBOOT.BIN, or in the home
  sys_snprintf(path, MAX_PATH_LEN-1, "%scert.pem", boot);
  if (!file_exists(path)) {
    sys_snprintf(path, MAX_PATH_LEN-1, "%scert.pem", home);
  }
  if (file_exists(path)) {
    debug(DEBUG_INFO, "MAIN", "CA bundle \"%s\"", path);
    setenv("SSL_CERT_FILE", path, 0);
  }
  debug(DEBUG_INFO, "MAIN", "loading the TLS provider");
  liblopenssl_load();
  liblopenssl_init(0, 0);
  debug(DEBUG_INFO, "MAIN", "TLS provider loaded (OpenSSL)");
#endif
  sys_ps3_memlog("TLS provider loaded");

  libos_app_init(0);
  libos_start(0);

  vfs_finish();
  thread_close();
  liblsdl3_unload();
  debug(DEBUG_INFO, "MAIN", "%s stopping", SYSTEM_NAME);
  debug_close();

  sys_ps3_net_finish();
  sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);

  return 0;
}
