// PumpkinOS entry point for the Nintendo Switch (devkitPro libnx + SDL3).
//
// Mirrors src/vita/main.c: everything is linked statically, there is no
// script engine and the OS runs in single application mode (the Launcher and
// the applications run in-process, as cooperative threads).
//
// Files live in sdmc:/switch/pumpkinos/ (created on the first run): the VFS
// (vfs/, seeded from the read-only romfs:/vfs/ embedded in the NRO), tmp/,
// the log (pumpkin.log) and stderr (pumpkin.err).
//
// Installed add-on content (DLC, see aoc.c and README.md) is merged into the
// home directory at boot: the files of its RomFS (vfs/..., soundfont.sf2,
// cert.pem) that are missing there are copied, once per DLC version (its
// pumpkinos_dlc.id stamp, remembered in dlc/<data id>.id).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <switch/types.h>
#include <switch/result.h>
#include <switch/runtime/devices/romfs_dev.h>
#include <switch/runtime/nxlink.h>
#include <switch/services/applet.h>

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
#include "switch.h"
#include "aoc.h"

#define MAX_PATH_LEN 256

// writable home of the port, and the read-only NRO contents
#define SWITCH_HOME  "sdmc:/switch/pumpkinos/"
#define SWITCH_APP   "romfs:/"

// add-on content: mount point, stamp file, and where the applied stamps go
#define DLC_DEVICE   "dlc"
#define DLC_ROOT     DLC_DEVICE ":/"
#define DLC_STAMP    "pumpkinos_dlc.id"
#define DLC_STAMPS   SWITCH_HOME "dlc"

extern int liblsdl3_load(void);
extern int liblsdl3_init(int pe, script_ref_t obj);
extern int liblsdl3_unload(void);

extern int libos_app_init(int pe);
extern int libos_start(int pe);

#ifndef NO_HTTPS
extern int liblopenssl_load(void);
extern int liblopenssl_init(int pe, script_ref_t obj);
#endif

static void usage(const char *prog) {
  fprintf(stderr, "usage: %s [options]\n", prog);
  fprintf(stderr, "  -width <n>     screen width in pixels (default 1280)\n");
  fprintf(stderr, "  -height <n>    screen height in pixels (default 720)\n");
  fprintf(stderr, "  -density <n>   72 (low) or 144 (double, default)\n");
  fprintf(stderr, "  -depth <n>     application bit depth (default 16)\n");
  fprintf(stderr, "  -hdepth <n>    host bit depth, 16 or 32 (default 16)\n");
  fprintf(stderr, "  -mode <n>      1 = single application (default), 0 = desktop\n");
  fprintf(stderr, "  -dia <0|1>     dynamic input area (default 1)\n");
  fprintf(stderr, "  -app <name>    start this application instead of the Launcher\n");
  fprintf(stderr, "  -vfs <dir>     VFS root directory (default " SWITCH_HOME "vfs)\n");
  fprintf(stderr, "  -log <file>    log file (default " SWITCH_HOME "pumpkin.log)\n");
  fprintf(stderr, "  -d <level>     log level: 0 = errors, 1 = info (default), 2 = trace; <level>:<system> for one system\n");
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

// recursive copy of a directory tree (the initial VFS from the romfs, the
// contents of a DLC); with missing_only, files that exist are kept
static int copy_tree(const char *src, const char *dst, int missing_only) {
  char name[FILE_PATH], s[MAX_PATH_LEN], d[MAX_PATH_LEN];
  sys_dir_t *dir;
  sys_stat_t st;
  int r = 0;

  if (sys_stat(dst, &st) != 0 && sys_mkdir(dst) != 0) {
    debug(DEBUG_ERROR, "MAIN", "could not create directory \"%s\"", dst);
    return -1;
  }

  if ((dir = sys_opendir(src)) == NULL) {
    debug(DEBUG_ERROR, "MAIN", "could not open directory \"%s\"", src);
    return -1;
  }

  while (sys_readdir(dir, name, sizeof(name)) == 0) {
    if (!strcmp(name, ".") || !strcmp(name, "..") || !strcmp(name, DLC_STAMP)) continue;
    sys_snprintf(s, MAX_PATH_LEN-1, "%s%s%s", src, src[strlen(src)-1] == '/' ? "" : "/", name);
    sys_snprintf(d, MAX_PATH_LEN-1, "%s%s%s", dst, dst[strlen(dst)-1] == '/' ? "" : "/", name);
    if (sys_stat(s, &st) != 0) continue;
    if (st.mode & SYS_IFDIR) {
      if (copy_tree(s, d, missing_only) != 0) r = -1;
    } else if (missing_only && sys_stat(d, &st) == 0) {
      continue;
    } else {
      debug(DEBUG_INFO, "MAIN", "copying \"%s\" to \"%s\"", s, d);
      if (copy_file(s, d) != 0) {
        debug(DEBUG_ERROR, "MAIN", "could not copy \"%s\"", s);
        r = -1;
      }
    }
  }
  sys_closedir(dir);

  return r;
}

static void mkdir_if_missing(const char *path) {
  sys_stat_t st;

  if (sys_stat(path, &st) != 0) {
    sys_mkdir(path);
  }
}

// reads a small text file (a DLC stamp), returns its length or -1
static int read_stamp(const char *path, char *buf, int size) {
  sys_stat_t st;
  int fd, n;

  if (sys_stat(path, &st) != 0) return -1;
  if ((fd = sys_open(path, SYS_READ)) == -1) return -1;
  n = sys_read(fd, (uint8_t *)buf, size - 1);
  sys_close(fd);
  if (n < 0) return -1;
  buf[n] = 0;

  return n;
}

// merge the installed add-on contents into the home directory
static void apply_dlc(void) {
  uint64_t ids[SWITCH_DLC_MAX];
  char stamp[128], applied[128], path[MAX_PATH_LEN];
  int i, n, fd;

  if (switch_dlc_init() != 0) {
    debug(DEBUG_INFO, "MAIN", "add-on content service not available");
    return;
  }

  // only the add-on contents of PumpkinOS itself: an NRO runs inside the
  // process of another title (hbmenu title override), whose DLCs aoc:u lists
  n = switch_dlc_list(SWITCH_TITLE_ID, ids, SWITCH_DLC_MAX);
  if (n == -2) {
    debug(DEBUG_INFO, "MAIN", "add-on contents of another title ignored");
    n = 0;
  }
  debug(DEBUG_INFO, "MAIN", "%d add-on content(s) installed", n < 0 ? 0 : n);

  for (i = 0; i < n; i++) {
    if (switch_dlc_mount(ids[i], DLC_DEVICE) != 0) {
      debug(DEBUG_ERROR, "MAIN", "add-on content %016llX could not be mounted", (unsigned long long)ids[i]);
      continue;
    }
    if (read_stamp(DLC_ROOT DLC_STAMP, stamp, sizeof(stamp)) < 0) stamp[0] = 0;
    sys_snprintf(path, MAX_PATH_LEN-1, DLC_STAMPS "/%016llX.id", (unsigned long long)ids[i]);
    if (stamp[0] && read_stamp(path, applied, sizeof(applied)) >= 0 && !strcmp(stamp, applied)) {
      debug(DEBUG_INFO, "MAIN", "add-on content %016llX already applied", (unsigned long long)ids[i]);
    } else {
      debug(DEBUG_INFO, "MAIN", "applying add-on content %016llX", (unsigned long long)ids[i]);
      if (copy_tree(DLC_ROOT, SWITCH_HOME, 1) == 0) {
        mkdir_if_missing(DLC_STAMPS);
        if ((fd = sys_create(path, SYS_WRITE | SYS_TRUNC, 0644)) != -1) {
          sys_write(fd, (uint8_t *)stamp, strlen(stamp));
          sys_close(fd);
        }
      } else {
        debug(DEBUG_ERROR, "MAIN", "add-on content %016llX not completely applied", (unsigned long long)ids[i]);
      }
    }
    switch_dlc_unmount(DLC_DEVICE);
  }

  switch_dlc_exit();
}

int main(int argc, char *argv[]) {
  char vfsdir[MAX_PATH_LEN], tmpdir[MAX_PATH_LEN], logfile[MAX_PATH_LEN], errfile[MAX_PATH_LEN], cafile[MAX_PATH_LEN];
  const char *vfs_arg = NULL, *log_arg = NULL;
  char *d, *p;
  sys_stat_t st;
  int i, level = DEBUG_INFO, romfs = 0, nxlink = 0;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-nodia")) {
      switch_script_set("dia", 0);
    } else if (!strcmp(argv[i], "-nxlink")) {
      // mirror stderr to the nxlink host (nxlink -s PumpkinOS.nro)
      nxlink = 1;
    } else if (i < argc-1 && !strcmp(argv[i], "-app")) {
      switch_script_set_launcher(argv[++i]);
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
    } else if (i < argc-1 && argv[i][0] == '-' && switch_script_set(argv[i]+1, sys_atoi(argv[i+1])) == 0) {
      i++;
    } else if (argv[i][0] == '-') {
      usage(argv[0] ? argv[0] : "pumpkinos");
      return 1;
    }
    // anything else (hbmenu passes the NRO path as argv[0] only) is ignored
  }

  // the read-only data embedded in the NRO (vfs seed, soundfont, CA bundle)
  romfs = R_SUCCEEDED(romfsInit());

  // the writable home directory (sdmc:/switch always exists with hbmenu)
  mkdir_if_missing("sdmc:/switch");
  mkdir_if_missing("sdmc:/switch/pumpkinos");

  if (vfs_arg) {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%s", vfs_arg);
  } else {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%svfs", SWITCH_HOME);
  }
  i = sys_strlen(vfsdir);
  if (i > 0 && vfsdir[i-1] != '/') {
    vfsdir[i] = '/';
    vfsdir[i+1] = 0;
  }
  sys_snprintf(tmpdir, MAX_PATH_LEN-1, "%stmp", SWITCH_HOME);
  if (log_arg) {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%s", log_arg);
  } else {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%spumpkin.log", SWITCH_HOME);
  }

  // there is no console: keep stderr in a file next to the log (or send it
  // to the nxlink host when asked to)
  sys_snprintf(errfile, MAX_PATH_LEN-1, "%spumpkin.err", SWITCH_HOME);
  if (nxlink) {
    // nxlinkStdio needs the socket service: initialized below, so start the
    // network first in that case
    sys_switch_net_init();
    if (nxlinkStdio() < 0) {
      freopen(errfile, "w", stderr);
    }
  } else {
    freopen(errfile, "w", stderr);
  }

  sys_init();
  debug_setsyslevel(NULL, level);
  debug_init(logfile);
  debug(DEBUG_INFO, "MAIN", "%s starting on %s (vfs \"%s\")", SYSTEM_NAME, SYSTEM_OS, vfsdir);
  if (!romfs) {
    debug(DEBUG_ERROR, "MAIN", "romfs not available: the NRO carries no data");
  }

  // first run: seed the VFS (application PRCs) from the read-only copy in the NRO
  if (!vfs_arg && (sys_stat(vfsdir, &st) != 0 || !(st.mode & SYS_IFDIR))) {
    debug(DEBUG_INFO, "MAIN", "creating the VFS from " SWITCH_APP "vfs");
    if (copy_tree(SWITCH_APP "vfs", SWITCH_HOME "vfs", 0) != 0) {
      debug(DEBUG_ERROR, "MAIN", "the VFS could not be created");
    }
  }
  mkdir_if_missing(SWITCH_HOME "vfs/app_install");
  mkdir_if_missing(SWITCH_HOME "vfs/app_storage");
  mkdir_if_missing(SWITCH_HOME "vfs/app_card");
  mkdir_if_missing(SWITCH_HOME "vfs/app_card/PALM");
  mkdir_if_missing(SWITCH_HOME "vfs/app_card/PALM/Programs");

  // DLC: application PRCs, card files, soundfont...
  if (!vfs_arg) {
    apply_dlc();
  }

  // BSD sockets (use the network connection configured in the system settings)
  if (!nxlink) {
    sys_switch_net_init();
  }

  ptr_init();
  thread_init();
  vfs_init();

  if (liblsdl3_load() != 0) {
    debug(DEBUG_ERROR, "MAIN", "SDL3 initialization failed");
    vfs_finish();
    thread_close();
    debug_close();
    fprintf(stderr, "SDL3 initialization failed, see %s\n", logfile);
    sys_switch_net_finish();
    if (romfs) romfsExit();
    return 1;
  }
  liblsdl3_init(0, 0);

  vfs_local_mount(vfsdir, "/");
  mkdir_if_missing(tmpdir);
  sys_set_tmpdir(tmpdir);

#ifndef NO_HTTPS
  // OpenSSL looks for the CA bundle in SSL_CERT_FILE: use cert.pem from
  // the home directory, or the one embedded in the NRO, when it exists
  sys_snprintf(cafile, MAX_PATH_LEN-1, "%scert.pem", SWITCH_HOME);
  if (sys_stat(cafile, &st) != 0) {
    sys_snprintf(cafile, MAX_PATH_LEN-1, "%scert.pem", SWITCH_APP);
  }
  if (sys_stat(cafile, &st) == 0) {
    setenv("SSL_CERT_FILE", cafile, 0);
  }
  liblopenssl_load();
  liblopenssl_init(0, 0);
  debug(DEBUG_INFO, "MAIN", "TLS provider loaded (OpenSSL)");
#endif

  libos_app_init(0);
  libos_start(0);

  vfs_finish();
  thread_close();
  liblsdl3_unload();
  debug(DEBUG_INFO, "MAIN", "%s stopping", SYSTEM_NAME);
  debug_close();

  sys_switch_net_finish();
  if (romfs) romfsExit();

  return 0;
}
