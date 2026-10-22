// PumpkinOS entry point for MS-DOS (DJGPP + SDL3). Built as pumpkin.exe, every
// file name used at runtime fits 8.3 (see README.md).
//
// Mirrors src/main/main.c (ESP32) and src/kernel/main.c: everything is linked
// statically, there is no script engine and the OS runs in single
// application mode (the Launcher and the applications run in-process).
//
// SDL3 on DOS provides main(): it locks the code/data/stack pages and enables
// the DJGPP "fat DS" near pointers before calling SDL_main (our main below).

#include <SDL3/SDL_main.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

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
#include "msdos.h"

#define MAX_PATH_LEN 256

extern int liblsdl3_load(void);
extern int liblsdl3_init(int pe, script_ref_t obj);
extern int liblsdl3_unload(void);

extern int libos_app_init(int pe);
extern int libos_start(int pe);

static void usage(const char *prog) {
  fprintf(stderr, "usage: %s [options]\n", prog);
  fprintf(stderr, "  -width <n>     screen width in pixels (default 640)\n");
  fprintf(stderr, "  -height <n>    screen height in pixels (default 480)\n");
  fprintf(stderr, "  -density <n>   72 (low) or 144 (double, default)\n");
  fprintf(stderr, "  -depth <n>     application bit depth (default 16)\n");
  fprintf(stderr, "  -hdepth <n>    host bit depth, 16 or 32 (default 16)\n");
  fprintf(stderr, "  -mode <n>      1 = single application (default), 0 = desktop\n");
  fprintf(stderr, "  -dia <0|1>     dynamic input area (default 1)\n");
  fprintf(stderr, "  -window        do not switch to fullscreen\n");
  fprintf(stderr, "  -vfs <dir>     VFS root directory (default <exe dir>/vfs)\n");
  fprintf(stderr, "  -log <file>    log file (default <exe dir>/pumpkin.log; stderr goes to pumpkin.err)\n");
  fprintf(stderr, "  -d <level>     log level: 0 = errors, 1 = info (default), 2 = trace; <level>:<system> for one system\n");
}

// directory of the executable, with trailing separator
static void exe_dir(const char *argv0, char *buf, int len) {
  int i, n;

  sys_memset(buf, 0, len);
  if (argv0 == NULL) return;

  for (n = -1, i = 0; argv0[i] && i < len-1; i++) {
    buf[i] = argv0[i];
    if (argv0[i] == '/' || argv0[i] == '\\') n = i;
  }
  buf[n+1] = 0;
}

int main(int argc, char *argv[]) {
  char dir[MAX_PATH_LEN], vfsdir[MAX_PATH_LEN], tmpdir[MAX_PATH_LEN], logfile[MAX_PATH_LEN], errfile[MAX_PATH_LEN];
  const char *vfs_arg = NULL, *log_arg = NULL;
  char *d, *p;
  sys_stat_t st;
  int i, level = DEBUG_INFO;

  // DJGPP opens files in text mode by default (CR/LF translation, Ctrl-Z is
  // EOF), which corrupts the PRC/PDB databases: make binary the default
  _fmode = O_BINARY;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-nodia")) {
      msdos_script_set("dia", 0);
    } else if (!strcmp(argv[i], "-window")) {
      msdos_script_set("fullscreen", 0);
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
    } else if (i < argc-1 && argv[i][0] == '-' && msdos_script_set(argv[i]+1, sys_atoi(argv[i+1])) == 0) {
      i++;
    } else {
      usage(argv[0]);
      return 1;
    }
  }

  exe_dir(argv[0], dir, MAX_PATH_LEN);

  if (vfs_arg) {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%s", vfs_arg);
  } else {
    // vfs/ next to the executable, or the PumpkinOS tree layout (bin/ and vfs/ side by side)
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%svfs", dir);
    if (sys_stat(vfsdir, &st) != 0 || !(st.mode & SYS_IFDIR)) {
      sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%s../vfs", dir);
    }
  }
  i = sys_strlen(vfsdir);
  if (i > 0 && vfsdir[i-1] != '/' && vfsdir[i-1] != '\\') {
    vfsdir[i] = '/';
    vfsdir[i+1] = 0;
  }
  sys_snprintf(tmpdir, MAX_PATH_LEN-1, "%stmp", dir);
  if (log_arg) {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%s", log_arg);
  } else {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%spumpkin.log", dir);
  }

  // the screen is in graphics mode while PumpkinOS runs: keep stderr (and the
  // DJGPP crash traceback, if any) in a file next to the log
  sys_snprintf(errfile, MAX_PATH_LEN-1, "%spumpkin.err", dir);
  freopen(errfile, "w", stderr);

  sys_init();
  debug_setsyslevel(NULL, level);
  debug_init(logfile);
  debug(DEBUG_INFO, "MAIN", "%s starting on %s (vfs \"%s\")", SYSTEM_NAME, SYSTEM_OS, vfsdir);

  ptr_init();
  thread_init();
  vfs_init();

  if (liblsdl3_load() != 0) {
    debug(DEBUG_ERROR, "MAIN", "SDL3 initialization failed");
    vfs_finish();
    thread_close();
    debug_close();
    fprintf(stderr, "SDL3 initialization failed, see %s\n", logfile);
    return 1;
  }
  liblsdl3_init(0, 0);

  vfs_local_mount(vfsdir, "/");
  if (sys_stat(tmpdir, &st) != 0) {
    sys_mkdir(tmpdir);
  }
  sys_set_tmpdir(tmpdir);

  libos_app_init(0);
  libos_start(0);

  vfs_finish();
  thread_close();
  liblsdl3_unload();
  debug(DEBUG_INFO, "MAIN", "%s stopping", SYSTEM_NAME);
  debug_close();

  return 0;
}
