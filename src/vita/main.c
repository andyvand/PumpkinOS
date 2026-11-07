// PumpkinOS entry point for the PlayStation Vita (vitasdk + SDL3).
//
// Mirrors src/msdos/main.c: everything is linked statically, there is no
// script engine and the OS runs in single application mode (the Launcher and
// the applications run in-process, as cooperative threads).
//
// Files live in ux0:data/pumpkinos/ (created on the first run): the VFS
// (vfs/, seeded from the read-only app0:vfs/ shipped in the VPK), tmp/, the
// log (pumpkin.log) and stderr (pumpkin.err).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/kernel/processmgr.h>

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
#include "vita.h"

#define MAX_PATH_LEN 256

// writable home of the port, and the read-only VPK contents
#define VITA_HOME  "ux0:data/pumpkinos/"
#define VITA_APP   "app0:"

// newlib heap (the default is far too small): PumpkinOS allocates a 64MB
// global heap plus 8MB per task (libpumpkin HEAP_SIZE), SDL its textures
int _newlib_heap_size_user = 192 * 1024 * 1024;

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
  fprintf(stderr, "  -width <n>     screen width in pixels (default 960)\n");
  fprintf(stderr, "  -height <n>    screen height in pixels (default 544)\n");
  fprintf(stderr, "  -density <n>   72 (low) or 144 (double, default)\n");
  fprintf(stderr, "  -depth <n>     application bit depth (default 16)\n");
  fprintf(stderr, "  -hdepth <n>    host bit depth, 16 or 32 (default 16)\n");
  fprintf(stderr, "  -mode <n>      1 = single application (default), 0 = desktop\n");
  fprintf(stderr, "  -dia <0|1>     dynamic input area (default 1)\n");
  fprintf(stderr, "  -app <name>    start this application instead of the Launcher\n");
  fprintf(stderr, "  -vfs <dir>     VFS root directory (default " VITA_HOME "vfs)\n");
  fprintf(stderr, "  -log <file>    log file (default " VITA_HOME "pumpkin.log)\n");
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

// recursive copy of a directory tree (the initial VFS from the VPK)
static int copy_tree(const char *src, const char *dst) {
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
    if (!strcmp(name, ".") || !strcmp(name, "..")) continue;
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
  sys_closedir(dir);

  return r;
}

static void mkdir_if_missing(const char *path) {
  sys_stat_t st;

  if (sys_stat(path, &st) != 0) {
    sys_mkdir(path);
  }
}

int main(int argc, char *argv[]) {
  char vfsdir[MAX_PATH_LEN], tmpdir[MAX_PATH_LEN], logfile[MAX_PATH_LEN], errfile[MAX_PATH_LEN], cafile[MAX_PATH_LEN];
  const char *vfs_arg = NULL, *log_arg = NULL;
  char *d, *p;
  sys_stat_t st;
  int i, level = DEBUG_INFO;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-nodia")) {
      vita_script_set("dia", 0);
    } else if (i < argc-1 && !strcmp(argv[i], "-app")) {
      vita_script_set_launcher(argv[++i]);
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
    } else if (i < argc-1 && argv[i][0] == '-' && vita_script_set(argv[i]+1, sys_atoi(argv[i+1])) == 0) {
      i++;
    } else {
      usage(argv[0] ? argv[0] : "pumpkinos");
      return 1;
    }
  }

  // the writable home directory (ux0:data always exists)
  mkdir_if_missing("ux0:data");
  mkdir_if_missing("ux0:data/pumpkinos");

  if (vfs_arg) {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%s", vfs_arg);
  } else {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%svfs", VITA_HOME);
  }
  i = sys_strlen(vfsdir);
  if (i > 0 && vfsdir[i-1] != '/') {
    vfsdir[i] = '/';
    vfsdir[i+1] = 0;
  }
  sys_snprintf(tmpdir, MAX_PATH_LEN-1, "%stmp", VITA_HOME);
  if (log_arg) {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%s", log_arg);
  } else {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%spumpkin.log", VITA_HOME);
  }

  // there is no console: keep stderr in a file next to the log
  sys_snprintf(errfile, MAX_PATH_LEN-1, "%spumpkin.err", VITA_HOME);
  freopen(errfile, "w", stderr);

  sys_init();
  debug_setsyslevel(NULL, level);
  debug_init(logfile);
  debug(DEBUG_INFO, "MAIN", "%s starting on %s (vfs \"%s\")", SYSTEM_NAME, SYSTEM_OS, vfsdir);

  // first run: seed the VFS (application PRCs) from the read-only copy in the VPK
  if (!vfs_arg && (sys_stat(vfsdir, &st) != 0 || !(st.mode & SYS_IFDIR))) {
    debug(DEBUG_INFO, "MAIN", "creating the VFS from " VITA_APP "vfs");
    if (copy_tree(VITA_APP "vfs", VITA_HOME "vfs") != 0) {
      debug(DEBUG_ERROR, "MAIN", "the VFS could not be created");
    }
  }
  mkdir_if_missing(VITA_HOME "vfs/app_install");
  mkdir_if_missing(VITA_HOME "vfs/app_storage");
  mkdir_if_missing(VITA_HOME "vfs/app_card");
  mkdir_if_missing(VITA_HOME "vfs/app_card/PALM");
  mkdir_if_missing(VITA_HOME "vfs/app_card/PALM/Programs");

  // SceNet TCP/IP (uses the Wi-Fi connection configured in the system settings)
  sys_vita_net_init();

  ptr_init();
  thread_init();
  vfs_init();

  if (liblsdl3_load() != 0) {
    debug(DEBUG_ERROR, "MAIN", "SDL3 initialization failed");
    vfs_finish();
    thread_close();
    debug_close();
    fprintf(stderr, "SDL3 initialization failed, see %s\n", logfile);
    sceKernelExitProcess(1);
    return 1;
  }
  liblsdl3_init(0, 0);

  vfs_local_mount(vfsdir, "/");
  mkdir_if_missing(tmpdir);
  sys_set_tmpdir(tmpdir);

#ifndef NO_HTTPS
  // OpenSSL looks for the CA bundle in SSL_CERT_FILE: use cacert.pem from
  // the home directory, or the one shipped in the VPK, when it exists
  sys_snprintf(cafile, MAX_PATH_LEN-1, "%scacert.pem", VITA_HOME);
  if (sys_stat(cafile, &st) != 0) {
    sys_snprintf(cafile, MAX_PATH_LEN-1, "%scacert.pem", VITA_APP);
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

  sceKernelExitProcess(0);
  return 0;
}
