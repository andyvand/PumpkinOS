// PumpkinOS entry point for the Nintendo Wii U (wut + SDL3).
//
// Mirrors src/ps3/main.c: everything is linked statically, there is no
// script engine and the OS runs in single application mode (the Launcher and
// the applications run in-process, as cooperative threads).
//
// Layout: the program is started from a WUHB bundle (Aroma), from the
// Wii U Archive / installed title (Cemu, an installer) or as a bare RPX
// (Homebrew Launcher). The bundle and the title carry vfs/ (the application
// PRCs in vfs/app_install), cert.pem, soundfont.sf2 and the options file
// pumpkin.cfg in their content, mounted read-only at fs:/vol/content/; a
// bare RPX looks for them in the directory it was started from, or in
// sd:/wiiu/apps/pumpkinos/. The home, where PumpkinOS keeps its storage
// (vfs/), its temporary files (tmp/) and the log (pumpkin.log,
// pumpkin.err), is sd:/wiiu/pumpkinos/ (fs:/vol/external01/wiiu/pumpkinos/)
// by default. On the first boot vfs/ is copied from the boot directory to
// the home and PumpkinOS deploys the PRCs from there; when the boot
// directory carries PRCs of another build (an update) they are copied to
// the home's vfs/app_install again (the stamp file vfs/seed.txt tells).
//
// The GamePad touch screen is the pen, its buttons (and a Pro Controller's)
// stand in for a few keys (see the Wii U notes in liblsdl3). The picture is
// shown on the TV and on the GamePad. HOME opens the HOME menu (Aroma; in
// the Homebrew Launcher it quits): closing the software from there stops
// PumpkinOS (SDL runs ProcUI and turns it into SDL_EVENT_QUIT).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <coreinit/debug.h>

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
#include "wiiu.h"

#define MAX_PATH_LEN 256

// the SD card as wut's devoptab sees it (the "fs:" device; "sd:" is not
// always mounted)
#define WIIU_SD       "fs:/vol/external01/"
// the default home
#define WIIU_HOME     WIIU_SD "wiiu/pumpkinos/"
// the content of the bundle / title (read-only)
#define WIIU_CONTENT  "fs:/vol/content/"
// where a bare RPX keeps vfs/ when argv[0] gives no usable path
#define WIIU_RPX_BOOT WIIU_SD "wiiu/apps/pumpkinos/"

// how long to wait for the network connection at startup
#define WIIU_NET_TIMEOUT_MS 5000

// the options files: next to vfs/ in the boot directory, then in the home
// (the boot directory of a bundle is read-only: the home one is the one to
// edit); the Wii U passes no arguments
#define WIIU_CONFIG_FILE "pumpkin.cfg"
#define WIIU_CONFIG_MAX_ARGS 64
#define WIIU_CONFIG_MAX_SIZE 4096

// what build the PRCs in the home were copied from
#define WIIU_SEED_STAMP "seed.txt"
#define WIIU_BUILD_STAMP __DATE__ " " __TIME__

extern int liblsdl3_load(void);
extern int liblsdl3_init(int pe, script_ref_t obj);
extern int liblsdl3_unload(void);

extern int libos_app_init(int pe);
extern int libos_start(int pe);

#ifndef NO_HTTPS
extern int liblopenssl_load(void);
extern int liblopenssl_init(int pe, script_ref_t obj);
#endif

static void usage(void) {
  fprintf(stderr, "usage: pumpkinos.rpx [options]\n");
  fprintf(stderr, "  -width <n>     screen width in pixels (default 1280)\n");
  fprintf(stderr, "  -height <n>    screen height in pixels (default 720)\n");
  fprintf(stderr, "  -density <n>   72 (low) or 144 (double, default)\n");
  fprintf(stderr, "  -depth <n>     application bit depth (default 16)\n");
  fprintf(stderr, "  -hdepth <n>    host bit depth, 16 or 32 (default 16)\n");
  fprintf(stderr, "  -mode <n>      1 = single application (default), 0 = desktop\n");
  fprintf(stderr, "  -dia <0|1>     dynamic input area (default 1)\n");
  fprintf(stderr, "  -app <name>    start this application instead of the Launcher\n");
  fprintf(stderr, "  -boot <dir>    directory holding vfs/ to seed from (default: the content, or where the RPX is)\n");
  fprintf(stderr, "  -home <dir>    PumpkinOS home (default %s)\n", WIIU_HOME);
  fprintf(stderr, "  -vfs <dir>     VFS root directory (default <home>/vfs)\n");
  fprintf(stderr, "  -log <file>    log file (default <home>/pumpkin.log, \"stderr\" for the console)\n");
  fprintf(stderr, "  -seed          copy vfs/ from the boot directory again\n");
  fprintf(stderr, "  -nonet         do not wait for the network connection\n");
  fprintf(stderr, "  -d <level>     log level: 0 = errors, 1 = info (default), 2 = trace; <level>:<system> for one system\n");
  fprintf(stderr, "the same options are read from %s in the boot directory and the home first\n", WIIU_CONFIG_FILE);
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

// creates every directory of "a/b/c/" that is missing (quietly: the log is
// not open yet when the home is made)
static void mkdirs(const char *path) {
  char buf[MAX_PATH_LEN];
  int i, n, slashes = 0;

  sys_strncpy(buf, path, MAX_PATH_LEN-1);
  buf[MAX_PATH_LEN-1] = 0;
  n = sys_strlen(buf);
  for (i = 0; i < n; i++) {
    if (buf[i] != '/') continue;
    // "fs:/vol/external01/": the device and the volume are not directories
    // to create (on other devices only the root is skipped)
    if (++slashes <= (sys_strncmp(buf, "fs:/", 4) ? 1 : 3)) continue;
    buf[i] = 0;
    if (!dir_exists(buf)) sys_mkdir(buf);
    buf[i] = '/';
  }
  if (n > 0 && buf[n-1] != '/' && !dir_exists(buf)) sys_mkdir(buf);
}

// "dir/" from "dir/" or "dir"
static void add_slash(char *path, int len) {
  int n = sys_strlen(path);

  if (n > 0 && n < len-1 && path[n-1] != '/') {
    path[n] = '/';
    path[n+1] = 0;
  }
}

// a "sd:/..." path as the "fs:" device has it
static void sd_to_fs(const char *src, char *dst, int len) {
  if (!sys_strncmp(src, "sd:/", 4)) {
    sys_snprintf(dst, len-1, "%s%s", WIIU_SD, src + 4);
  } else {
    sys_strncpy(dst, src, len-1);
  }
  dst[len-1] = 0;
}

// where vfs/ comes from: the bundle / title content when it has one, else
// the directory the RPX was started from (argv[0], "fs:/vol/external01/
// wiiu/apps/pumpkinos/pumpkinos.rpx" or "sd:/..."), else the usual
// Homebrew Launcher directory
static void find_boot(const char *arg0, char *boot, int len) {
  char path[MAX_PATH_LEN];
  char *p;

  sys_snprintf(path, MAX_PATH_LEN-1, "%svfs", WIIU_CONTENT);
  if (dir_exists(path)) {
    sys_strncpy(boot, WIIU_CONTENT, len-1);
    boot[len-1] = 0;
    return;
  }

  boot[0] = 0;
  if (arg0 && (!sys_strncmp(arg0, "fs:/", 4) || !sys_strncmp(arg0, "sd:/", 4))) {
    sd_to_fs(arg0, boot, len);
    if ((p = strrchr(boot, '/')) != NULL) p[1] = 0;
    else boot[0] = 0;
    sys_snprintf(path, MAX_PATH_LEN-1, "%svfs", boot);
    if (!dir_exists(path)) boot[0] = 0;
  }

  if (boot[0] == 0) {
    sys_strncpy(boot, WIIU_RPX_BOOT, len-1);
    boot[len-1] = 0;
  }
  add_slash(boot, len);
}

// an options file: the same options as the command line, separated by
// blanks or line breaks, '#' starts a comment. Returns the number of
// arguments stored in argv (pointing into buf).
static int read_config(const char *dir, char **argv, int max, char *buf, int size) {
  char path[MAX_PATH_LEN];
  int fd, n, i, argc = 0, comment = 0;

  if (max <= 0) return 0;
  sys_snprintf(path, MAX_PATH_LEN-1, "%s%s", dir, WIIU_CONFIG_FILE);
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
  sys_snprintf(path, MAX_PATH_LEN-1, "%s%s", vfsdir, WIIU_SEED_STAMP);
  if ((fd = sys_open(path, SYS_READ)) == -1) return;
  n = sys_read(fd, (uint8_t *)stamp, len-1);
  sys_close(fd);
  stamp[n > 0 ? n : 0] = 0;
}

static void write_stamp(const char *vfsdir) {
  char path[MAX_PATH_LEN];
  int fd;

  sys_snprintf(path, MAX_PATH_LEN-1, "%s%s", vfsdir, WIIU_SEED_STAMP);
  if ((fd = sys_create(path, SYS_WRITE | SYS_TRUNC, 0644)) == -1) return;
  sys_write(fd, (uint8_t *)WIIU_BUILD_STAMP, sys_strlen(WIIU_BUILD_STAMP));
  sys_close(fd);
}

int main(int argc, char *argv[]) {
  char boot[MAX_PATH_LEN], home[MAX_PATH_LEN], stamp[64];
  char vfsdir[MAX_PATH_LEN], tmpdir[MAX_PATH_LEN], logfile[MAX_PATH_LEN], errfile[MAX_PATH_LEN], path[MAX_PATH_LEN], seed[MAX_PATH_LEN];
  char *args[WIIU_CONFIG_MAX_ARGS + 1];
  static char config_boot[WIIU_CONFIG_MAX_SIZE], config_home[WIIU_CONFIG_MAX_SIZE];
  const char *vfs_arg = NULL, *log_arg = NULL, *boot_arg = NULL, *home_arg = NULL;
  char *d, *p;
  int i, nargs = 0, level = DEBUG_INFO, net = 1, reseed = 0;

  // "-boot" and "-home" must be known before the options files can be read
  for (i = 1; i < argc-1; i++) {
    if (!strcmp(argv[i], "-boot")) boot_arg = argv[i+1];
    if (!strcmp(argv[i], "-home")) home_arg = argv[i+1];
  }
  if (boot_arg) {
    sd_to_fs(boot_arg, boot, MAX_PATH_LEN);
    add_slash(boot, MAX_PATH_LEN);
  } else {
    find_boot(argc > 0 ? argv[0] : NULL, boot, MAX_PATH_LEN);
  }
  if (home_arg) {
    sd_to_fs(home_arg, home, MAX_PATH_LEN);
  } else {
    sys_strncpy(home, WIIU_HOME, MAX_PATH_LEN-1);
    home[MAX_PATH_LEN-1] = 0;
  }
  add_slash(home, MAX_PATH_LEN);

  // the options: pumpkin.cfg in the boot directory, then the one in the
  // home, then the command line (the later ones win)
  args[nargs++] = argc > 0 ? argv[0] : "pumpkinos.rpx";
  nargs += read_config(boot, &args[nargs], WIIU_CONFIG_MAX_ARGS - argc - nargs, config_boot, WIIU_CONFIG_MAX_SIZE);
  if (sys_strcmp(boot, home)) {
    nargs += read_config(home, &args[nargs], WIIU_CONFIG_MAX_ARGS - argc - nargs, config_home, WIIU_CONFIG_MAX_SIZE);
  }
  for (i = 1; i < argc && nargs < WIIU_CONFIG_MAX_ARGS; i++) {
    args[nargs++] = argv[i];
  }
  args[nargs] = NULL;
  argv = args;
  argc = nargs;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-nodia")) {
      wiiu_script_set("dia", 0);
    } else if (!strcmp(argv[i], "-nonet")) {
      net = 0;
    } else if (!strcmp(argv[i], "-seed")) {
      reseed = 1;
    } else if (i < argc-1 && !strcmp(argv[i], "-app")) {
      wiiu_script_set_launcher(argv[++i]);
    } else if (i < argc-1 && (!strcmp(argv[i], "-boot") || !strcmp(argv[i], "-home"))) {
      i++;  // already handled (from the command line; ignored in the options files)
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
    } else if (i < argc-1 && argv[i][0] == '-' && wiiu_script_set(argv[i]+1, sys_atoi(argv[i+1])) == 0) {
      i++;
    } else if (argv[i][0] == '-') {
      usage();
      OSReport("PumpkinOS: unknown option \"%s\"\n", argv[i]);
    }
    // anything else is ignored
  }

  sys_init();
  debug_setsyslevel(NULL, level);

  // the home directory must exist before anything is written there
  mkdirs(home);
  if (!dir_exists(home)) {
    OSReport("PumpkinOS: could not create the home \"%s\" (no SD card?)\n", home);
  }

  if (log_arg) {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%s", log_arg);
  } else {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%spumpkin.log", home);
  }
  sys_snprintf(errfile, MAX_PATH_LEN-1, "%spumpkin.err", home);
  // stderr goes to a file unless the log goes there itself
  if (sys_strcmp(logfile, "stderr")) freopen(errfile, "w", stderr);
  debug_init(logfile);
  debug(DEBUG_INFO, "MAIN", "%s starting on %s (boot \"%s\", home \"%s\")", SYSTEM_NAME, SYSTEM_OS, boot, home);
  OSReport("PumpkinOS: starting (boot \"%s\", home \"%s\", log \"%s\")\n", boot, home, logfile);
  if (argc > 0) debug(DEBUG_INFO, "MAIN", "argv[0] \"%s\"", argv[0]);
  for (i = 1; i < argc; i++) {
    debug(DEBUG_INFO, "MAIN", "option \"%s\"", argv[i]);
  }

  // relative names (soundfont.sf2) are looked up in the boot directory
  if (chdir(boot) != 0) {
    debug(DEBUG_ERROR, "MAIN", "could not change to \"%s\"", boot);
  }

  if (vfs_arg) {
    sd_to_fs(vfs_arg, vfsdir, MAX_PATH_LEN);
  } else {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%svfs", home);
  }
  add_slash(vfsdir, MAX_PATH_LEN);
  // the temporary files live in the home, next to vfs/
  sys_snprintf(tmpdir, MAX_PATH_LEN-1, "%stmp", home);

  if (!vfs_arg) {
    // first boot: copy the VFS shipped in the boot directory to the home;
    // -seed does it again, and so does a home whose app_install and
    // app_storage are both empty (an install that failed: the PRCs are
    // removed as they are deployed)
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
            debug(DEBUG_ERROR, "MAIN", "the VFS copy failed (SD card full or locked?)");
          }
          write_stamp(vfsdir);
        } else {
          debug(DEBUG_ERROR, "MAIN", "\"%s\" not found: nothing to install", seed);
        }
      } else {
        // the PRCs of another build (an update): installed again
        read_stamp(vfsdir, stamp, sizeof(stamp));
        if (sys_strcmp(stamp, WIIU_BUILD_STAMP)) {
          sys_snprintf(seed, MAX_PATH_LEN-1, "%svfs/app_install", boot);
          debug(DEBUG_INFO, "MAIN", "PRCs from build \"%s\", this is \"%s\": copying \"%s\" to \"%s\"", stamp, WIIU_BUILD_STAMP, seed, path);
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

  // the connection configured in the console's internet settings (wut's
  // socket layer starts it; wait a little for it)
  if (net) {
    sys_wiiu_net_init(WIIU_NET_TIMEOUT_MS);
  } else {
    debug(DEBUG_INFO, "MAIN", "not waiting for the network");
  }

  sys_wiiu_memlog("startup");
  ptr_init();
  thread_init();
  vfs_init();

  if (liblsdl3_load() != 0) {
    debug(DEBUG_ERROR, "MAIN", "SDL3 initialization failed");
    OSReport("PumpkinOS: SDL3 initialization failed\n");
    vfs_finish();
    thread_close();
    debug_close();
    sys_wiiu_net_finish();
    return 1;
  }
  liblsdl3_init(0, 0);
  sys_wiiu_memlog("SDL loaded");

  vfs_local_mount(vfsdir, "/");
  mkdir_if_missing(tmpdir);
  sys_set_tmpdir(tmpdir);

#ifndef NO_HTTPS
  // OpenSSL looks for the CA bundle in SSL_CERT_FILE: cert.pem in the boot
  // directory, or in the home
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
  sys_wiiu_memlog("TLS provider loaded");

  libos_app_init(0);
  libos_start(0);

  vfs_finish();
  thread_close();
  liblsdl3_unload();
  debug(DEBUG_INFO, "MAIN", "%s stopping", SYSTEM_NAME);
  debug_close();
  OSReport("PumpkinOS: stopped\n");

  sys_wiiu_net_finish();

  return 0;
}
