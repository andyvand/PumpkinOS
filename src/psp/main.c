// PumpkinOS entry point for the Sony PlayStation Portable (pspsdk + SDL3).
//
// Mirrors src/vita/main.c and src/switch/main.c: everything is linked
// statically, there is no script engine and the OS runs in single
// application mode (the Launcher and the applications run in-process, as
// cooperative threads).
//
// Files live next to EBOOT.PBP, normally ms0:/PSP/GAME/PumpkinOS/ (ef0: on a
// PSP go): the VFS (vfs/, shipped with the PBP), tmp/, the log (pumpkin.log)
// and stderr (pumpkin.err).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspkernel.h>
#include <pspthreadman.h>
#include <psploadexec.h>
#include <pspmoduleinfo.h>

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
#include "psp.h"

// user mode module, running on the main thread with the VFPU enabled (SDL
// and FluidSynth use it); its stack only serves main() and the SDL calls,
// the PumpkinOS tasks run on their own (heap allocated) stacks
PSP_MODULE_INFO("PumpkinOS", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(512);
// the newlib heap takes all the free user memory but PSP_HEAP_THRESHOLD: the
// network modules and their memory pool, the thread stacks SDL creates and
// the exit callback thread live there. PumpkinOS sizes its own heaps to what
// is left (see HEAP_SIZE in libpumpkin/pumpkin.c).
PSP_HEAP_SIZE_KB(-1);
PSP_HEAP_THRESHOLD_SIZE_KB(2560);

#define MAX_PATH_LEN 256

// default home when the launcher does not tell where EBOOT.PBP is
#define PSP_DEFAULT_HOME "ms0:/PSP/GAME/PumpkinOS/"

// how long to wait for the access point at startup
#define PSP_NET_TIMEOUT_MS 15000

extern int liblsdl3_load(void);
extern int liblsdl3_init(int pe, script_ref_t obj);
extern int liblsdl3_unload(void);

extern int libos_app_init(int pe);
extern int libos_start(int pe);

#ifndef NO_HTTPS
extern int liblopenssl_load(void);
extern int liblopenssl_init(int pe, script_ref_t obj);
#endif

static volatile int finished = 0;

// HOME button: ask PumpkinOS to stop (the quit event reaches the window
// provider, which ends the Launcher loop and lets main() clean up); if it
// does not come back in a few seconds, leave anyway
static int exit_callback(int arg1, int arg2, void *common) {
  SDL_Event ev;
  int i;

  sys_memset(&ev, 0, sizeof(ev));
  ev.type = SDL_EVENT_QUIT;
  SDL_PushEvent(&ev);

  for (i = 0; i < 50 && !finished; i++) {
    sceKernelDelayThread(100 * 1000);
  }
  sceKernelExitGame();

  return 0;
}

static int callback_thread(SceSize args, void *argp) {
  int cbid;

  if ((cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL)) >= 0) {
    sceKernelRegisterExitCallback(cbid);
  }
  sceKernelSleepThreadCB();

  return 0;
}

static void setup_callbacks(void) {
  SceUID thid;

  if ((thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, NULL)) >= 0) {
    sceKernelStartThread(thid, 0, NULL);
  }
}

static void usage(const char *prog) {
  fprintf(stderr, "usage: %s [options]\n", prog);
  fprintf(stderr, "  -width <n>     screen width in pixels (default 480)\n");
  fprintf(stderr, "  -height <n>    screen height in pixels (default 272)\n");
  fprintf(stderr, "  -density <n>   72 (low, default) or 144 (double)\n");
  fprintf(stderr, "  -depth <n>     application bit depth (default 16)\n");
  fprintf(stderr, "  -hdepth <n>    host bit depth, 16 or 32 (default 16)\n");
  fprintf(stderr, "  -mode <n>      1 = single application (default), 0 = desktop\n");
  fprintf(stderr, "  -dia <0|1>     dynamic input area (default 1)\n");
  fprintf(stderr, "  -app <name>    start this application instead of the Launcher\n");
  fprintf(stderr, "  -vfs <dir>     VFS root directory (default <home>/vfs)\n");
  fprintf(stderr, "  -log <file>    log file (default <home>/pumpkin.log)\n");
  fprintf(stderr, "  -nonet         do not bring the network up\n");
  fprintf(stderr, "  -d <level>     log level: 0 = errors, 1 = info (default), 2 = trace; <level>:<system> for one system\n");
}

static void mkdir_if_missing(const char *path) {
  sys_stat_t st;

  if (sys_stat(path, &st) != 0) {
    sys_mkdir(path);
  }
}

// the directory holding EBOOT.PBP, from argv[0] ("ms0:/PSP/GAME/PumpkinOS/EBOOT.PBP")
static void find_home(const char *arg0, char *home, int len) {
  const char *p;
  int n;

  if (arg0 && (p = strrchr(arg0, '/')) != NULL && strchr(arg0, ':') != NULL) {
    n = p - arg0 + 1;
    if (n < len) {
      sys_memcpy(home, arg0, n);
      home[n] = 0;
      return;
    }
  }
  sys_strncpy(home, PSP_DEFAULT_HOME, len-1);
}

int main(int argc, char *argv[]) {
  char home[MAX_PATH_LEN], vfsdir[MAX_PATH_LEN], tmpdir[MAX_PATH_LEN], logfile[MAX_PATH_LEN], errfile[MAX_PATH_LEN], cafile[MAX_PATH_LEN];
  const char *vfs_arg = NULL, *log_arg = NULL;
  char *d, *p;
  sys_stat_t st;
  int i, level = DEBUG_INFO, net = 1;

  setup_callbacks();
  find_home(argc > 0 ? argv[0] : NULL, home, MAX_PATH_LEN);

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-nodia")) {
      psp_script_set("dia", 0);
    } else if (!strcmp(argv[i], "-nonet")) {
      net = 0;
    } else if (i < argc-1 && !strcmp(argv[i], "-app")) {
      psp_script_set_launcher(argv[++i]);
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
    } else if (i < argc-1 && argv[i][0] == '-' && psp_script_set(argv[i]+1, sys_atoi(argv[i+1])) == 0) {
      i++;
    } else if (argv[i][0] == '-') {
      usage("pumpkinos");
      sceKernelExitGame();
      return 1;
    }
    // anything else is ignored
  }

  if (vfs_arg) {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%s", vfs_arg);
  } else {
    sys_snprintf(vfsdir, MAX_PATH_LEN-1, "%svfs", home);
  }
  i = sys_strlen(vfsdir);
  if (i > 0 && vfsdir[i-1] != '/') {
    vfsdir[i] = '/';
    vfsdir[i+1] = 0;
  }
  sys_snprintf(tmpdir, MAX_PATH_LEN-1, "%stmp", home);
  if (log_arg) {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%s", log_arg);
  } else {
    sys_snprintf(logfile, MAX_PATH_LEN-1, "%spumpkin.log", home);
  }

  // there is no console: keep stderr in a file next to the log
  sys_snprintf(errfile, MAX_PATH_LEN-1, "%spumpkin.err", home);
  freopen(errfile, "w", stderr);

  sys_init();
  debug_setsyslevel(NULL, level);
  debug_init(logfile);
  debug(DEBUG_INFO, "MAIN", "%s starting on %s (home \"%s\", vfs \"%s\")", SYSTEM_NAME, SYSTEM_OS, home, vfsdir);

  // the VFS ships with the PBP (vfs/app_install); make sure the writable
  // parts exist
  if (!vfs_arg) {
    mkdir_if_missing(vfsdir);
    sys_snprintf(cafile, MAX_PATH_LEN-1, "%svfs/app_install", home);
    mkdir_if_missing(cafile);
    sys_snprintf(cafile, MAX_PATH_LEN-1, "%svfs/app_storage", home);
    mkdir_if_missing(cafile);
    sys_snprintf(cafile, MAX_PATH_LEN-1, "%svfs/app_card", home);
    mkdir_if_missing(cafile);
    sys_snprintf(cafile, MAX_PATH_LEN-1, "%svfs/app_card/PALM", home);
    mkdir_if_missing(cafile);
    sys_snprintf(cafile, MAX_PATH_LEN-1, "%svfs/app_card/PALM/Programs", home);
    mkdir_if_missing(cafile);
  }

  // Wi-Fi: the first infrastructure connection configured in the system settings
  if (net) {
    sys_psp_net_init(PSP_NET_TIMEOUT_MS);
  } else {
    debug(DEBUG_INFO, "MAIN", "network disabled");
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
    sys_psp_net_finish();
    finished = 1;
    sceKernelExitGame();
    return 1;
  }
  liblsdl3_init(0, 0);

  vfs_local_mount(vfsdir, "/");
  mkdir_if_missing(tmpdir);
  sys_set_tmpdir(tmpdir);

#ifndef NO_HTTPS
  // OpenSSL looks for the CA bundle in SSL_CERT_FILE: use cert.pem next to
  // EBOOT.PBP when it exists
  sys_snprintf(cafile, MAX_PATH_LEN-1, "%scert.pem", home);
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

  sys_psp_net_finish();
  finished = 1;
  sceKernelExitGame();

  return 0;
}
